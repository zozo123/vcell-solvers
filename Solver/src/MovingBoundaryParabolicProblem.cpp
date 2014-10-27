//#pragma warning ( disable: 4996 )
#pragma warning ( disable: 4244 )
#pragma warning ( disable: 4267 )
#include <MPoint.h>
#include <World.h>
#include <VoronoiMesh.h>
#include <sstream>
#include <iomanip>
#include <exception>
#include <forward_list>
#include <vcellutil.h>
#include <Expression.h>
#include <SimpleSymbolTable.h>
#include <VCellFront.h>
#include <algo.h>
#include <MovingBoundaryParabolicProblem.h>
#include <MeshElementSpecies.h>
#include <VoronoiMesh.h>
#include <Logger.h>
#include <Modulo.h>
#include <boundaryProviders.h>

#include <MBridge/FronTierAdapt.h>
#include <MBridge/Figure.h>
#include <MBridge/MBPatch.h>
#include <MBridge/MBMovie.h>
#include <MBridge/MatlabDebug.h>
using spatial::cX;
using spatial::cY;

//*********************************************************
// Validation of input to problem functors & ValidationBase 
//*********************************************************
namespace {

	typedef double (*ConcentrationFunction)(double x, double y);

	//std::ofstream fspy("fcalls.txt"); //temp, frontier spy
	//std::ofstream lspy("levelcalls.txt"); //temp, frontier spy
	//concentration function implement -- use either function pointer or expression parser
	struct ConcentrationProvider {
		virtual double conc(double x, double y) = 0;
	};

	struct ConcentrationPointer : public ConcentrationProvider {
		ConcentrationPointer(const ConcentrationFunction &p)
			:cfunc(p) {}

		virtual double conc(double x, double y) {
			return cfunc(x,y);
		}
		const ConcentrationFunction cfunc;
	};

	template<int NUMBER_SYMBOLS>
	struct ConcentrationExpression : public ConcentrationProvider {
		/**
		* runtime accessible #NUMBER_SYMBOLS
		*/
		static const int numberSymbols = NUMBER_SYMBOLS;

		ConcentrationExpression(VCell::Expression &e) 
			:exp(e) {
				VCELL_KEY_LOG(debug,Key::concentrationExpression, "expression: " << exp.infix( ));
		}

		virtual double conc(double x, double y) {
			double in[NUMBER_SYMBOLS] = {x,y,0};
			const double rval =  exp.evaluateVector(in);
			VCELL_KEY_LOG(debug,Key::concentrationExpression, "f(" << x << ", " << y << ") = " << rval);
			return rval;
		}
		VCell::Expression exp; 
	};

	/**
	* abstract class which validates input
	*/
	struct ValidationBase: public spatial::FronTierLevel, public spatial::FronTierVelocity {

		/**
		* number of user input functions 
		*/
		const static int NUMBER_FUNCTIONS = 3;

		ValidationBase(const moving_boundary::MovingBoundarySetup &mbs)
			:nFunctionPointers(0) { 
				std::stringstream badset;

#define CHECK(X) checkSet(badset,X, #X);  
#define CHECK2(X,Y) checkSet(badset,X, Y,#X);  

				CHECK(mbs.maxTime);
				CHECK2(mbs.numberTimeSteps, mbs.timeStep);
#ifdef OLD_FUNCTION_POINTER_IMPLEMENTATION
				CHECK2(reinterpret_cast<void *>(mbs.concentrationFunction),mbs.concentrationFunctionStr);
				CHECK2(reinterpret_cast<void*>(mbs.velocityFunction),mbs.advectVelocityFunctionStrX)
					if (mbs.alternateFrontProvider == 0) {
						CHECK2(reinterpret_cast<void *>(mbs.levelFunction),mbs.levelFunctionStr);
					}
#endif
#undef CHECK
#undef CHECK2
					switch (nFunctionPointers) {
					case 0:
					case NUMBER_FUNCTIONS:
						break;
					default:
						badset << "functions must be specified by pointers or strings, not both ";
						break;
					}
#ifdef OLD_FUNCTION_POINTER_IMPLEMENTATION
					if (mbs.velocityFunction == 0 && mbs.advectVelocityFunctionStrY.empty( )) {
#else
					if (mbs.advectVelocityFunctionStrY.empty( )) {
#endif
						badset << "Unset Y velocity " << std::endl; 
					}

					validateSet(badset);
					if (mbs.frontVelocityFunctionStrX.empty( )) {
						const_cast<std::string &>(mbs.frontVelocityFunctionStrX) = mbs.advectVelocityFunctionStrX;
					}
					if (mbs.frontVelocityFunctionStrY.empty( )) {
						const_cast<std::string &>(mbs.frontVelocityFunctionStrY) = mbs.advectVelocityFunctionStrY;
					}
		}

		void checkSet(std::stringstream & badset,const double value, const char * const description) {
			if (value == 0) {
				badset << "Unset value " << description << std::endl;  
			}
		}
		void checkSet(std::stringstream & badset,void * value, const char * const description) {
			if (value == 0) {
				badset << "Unset value " << description << std::endl;  
			}
		}
		/**
		* check something set only one of two ways
		* @param badset 
		* @param value
		* @param str
		* put error message in badset if not exactly one of value, str, is set
		*/
		void checkSet(std::stringstream & badset, void * value, const std::string & str, const char * const description) {
			int argCount = 0;
			if (value != 0) {
				++argCount;
				++nFunctionPointers;
			}
			if (!str.empty( )) { 
				++argCount;
			}
			switch (argCount) {
			case 0:
				badset << "Unset value " << description << std::endl;  
				break;
			case 1:
				break;
			default:
				badset << description << " set more than once" << std::endl;  
			}
		}
		/**
		* check something set only one of two ways
		* @param badset 
		* @param one 
		* @param other 
		* put error message in badset if not exactly one of one, other is et 
		*/
		template<typename T, typename U>
		void checkSet(std::stringstream & badset, T one, U other, const char * const description) {
			if (one == 0 && other == 0) {
				badset << "Unset value " << description << std::endl;  
			}
			if (one != 0 && other != 0) {
				badset << description << " set twice " << std::endl; 
			}
		}

		void validateSet(std::stringstream & badset ) {
			if (badset.str( ).length() > 0) {
				badset << " in MovingBoundarySetup ";
				std::invalid_argument ia(badset.str( ));
				throw ia; 
			}
		}
		int nFunctionPointers;
	};

}

//**************************************************
// main solver, local definition 
//**************************************************
namespace moving_boundary {
	struct MovingBoundaryParabolicProblemImpl : public ValidationBase {
		typedef MovingBoundaryParabolicProblemImpl Outer;
		typedef VoronoiMesh VrnMesh;
		typedef VrnMesh::MBMeshDef MBMeshDef;
		typedef VrnMesh::Element Element;
		typedef VrnMesh::MBMesh MBMesh;
		//typedef VrnMesh::FrontType FrontType;
		typedef std::vector<spatial::TPoint<moving_boundary::CoordinateType,2> > FrontType;
		typedef World<moving_boundary::CoordinateType,2> WorldType;
		typedef spatial::TPoint<moving_boundary::CoordinateType,2> WorldPoint;
		typedef spatial::TPoint<double,2> ProblemDomainPoint; 


		MovingBoundaryParabolicProblemImpl(const moving_boundary::MovingBoundarySetup &mbs) 
			:ValidationBase(mbs),
			world(WorldType::get( )),
			setup_(mbs),
			diffusionConstant(mbs.diffusionConstant),
			currentTime(0),
			maxTime(mbs.maxTime),
			timeStep(mbs.timeStep),
			baselineTime(0),
			baselineGeneration(0),
			minimimMeshInterval(0),

			symTable(buildSymTable( )),
			levelExp(mbs.levelFunctionStr,symTable), 
			advectVelocityExpX(mbs.advectVelocityFunctionStrX,symTable), 
			advectVelocityExpY(mbs.advectVelocityFunctionStrY,symTable), 
			frontVelocityExpX(mbs.frontVelocityFunctionStrX,symTable), 
			frontVelocityExpY(mbs.frontVelocityFunctionStrY,symTable), 
			concentrationExp(mbs.concentrationFunctionStr,symTable), 

			vcFront(initFront(world, *this,mbs)),
			currentFront( ),
			meshDefinition(createMeshDef(world, mbs)),
			interiorVolume(calculateInteriorVolume(world, meshDefinition)),
			primaryMesh(meshDefinition),
			voronoiMesh(primaryMesh),
			//alloc( ),
			boundaryElements( ),
			lostElements( ),
			gainedElements(  ),
			heartbeat(0),
			heartbeatSymbol( )
		{  

			//init using either function pointer or string
			if (nFunctionPointers == 0) {
				ConcentrationExpression<3> ce(concentrationExp);
				assert (ce.numberSymbols == symTable.size( )); 
				setInitialValues(ce);
			}
			else {
#ifdef OLD_FUNCTION_POINTER_IMPLEMENTATION
				ConcentrationPointer cp(mbs.concentrationFunction);
				setInitialValues(cp);
#endif
			}

			//check time step
			if (timeStep == 0) {
				timeStep = maxTime/mbs.numberTimeSteps;
			}
			CoordinateType cMin = std::min(meshDefinition.interval(spatial::cX),meshDefinition.interval(spatial::cY));
			minimimMeshInterval = world.distanceToProblemDomain(cMin); 
			double maxStep = minimimMeshInterval * minimimMeshInterval/(4 * diffusionConstant);
			if (maxStep < timeStep) {
				if (mbs.hardTime) {
					VCELL_EXCEPTION(logic_error,"hard set input time step " << timeStep << " greater than maximum allowed by problem (" << maxStep << ')');
				}
				updateTimeStep(maxStep,0);
				if (timeStep > maxStep) {
					VCELL_EXCEPTION(logic_error,"calculated input time step " << timeStep << " greater than maximum allowed by problem (" << maxStep << ')');
				}
			}


			using matlabBridge::MatLabDebug;
			if (MatLabDebug::on("tiling")) {
				std::ofstream master("tilingMaster.m");
				matlabBridge::TPolygon<moving_boundary::CoordinateType> pFront("b");
				frontTierAdapt::copyVectorInto(pFront,currentFront);
				int i = 0;
				for (MBMesh::iterator iter = primaryMesh.begin( ); iter != primaryMesh.end( ); ++iter) {
					Element &e = *iter;
					const Volume2DClass & vol = e.getControlVolume();
					if (!vol.empty( )) {
						std::stringstream command;
						command << "tilingX" << e.indexOf(spatial::cX) << 'Y' << e.indexOf(spatial::cY) << std::ends;

						std::stringstream filestr;
						filestr <<  command.str( ).c_str( ) << ".m" << std::ends;
						std::ofstream os(filestr.str( ));

						master << command.str( ) << std::endl;
						master << matlabBridge::PauseSeconds(0.5) << std::endl;
						//os << matlabBridge::FigureName(command.str( ).c_str( ));
						os << matlabBridge::clearFigure << pFront;
						e.writeMatlab(os);
					}
				}
			}
			if (MatLabDebug::on("tilingmovie")) {
				std::ofstream master("tilingMovie.m");
				matlabBridge::Movie movie(master);
				master << matlabBridge::FigureName("Tiling, the movie");
				matlabBridge::Polygon pFront("b");
				frontTierAdapt::copyVectorInto(pFront,currentFront);
				int i = 0;
				for (MBMesh::iterator iter = primaryMesh.begin( ); iter != primaryMesh.end( ); ++iter) {
					Element &e = *iter;
					const Volume2DClass & vol = e.getControlVolume();
					if (!vol.empty( )) {
						master << matlabBridge::clearFigure << pFront;
						e.writeMatlab(master);
						movie.recordFrame( );
					}
				}
				movie.play( );
			}
#ifdef TEST_DUMP 
			std::ofstream bp("bp.txt");
			for (MBMesh::iterator iter = primaryMesh.begin( ); iter != primaryMesh.end( ); ++iter) {
				iter->listBoundary(bp,primaryMesh);
			}
#endif
			//validate grid 
			CoordinateType hSpace = primaryMesh.interval(cX);
			CoordinateType vSpace = primaryMesh.interval(cX);
			if (hSpace%2 != 0 || vSpace%2 != 0) {
				VCELL_EXCEPTION(logic_error,"spacings horiz " << hSpace << " and vert " << vSpace << " not even");
			}
			const size_t maxI =  primaryMesh.numCells(cX) - 1;
			const size_t maxJ =  primaryMesh.numCells(cY) - 1;
			for (size_t i = 0; i < maxI; i++) 
				for (size_t j = 0; j < maxJ - 1; j++) 
				{
					std::array<size_t,2> p = {i    ,j};
					std::array<size_t,2> r = {i + 1,j};
					std::array<size_t,2> d = {i    ,j + 1};
					Element &  e = primaryMesh.get(p);
					Element & down = primaryMesh.get(d);
					Element & right = primaryMesh.get(r);
					CoordinateType hGap = right(cX) - e(cX); 
					CoordinateType vGap = down(cY) - e(cY); 
					if (hGap != hSpace) {
						VCELL_EXCEPTION(logic_error, "bad h gap between " << e << " and " << right << ", " << hGap << " != expected " << hSpace);
					}
					if (vGap != vSpace) {
						VCELL_EXCEPTION(logic_error, "bad v gap between " << e << " and " << down << ", " << vGap << " != expected " << vSpace);
					}
				}
		}

		~MovingBoundaryParabolicProblemImpl( ) {
			delete vcFront;
		}

		void setInterior(Element &e) {
			e.setInteriorVolume(interiorVolume);
		}

		/**
		* set initial mesh sizes
		*/
		void setInitialValues(ConcentrationProvider &provider) {
			using std::vector;
			//set inside volumes on all elements; they may become inside as front moves.
			/*
			for (MBMesh::iterator iter = primaryMesh.begin( ); iter != primaryMesh.end( ); ++iter) {
				iter->setInteriorVolume(interiorVolume);
			}
			*/
			//std::for_each(primaryMesh.begin( ), primaryMesh.end( ), setInterior);
			for (Element & e: primaryMesh) {
				e.setInteriorVolume(interiorVolume);
			}
			currentFront = vcFront->retrieveFront( );
			std::cout << "front size " << currentFront.size( ) << std::endl;
			if (currentFront.empty( )) {
				throw std::invalid_argument("empty front");
			}
			VCELL_LOG(warn,"initial front has " << currentFront.size( ) << " points");
			assert(currentFront.front( ) == currentFront.back( ));  //verify closed 
			voronoiMesh.setFront(currentFront);
			//spatial::Positions<Element> positions = spatial::classify2(voronoiMesh,currentFront);
			moving_boundary::Positions<Element> positions = voronoiMesh.classify2(currentFront);
			for (vector<Element *>::iterator iter = positions.inside.begin( ); iter != positions.inside.end( ); ++iter) {
				Element  & insideElement = **iter;
				ProblemDomainPoint pdp = world.toProblemDomain(insideElement);
				double mu = provider.conc(pdp(cX),pdp(cY));
				insideElement.setConcentration(0,mu);
				insideElement.getControlVolume();
			}
			for (vector<Element *>::iterator iter = positions.boundary.begin( ); iter != positions.boundary.end( ); ++iter) {
				Element  & boundaryElement = **iter;
				ProblemDomainPoint pdp = world.toProblemDomain(boundaryElement);
				double mu = provider.conc(pdp(cX),pdp(cY));
				boundaryElement.setConcentration(0,mu);
				boundaryElements.push_back(&boundaryElement);
				boundaryElement.findNeighborEdges();
			}
			for (vector<Element *>::iterator iter = positions.outside.begin( ); iter != positions.outside.end( ); ++iter) {
				Element  & boundaryElement = **iter;
				boundaryElement.setConcentration(0,0);
			}
		}

		void boundaryElementSetup(Element & bElem) {
			assert (bElem.mPos( ) == spatial::boundarySurface);
		}

		/**
		* constructor support; front initialization
		*/
		static spatial::FrontProvider<moving_boundary::CoordinateType> *initFront(const WorldType & world, const ValidationBase & base,const moving_boundary::MovingBoundarySetup &mbs)  {
			if (mbs.alternateFrontProvider == nullptr) {
				typedef spatial::TGeoLimit<moving_boundary::CoordinateType> LimitType;
				const std::array<LimitType,2> & worldLimits = world.limits( );
				std::vector<spatial::GeoLimit> limits(worldLimits.size( ));
				std::transform(worldLimits.begin( ),worldLimits.end( ),limits.begin( ),spatial::GeoLimitConvert<moving_boundary::CoordinateType,double>( ) );
				const std::array<Universe<2>::CountType,2> & nodes = world.universe( ).numNodes( ); 
				int mnode = *std::max_element(nodes.begin( ),nodes.end( ));
				int numFrontRegions = mnode * mbs.frontToNodeRatio;
				if (base.nFunctionPointers == 0) {
					return new spatial::VCellFront<moving_boundary::CoordinateType>(limits,numFrontRegions,mbs.maxTime,base,base);
				}  
				/* else */
				throw std::domain_error("function pointers no longer supported");
				/*
				return new VCellFront(limits,numFrontRegions,mbs.maxTime,
				mbs.levelFunction,mbs.velocityFunction);
				*/
			}
			return mbs.alternateFrontProvider;
		}

		/**
		* constructor support; mesh initialization
		*/
		static MBMeshDef createMeshDef(const WorldType & world, const moving_boundary::MovingBoundarySetup &mbs)  {

			MeshElementSpecies::setProblemToWorldDistanceScale(world.theScale( ));

			typedef spatial::TGeoLimit<moving_boundary::CoordinateType> LimitType;
			using vcell_util::arrayInit;
			const std::array<LimitType,2> & worldLimits = world.limits( );
			std::array<moving_boundary::CoordinateType,2> origin= arrayInit<moving_boundary::CoordinateType>(worldLimits[0].low( ),worldLimits[1].low( ) );
			std::array<moving_boundary::CoordinateType,2> c = arrayInit<moving_boundary::CoordinateType>(worldLimits[0].span( ),worldLimits[1].span( ) );
			const Universe<2> & universe = world.universe( );
			std::array<size_t,2> p = { universe.numNodes( )[0], universe.numNodes( )[1] };
			return MBMeshDef(origin,c,p);
		}

		/**
		* constructor support; calculate interior volume 
		*/
		static moving_boundary::VolumeType calculateInteriorVolume(const WorldType & world, const MBMeshDef & meshDefinition) {
			//direct multiplication may not work due to type overflow -- assign to VolumeType for type conversion
			moving_boundary::VolumeType rval = meshDefinition.interval(spatial::cX);
			rval *= meshDefinition.interval(spatial::cY);
			rval /= world.theScale( ) * world.theScale( );
			return rval;
		}

		/**
		* constructor support; build SymbolTable 
		*/
		static SimpleSymbolTable buildSymTable( ) {
			std::string syms[] = { "x","y","t"};
			return SimpleSymbolTable(syms, sizeof(syms)/sizeof(syms[0]));
		}

		virtual double level(double *in) const {
			//lspy << in[0] << ',' << in[1] << std::endl; 
			double problemDomainValues[2];
			world.toProblemDomain(in,problemDomainValues);
			double r = levelExp.evaluateVector(problemDomainValues);
			return r;
		}

		spatial::SVector<moving_boundary::VelocityType,2> frontVelocity(double x, double y) const {
			//fspy << x << ',' << y << ',' << currentTime << std::endl;
			double worldValues[2] = {x,y};
			enum {ex = 0, ey = 1, et = 2};
			double syms[3];
			world.toProblemDomain(worldValues,syms);
			syms[et] = currentTime; 
			double vX = frontVelocityExpX.evaluateVector(syms);
			double vY = frontVelocityExpY.evaluateVector(syms);
			return world.toWorld<moving_boundary::VelocityType>(spatial::SVector<double,2>(vX,vY)); 
		}

		spatial::SVector<moving_boundary::VelocityType,2> advectionVelocity(double x, double y) const {

			double worldValues[2] = {x,y};
			enum {ex = 0, ey = 1, et = 2};
			double syms[3];
			world.toProblemDomain(worldValues,syms);
			syms[et] = currentTime; 
			double vX = advectVelocityExpX.evaluateVector(syms);
			double vY = advectVelocityExpY.evaluateVector(syms);
			return world.toWorld<moving_boundary::VelocityType>(spatial::SVector<double,2>(vX,vY)); 
		}

		virtual int velocity(Frontier::Front*,Frontier::POINT* fpoint,HYPER_SURF_ELEMENT*, HYPER_SURF*,double* out) const {
			const spatial::SVector<moving_boundary::VelocityType,2> & v = frontVelocity(fpoint->_coords[cX],fpoint->_coords[cY]);
			out[cX] = v(cX); 
			out[cY] = v(cY); 

			return 0; //return value not used by frontier
		}
		/**
		* @return <timeNow, timeIncrement>
		*/
		std::pair<double,double> times(int generationCount) { 
			double timeNow = baselineTime + (generationCount - baselineGeneration) * timeStep;
			if (timeNow > maxTime) {
				timeNow = maxTime;
			}
			double incr = timeNow - currentTime;
			return std::pair<double,double>(timeNow,incr);
		}

		void updateTimeStep(double desiredStep, int generationCount) {
			double intervalTilEnd = maxTime - baselineTime;
			int n = ceil(intervalTilEnd / desiredStep);
			timeStep = intervalTilEnd / n;
			baselineGeneration = generationCount;
			baselineTime = currentTime;
		}

		/**
		* @param changed nodes have changed since last report
		*/
		void giveElementsToClient(MovingBoundaryClient & client, size_t generationCount, bool changed) {
			GeometryInfo<moving_boundary::CoordinateType> gi(currentFront,changed);
			client.time(currentTime, generationCount, currentTime == maxTime, gi);
			for (MBMesh::const_iterator iter = primaryMesh.begin( ); iter != primaryMesh.end( ); ++iter) {
				//std::cout << iter->ident( ) << std::endl;
				if (iter->isInside( )) {
					client.element(*iter);
				}
			}
			client.iterationComplete();
		}

		void debugDump(size_t gc, char key) {
			std::ostringstream oss;
			oss << "frontmove"  << std::setfill('0') << std::setw(3) << gc << key << ".m" << std::ends;
			std::ofstream dump(oss.str( ));
			{
				std::ostringstream mbuf;
				mbuf << "Time " << currentTime;
				dump << matlabBridge::ConsoleMessage(mbuf.str());
				dump << matlabBridge::FigureName(oss.str( ).c_str( ));
			}
			matlabBridge::TScatter<CoordinateType> deepInside('y',3,true,0);
			matlabBridge::TScatter<CoordinateType> inside('b',2,true,1);
			matlabBridge::TScatter<CoordinateType> boundary('g',2,true,2);
			matlabBridge::TScatter<CoordinateType> outside('m',2,true,3);
			matlabBridge::TScatter<CoordinateType> deepOutside('r',2,false,4);
			for (MBMesh::const_iterator iter = primaryMesh.begin( ); iter != primaryMesh.end( ); ++iter) {
				switch(iter->mPos( )) {
				case spatial::deepInteriorSurface:
					frontTierAdapt::copyPointInto(deepInside,*iter);
					break;
				case spatial::interiorSurface:
					frontTierAdapt::copyPointInto(inside,*iter);
					break;
				case spatial::boundarySurface:
					frontTierAdapt::copyPointInto(boundary,*iter);
					break;
				case spatial::outsideSurface:
					frontTierAdapt::copyPointInto(outside,*iter);
					break;
				case spatial::deepOutsideSurface:
					frontTierAdapt::copyPointInto(deepOutside,*iter);
					break;
				default:
					VCELL_EXCEPTION(domain_error,"bad state " << static_cast<int>(iter->mPos( )) << iter->ident( )) ;
				}
				/*
				double  x = iter->get(cX);
				double  y = iter->get(cY);

				std::stringstream ss;
				ss << iter->indexOf(cX) << ',' << iter->indexOf(cY); 
				dump << matlabBridge::Text(x,y, ss.str( ).c_str( ));
				*/
			}
			dump << deepInside << inside << boundary << outside << deepOutside; 
			matlabBridge::TPolygon<CoordinateType> pPoly("k",1);
			frontTierAdapt::copyVectorInto(pPoly,currentFront);
			dump << pPoly; 
		}

		static void commenceSimulation(Element &e) {
			e.beginSimulation( );
		}

		/**
		* Functor
		* base class for functors which need access to #Outer
		*/
		struct FunctorBase {
			FunctorBase(const Outer &o)
				:outer(o) {}
			const Outer & outer;
		};
		/**
		* Functor
		* move front
		*/
		struct MoveFront : public FunctorBase {
			MoveFront(Outer &o)
				:FunctorBase(o){}
			void operator( )(Element * p) {
				assert(p != nullptr);
				VCELL_LOG(trace,p->indexInfo( ) << " front move")
					if (p->mPos() == spatial::boundarySurface) {
						p->moveFront(this->outer.currentFront);
					}
					else {
						throw std::domain_error("MoveFront");
					}
			}
		};
		/**
		* Functor
		* FindNeighborEdges 
		*/
		struct FindNeighborEdges  : public FunctorBase {
			FindNeighborEdges (Outer &o)
				:FunctorBase(o) {}

			void operator( )(Element * p) {
				if (p->isInside( ) ) {
					p->findNeighborEdges( );
				}
			}
		};
#ifdef HOLD
#error should use front velocity, not elements
		/**
		* Functor
		* apply velocity, record max
		*/
		struct MaxVel : public FunctorBase {
			MaxVel (Outer &o)
				:FunctorBase(o),
				maxVel(0) {}
			void operator( )(Element & e) {
				VCELL_LOG(trace,e.indexInfo( ) << " max velocity")
					SVector<double,2> v = this->outer.velocity(e(cX),e(cY));
				maxVel  = std::max(maxVel, v(cX));
				maxVel  = std::max(maxVel, v(cY));
				e.setVelocity(v);
			}

			REAL maxVel;
		};
#endif
		/**
		* Functor
		* find maximum velocity of front points (squared) 
		*/
		struct FrontVelocity : public FunctorBase {
			FrontVelocity (Outer &o)
				:FunctorBase(o),
				maxSquaredVel(0){}
			void operator( )(const spatial::TPoint<CoordinateType,2> & point) {
				spatial::SVector<VelocityType,2> velVector = this->outer.frontVelocity(point(cX),point(cY));
				maxSquaredVel = std::max(maxSquaredVel,velVector.magnitudeSquared( ));
			}
			moving_boundary::VelocityType maxSquaredVel;
		};

		/**
		* Functor
		* apply velocity, record max
		*/
		struct ApplyVelocity : public FunctorBase {
			ApplyVelocity (Outer &o)
				:FunctorBase(o),
				maxVel(0){}
			void operator( )(Element & e) {
				VCELL_LOG(trace,e.indexInfo( ) << " set velocity");
				spatial::SVector<moving_boundary::VelocityType,2> v = this->outer.advectionVelocity(e(cX),e(cY));
				e.setVelocity(v);
				maxVel = std::max(maxVel, std::max(v(cX),v(cY)));
			}
			moving_boundary::VelocityType maxVel;
		};

		/**
		* Functor diffuseAdvect
		*/
		struct DiffuseAdvect {
			DiffuseAdvect(spatial::DiffuseAdvectCache &c, double d, double ts, bool &errFlag)
				:dac(c),
				diffusionConstant(d),
				timeStep(ts),
				errorFlag(errFlag) {}

			void operator( )(Element & e) {
				if (e.isInside( )) {
					VCELL_LOG(trace,e.ident( ) << " diffuseAdvect")
						e.diffuseAdvect(dac,diffusionConstant, timeStep, errorFlag);
				}
			}

			spatial::DiffuseAdvectCache &dac;
			const double diffusionConstant;
			const double timeStep;
			bool & errorFlag;
		};

#if 0
		/**
		* Functor ClearDiffuseAdvect
		*/
		struct ClearDiffuseAdvect {
			void operator( )(Element & e) {
				if (e.mPos( ) != outsideSurface) {
					VCELL_LOG(trace,e.ident( ) << " clearDiffuseAdvect")
						e.clearDiffuseAdvectAmounts( );
				}
			}

		};
#endif

		/**
		* Functor
		* collect mass 
		*/
		struct CollectMass : public FunctorBase {
			CollectMass (Outer &o)
				:FunctorBase(o) {}
			void operator( )(Element *p) {
				VCELL_LOG(trace,p->indexInfo( ) << " collectMass")
					p->collectMass(this->outer.currentFront);
			}
		};

		/**
		* Functor
		* apply front
		*/
		struct ApplyFront : public FunctorBase {
			ApplyFront(Outer &o)
				:FunctorBase(o) {}
			void operator( )(Element &e) {
				VCELL_LOG(trace,e.indexInfo( ) << " applyFront")
					e.applyFront(this->outer.currentFront, this->outer.interiorVolume);
			}
		};

		/**
		* Functor
		* distribute lost mass 
		*/
		struct DistributeLost {
			void operator( )(Element &e) {
				VCELL_LOG(trace,e.indexInfo( ) << " distributeLost")
					e.distributeMassToNeighbors();
			}
		};

		/**
		* Functor
		* distribute lost mass 
		*/
		struct EndCycle : public FunctorBase {
			EndCycle(Outer &o)
				:FunctorBase(o) {}
			void operator( )(Element &e) {
				VCELL_LOG(trace,e.indexInfo( ) << " end of cycle");
				e.endOfCycle( );
			}
		};

		void run(MovingBoundaryClient & client) {
			VCELL_LOG(info,"commence simulation");
			const bool frontMoveTrace = matlabBridge::MatLabDebug::on("frontmove");
			FrontType oldFront; //for debugging

			size_t generationCount = 0;
			try { 
				std::for_each(primaryMesh.begin( ),primaryMesh.end( ),commenceSimulation);
				giveElementsToClient(client,generationCount,true);
				if (matlabBridge::MatLabDebug::on("frontmove")) {
					debugDump(generationCount, 's');
				}

				while (currentTime < maxTime) {
					std::pair<double,double> nowAndStep = times(generationCount); 
					//TODO -- we're approximating front velocity for time step with velocity at beginning of time step
						FrontVelocity fv(*this);
						std::for_each(currentFront.begin( ),currentFront.end( ),fv);
						double maxVel = sqrt(fv.maxSquaredVel);
						double maxTime = minimimMeshInterval / (2 * maxVel);
						if (nowAndStep.second > maxTime) {
							updateTimeStep(maxTime,generationCount);
							nowAndStep = times(generationCount); 
					}
					currentTime = nowAndStep.first;
					double timeIncr = nowAndStep.second;

					ApplyVelocity mv(*this);
					std::for_each(primaryMesh.begin( ),primaryMesh.end( ),mv);

					VCELL_LOG(debug, "moved to time " << currentTime << " generation " << generationCount);
					vcFront->propagateTo(currentTime); 
					/*
					VCellFront *vcp = dynamic_cast<VCellFront *>(vcFront);
					if (vcp != nullptr) {
					Front * checkpoint = copy_front(vcp->c_ptr( ));
					FT_Save(checkpoint, "testsave.txt");
					}
					*/


					if (frontMoveTrace) {
						oldFront = currentFront; //for debugging
					}
					currentFront = vcFront->retrieveFront( );
					voronoiMesh.setFront(currentFront);

					{
						//					std::ofstream f("fullvdump.m");
						//					voronoiMesh.matlabPlot(f, &currentFront);
					}
					std::for_each(boundaryElements.begin( ),boundaryElements.end( ),MoveFront(*this));
					//std::for_each(boundaryElements.begin( ),boundaryElements.end( ),FindNeighborEdges( );


#ifdef HOLD_FOR_POSSIBLE_IMPLEMENTATION_LATER
					double diffuseAdvectStep = timeIncr;
					double totalStepped = 0;
					int numberDASteps = 1;
					bool tooBig = false;
					do {	
						tooBig = false;
						for (int step = 0 ; step < numberDASteps; ++step ) {
							primaryMesh.diffuseAdvectCache( ).clearDiffuseAdvectCache( );
							std::for_each(primaryMesh.begin( ),primaryMesh.end( ), 
								DiffuseAdvect(primaryMesh.diffuseAdvectCache( ),diffusionConstant, diffuseAdvectStep, tooBig));
							if (tooBig) {
								diffuseAdvectStep /= 2;
								numberDASteps *= 2; 
								VCELL_LOG(debug,"Reducing da time step to " << diffuseAdvectStep <<  ", steps = " << numberDASteps);
								break;
							}
							primaryMesh.diffuseAdvectCache( ).checkDiffuseAdvectCache( );
						}
					} while (tooBig);
#endif
					primaryMesh.diffuseAdvectCache( ).start( );
					bool tooBig = false;
					try {
						std::for_each(primaryMesh.begin( ),primaryMesh.end( ), 
							DiffuseAdvect(primaryMesh.diffuseAdvectCache( ),diffusionConstant, timeIncr, tooBig));
						if (tooBig) {
							throw TimeStepTooBig("time step makes mass go negative");
						}
					} catch (ReverseLengthException &rle) {
						std::ofstream s("rle.m");
						rle.aElement.writeMatlab(s, true, 20);
						rle.bElement.writeMatlab(s, true, 20);
						throw rle;
					}
					primaryMesh.diffuseAdvectCache( ).finish( );
					if (frontMoveTrace) {
						debugDump(generationCount,'b');
					}

					bool changed;
					//adjustNodes calls MeshElementSpecies.updateBoundaryNeighbors
					try {
						changed = voronoiMesh.adjustNodes(boundaryElements,currentFront);
					} catch (SkipsBoundary & skips) {
						std::ofstream sb("sb.m");
						matlabBridge::TPolygon<long long> oldPoly("k",1);
						frontTierAdapt::copyVectorInto(oldPoly,oldFront);
						matlabBridge::TPolygon<long long> newPoly("r",1);
						frontTierAdapt::copyVectorInto(newPoly,currentFront);
						matlabBridge::TScatter<long> scatter('b',2,true);
						scatter.add(skips.mes(cX),skips.mes(cY));
						sb << '%' << skips.mes.ident( ) << std::endl;
						sb << oldPoly << newPoly << scatter;
						std::cerr << "rethrowing skips" << std::endl;
						debugDump(generationCount,'e');
						throw skips;
					}
					if (frontMoveTrace) {
						debugDump(generationCount,'a');
					}
					//TODO: nochange

					std::for_each(boundaryElements.begin( ),boundaryElements.end( ),CollectMass(*this));

					std::for_each(primaryMesh.begin( ),primaryMesh.end( ),ApplyFront(*this));

					std::for_each(primaryMesh.begin( ),primaryMesh.end( ),DistributeLost());

					std::for_each(primaryMesh.begin( ),primaryMesh.end( ),EndCycle(*this));

					//tell the client about it
					giveElementsToClient(client,++generationCount, changed);
					if (heartbeat && generationCount%heartbeat == 0) {
						std::cout << heartbeatSymbol;
					}
#ifdef OLD				
					lostElements.clear( );
					gainedElements.clear( );
					for (MBMesh::const_iterator iter = primaryMesh.begin( ); iter != primaryMesh.end( ); ++iter) {

						const Element &previous = *iter;
						Element &next = previous.doppelganger( );
						if (next.isInside( )) {
							next.setVelocity(velocity(next(cX),next(cY)) );
						}
						Element::Transition tns = previous.stateChange( );
						switch (tns) {
						case Element::same:
							for (size_t s = 0; s < NUM_S;s++) {
								const REAL mass = previous.mass(s);
								next.setMass(s,mass);
							}
							break;
						case Element::lost:
							assert(previous.mPos( ) == boundarySurface);
							boundaryElements.remove(const_cast<Element *>(&previous));
							lostElements.push_back(&previous);
							break;
						case Element::gained:
							assert(next.mPos( ) == boundarySurface);
							boundaryElements.emplace_front(const_cast<Element *>(&previous));
							gainedElements.push_back(&next);
							break;
						}
					}
					for (std::vector<const Element *>::const_iterator iter = lostElements.begin( );iter != lostElements.end( );++iter) {
						const Element * e = (*iter);
						size_t x = e->indexOf(0);
						size_t y = e->indexOf(1);
						e->distributeMassToNeighbors(meshDefinition);
					}
					for (std::vector<Element *>::iterator iter = gainedElements.begin( );iter != gainedElements.end( );++iter) {
						Element * e = (*iter);
						e->collectMassFromNeighbors(meshDefinition);
					}

					//tell the client about it
					giveElementsToClient(client);


#endif
				}
				client.simulationComplete( );
			} catch (std::exception &e) {
				VCELL_LOG(fatal,"run( ) caught " << e.what( )  << " generation " << generationCount);
				throw e;
			}
		}

		const spatial::MeshDef<moving_boundary::CoordinateType,2> & meshDef( ) const {
			return meshDefinition;
		}

		unsigned int numberTimeSteps( ) const {
			return static_cast<unsigned int>(maxTime / timeStep);
		}
		void setHeartbeat(size_t numGen, const std::string &symbol)  {
			heartbeat = numGen;
			heartbeatSymbol = symbol;
		}

		const MovingBoundarySetup & setup( ) const {
			return setup_;
		}

		static void registerType( ) {
			MBMesh::registerType( );
			vcell_persist::Registrar::reg<MovingBoundaryParabolicProblemImpl>("MovingBoundaryParabolicProblemImpl");
		}

		/**
		* Functor -- convert Element * to index 
		*/
		typedef unsigned short ElementStorageType;

		struct ElementToIndex : public FunctorBase {
			ElementToIndex(const Outer &o)
				:FunctorBase(o) {}

			ElementStorageType operator( )(const Element *in) {
				spatial::MeshPosition mp = outer.primaryMesh.indexOf(in->indexes( ));
				return mp.to<ElementStorageType>( ); 
			}
		};

		/**
		* Functor -- convert index to Element *  
		*/
		struct IndexToElement : public FunctorBase {
			IndexToElement(const Outer &o)
				:FunctorBase(o) {}

			Element * operator( )(ElementStorageType i) {
				spatial::MeshPosition mp(i);
				return & outer.primaryMesh.get(mp);
			}
		};

		/**
		* persist given vector of elements to os
		* converts Element * to ElementStorageType
		* @tparam E const Element or Element
		*/
		template <class E>
		void persistElementsVector(std::ostream &os, const vector<E *>  &vec) const {
			std::vector<ElementStorageType> writeBuffer(vec.size( ));
			ElementToIndex eToIndex(*this);
			std::transform(vec.begin( ),vec.end( ),writeBuffer.begin( ),eToIndex);
			vcell_persist::save(os,writeBuffer);
		}

		/**
		* restore given vector of elements from is 
		* converts ElementStorageType to (const) Element * 
		* @tparam E const Element or Element
		*/
		template <class E>
		void restoreElementsVector(std::istream &is, vector<E *>  &vec) const {
			std::vector<ElementStorageType> readBuffer; 
			vcell_persist::restore(is,readBuffer);
			IndexToElement iToElement(*this);
			vec.resize( readBuffer.size( ) );
			std::transform(readBuffer.begin( ),readBuffer.end( ),vec.begin( ),iToElement);
		}

		/**
		* save this. Note #MovingBoundarySetup must be saved separately by client
		*/
		void persist(std::ostream &os) const {
			//setup_.persist(os);
			vcell_persist::Token::insert<MovingBoundaryParabolicProblemImpl>(os);
			//vcell_persist::binaryWrite(os,diffusionConstant);
			vcell_persist::binaryWrite(os,currentTime);
			//vcell_persist::binaryWrite(os,maxTime);
			vcell_persist::binaryWrite(os,timeStep);
			vcell_persist::binaryWrite(os,baselineTime);
			vcell_persist::binaryWrite(os,baselineGeneration);
			vcell_persist::binaryWrite(os,minimimMeshInterval);
			vcFront->persist(os);
			vcell_persist::save(os,currentFront);
			meshDefinition.persist(os);
			primaryMesh.persist(os);
			vcell_persist::binaryWrite(os,interiorVolume);

			persistElementsVector(os, boundaryElements);
			persistElementsVector(os, lostElements);
			persistElementsVector(os, gainedElements);
			vcell_persist::binaryWrite(os,heartbeat);
			vcell_persist::StdString<>::save(os,heartbeatSymbol);
		}

		MovingBoundaryParabolicProblemImpl(const moving_boundary::MovingBoundarySetup &mbs, std::istream &is) 
			:ValidationBase(mbs),
			world(WorldType::get( )),
			setup_(mbs),
			diffusionConstant(mbs.diffusionConstant),
			currentTime(0),
			maxTime(mbs.maxTime),
			timeStep(mbs.timeStep),
			baselineTime(0),
			baselineGeneration(0),
			minimimMeshInterval(0),

			symTable(buildSymTable( )),
			levelExp(mbs.levelFunctionStr,symTable), 
			advectVelocityExpX(mbs.advectVelocityFunctionStrX,symTable), 
			advectVelocityExpY(mbs.advectVelocityFunctionStrY,symTable), 
			frontVelocityExpX(mbs.frontVelocityFunctionStrX,symTable), 
			frontVelocityExpY(mbs.frontVelocityFunctionStrY,symTable), 
			concentrationExp(mbs.concentrationFunctionStr,symTable), 

			vcFront(nullptr),
			currentFront( ),
			meshDefinition( ),
			interiorVolume( ),
			primaryMesh(),
			voronoiMesh(),
			//alloc( ),
			boundaryElements( ),
			lostElements( ),
			gainedElements(  ),
			heartbeat(0),
			heartbeatSymbol( ) {
				vcell_persist::Token::check<MovingBoundaryParabolicProblemImpl>(is);
				//vcell_persist::binaryRead(is,diffusionConstant);
				vcell_persist::binaryRead(is,currentTime);
				//vcell_persist::binaryRead(is,maxTime);
				vcell_persist::binaryRead(is,timeStep);
				vcell_persist::binaryRead(is,baselineTime);
				vcell_persist::binaryRead(is,baselineGeneration);
				vcell_persist::binaryRead(is,minimimMeshInterval);
				vcFront = moving_boundary::restoreFrontProvider(is);
				vcell_persist::restore(is,currentFront);
				meshDefinition = MBMeshDef(is);
				primaryMesh = MBMesh(is);
				vcell_persist::binaryRead(is,interiorVolume);

				restoreElementsVector(is, boundaryElements);
				restoreElementsVector(is, lostElements);
				restoreElementsVector(is, gainedElements);
				vcell_persist::binaryRead(is,heartbeat);
				vcell_persist::StdString<>::restore(is,heartbeatSymbol);
				voronoiMesh.setMesh(primaryMesh);
			}
	#ifdef PROB_NOT
		/**

		* this needs reference to #MovingBoundarySetup, so parse that first
		* before creating object
		*/
		static MovingBoundaryParabolicProblemImpl * restore(std::istream &is) {
			MovingBoundarySetup(
		}
#endif

		WorldType & world;
		const MovingBoundarySetup &setup_;
		const double diffusionConstant;
		/**
		* currentTime must be set before #vcFront initialized
		*/
		double currentTime;
		const double maxTime;
		double timeStep; 
		double baselineTime;
		unsigned int baselineGeneration;
		/**
		* in problem domain units
		*/
		double minimimMeshInterval;

		/**
		* symbol table for expressions, must be initialized prior to xxExp
		*/
		SimpleSymbolTable symTable;
		/**
		* level expression, must be created before front
		*/
		mutable VCell::Expression levelExp; //Expression doesn't currently support "const"
		mutable VCell::Expression advectVelocityExpX;
		mutable VCell::Expression advectVelocityExpY;
		mutable VCell::Expression frontVelocityExpX;
		mutable VCell::Expression frontVelocityExpY;
		mutable VCell::Expression concentrationExp;
		/**
		* FronTier integration
		*/
		spatial::FrontProvider<moving_boundary::CoordinateType> * vcFront;
		FrontType currentFront;
		MBMeshDef meshDefinition;

		/**
		* standard volume of inside point
		*/
		double interiorVolume;
		MBMesh primaryMesh;
		VrnMesh voronoiMesh;
		//		vcell_util::ChunkAllocator<VoronoiResult,60> alloc;
		/**
		* current boundary elements
		*/
		//typedef std::forward_list<Element *> ElementList; 
		std::vector<Element *> boundaryElements;
		/**
		* elements "lost" (moved outside of boundary) during transition between generations
		* point to previous generation 
		*/
		std::vector<const Element *> lostElements;
		/**
		* elements "gained" (moved inside of boundary) during transition between generations
		* point to next generation 
		*/
		std::vector<Element *> gainedElements;
		size_t heartbeat;
		std::string heartbeatSymbol;

	};

	//**************************************************
	// local definitions, functors / functions 
	//**************************************************
	namespace {
		struct Evaluator {
			virtual ~Evaluator( ) {}
			virtual double evaluate(MovingBoundaryParabolicProblemImpl::Element & mes) const = 0;
			virtual const char * const label( ) const = 0;
			virtual const char * const checkFunction( ) const = 0;
			bool supportsCheck( ) const {
				return checkFunction( ) != 0; 
			}
		};

		struct Concentration : public Evaluator {
			virtual double evaluate(MovingBoundaryParabolicProblemImpl::Element & mes) const { 
				return mes.concentration(0);
			}
			virtual const char * const label( ) const {
				return "concentration";
			}
			virtual const char * const checkFunction( ) const {
				return 0;
			}
		};

		struct VolumeEval : public Evaluator {
			virtual double evaluate(MovingBoundaryParabolicProblemImpl::Element & mes) const { 
				double v = mes.volumePD( );
				return v;
			}
			virtual const char * const label( ) const {
				return "volume";
			}
			virtual const char * const checkFunction( ) const {
				return  "polyarea";
			}
		};

		void meshElementPlot(const Evaluator & ev, MovingBoundaryParabolicProblemImpl & impl, std::ostream &os) {
			char colors[] = {'y','m','c','g','b','k'};	
			vcell_util::Modulo<int,int> cIndex(0,sizeof(colors)/sizeof(colors[0]));

			MovingBoundaryParabolicProblemImpl::MBMesh::iterator iter = impl.primaryMesh.begin( );
			matlabBridge::Patch patch(ev.label( ), ev.checkFunction( ), "1e-3");
			matlabBridge::Scatter scatter('g',2,true);
			matlabBridge::Scatter empty('r',2,true);
			int seqNo = 1; //used to cause matlab variables to change
			for (;iter != impl.primaryMesh.end( ); ++iter, ++seqNo) {
				MovingBoundaryParabolicProblemImpl::Element & mes = *iter;
				/*
				if (mes.indexOf(cX) != 5) 
				continue;
				if (mes.indexOf(cY) != 11 && mes.indexOf(cY) != 11 ) {
				continue;
				}
				*/

				std::stringstream ss; ss << '[' << mes.indexOf(cX) << ',' << mes.indexOf(cY) << ']';
				std::string spec("-");
				spec += colors[cIndex];
				++cIndex;
				matlabBridge::Polygon boundingPoly("k",2, seqNo);
				const Volume2DClass & vol = mes.getControlVolume();
				if (vol.empty( )) {
					empty.add(mes(cX),mes(cY));
					ss << "empty";
					os << matlabBridge::ConsoleMessage(ss.str( ).c_str( )); 
					continue;
				}
				double c =  ev.evaluate(mes);
				Volume2DClass::VectorOfVectors vOfV = vol.points( );
				const bool checkVal = vOfV.size( ) == 1;
				patch.checkControl( ) = ev.supportsCheck( ) && checkVal;
				for (Volume2DClass::VectorOfVectors::const_iterator vvIter = vOfV.begin( ); vvIter != vOfV.end( );++vvIter) {
					matlabBridge::Polygon pPoly("k",3);
					patch.specifyValueAndClear(c, seqNo);
					frontTierAdapt::copyVectorInto(pPoly,*vvIter);
					frontTierAdapt::copyVectorInto(patch,*vvIter);
					os << patch << pPoly;
				}
				scatter.add(mes(cX),mes(cY));
				os << matlabBridge::ConsoleMessage(ss.str( ).c_str( )); 
			}
			patch.setScale(os);
			os << scatter << empty;
		}
	}

	//**************************************************
	// MovingBoundary proper
	//**************************************************

	MovingBoundaryParabolicProblem::MovingBoundaryParabolicProblem(const MovingBoundarySetup &mbs)
		:impl(new MovingBoundaryParabolicProblemImpl(mbs)) { }

	MovingBoundaryParabolicProblem::MovingBoundaryParabolicProblem(const MovingBoundarySetup &mbs, std::istream &is) 
		:impl(new MovingBoundaryParabolicProblemImpl(mbs,is)) { }

	const spatial::MeshDef<moving_boundary::CoordinateType,2> & MovingBoundaryParabolicProblem::meshDef( ) const {
		return impl->meshDef( );
	}
	double MovingBoundaryParabolicProblem::baseTimeStep( ) const {
		return impl->timeStep;
	}
	unsigned int MovingBoundaryParabolicProblem::numberTimeSteps( ) const {
		return impl->numberTimeSteps( );
	}

	MovingBoundaryParabolicProblem::~MovingBoundaryParabolicProblem( ) {
		delete impl;
	}

	void MovingBoundaryParabolicProblem::setHeartbeat(size_t numGen, const std::string &symbol) {
		impl->setHeartbeat(numGen,symbol);
	}
	void MovingBoundaryParabolicProblem::run(moving_boundary::MovingBoundaryClient & client) {
		impl->run(client);
	}

	void MovingBoundaryParabolicProblem::plotPolygons(std::ostream &os)  const {
		meshElementPlot(Concentration( ),*impl,os);
	}
	void MovingBoundaryParabolicProblem::plotAreas(std::ostream &os) const {
		meshElementPlot(VolumeEval( ),*impl,os);
	}
	double MovingBoundaryParabolicProblem::endTime( ) const {
		return impl->maxTime;
	}
	const MovingBoundarySetup &MovingBoundaryParabolicProblem::setup( ) const {
		return impl->setup( );
	}

	void MovingBoundaryParabolicProblem::persist(std::ostream &os) const {
		impl->persist(os);
	}
	void MovingBoundaryParabolicProblem::registerType( ) {
		MovingBoundaryParabolicProblemImpl::registerType( );
	}

}
