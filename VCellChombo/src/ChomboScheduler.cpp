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

static const int blockFactor = 8;  // smallest box, 8 is recommended.
static const int nestingRadius  = 2; //ghostPhi[0];  // should be the same as ghost phi size, but Terry used 2
//static int maxBoxSize = 1024;  // biggest box 32(3D) and 64(2D), but Terry used 1024
static const int maxCoarsen = -1;
static const int numPreCondIters = 4;
static const int relaxType = 2;

static const int numGhostEBISLayout = 4;

extern bool refine_all_irregular;

#define HDF5_FILE_EXT ".hdf5"
#define EDGE_CROSS_POINTS_FILE_EXT ".crspts"
#define CHOMBO_MEMBRANE_METRICS_FILE_EXT ".chombo.memmetrics"
#define MEMBRANE_SLICE_CROSS_FILE_EXT ".slccrs"

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
	defineGrids();
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

void ChomboScheduler::initValues() {
	writeMembraneMetrics();
	writeMembraneEdgeCrossPoints();
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

//double ChomboScheduler::evaluateExpressionRegular(Variable* var, ExpressionIndex expIndex, int level, Feature* feature,
//			const IntVect& gridIndex, double* variableValues) {
//	RealVect coord = EBArith::getIVLocation(gridIndex, vectDxes[level], chomboGeometry->getDomainOrigin());
//	vectValues[0] = simulation->getTime_sec();
//	vectValues[1] = coord[0];
//	vectValues[2] = coord[1];
//	vectValues[3] = SpaceDim < 3 ? 0.5 : coord[2];
//	if (variableValues != 0) {
//		vectValues[4] = variableValues[0];
//	}
//
//	VolumeVarContextExpression* varContextExp =	(VolumeVarContextExpression*)feature->getVolumeVarContext((VolumeVariable*)var);
//	return varContextExp->evaluateExpression(expIndex, vectValues);
//
//}

void ChomboScheduler::defineGrids() {
	if (SpaceDim != chomboGeometry->getDimension()) {
		stringstream ss;
		ss << "Wrong executable (" << SpaceDim << "d) for this simulation (" << chomboGeometry->getDimension() << "d)" << endl;
		throw ss.str();
	}

	cout << endl << "ChomboScheduler:: define grids" << endl;
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
	cout << "ChomboScheduler:: define geometry" << endl;

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

//		RealVect lofilter(D_DECL(0.15, -0.25, -0.25));
//		RealVect hifilter(D_DECL(1, 0.25, 0.25));
//		RealVect lofilter(D_DECL(0.425,-0.24,-0.24));
//		RealVect hifilter(D_DECL(0.875, 0.125, 0.125));
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
//			RealVect lofilter1(D_DECL(1,10,-0.1));
//			RealVect hifilter1(D_DECL(3,18,0.1));
//			RealVect lofilter2(D_DECL(-3,10,-0.1));
//			RealVect hifilter2(D_DECL(-1,18,0.1));
//			const RealVect& origin = chomboGeometry->getDomainOrigin();
//
//			IntVect lofilterIndex1;
//			IntVect hifilterIndex1;
//			IntVect lofilterIndex2;
//			IntVect hifilterIndex2;
//
//			for (int i = 0; i < SpaceDim; i++) {
//				lofilterIndex1[i] = floor((lofilter1[i] - origin[i]) / vectDxes[taglevel][i]);
//				hifilterIndex1[i] = ceil ((hifilter1[i] - origin[i]) / vectDxes[taglevel][i]);
//				lofilterIndex2[i] = floor((lofilter2[i] - origin[i]) / vectDxes[taglevel][i]);
//				hifilterIndex2[i] = ceil ((hifilter2[i] - origin[i]) / vectDxes[taglevel][i]);
//			}
//
//			Box tagVol1(lofilterIndex1,hifilterIndex1);
//			Box tagVol2(lofilterIndex2,hifilterIndex2);
//
//			tags[taglevel] |= tagVol1;
//			tags[taglevel] |= tagVol2;
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
//		Vector<IntVect> ivs;
//		ivs.push_back(IntVect(9,8));
//		ivs.push_back(IntVect(9,9));
//		ivs.push_back(IntVect(9,10));
//		for (int phase0 = 0; phase0 < NUM_PHASES; phase0 ++) {
//			for (int ivol = 0; ivol < phaseVolumeList[phase0].size(); ivol++) {
//				Feature* feature = phaseVolumeList[phase0][ivol]->feature;
//				for (int ilev = 0; ilev < 1; ilev ++) {
//					for(DataIterator dit = vectGrids[0].dataIterator(); dit.ok(); ++dit) {
//						const EBISBox& currEBISBox = vectEbis[phase0][ivol][ilev][dit()];
//						
//						for (int a = 0; a < ivs.size(); ++ a) {
//							IntVect& iv = ivs[a];
//							pout() << "phase " << phase0 << ":feature " << feature->getName() << ":volume " << ivol << ":level " << ilev << ":" << iv << " is ";
//							//RealVect vol_point = EBArith::getIVLocation(iv, vectDxes[ilev], chomboGeometry->getDomainOrigin());
//							if (currEBISBox.isCovered(iv)) {
//								pout() << "covered";
//							} else if (currEBISBox.isRegular(iv)) {
//								pout() << "regular";
//							} else if (currEBISBox.isIrregular(iv)) {
//								VolIndex vof(iv, 0);
//								double volfrac = currEBISBox.volFrac(vof);
//								double areafrac = currEBISBox.bndryArea(vof);
//								pout() << "irrregular, volfrac=" << volfrac << ",bndryArea=" << areafrac;
//							}
//							pout() << endl;
//						}
//					}
//				}
//			}
//		}
		
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
							//throw ss.str();
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
//					const EBGraph& currEBGraph = currEBISBox.getEBGraph();
//					for (VoFIterator vofit(irregCells,currEBGraph); vofit.ok(); ++vofit) {
//						const VolIndex& vof = vofit();
//						const IntVect& gridIndex = vof.gridIndex();
//						Vector<VolIndex> vofs = currEBISBox.getVoFs(gridIndex);
//						if (ilev == 1) {		
//							RealVect vol_point = EBArith::getVofLocation(vof,vectDxes[ilev], chomboGeometry->getDomainOrigin());
//							const RealVect& mem_centroid = currEBISBox.bndryCentroid(vof);
//							RealVect mem_point = mem_centroid;
//							mem_point *= vectDxes[ilev];
//							mem_point += vol_point;
//							pout() << "phase " << phase0 << ":feature " << feature->getName() << ":volume " << ivol << ":level " << ilev << ":" << ", at " << gridIndex << ", centroid " << mem_point << endl;
//						}
//						if (vofs.size() > 1) {
//							pout() << "phase " << phase0 << ":feature " << feature->getName() << ":volume " << ivol << ":level " << ilev << ":" << ", multi valued (" << vofs.size() << ") cells at " << gridIndex << endl;
//						}
//					}
					
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
//	double alpha = 0.2;
//	double lambda = 0.5;
//	double max_error = 0;
//	double max_error_ignore_small_volfrac = 0;
//	double t = simulation->getTime_sec();

//	cout << "+++++++vol_frac_cutoff = " << vol_frac_cutoff << "+++++++" << endl;
		
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

//									VolIndex vof(gridIndex, 0);
//									double volFrac = currEBISBox.volFrac(vof);

									double sol = solnDataPtr[getChomboBoxLocalIndex(solnSize, ivar, D_DECL(i, j, k))];

//									RealVect coord = EBArith::getIVLocation(gridIndex, vectDxes[ilev], chomboGeometry->getDomainOrigin());
//									double x = coord[0];
//									double y = coord[1];
//#if CH_SPACEDIM==3
//									double z = coord[2];
//									double rho = sqrt(x * x + y * y + z * z);
//#else
//									double rho = sqrt(x * x + y * y);
//#endif
//									double f = cos(alpha * rho);
//									double exact_sol =  f * exp(-t * lambda);
//									double error = fabs(sol - exact_sol);
//									max_error = max(max_error, error);
//									if (volFrac > vol_frac_cutoff) {
//										max_error_ignore_small_volfrac = max(max_error_ignore_small_volfrac, error);
//									}
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
//	pout() << "max error " << t << " " << max_error << endl;
//	pout() << "max error " << t << " " << max_error_ignore_small_volfrac << endl;
}

void ChomboScheduler::writeData(char* dataFileName) {
	bool bWriteSim = true;
	if (bWriteSim) {
		writeVolumeSolution();
		writeMembraneSolution();
		DataSet::write(dataFileName, simulation);
	}
	static bool bFirstTime = true;
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
					const RealVect gridIndex = vof.gridIndex();
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

void ChomboScheduler::writeMembraneEdgeCrossPoints()
{
	char fileName[128];
	sprintf(fileName, "%s%s", SimTool::getInstance()->getBaseFileName(), EDGE_CROSS_POINTS_FILE_EXT);

	ofstream ofs(fileName);
	ofstream ofs2;
	if (SpaceDim == 2)
	{
		ofs << "index,i,j,x0,y0,x1,y1,x2,y2" << endl;
	}
	else if (SpaceDim == 3)
	{
		ofs << "index,face,i,j,k,x0,y0,z0,x1,y1,z1,x2,y2,z2" << endl;
		char fileName2[128];
		sprintf(fileName2, "%s%s", SimTool::getInstance()->getBaseFileName(), MEMBRANE_SLICE_CROSS_FILE_EXT);
		ofs2.open(fileName2);
		ofs2 << "index,y1,z1,y2,z2,x1,z1,x2,z2,x1,y1,x2,y2" << endl;
	}
	int iphase = 0;
	int ilev = numLevels - 1; // only consider the finest level
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
				
				const RealVect& gridIndex = vof.gridIndex();
				RealVect vol_point = EBArith::getVofLocation(vof, vectDxes[ilev], chomboGeometry->getDomainOrigin());
				const RealVect& mem_centroid = currEBISBox.bndryCentroid(vof);
				RealVect mem_point = mem_centroid;
				mem_point *= vectDxes[ilev];
				mem_point += vol_point;
				
				if (SpaceDim == 2)
				{
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
					stringstream ss;
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
							for (int i = 0; i < SpaceDim; ++ i)
							{
								ss << "," << cross_point[i];
							}
							crossedEdgeCount ++;
						}
					}
					assert(crossedEdgeCount == 0 || crossedEdgeCount == 2);
					if (crossedEdgeCount == 2)
					{
						ofs << memIndex;
						for (int i = 0; i < SpaceDim; ++ i)
						{
							ofs << "," << gridIndex[i];
						}
						for (int i = 0; i < SpaceDim; ++ i)
						{
							ofs << "," << mem_point[i];
						}
						ofs << ss.str() << endl;
					}
				}
				else
				{
					int faceCount = -1;
					RealVect V[2];
					Real sliceCrossPoints[SpaceDim][4];
					int sliceCrossPointCount[SpaceDim];
					const RealVect& origin = getChomboGeometry()->getDomainOrigin();
					Real minOrigin = std::min<Real>(origin[0], origin[1]);
					minOrigin = std::min<Real>(minOrigin, origin[2]);
					Real dv = minOrigin - 1;
					for (int dir = 0; dir < SpaceDim; ++ dir)
					{
						sliceCrossPointCount[dir] = 0;
						for (int i = 0; i < 4; ++ i)
						{
							sliceCrossPoints[dir][i] = dv;
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
									if (crossedEdgeCount < 2)
									{
										V[crossedEdgeCount] = cross_point;
									}
									crossedEdgeCount ++;
								}
							}
							assert(crossedEdgeCount == 0 || crossedEdgeCount == 2);
							if (crossedEdgeCount == 2)
							{
								ofs << memIndex << "," << faceCount;
								for (int i = 0; i < SpaceDim; ++ i)
								{
									ofs << "," << gridIndex[i];
								}
								for (int i = 0; i < SpaceDim; ++ i)
								{
									ofs << "," << mem_point[i];
								}
								for (int v = 0; v < 1; ++ v)
								{
									for (int i = 0; i < SpaceDim; ++ i)
									{
										ofs << "," << V[v][i];
									}
								}
								ofs << endl;
								for(int dir = 0; dir < SpaceDim; ++ dir)
								{
									RealVect crossPoint;
									bool oneFaceCross = computeOneFaceCross(dir, face, hiLoFace == 0 ? -1 : 1, vectDxes[ilev], 
													vol_point, V[0], V[1], crossPoint);
									if (oneFaceCross)
									{
										sliceCrossPoints[dir][sliceCrossPointCount[dir] * 2] = crossPoint[0];
										sliceCrossPoints[dir][sliceCrossPointCount[dir] * 2 + 1] = crossPoint[1];
										++ sliceCrossPointCount[dir];
									}
								}
							}
						} // end for (int hiLoFace
					} // end for (int face
					ofs2 << memIndex;
					for(int dir = 0; dir < SpaceDim; ++ dir)
					{
						for (int i = 0; i < 4; ++ i)
						{
							ofs2 << "," << sliceCrossPoints[dir][i];
						}
					}
					ofs2 << endl;
				} // end if (SpaceDim == 2)
			} // end for (VoFIterator vofit
		} // end for(DataIterator dit
	} // end 	for (int ivol 
	ofs.close();
	ofs2.close();
}

bool ChomboScheduler::computeOneFaceCross(int dir, int face, int hiLoFace, RealVect& H, RealVect& v0, 
				RealVect& v1, RealVect& v2, RealVect& crossPoint)
{
	if (dir == face)
	{
		return false;
	}
	static int slicedirs[SpaceDim][2] = {{1, 2}, {0, 2}, {0, 1}};
	
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

void ChomboScheduler::writeMembraneMetrics() {
	
	char fileName[128];
	sprintf(fileName, "%s%s", SimTool::getInstance()->getBaseFileName(), CHOMBO_MEMBRANE_METRICS_FILE_EXT);
	ofstream outfile (fileName);
	
	int iphase = 0;
	int ilev = numLevels - 1;  // only consider finest level
	outfile << "index,";
	if (SpaceDim == 2)
	{
		outfile << "i,j,x,y,normalX,normalY,";
	}
	else
	{
		outfile << "i,j,k,x,y,z,normalX,normalY,normalZ,";
	}
	outfile << "areaFraction" << endl;
	for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ivol ++) {

		DisjointBoxLayout& currGrids = vectGrids[ilev];

		for(DataIterator dit = currGrids.dataIterator(); dit.ok(); ++dit)	{
			const EBISBox& currEBISBox = vectEbis[iphase][ivol][ilev][dit()];
			const Box& currBox = vectGrids[ilev][dit()];

			const EBGraph& currEBGraph = currEBISBox.getEBGraph();
			IntVectSet irregCells = currEBISBox.getIrregIVS(currBox);
			for (VoFIterator vofit(irregCells,currEBGraph); vofit.ok(); ++vofit) {
				const VolIndex& vof = vofit();
				int memIndex = (*membranePointIndexes[ivol])[dit()](vof, 0);

				const RealVect gridIndex = vof.gridIndex();
				const RealVect& mem_centroid = currEBISBox.bndryCentroid(vof);
				RealVect vol_point = EBArith::getVofLocation(vof,vectDxes[ilev], chomboGeometry->getDomainOrigin());
				RealVect memPointNormal = currEBISBox.normal(vof);
				Real memAreaFrac = currEBISBox.bndryArea(vof);

				RealVect mem_point = mem_centroid;
				mem_point *= vectDxes[ilev];
				mem_point += vol_point;

				outfile << memIndex << ",";
				for (int i = 0; i < SpaceDim; ++ i)
				{
					outfile << gridIndex[i] << ",";
				}
				for (int i = 0; i < SpaceDim; ++ i)
				{
					outfile << mem_point[i] << ",";
				}
				for (int i = 0; i < SpaceDim; ++ i)
				{
					outfile << memPointNormal[i] << ",";
				}
				outfile << memAreaFrac << endl;
			} // for (VoFIterator vofit(irregCells,currEBGraph);
		} // end DataIter
	} // end ivol
	outfile.close(); 
}

