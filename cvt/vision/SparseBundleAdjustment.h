/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2013, Philipp Heise and Sebastian Klose

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#ifndef CVT_SPARSE_BUNDLE_ADJUSTMENT
#define CVT_SPARSE_BUNDLE_ADJUSTMENT

#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET
#include <Eigen/StdVector>
#include <set>

#include <Eigen/Core>
#include <Eigen/Sparse>

#include <cvt/vision/slam/SlamMap.h> 
#include <cvt/math/SparseBlockMatrix.h>
#include <cvt/math/JointMeasurements.h>
#include <cvt/math/TerminationCriteria.h>

namespace cvt {
	class SparseBundleAdjustment
	{
		public:
			SparseBundleAdjustment();
			~SparseBundleAdjustment();

			void optimize( SlamMap & data, const TerminationCriteria<double> & criteria );

			size_t iterations() const { return _iterations; }
			void setIterations( size_t newValue ) { _iterations = newValue; }

			double costs( )  const { return _costs; }
			void setCosts( double newValue ) { _costs = newValue; }

			double lambda( ) const { return _lambda; }
			void setLambda( double newValue ) { _lambda = newValue; }

//		private:
			/* jacobians for each point */		
			static const size_t pointParamDim = 3;
			static const size_t camParamDim   = 6;
			typedef Eigen::Matrix<double, 2, pointParamDim>				PointScreenJacType;
			typedef Eigen::Matrix<double, 2, camParamDim>				CamScreenJacType;
			typedef Eigen::Matrix<double, camParamDim, camParamDim>		CamJTJ;
			typedef Eigen::Matrix<double, pointParamDim, pointParamDim>	PointJTJ;
			typedef Eigen::Matrix<double, camParamDim, 1>				CamResidualType;
			typedef Eigen::Matrix<double, pointParamDim, 1>				PointResidualType;			
			typedef Eigen::Matrix<double, camParamDim, pointParamDim>   CamPointJTJ;

			const PointJTJ* getPJTJDiagonalElement( int index ){ return ( const PointJTJ* ) &( _pointsJTJ[ index ] ); }
			const PointJTJ* getInvAugPJTJDiagonalElement( int index ){ return ( const PointJTJ* ) &( _invAugPJTJ[ index ] ); }
			const CamJTJ* getCJTJDiagonalElement( int index ){ return ( const CamJTJ* ) &( _camsJTJ[ index ] ); }
			const CamResidualType* getCamResidual( int index ){ return ( const CamResidualType* ) &( _camResiduals[ index ] ); }
			const PointResidualType* getPointResudual( int index ){ return ( const PointResidualType* ) &( _pointResiduals[ index ] ); }
			const Eigen::Matrix<double, camParamDim, pointParamDim>* getElementOfCamPointJTJ( int row, int column )
			{
				return ( const CamPointJTJ* ) &( _camPointJTJ.block( row, column ) );
			}

			const float sparseReducedGetElement( int row, int column ){return _sparseReduced.coeff( row, column ); }
			const float reducedRHSGetElement( int index ){return _reducedRHS( index ); }

			const Eigen::SparseMatrix<double, Eigen::ColMajor>* getSparseReduced( ){return ( const Eigen::SparseMatrix<double, Eigen::ColMajor>* ) & _sparseReduced; }
			const Eigen::VectorXd* getReducedRHS( ){ return ( const Eigen::VectorXd* ) &_reducedRHS; }


		private:
			size_t _nPts;
			size_t _nCams;
			size_t _nMeas;

			/* approx. Point Hessians */
			PointJTJ*			_pointsJTJ;

			/* inverse of the augmented approx Point Hessians */
			PointJTJ*			_invAugPJTJ;

			CamJTJ*				_camsJTJ;
			CamResidualType*	_camResiduals;
			PointResidualType*	_pointResiduals;
			
			/* Sparse Upper Left of the approx. Hessian */
			SparseBlockMatrix<camParamDim, pointParamDim>			_camPointJTJ;
			JointMeasurements										_jointMeasures;
			
			Eigen::SparseMatrix<double, Eigen::ColMajor>			_sparseReduced;
			Eigen::VectorXd											_reducedRHS;

			// levenberg marquard damping
			double _lambda;
			size_t _iterations;
			double _costs;

		public:
			void buildReducedCameraSystem( const SlamMap & map );
			void evaluateApproxHessians( const SlamMap & map );
			void fillSparseMatrix( const SlamMap & map );

			// calculate the augmented inverse Hessians of the points:
			void updateInverseAugmentedPointHessians();

			// set the cam sums to zero 
			void clear();

			/* reserve appropriate space and create internal blocks once! */
			void prepareSparseMatrix( size_t numCams );

			void setBlockInReducedSparse( const CamJTJ & m,
										  size_t bRow,
										  size_t bCol );

			void updateCameras( const Eigen::VectorXd & deltaCam,
							    SlamMap & map );


			/* solve Structure and re-eval costs on the fly */ 
			void solveStructure( Eigen::VectorXd & deltaStruct, 
								 const Eigen::VectorXd & deltaCam,
								 SlamMap & map );

			void undoStep( const Eigen::VectorXd & dCam,
						   const Eigen::VectorXd & dPoint,
						   SlamMap & map );

			void resize( size_t numCams, size_t numPoints, size_t numMeas );

			void evalScreenJacWrtPoint( Eigen::Matrix<double, 2, 1> & reproj,
										PointScreenJacType & jac,
										const Eigen::Matrix<double, 3, 1> & pInCam,
										const Eigen::Matrix<double, 3, 3> & K,
										const Eigen::Matrix<double, 3, 3> & R ) const;

	};
}

#endif
