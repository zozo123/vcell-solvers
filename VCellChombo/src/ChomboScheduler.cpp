#include <VCELL/ChomboScheduler.h>
#include <BRMeshRefine.H>

#include <EBISLayout.H>
#include <EBAMRPoissonOp.H>
#include <EBLevelGrid.H>
#include <GeometryShop.H>
#include <EBEllipticLoadBalance.H>
#include <EBCoarseAverage.H>
#include <EBPWLFineInterp.H>
#include <EBAMRDataOps.H>
#include <EBLevelDataOps.H>
#include <MFIndexSpace.H>
#include <ComplementIF.H>
#include <UnionIF.H>

#include <VCELL/VarContext.h>
#include <Expression.h>
#include <SimpleSymbolTable.h>
#include <VCELL/VolumeVariable.h>
#include <VCELL/DataSet.h>
#include <VCELL/VolumeRegionVariable.h>
#include <VCELL/MembraneVariable.h>
#include <VCELL/MembraneRegionVariable.h>
#include <VCELL/VCellModel.h>
#include <VCELL/Feature.h>
#include <VCELL/SimTool.h>
#include <VCELL/VarContext.h>
#include <VCELL/VolumeVarContextExpression.h>
#include <VCELL/VolumeRegionVarContextExpression.h>
#include <VCELL/MembraneVarContextExpression.h>
#include <VCELL/MembraneRegionVarContextExpression.h>
#include <VCELL/SimulationMessaging.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/Membrane.h>
#include <VCELL/ChomboIF.h>
#include <VCELL/ChomboSpec.h>
#include <VCELL/ChomboGeometry.h>
#include <VCELL/ChomboGeometryShop.h>
#include <assert.h>
#include <sstream>
#include <H5Cpp.h>

static const int blockFactor = 8;  // smallest box, 8 is recommended.
static const int nestingRadius  = 2; //ghostPhi[0];  // should be the same as ghost phi size, but Terry used 2
static const int maxCoarsen = -1;
static const int numPreCondIters = 4;
static const int relaxType = 2;

static const int numGhostEBISLayout = 4;

extern bool refine_all_irregular;

ChomboScheduler::ChomboScheduler(SimulationExpression* sim, ChomboSpec* chomboSpec) {
	simulation = sim;
	this->chomboSpec = chomboSpec;
	chomboGeometry = this->chomboSpec->getChomboGeometry();
	numLevels = chomboSpec->getNumLevels();
	int* ratios = chomboSpec->getRefRatios();
	for (int i = 0; i < numLevels; i ++) {
		vectRefRatios.push_back(ratios[i]);
	}
	hdf5FileCount = 0;
	numGhostSoln = IntVect::Unit * 3;
	cout << "maxBoxSiz=" << chomboSpec->getMaxBoxSize() << endl;
	cout << "fillRatio=" << chomboSpec->getFillRatio() << endl;
}

ChomboScheduler::~ChomboScheduler() {
	for (int iphase = 0; iphase < NUM_PHASES; iphase ++) {
		delete geoIfs[iphase];
		for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ivol ++) {
			volSoln[iphase][ivol].clear();
			irregularPointMembraneIDs[iphase][ivol].clear();
			delete phaseVolumeList[iphase][ivol];
		}
		phaseVolumeList[iphase].clear();
		irregularPointMembraneIDs[iphase].clear();
	}
	phaseVolumeList.clear();
	irregularPointMembraneIDs.clear();
	membranePointIndexes.clear();
}

bool ChomboScheduler::isInNextFinerLevel(int level, const IntVect& gridIndex) {
	if (level >= numLevels - 1) {
		return false;
	}

	int nextFinerLevel = level + 1;
	int refRatio = vectRefRatios[nextFinerLevel];
	IntVect gridIndexInNextFinerLevel = gridIndex * refRatio;
	
	for (LayoutIterator lit = vectGrids[nextFinerLevel].layoutIterator(); lit.ok(); ++ lit) {
		const Box& box = vectGrids[nextFinerLevel].get(lit());
		if (box.contains(gridIndexInNextFinerLevel)) {
			return true;
		}
	}
	return false;
}

int ChomboScheduler::getChomboBoxLocalIndex(const IntVect& size, int ivar, const IntVect& ijk) {
	return getChomboBoxLocalIndex(size, ivar, D_DECL(ijk[0], ijk[1], ijk[2]));
}

int ChomboScheduler::getChomboBoxLocalIndex(const IntVect& size, int ivar, D_DECL(int i, int j, int k)) {
#if CH_SPACEDIM==2
	return ivar * size[0] * size[1] + j * size[0] + i;
#else 
	return ivar * size[0] * size[1] * size[2] + k * size[0] * size[1] + j * size[0] + i;
#endif
}

double ChomboScheduler::getExpressionConstantValue(Variable* var, ExpressionIndex expIndex, Feature* feature) {
	VolumeVarContextExpression* varContextExp =	(VolumeVarContextExpression*)feature->getVolumeVarContext((VolumeVariable*)var);
	return varContextExp->evaluateConstantExpression(expIndex);
}

void ChomboScheduler::initializeGrids() {
	if (SpaceDim != chomboGeometry->getDimension()) {
		stringstream ss;
		ss << "Wrong executable (" << SpaceDim << "d) for this simulation (" << chomboGeometry->getDimension() << "d)" << endl;
		throw ss.str();
	}

	cout << endl << "ChomboScheduler:: generating grids" << endl;
	IntVect coarsestNx = chomboGeometry->getGridSize();
	RealVect coarsestDx = chomboGeometry->getDomainSize();
	
	IntVect lo = IntVect::Zero;
	IntVect hi = coarsestNx;
	hi -= IntVect::Unit;
	Box crseDomBox(lo,hi);
	ProblemDomain coarsestDomain(crseDomBox);
	
	for (int idir = 0; idir < SpaceDim; idir ++) {
		coarsestDx[idir] /= coarsestNx[idir];
		coarsestDomain.setPeriodic(idir, false);
	}

	// setup Domains Dxes;
	vectDomains.resize(numLevels);
	vectDomains[0] = coarsestDomain;
	vectDxes.resize(numLevels);
	vectDxes[0] = coarsestDx;
	vectNxes.resize(numLevels);
	vectNxes[0] = coarsestNx;
	for(int ilev = 1; ilev < numLevels; ilev++) {
		vectDxes[ilev] = vectDxes[ilev-1];
		vectDxes[ilev] /= vectRefRatios[ilev-1];
		vectNxes[ilev] = vectNxes[ilev-1];
		vectNxes[ilev] *= vectRefRatios[ilev-1];
		vectDomains[ilev] = refine(vectDomains[ilev-1], vectRefRatios[ilev-1]);
	}
    
    // Define Geometry
	cout << "ChomboScheduler:: processing geometry" << endl;

	ProblemDomain finestDomain = vectDomains[numLevels - 1];
	RealVect fineDx = vectDxes[numLevels - 1];

	Vector<GeometryService*> geometries(NUM_PHASES, NULL);
	Vector<BaseIF*> phase0List;
	for (int i = 0; i < chomboGeometry->getNumSubdomains(); ++ i) {
		ChomboIF* chomboIF = chomboGeometry->getChomboIF(i);
		if (chomboIF->getPhaseIndex() == 0) {
			phase0List.push_back(chomboIF);
		}
	}
	geoIfs[0] = new UnionIF(phase0List);
	GeometryShop *workshop0 = new GeometryShop(*(geoIfs[0]), 0, fineDx);
	workshop0->m_phase = 0;
	geometries[0] = workshop0;

	geoIfs[1] = new ComplementIF(*(geoIfs[0]));
	workshop0 = new GeometryShop(*(geoIfs[1]), 0, fineDx);
	workshop0->m_phase = 1;
	geometries[1] = workshop0;

	const RealVect& domainOrigin = chomboGeometry->getDomainOrigin();
	MFIndexSpace mfIndexSpace;
	mfIndexSpace.define(finestDomain.domainBox(), domainOrigin, fineDx[0], geometries, chomboSpec->getMaxBoxSize()/* ,maxCoarsenings */);

	phaseVolumeList.resize(NUM_PHASES);
	for (int phase0 = 0; phase0 < NUM_PHASES; phase0 ++) {
		// Select one index-space
		Chombo_EBIS::alias((EBIndexSpace*) mfIndexSpace.EBIS(phase0));
		EBIndexSpace* ebisPtr = Chombo_EBIS::instance();
		Vector<RefCountedPtr<EBIndexSpace> > volumes = ebisPtr->connectedComponents();
		for (int ivol = 0; ivol < volumes.size(); ivol++) {
			// find the feature for this volume
			RefCountedPtr<EBIndexSpace> volume = volumes[ivol];
			int depth = volume->getLevel(finestDomain);
			DisjointBoxLayout grids = volume->levelGrids(depth);
			EBISLayout ebisLayout;
			volume->fillEBISLayout(ebisLayout, grids, finestDomain, numGhostEBISLayout);
			Feature* featureFoundRegular = NULL;
			Feature* featureFoundIrregular = NULL;

			for(DataIterator dit = grids.dataIterator(); dit.ok() && featureFoundRegular == NULL; ++dit) {
				const Box& currBox = grids[dit()];
				const EBISBox& currEBISBox = ebisLayout[dit()];
				const IntVect& smallEnd = currBox.smallEnd();
				const IntVect& boxSize = currBox.size();
#if CH_SPACEDIM==3
				for (int k = 0; k < boxSize[2] && featureFoundRegular == NULL; ++k) {
#endif
					for (int j = 0; j < boxSize[1] && featureFoundRegular == NULL; ++j) {
						for (int i = 0; i < boxSize[0] && featureFoundRegular == NULL; ++i) {
							IntVect gridIndex(D_DECL(i + smallEnd[0], j + smallEnd[1], k + smallEnd[2]));
							if (currEBISBox.isRegular(gridIndex)) {
								RealVect a_point = EBArith::getIVLocation(gridIndex, fineDx, chomboGeometry->getDomainOrigin());
								featureFoundRegular = chomboGeometry->getFeature(a_point);
								break;
							}
						} // end for i
					} // end for j
#if CH_SPACEDIM==3
				} // end for k
#endif
				if (featureFoundRegular == NULL) {
					const EBGraph& currEBGraph = currEBISBox.getEBGraph();
					IntVectSet irregCells = currEBISBox.getIrregIVS(currBox);
					for (VoFIterator vofit(irregCells,currEBGraph); vofit.ok(); ++vofit) {
						const VolIndex& vof = vofit();

						const IntVect& gridIndex = vof.gridIndex();
						RealVect a_point = EBArith::getIVLocation(gridIndex, fineDx, chomboGeometry->getDomainOrigin());
						RealVect centroid = currEBISBox.centroid(vof);
						centroid *= fineDx;
						a_point += centroid;
						Feature* fi = chomboGeometry->getFeature(a_point);
						if (featureFoundIrregular == NULL) {
							featureFoundIrregular = fi;
						} else if (featureFoundIrregular != fi) {
							throw "Mesh is too coarse!";
						}
					}
				}
			} // end for DataIterator

			if (featureFoundRegular == NULL && featureFoundIrregular == NULL) {
				throw "Mesh is too coarse!";
			}
			Feature* finalFeature = featureFoundRegular != NULL ? featureFoundRegular : featureFoundIrregular;
			ConnectedComponent* cc = new ConnectedComponent;
			cc->feature = finalFeature;
			cout << "Phase " << phase0 << ", Volume " << ivol << ", Feature " << finalFeature->getName() << endl;
			cc->phase = phase0;
			cc->volumeIndexInPhase = ivol;
			cc->volume = volume;
			phaseVolumeList[phase0].push_back(cc);
		} // end for comp
	}// end for phase


	// define grids for each level
  Vector<int> coarsestProcs;
	Vector<Box> coarsestBoxes;
	
	domainSplit(coarsestDomain, coarsestBoxes, chomboSpec->getMaxBoxSize(), blockFactor);
	mortonOrdering(coarsestBoxes);
	LoadBalance(coarsestProcs, coarsestBoxes);

	vectGrids.resize(numLevels);
	vectGrids[0] = DisjointBoxLayout(coarsestBoxes, coarsestProcs, coarsestDomain);

	vectEbis.resize(NUM_PHASES);
	for(int phase0 = 0; phase0 < NUM_PHASES; phase0 ++) {
    vectEbis[phase0].resize(phaseVolumeList[phase0].size());
		for (int ivol = 0; ivol < phaseVolumeList[phase0].size(); ivol ++) {
			vectEbis[phase0][ivol].resize(numLevels);
			phaseVolumeList[phase0][ivol]->volume->fillEBISLayout(vectEbis[phase0][ivol][0],
									  vectGrids[0],
									  coarsestDomain,
									  numGhostEBISLayout);
		}
  }

	if (numLevels > 1) {
		// If there is more than one level, the finer levels need to created by "BRMeshRefine"
	  BRMeshRefine meshRefine(coarsestDomain, vectRefRatios, chomboSpec->getFillRatio(), blockFactor, nestingRadius, chomboSpec->getMaxBoxSize());

	    //tag all irregular coarse iv's coarsest domain
		ProblemDomain secondFinestDomain = coarsestDomain;
		 // Compute the second finest domain
		for (int ilev = 0; ilev < numLevels - 2; ilev ++) {
			secondFinestDomain.refine(vectRefRatios[ilev]);
		}
		// Tags for creating the finer levels
		Vector<IntVectSet> tags(numLevels);

		if (refine_all_irregular) {
			pout() << "ChomboScheduler:: tag all irregular cells" << endl;
		} else {
			pout() << "ChomboScheduler:: tag user defined cells" << endl;
		}
		
		int taglevel = numLevels - 2;
		if (refine_all_irregular) {
			for (int phase0 = 0; phase0 < NUM_PHASES; phase0 ++) {
				for (int ivol = 0; ivol < phaseVolumeList[phase0].size(); ivol ++) {
					// Get the depth of the second to finest level
					int depth = phaseVolumeList[phase0][ivol]->volume->getLevel(secondFinestDomain);

					IntVectSet tagsVol = phaseVolumeList[phase0][ivol]->volume->irregCells(depth);
					tagsVol.grow(2);
					tags[taglevel] |= tagsVol;
				}
			}
		} else {
			for (int phase0 = 0; phase0 < NUM_PHASES; phase0 ++) {
				for (int ivol = 0; ivol < phaseVolumeList[phase0].size(); ivol ++) {
					// Get the depth of the second to finest level
					int depth = phaseVolumeList[phase0][ivol]->volume->getLevel(secondFinestDomain);
					if (phaseVolumeList[phase0][ivol]->feature->getName() == "SR") {
						IntVectSet tagsVol = phaseVolumeList[phase0][ivol]->volume->irregCells(depth);
						tagsVol.grow(2);
						tags[taglevel] |= tagsVol;
					}
				}
			}
		}

		if (tags[taglevel].isEmpty()) {
			MayDay::Error("No cells tagged for refinement");
		}

		Vector<Vector<Box> > oldBoxes(numLevels);
		Vector<Vector<Box> > newBoxes;
	    // Need the boxes on the coarsest level and the tags on the second to
	    // finest level to make all the boxes on all the levels
	 	oldBoxes[0]= coarsestBoxes;
	 	 // Make all the boxes on all the levels
	 	meshRefine.regrid(newBoxes, tags, 0, numLevels - 1, oldBoxes);
	
	  ProblemDomain domainLev = coarsestDomain;
		for(int ilev = 1; ilev < numLevels; ilev++) {
			Vector<int> newProcs;
			domainLev.refine(vectRefRatios[ilev-1]);
			mortonOrdering(newBoxes[ilev]);
			LoadBalance(newProcs, newBoxes[ilev]);

			vectGrids[ilev] = DisjointBoxLayout(newBoxes[ilev], newProcs, domainLev);
		    //generate ebislayout
		  for(int phase0 = 0; phase0 < NUM_PHASES; phase0 ++) {
				for (int ivol = 0; ivol < phaseVolumeList[phase0].size(); ivol ++) {
					vectEbis[phase0][ivol].resize(numLevels);
					phaseVolumeList[phase0][ivol]->volume->fillEBISLayout(vectEbis[phase0][ivol][ilev],
											  vectGrids[ilev],
											  domainLev,
											  numGhostEBISLayout);
				}
		  }
		}
	}

	// compute membrane id = ivol * 1000 + j
	// allocate storage
	irregularPointMembraneIDs.resize(NUM_PHASES);
	for (int iphase = 0; iphase < NUM_PHASES; iphase ++) {
		int numVols = phaseVolumeList[iphase].size();
		irregularPointMembraneIDs[iphase].resize(numVols);
		if (iphase == 0)
		{
			membranePointIndexes.resize(numVols);
		}
		for (int ivol = 0; ivol < numVols; ++ ivol) 
		{
			irregularPointMembraneIDs[iphase][ivol].resize(numLevels);
			for (int ilev = 0; ilev < numLevels; ilev ++) {
				RefCountedPtr< LayoutData<IntVectSet> > irrSet = RefCountedPtr<LayoutData<IntVectSet> >(new LayoutData<IntVectSet>(vectGrids[ilev]));

				for(DataIterator dit = vectGrids[ilev].dataIterator(); dit.ok(); ++dit) {
					const EBISBox& currEBISBox = vectEbis[iphase][ivol][ilev][dit()];
					(*irrSet)[dit()] = currEBISBox.getIrregIVS(vectGrids[ilev][dit()]);
				} // end DataIterator
				BaseIVFactory<int>  bivfabFactory(vectEbis[iphase][ivol][ilev], *irrSet);
				irregularPointMembraneIDs[iphase][ivol][ilev] = RefCountedPtr<LevelData< BaseIVFAB<int> > >(new LevelData< BaseIVFAB<int> >(vectGrids[ilev], 1, IntVect::Zero, bivfabFactory));
				if (iphase == 0 && ilev == numLevels - 1)
				{
					membranePointIndexes[ivol] = RefCountedPtr<LevelData< BaseIVFAB<int> > >(new LevelData< BaseIVFAB<int> >(vectGrids[ilev], 1, IntVect::Zero, bivfabFactory));
				}
			}
		}
	}

	// find membrane for each irregular point
	{		
		// initialize all membrane IDs to -1 first
		for (int iphase = 0; iphase < NUM_PHASES; ++ iphase) {
			for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ivol ++) {
				for(int ilev = 0; ilev < numLevels; ++ ilev) {
					LevelData< BaseIVFAB<int> >& levelData = *irregularPointMembraneIDs[iphase][ivol][ilev];
					for (DataIterator dit = levelData.dataIterator(); dit.ok(); ++ dit) {
						DataIndex dataIndex = dit();
						BaseIVFAB<int>& ivfab = levelData[dataIndex];
						ivfab.setVal(-1);
					}
				}
			}
		}
		
		int phase0 = 0;
		int phase1 = 1;		
		int totalVolumes = phaseVolumeList[phase0].size() + phaseVolumeList[phase1].size();		
		bool* bAdjacent = new bool[phaseVolumeList[phase1].size()];
		numMembranePoints = 0;
		for (int ivol = 0; ivol < phaseVolumeList[phase0].size(); ivol++) {
			memset(bAdjacent, 0, phaseVolumeList[phase1].size() * sizeof(bool));
			
			for (int ilev = 0; ilev < numLevels; ilev ++) {
				for(DataIterator dit = vectGrids[ilev].dataIterator(); dit.ok(); ++dit) {
					const Box& currBox = vectGrids[ilev][dit()];
					const EBISBox& currEBISBox = vectEbis[phase0][ivol][ilev][dit()];
					const EBGraph& currEBGraph = currEBISBox.getEBGraph();
					IntVectSet irregCells = currEBISBox.getIrregIVS(currBox);
					for (VoFIterator vofit(irregCells,currEBGraph); vofit.ok(); ++vofit) {
						const VolIndex& vof = vofit();
						const IntVect& gridIndex = vof.gridIndex();
						if (ilev == numLevels  - 1)
						{
							(*membranePointIndexes[ivol])[dit()](vof, 0) = numMembranePoints ++;
						}

						bool bFoundMembrane = false;
						for (int jvol = 0; jvol < phaseVolumeList[phase1].size(); jvol ++) {
							const IntVectSet& ivs = vectEbis[phase1][jvol][ilev][dit()].getIrregIVS(currBox);
							if (ivs.contains(gridIndex)) {
								int membraneID = ivol * totalVolumes + jvol;
								(*irregularPointMembraneIDs[phase0][ivol][ilev])[dit()](vof, 0) = membraneID;
								(*irregularPointMembraneIDs[phase1][jvol][ilev])[dit()](vof, 0) = membraneID;
								bAdjacent[jvol] = true;
								bFoundMembrane = true;
								break;
							}
						}
						if (!bFoundMembrane) {
							Feature* feature = phaseVolumeList[phase0][ivol]->feature;
							RealVect vol_point = EBArith::getVofLocation(vof, vectDxes[ilev], chomboGeometry->getDomainOrigin());
							stringstream ss;
							ss << "phase " << phase0 << ":feature " << feature->getName() << ":volume " << ivol << ":level " << ilev 
									<< ": no membrane id for point " << vof.gridIndex() << " at "  << vol_point << ".";
							cout << ss.str() << endl;
							(*irregularPointMembraneIDs[phase0][ivol][ilev])[dit()](vof, 0) = -1;
						}
					}
				}
			}
			for (int jvol = 0; jvol < phaseVolumeList[phase1].size(); jvol ++) {
				if (bAdjacent[jvol]) {
					phaseVolumeList[phase0][ivol]->adjacentVolumes.push_back(phaseVolumeList[phase1][jvol]);
					phaseVolumeList[phase1][jvol]->adjacentVolumes.push_back(phaseVolumeList[phase0][ivol]);
				}
			}			
		}
		delete[] bAdjacent;
	}
	
	for(int phase0 = 0; phase0 < NUM_PHASES; phase0 ++) {
		for (int ivol = 0; ivol < phaseVolumeList[phase0].size(); ivol ++) {
			Feature* feature = phaseVolumeList[phase0][ivol]->feature;
			for(int ilev = 0; ilev < numLevels; ilev++) {
				long totalIrregLevel   = 0.0;
				long totalPtsLevel   = 0.0;

				for (DataIterator dit = vectGrids[ilev].dataIterator(); dit.ok(); ++dit) {
					const Box&  currBox      = vectGrids[ilev][dit()];
					
					const EBISBox& currEBISBox = vectEbis[phase0][ivol][ilev][dit()];
					IntVectSet irregCells = currEBISBox.getIrregIVS(currBox);
					
					pout() << "phase " << phase0 << ":feature " << feature->getName() << ":volume " << ivol << ":level " << ilev << ":" << " Box " << currBox << endl;
					totalPtsLevel += currBox.numPts();
					totalIrregLevel += irregCells.numPts();
				}

				std::ios::fmtflags origFlags = cout.flags();
				int origWidth = cout.width();
				int origPrecision = cout.precision();

				pout() << "phase " << phase0 << ":feature " << feature->getName() << ":volume " << ivol << ":level " << ilev << ":" << endl;
				pout() << setiosflags(ios::right);
				pout() << "  Total computation cells: " << setw(10) << totalPtsLevel << endl;
				pout() << "  Total irregular cells:   " << setw(10) << totalIrregLevel << endl;
				pout() << endl;

				pout().flags(origFlags);
				pout().width(origWidth);
				pout().precision(origPrecision);
			}
		}
  }
}

//extern double vol_frac_cutoff;
void ChomboScheduler::writeVolumeSolution() {
	for (int ifeature = 0; ifeature < chomboGeometry->getNumSubdomains(); ifeature ++) {
		Feature* feature = chomboGeometry->getFeature(ifeature);
		int numDefinedVars = feature->getNumDefinedVariables();
		for (int ivar = 0; ivar < numDefinedVars; ivar ++) {
			Variable* var = feature->getDefinedVariable(ivar);

			double* varCurr = var->getCurr();
			memset(varCurr, 0, sizeof(double) * var->getSize());
		}
	}
		
	for (int iphase = 0; iphase < NUM_PHASES; iphase ++) {
		for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ivol ++) {
			Feature* feature = phaseVolumeList[iphase][ivol]->feature;
			int numDefinedVars = feature->getNumDefinedVariables();
			int numDefinedVolVars = feature->getNumDefinedVariables();
			int numDefinedMemVars = iphase == 0 ? feature->getMemVarIndexesInAdjacentMembranes().size() : 0;
			if (numDefinedVolVars == 0 && numDefinedMemVars == 0) {
				continue;
			}
			for (int ivar = 0; ivar < numDefinedVars; ivar ++) {
				Variable* var = feature->getDefinedVariable(ivar);
				double* varCurr = var->getCurr();

				int refratio = 1;
				for(int ilev = 0; ilev < numLevels - 1; ilev ++) {
					refratio *= vectRefRatios[ilev];
				}
				// copy phi to var, repeat values for coarse levels
				int finestLevel = numLevels - 1;
				for(int ilev = 0; ilev < numLevels; ilev ++) {
					for(DataIterator dit = vectGrids[ilev].dataIterator(); dit.ok(); ++dit)	{
						const EBISBox& currEBISBox = vectEbis[iphase][ivol][ilev][dit()];
						
						EBCellFAB& solnEBCellFAB = (*volSoln[iphase][ivol][ilev])[dit()];
						FArrayBox& solnFab = solnEBCellFAB.getFArrayBox();
						const IntVect& solnSize = solnFab.size();
						const int* solnLo = solnFab.loVect();
						Real* solnDataPtr = solnFab.dataPtr();

#if CH_SPACEDIM==3
						for (int k = numGhostSoln[2]; k < solnSize[2] - numGhostSoln[2]; k ++) {
							int koffset = k + solnLo[2];
#endif
							for (int j = numGhostSoln[1]; j < solnSize[1] - numGhostSoln[1]; j ++) {
								int joffset = j + solnLo[1];
								for (int i = numGhostSoln[0]; i < solnSize[0] - numGhostSoln[0]; i ++) {
									int ioffset = i + solnLo[0];

									IntVect gridIndex(D_DECL(ioffset, joffset, koffset));
									if (currEBISBox.isCovered(gridIndex) || isInNextFinerLevel(ilev, gridIndex)) {
										continue;
									}

									double sol = solnDataPtr[getChomboBoxLocalIndex(solnSize, ivar, D_DECL(i, j, k))];
#if CH_SPACEDIM==3
									for (int kk = 0; kk < refratio; kk ++) {
#endif
										for (int jj = 0; jj < refratio; jj ++) {
											for (int ii = 0; ii < refratio; ii ++) {
												IntVect finestGridIndex(D_DECL(ioffset * refratio + ii, joffset * refratio + jj, koffset * refratio + kk));
												int globalIndex = getChomboBoxLocalIndex(vectNxes[finestLevel], 0, finestGridIndex);
												varCurr[globalIndex] = sol;
											} // end for ii
										} // end for jj
#if CH_SPACEDIM==3
									} // end for kk
#endif
								} // end for i
							} // end for j
#if CH_SPACEDIM==3
						} // end for k
#endif
					} // end for (DataIterator
					refratio /= vectRefRatios[ilev];
				} // end for ilev
			} // for (int ivar)
		} // for ivol
	} // for iphase
}

void ChomboScheduler::writeData(char* dataFileName) {
	bool bWriteSim = true;
	if (bWriteSim) {
		writeVolumeSolution();
		writeMembraneSolution();
		DataSet::write(dataFileName, simulation);
	}
	static bool bFirstTime = false;
	bool bWriteHdf5 = false;
	// we need at least one hdf5 to show mesh in viewer.
	if (bWriteHdf5 || bFirstTime) {
		for (int iphase = 0; iphase < NUM_PHASES; iphase ++) {
			for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ivol ++) {
				Feature* feature = phaseVolumeList[iphase][ivol]->feature;
				if (feature->getNumDefinedVariables() == 0) {
					continue;
				}
				char hdf5FileName[128];
				sprintf(hdf5FileName, "%s%06d.feature_%s.vol%d%s", SimTool::getInstance()->getBaseFileName(), simulation->getCurrIteration(), feature->getName().c_str(), ivol, HDF5_FILE_EXT);
				Vector<string> names(feature->getNumDefinedVariables());
				for (int ivar = 0; ivar < feature->getNumDefinedVariables(); ivar ++) {
					Variable* var = feature->getDefinedVariable(ivar);
					char charstr[100];
					sprintf(charstr, "%s.vol%d",var->getName().c_str(), ivol);
					names[ivar] = string(charstr);
				}

				bool replaceCovered = false;
				Vector<Real> dummy;

				writeEBHDF5(string(hdf5FileName), vectGrids, volSoln[iphase][ivol], names,
					 vectDomains[0].domainBox(), vectDxes[0][0], simulation->getDT_sec(), simulation->getTime_sec(),
					 vectRefRatios, numLevels, replaceCovered, dummy);
				if (!bWriteHdf5) {
					break;
				}
			}
			if (!bWriteHdf5) {
				break;
			}
		}
		bFirstTime = false;
	}
	hdf5FileCount ++;
}

void ChomboScheduler::writeMembraneSolution() {
	
	int totalVolumes = phaseVolumeList[0].size() + phaseVolumeList[1].size();
	int numMembraneVars = simulation->getNumMemVariables();
	
	int iphase = 0;
	int ilev = numLevels - 1;  // only consider the finest level
	// membrane variable names
	for(int memVarIdx = 0; memVarIdx < numMembraneVars; memVarIdx++){
		Variable* var = (Variable*)simulation->getMemVariable(memVarIdx);
		double* varCurr = var->getCurr();
		memset(varCurr, 0, sizeof(double) * var->getSize());
	}
	for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ivol ++) {
		Feature* iFeature = phaseVolumeList[iphase][ivol]->feature;

		DisjointBoxLayout& currGrids = vectGrids[ilev];

		for(DataIterator dit = currGrids.dataIterator(); dit.ok(); ++dit)	{
			const EBISBox& currEBISBox = vectEbis[iphase][ivol][ilev][dit()];
			const Box& currBox = vectGrids[ilev][dit()];

			const EBGraph& currEBGraph = currEBISBox.getEBGraph();
			IntVectSet irregCells = currEBISBox.getIrregIVS(currBox);
			Vector<ConnectedComponent*>& adjacentVolumes = phaseVolumeList[iphase][ivol]->adjacentVolumes;

			for (VoFIterator vofit(irregCells,currEBGraph); vofit.ok(); ++vofit) {
				const VolIndex& vof = vofit();
				int memIndex = (*membranePointIndexes[ivol])[dit()](vof, 0);;
				int membraneID = (*irregularPointMembraneIDs[iphase][ivol][ilev])[dit()](vof, 0);
				for (int j = 0; j < adjacentVolumes.size(); j ++) {
					int jphase = adjacentVolumes[j]->phase;
					int jvol = adjacentVolumes[j]->volumeIndexInPhase;
					int currentMembraneID = ivol * totalVolumes + jvol;
					if (membraneID != currentMembraneID) {
						continue;
					}

					Feature* jFeature = phaseVolumeList[jphase][jvol]->feature;
					Membrane* membrane = SimTool::getInstance()->getModel()->getMembrane(iFeature, jFeature);
					for (int memVarIdx = 0; memVarIdx < numMembraneVars; ++ memVarIdx)
					{
						Variable* var = (Variable*)simulation->getMemVariable(memVarIdx);
						double* varCurr = var->getCurr();
						varCurr[memIndex] = 0;
						for (int ivar = 0; ivar < iFeature->getMemVarIndexesInAdjacentMembranes().size(); ++ ivar) 
						{
							int varIndex =	iFeature->getMemVarIndexesInAdjacentMembranes()[ivar];
							if (varIndex == memVarIdx)
							{
								if (membrane->isVariableDefined(var)) 
								{
									varCurr[memIndex] = (*memSoln[ivol][ilev])[dit()](vof, ivar);
								}
								break;
							}
						}
					}
					break;
				} // end for jvol
			} // for (VoFIterator vofit(irregCells,currEBGraph);
		} // end DataIter
	} // end ivol
}

#define MESH_GROUP "/Mesh"
#define METRICS_DATASET MESH_GROUP"/Metrics"
#define CROSS_POINTS_DATASET MESH_GROUP"/Cross Points"
#if CH_SPACEDIM == 3
#define SLICE_VIEW_DATASET MESH_GROUP"/Slice View"
#endif

struct MeshMetrics
{
	int index;
	IntVect gridIndex;
	RealVect coord, normal;
	double volumeFraction,areaFraction;
	
	static void printTitle(ostream& os)
	{
		os << "index" D_TERM(<< ",i", <<",j", << ",k") D_TERM(<< ",x", << ",y", << ",z") 
			D_TERM(<< ",normalX", << ",normalY", << ",normalZ") << ",volumeFraction,areaFraction" << endl;
	}
	
	friend ostream& operator<<(ostream& os, const MeshMetrics& mm) {
		os << mm.index;
		for (int dir = 0; dir < SpaceDim; ++ dir)
		{
			os << "," << mm.gridIndex[dir];
		}
		for (int dir = 0; dir < SpaceDim; ++ dir)
		{
			os << "," << mm.coord[dir];
		}
		for (int dir = 0; dir < SpaceDim; ++ dir)
		{
			os << "," << mm.normal[dir];
		}
		os	<< "," << mm.volumeFraction << "," << mm.areaFraction;
		return os;
	}
};

void populateMetricsDataType(H5::CompType& metricsType)
{
		metricsType.insertMember("index", HOFFSET(MeshMetrics, index), H5::PredType::NATIVE_INT);
		D_TERM(metricsType.insertMember("i", HOFFSET(MeshMetrics, gridIndex[0]), H5::PredType::NATIVE_INT);,
			     metricsType.insertMember("j", HOFFSET(MeshMetrics, gridIndex[1]), H5::PredType::NATIVE_INT);,
			     metricsType.insertMember("k", HOFFSET(MeshMetrics, gridIndex[2]), H5::PredType::NATIVE_INT);)
		D_TERM(metricsType.insertMember("x", HOFFSET(MeshMetrics, coord[0]), H5::PredType::NATIVE_DOUBLE);,
			     metricsType.insertMember("y", HOFFSET(MeshMetrics, coord[1]), H5::PredType::NATIVE_DOUBLE);,
			     metricsType.insertMember("z", HOFFSET(MeshMetrics, coord[2]), H5::PredType::NATIVE_DOUBLE);)
		D_TERM(metricsType.insertMember("normalX", HOFFSET(MeshMetrics, normal[0]), H5::PredType::NATIVE_DOUBLE);,
			     metricsType.insertMember("normalY", HOFFSET(MeshMetrics, normal[1]), H5::PredType::NATIVE_DOUBLE);,
			     metricsType.insertMember("normalZ", HOFFSET(MeshMetrics, normal[2]), H5::PredType::NATIVE_DOUBLE);)
		metricsType.insertMember("volumeFraction", HOFFSET(MeshMetrics, volumeFraction), H5::PredType::NATIVE_DOUBLE);
		metricsType.insertMember("areaFraction", HOFFSET(MeshMetrics, areaFraction), H5::PredType::NATIVE_DOUBLE);
}

struct CrossPoint
{
	int index;
	IntVect gridIndex;
#if CH_SPACEDIM == 3
	int face;
#endif
	RealVect triVertices[3];
	
	static void printTitle(ostream& os)
	{
		os << "index" D_TERM(<< ",i", <<",j", << ",k") 
#if CH_SPACEDIM == 3
		<< ",face"
#endif
		D_TERM(<< ",x0", <<",y0", << ",z0") 
		D_TERM(<< ",x1", << ",y1", << ",z1") 
		D_TERM(<< ",x2", << ",y2", << ",z2") << endl;
	}
	
	friend ostream& operator<<(ostream& os, const CrossPoint& cp) {
		os << cp.index;
		for (int dir = 0; dir < SpaceDim; ++ dir)
		{
			os << "," << cp.gridIndex[dir]; 
		}
#if CH_SPACEDIM == 3
		os << "," << cp.face;
#endif
		for (int i = 0; i < 3; ++ i)
		{
			for (int dir = 0; dir < SpaceDim; ++ dir)
			{
				os << "," << cp.triVertices[i][dir]; 
			}
		}
		return os;
	}
};

void populateCrossPointDataType(H5::CompType& crossPointType)
{
		crossPointType.insertMember("index", HOFFSET(CrossPoint, index), H5::PredType::NATIVE_INT);
		D_TERM(crossPointType.insertMember("i", HOFFSET(CrossPoint, gridIndex[0]), H5::PredType::NATIVE_INT);,
					 crossPointType.insertMember("j", HOFFSET(CrossPoint, gridIndex[1]), H5::PredType::NATIVE_INT);,
			     crossPointType.insertMember("k", HOFFSET(CrossPoint, gridIndex[2]), H5::PredType::NATIVE_INT);)
#if CH_SPACEDIM == 3
		crossPointType.insertMember("face", HOFFSET(CrossPoint, face), H5::PredType::NATIVE_INT);
#endif
		D_TERM(crossPointType.insertMember("x0", HOFFSET(CrossPoint, triVertices[0][0]), H5::PredType::NATIVE_DOUBLE);,
			     crossPointType.insertMember("y0", HOFFSET(CrossPoint, triVertices[0][1]), H5::PredType::NATIVE_DOUBLE);,
			     crossPointType.insertMember("z0", HOFFSET(CrossPoint, triVertices[0][2]), H5::PredType::NATIVE_DOUBLE);)
		D_TERM(crossPointType.insertMember("x1", HOFFSET(CrossPoint, triVertices[1][0]), H5::PredType::NATIVE_DOUBLE);,
			     crossPointType.insertMember("y1", HOFFSET(CrossPoint, triVertices[1][1]), H5::PredType::NATIVE_DOUBLE);,
			     crossPointType.insertMember("z1", HOFFSET(CrossPoint, triVertices[1][2]), H5::PredType::NATIVE_DOUBLE);)
		D_TERM(crossPointType.insertMember("x2", HOFFSET(CrossPoint, triVertices[2][0]), H5::PredType::NATIVE_DOUBLE);,
					 crossPointType.insertMember("y2", HOFFSET(CrossPoint, triVertices[2][1]), H5::PredType::NATIVE_DOUBLE);,
					 crossPointType.insertMember("z2", HOFFSET(CrossPoint, triVertices[2][2]), H5::PredType::NATIVE_DOUBLE);)
}

#if CH_SPACEDIM == 3
struct SliceView
{
	int index;
	double crossPoints[3][4];
	
	static void printTitle(ostream& os)
	{
		os << "index,x_y1,x_z1,x_y2,x_z2,y_x1,y_z1,y_x2,y_z2,z_x1,z_y1,z_x2,z_y2" << endl;
	}
	
	friend ostream& operator<<(ostream& os, const SliceView& sv) {
		os << sv.index;
		for (int dir = 0; dir < 3; ++ dir)
		{
			for (int i = 0; i < 4; ++ i)
			{
				os << "," << sv.crossPoints[dir][i];
			}
		}
		return os;
	}
};

void populateSliceViewDataType(H5::CompType& sliceViewType)
{
		sliceViewType.insertMember("index", HOFFSET(SliceView, index), H5::PredType::NATIVE_INT);
		sliceViewType.insertMember("x_y1", HOFFSET(SliceView, crossPoints[0][0]), H5::PredType::NATIVE_DOUBLE);
		sliceViewType.insertMember("x_z1", HOFFSET(SliceView, crossPoints[0][1]), H5::PredType::NATIVE_DOUBLE);
		sliceViewType.insertMember("x_y2", HOFFSET(SliceView, crossPoints[0][2]), H5::PredType::NATIVE_DOUBLE);
		sliceViewType.insertMember("x_z2", HOFFSET(SliceView, crossPoints[0][3]), H5::PredType::NATIVE_DOUBLE);
		sliceViewType.insertMember("y_x1", HOFFSET(SliceView, crossPoints[1][0]), H5::PredType::NATIVE_DOUBLE);
		sliceViewType.insertMember("y_z1", HOFFSET(SliceView, crossPoints[1][1]), H5::PredType::NATIVE_DOUBLE);
		sliceViewType.insertMember("y_x2", HOFFSET(SliceView, crossPoints[1][2]), H5::PredType::NATIVE_DOUBLE);
		sliceViewType.insertMember("y_z2", HOFFSET(SliceView, crossPoints[1][3]), H5::PredType::NATIVE_DOUBLE);
		sliceViewType.insertMember("z_x1", HOFFSET(SliceView, crossPoints[2][0]), H5::PredType::NATIVE_DOUBLE);
		sliceViewType.insertMember("z_y1", HOFFSET(SliceView, crossPoints[2][1]), H5::PredType::NATIVE_DOUBLE);
		sliceViewType.insertMember("z_x2", HOFFSET(SliceView, crossPoints[2][2]), H5::PredType::NATIVE_DOUBLE);
		sliceViewType.insertMember("z_y2", HOFFSET(SliceView, crossPoints[2][3]), H5::PredType::NATIVE_DOUBLE);
}
#endif

void ChomboScheduler::writeMembraneFiles()
{
	char fileName[128];
	sprintf(fileName, "%s%s", SimTool::getInstance()->getBaseFileName(), CHOMBO_MEMBRANE_METRICS_FILE_EXT);
	ofstream metrics_ofs(fileName);
	
	sprintf(fileName, "%s%s", SimTool::getInstance()->getBaseFileName(), EDGE_CROSS_POINTS_FILE_EXT);
	ofstream crspts_ofs(fileName);

	MeshMetrics::printTitle(metrics_ofs);
	CrossPoint::printTitle(crspts_ofs);
#if (CH_SPACEDIM == 3)
	sprintf(fileName, "%s%s", SimTool::getInstance()->getBaseFileName(), MEMBRANE_SLICE_CROSS_FILE_EXT);
	ofstream slccrs_ofs(fileName);
	SliceView::printTitle(slccrs_ofs);

	const RealVect& origin = getChomboGeometry()->getDomainOrigin();
	Real minOrigin = std::min<Real>(origin[0], origin[1]);
	minOrigin = std::min<Real>(minOrigin, origin[2]);
	Real sliceCrossPointDefaultValue = minOrigin - 1;
	SliceView* sliceViewData = new SliceView[numMembranePoints];
#endif
	
	int iphase = 0;
	int ilev = numLevels - 1; // only consider the finest level
	MeshMetrics* metricsData = new MeshMetrics[numMembranePoints];
#if CH_SPACEDIM == 2
	CrossPoint* crossPointData = new CrossPoint[numMembranePoints];
#else
	CrossPoint* crossPointData = new CrossPoint[numMembranePoints*6];
#endif
	int crossPointCount = 0;
	
	ChomboGeometryShop chomboGeoShop(geoIfs[iphase], vectDxes[ilev]);
	for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ++ ivol) 
	{
		DisjointBoxLayout& currGrids = vectGrids[ilev];

		for(DataIterator dit = currGrids.dataIterator(); dit.ok(); ++ dit)	{
			const EBISBox& currEBISBox = vectEbis[iphase][ivol][ilev][dit()];
			const Box& currBox = vectGrids[ilev][dit()];

			const EBGraph& currEBGraph = currEBISBox.getEBGraph();
			IntVectSet irregCells = currEBISBox.getIrregIVS(currBox);
			for (VoFIterator vofit(irregCells,currEBGraph); vofit.ok(); ++ vofit) 
			{
				const VolIndex& vof = vofit();
				int memIndex = (*membranePointIndexes[ivol])[dit()](vof, 0);
				
				const IntVect& gridIndex = vof.gridIndex();
				RealVect vol_point = EBArith::getVofLocation(vof, vectDxes[ilev], chomboGeometry->getDomainOrigin());
				const RealVect& mem_centroid = currEBISBox.bndryCentroid(vof);
				RealVect mem_point = mem_centroid;
				mem_point *= vectDxes[ilev];
				mem_point += vol_point;
				
				metricsData[memIndex].index = memIndex;
				metricsData[memIndex].gridIndex = gridIndex;
				metricsData[memIndex].coord = mem_point;
				metricsData[memIndex].normal = currEBISBox.normal(vof);
				metricsData[memIndex].areaFraction = currEBISBox.bndryArea(vof);
				metricsData[memIndex].volumeFraction = currEBISBox.volFrac(vof);
	
				metrics_ofs << metricsData[memIndex] << endl;
				
#if CH_SPACEDIM == 2
				edgeMo edges[4];

				bool faceCovered;
				bool faceRegular;
				bool faceDontKnow;

				//get edgeType and intersection points
				chomboGeoShop.edgeData2D(edges,
									 faceCovered,
									 faceRegular,
									 faceDontKnow,
									 vectDxes[ilev][0],
									 vectDxes[ilev],
									 vof.gridIndex(),
									 vectDomains[ilev],
									 chomboGeometry->getDomainOrigin());
				int crossedEdgeCount = 0;
				for (int iedge = 0; iedge < 4; ++ iedge)
				{
					bool irreg = edges[iedge].dontKnow();
					if (irreg)
					{
						RealVect cp = (edges[iedge].getIntersectLo()) ? edges[iedge].getLo() : edges[iedge].getHi();
						// get the real coordinate
						RealVect cross_point = cp;
						cross_point *= vectDxes[ilev];
						cross_point += vol_point;
						crossedEdgeCount ++;
						if (crossedEdgeCount < 3)
						{
							crossPointData[crossPointCount].triVertices[crossedEdgeCount] = cross_point;
						}
					}
				}
				assert(crossedEdgeCount == 0 || crossedEdgeCount == 2);
				if (crossedEdgeCount == 2)
				{
					crossPointData[crossPointCount].index = memIndex;
					crossPointData[crossPointCount].gridIndex = gridIndex;
					crossPointData[crossPointCount].triVertices[0] = mem_point;
					crspts_ofs << crossPointData[crossPointCount] << endl;
					++ crossPointCount;
				}
#else
				int faceCount = -1;
				int sliceCrossPointCount[SpaceDim];
				sliceViewData[memIndex].index = memIndex;
				for (int dir = 0; dir < SpaceDim; ++ dir)
				{
					sliceCrossPointCount[dir] = 0;
					for (int i = 0; i < 4; ++ i)
					{
						sliceViewData[memIndex].crossPoints[dir][i] = sliceCrossPointDefaultValue;
					}
				}
				for (int face = 0; face < SpaceDim; ++ face)
				{
					for (int hiLoFace = 0; hiLoFace < 2; ++ hiLoFace)
					{
						++ faceCount;
						edgeMo edges[4];
						bool faceCovered;
						bool faceRegular;
						bool faceDontKnow;

						chomboGeoShop.edgeData3D(edges,
											 faceCovered,
											 faceRegular,
											 faceDontKnow,
											 hiLoFace,
											 face,
											 vectDxes[ilev][0],
											 vectDxes[ilev],
											 vof.gridIndex(),
											 vectDomains[ilev],
											 chomboGeometry->getDomainOrigin());
						int crossedEdgeCount = 0;
						for (int iedge = 0; iedge < 4; ++ iedge)
						{
							bool irreg = edges[iedge].dontKnow();
							if (irreg)
							{
								RealVect cp = (edges[iedge].getIntersectLo()) ? edges[iedge].getLo() : edges[iedge].getHi();
								// get the real coordinate
								RealVect cross_point = cp;
								cross_point *= vectDxes[ilev];
								cross_point += vol_point;
								crossedEdgeCount ++;
								if (crossedEdgeCount < 3)
								{
									crossPointData[crossPointCount].triVertices[crossedEdgeCount] = cross_point;
								}
							}
						}
						assert(crossedEdgeCount == 0 || crossedEdgeCount == 2);
						if (crossedEdgeCount == 2)
						{
							crossPointData[crossPointCount].index = memIndex;
							crossPointData[crossPointCount].gridIndex = gridIndex;
							crossPointData[crossPointCount].face = faceCount;
							crossPointData[crossPointCount].triVertices[0] = mem_point;
							crspts_ofs << crossPointData[crossPointCount] << endl;
							
							for(int dir = 0; dir < SpaceDim; ++ dir)
							{
								RealVect crossPoint;
								bool oneFaceCross = computeOneFaceCross(dir, face, hiLoFace == 0 ? -1 : 1, vectDxes[ilev], 
												vol_point, crossPointData[crossPointCount].triVertices[1], 
												crossPointData[crossPointCount].triVertices[2], crossPoint);
								if (oneFaceCross)
								{
									sliceViewData[memIndex].crossPoints[dir][sliceCrossPointCount[dir] * 2] = crossPoint[0];
									sliceViewData[memIndex].crossPoints[dir][sliceCrossPointCount[dir] * 2 + 1] = crossPoint[1];
									++ sliceCrossPointCount[dir];
								}
							}
							++ crossPointCount;
						}
					} // end for (int hiLoFace
				} // end for (int face
				slccrs_ofs << sliceViewData[memIndex] << endl;
#endif
			} // end for (VoFIterator vofit
		} // end for(DataIterator dit
	} // end 	for (int ivol 
	crspts_ofs.close();
	metrics_ofs.close();
#if CH_SPACEDIM == 3
	slccrs_ofs.close();
#endif
	
	try
	{
		/*
		* Turn off the auto-printing when failure occurs so that we can
		* handle the errors appropriately
		*/
		H5::Exception::dontPrint();
		sprintf(fileName, "%s%s", SimTool::getInstance()->getBaseFileName(), MESH_HDF5_FILE_EXT);
		H5::H5File* h5MeshFile = new H5::H5File(fileName, H5F_ACC_TRUNC);	
		h5MeshFile->createGroup(MESH_GROUP);   //  first level /Mesh

		{
		H5::CompType metricsType(sizeof(MeshMetrics));
		populateMetricsDataType(metricsType);
		hsize_t dim[] = {numMembranePoints};   /* Dataspace dimensions */
		int rank = 1;
		H5::DataSpace space(rank, dim);
		H5::DataSet* metricsDataset = new H5::DataSet(h5MeshFile->createDataSet(METRICS_DATASET, metricsType, space));
		metricsDataset->write(metricsData, metricsType);						
		delete[] metricsData;
		delete metricsDataset;
		}
		
		{
		H5::CompType crossPointType(sizeof(CrossPoint));
		populateCrossPointDataType(crossPointType);
		hsize_t dim[] = {crossPointCount};   /* Dataspace dimensions */
		int rank = 1;
		H5::DataSpace space(rank, dim);
		H5::DataSet* crossPointDataset = new H5::DataSet(h5MeshFile->createDataSet(CROSS_POINTS_DATASET, crossPointType, space));
		crossPointDataset->write(crossPointData, crossPointType);						
		delete[] crossPointData;
		delete crossPointDataset;
		}
		
#if CH_SPACEDIM == 3
		{
		H5::CompType sliceViewType(sizeof(SliceView));
		populateSliceViewDataType(sliceViewType);
		hsize_t dim[] = {numMembranePoints};   /* Dataspace dimensions */
		int rank = 1;
		H5::DataSpace space(rank, dim);
		H5::DataSet* sliceViewDataset = new H5::DataSet(h5MeshFile->createDataSet(SLICE_VIEW_DATASET, sliceViewType, space));
		sliceViewDataset->write(sliceViewData, sliceViewType);						
		delete[] sliceViewData;
		delete sliceViewDataset;
		}
#endif
		
		delete h5MeshFile;
	}
	catch (H5::Exception e)
	{
		cout << "H5 Exception:" << e.getCDetailMsg() << endl;
	}
}

bool ChomboScheduler::computeOneFaceCross(int dir, int face, int hiLoFace, RealVect& H, RealVect& v0, 
				RealVect& v1, RealVect& v2, RealVect& crossPoint)
{
	if (dir == face)
	{
		return false;
	}
	static int slicedirs[3][2] = {{1, 2}, {0, 2}, {0, 1}};
	
	int jlohi, jcross;
	int planedir0, planedir1;
	planedir0 = dir;
	if (face == slicedirs[dir][0])    // y- or y+
	{
    planedir1 = slicedirs[dir][1];   // not y direction,
    jlohi = 0;    
		jcross = 1;
	} 
	else                         // z- or z+
	{
    planedir1 = slicedirs[dir][0];   // not z direction,
    jlohi = 1;    
		jcross = 0;
	}
	
	RealVect DP = v2 - v1;
	Real thisDP0 = DP[planedir0];
	Real thisDP1 = DP[planedir1];

	if (abs(thisDP0) > 1.e-8 * abs(thisDP1))
	{
    // if slope too large, return a number out of range
    Real xc = v0[planedir0]; 
		Real yc = v0[planedir1];
    Real p10 = v1[planedir0];
    Real p11 = v1[planedir1];
    Real a = thisDP1 / thisDP0;
    Real value = a * (xc - p10) + p11;
    if (abs(value - yc) <= H[planedir1]/2)
		{
			crossPoint[jlohi]  = v0[face] + hiLoFace * H[face]/2 ;
			crossPoint[jcross] = value;
			return true;
    }
	}
	return false;
}
