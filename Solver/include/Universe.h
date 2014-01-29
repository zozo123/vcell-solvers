#ifndef Universe_h
#define Universe_h
#include <TPoint.h>
#include <MovingBoundaryTypes.h>
#include <VCellException.h>
#include <GeoLimit.h>

namespace moving_boundary {

	//forward
	//enum UniverseLock;
	enum UniverseLock {unset, set, lockedUniverse};
	template<int N>
	struct WorldBase; 

	template <int NUM_DIM>
	struct Universe {
		/**
		* type used for number of nodes in a dimension
		*/
		typedef unsigned short CountType;
		/**
		* return singleton instance
		*/
		static Universe<NUM_DIM> & get( );

		/**
		* initialize the world. std::logic_error
		* @param limits to use
		* @param numNodes in a dimension 
		* @param lock if true, prevents destory from being used
		* @throws std::logic_error if called more than once without #destroy call
		* @throws std::domain_error if low values not less than high values
		*/
		void init(std::array<spatial::GeoLimit, NUM_DIM> & limits, std::array<CountType,NUM_DIM> numNodes= std::array<CountType,NUM_DIM>( ),
			bool lock = false);

		/**
		* return limit for specific dimension
		* @param i
		* @throws std::domain_error if i < 0 or >= NUM_DIM
		*/
		spatial::GeoLimit limitFor(unsigned int i) const {
			if (i < NUM_DIM) {
				return inputLimits[i];
			}
			VCELL_EXCEPTION(domain_error,"Invalid index " << i << " for " << NUM_DIM << " dimensional world")
		}

		/**
		* return limit for specific dimension
		* @param a 
		* @throws std::domain_error if a out of range for NUM_DIM 
		*/
		spatial::GeoLimit limitFor(spatial::Axis a) const {
			return limitFor(static_cast<unsigned int>(a));
		}

		/**
		* @return reference to complete set of limits
		*/
		const std::array<spatial::GeoLimit,NUM_DIM> & limits( ) const {
			return inputLimits;
		}
		const std::array<double,NUM_DIM> & zeros ( ) const {
			return inputZeroPoint;
		}

		/**
		* reset the world to the unset state. Intended for testing only
		* @throws std::domain_error if locked
		*/
		void destroy( );

		/**
		* @return true if Universe locked from further initialization
		*/
		bool locked( ) const;

		const std::array<CountType,NUM_DIM> & numNodes( ) const {
			return nodeNumbers; 
		}

		double diagonal( ) const {
			return diagonal_;
		}

	private:
		Universe( );
		std::array<spatial::GeoLimit,NUM_DIM> inputLimits;
		std::array<CountType,NUM_DIM> nodeNumbers;
		std::array<double,NUM_DIM> inputZeroPoint;
		double diagonal_;
		UniverseLock lockState;
		WorldBase<NUM_DIM> *worlds;
		template <typename COORD_TYPE, int> friend struct World;
	};
}
#endif
