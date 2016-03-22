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

/*
 *  PointCorrespondence3d2d.h
 *  CVTools
 *
 *  Created by Sebastian Klose on 03.02.11.
 *  Copyright 2011. All rights reserved.
 *
 */

#ifndef CVT_POINTCORRESPONDENCES_3D2D_H
#define CVT_POINTCORRESPONDENCES_3D2D_H

#include <cvt/vision/MeasurementModel.h>
#include <cvt/vision/Vision.h>
#include <cvt/math/SE3.h>
#include <Eigen/StdVector>

namespace cvt {

	template <typename T>
	class PointCorrespondences3d2d : public MeasurementModel 
		< T, 
		  PointCorrespondences3d2d, 
		  Eigen::Matrix<T, 6, 6>, 
          Eigen::Matrix<T, 6, 1>, 
          Eigen::Matrix<T, 6, 1>,
          Eigen::Matrix<T, 2, 1> >
	{
		public:				
			typedef typename Eigen::Matrix<T, 6, 6> AType;
			typedef typename Eigen::Matrix<T, 6, 1> bType;
			typedef typename Eigen::Matrix<T, 6, 1> ParamType;
			typedef typename Eigen::Matrix<T, 2, 1> MeasType;

			EIGEN_MAKE_ALIGNED_OPERATOR_NEW
		
			PointCorrespondences3d2d( const Eigen::Matrix<T, 3, 3> & intr, const Eigen::Matrix<T, 4, 4> extr );
			~PointCorrespondences3d2d();
		
			T		buildLSSystem( AType & A, bType & b, const CostFunction<T, MeasType> & costFunc );			
			T		evaluateCosts( const CostFunction<T, MeasType> & costFunc );		
			void	apply( const ParamType & delta );
		
			void				add( const Eigen::Matrix<T, 3, 1> & p3d, const Eigen::Matrix<T, 2, 1> & p2d );
			const SE3<T> &		pose() const { return _pose; }
			void	setPose( const Eigen::Matrix<T, 4, 4> & poseT ){ _pose.set( poseT ); }

			const Eigen::Matrix<T, 3, 3> & camKR() const { return _KR; }
			const Eigen::Matrix<T, 3, 1> & camKt() const { return _Kt; }
		
		private:
			SE3<T>		_pose;
			std::vector<Eigen::Matrix<T, 3, 1>, Eigen::aligned_allocator<Eigen::Matrix<T, 3, 1> > >	_data;
			std::vector<Eigen::Matrix<T, 2, 1>, Eigen::aligned_allocator<Eigen::Matrix<T, 2, 1> > >	_meas;

			Eigen::Matrix<T, 3, 3>					_K;
			Eigen::Matrix<T, 3, 3>					_KR;
			Eigen::Matrix<T, 3, 1>					_Kt;
	};
	
	template < typename T >  
	inline PointCorrespondences3d2d<T>::PointCorrespondences3d2d( const Eigen::Matrix<T, 3, 3> & intr, const Eigen::Matrix<T, 4, 4> extr )
   	{
		_K  = intr;
		_KR = intr * extr.block( 0, 0, 3, 3 );
		_Kt = intr * extr.block( 0, 3, 3, 1 );
	}
	
	template < typename T > inline PointCorrespondences3d2d<T>::~PointCorrespondences3d2d()
   	{
	}

	template < typename T >
	inline T PointCorrespondences3d2d<T>::buildLSSystem( Eigen::Matrix<T, 6, 6> & A, 
														 Eigen::Matrix<T, 6, 1> & b,
														 const CostFunction<T, MeasType> & costFunc )
	{
		T costs = 0;
		
		A.setZero();
		b.setZero();
		Eigen::Matrix<T, 2, 1> w, r;
		Eigen::Matrix<T, 2, 6> J;
		Eigen::Matrix<T, 2, 2> weighting = Eigen::Matrix<T, 2, 2>::Identity();
        
        T weight;

		Eigen::Matrix<T, 3, 1> transformedPoint;
		
		for( size_t i = 0; i < _data.size(); i++ ){
			_pose.transform( transformedPoint, _data[ i ] );
			_pose.screenJacobian( J, transformedPoint, _K );
			Vision::project( w, _KR, _Kt, transformedPoint );
				
			r = _meas[ i ] - w;		

			costs += costFunc.cost( r, weight );
			
			// TODO: if the costfunction is robust, the jacobians and residuals need to be weighted
			if( costFunc.isRobust ){
				weighting( 0, 0 ) = weighting( 1, 1 ) = weight;
				A += ( J.transpose() * weighting * J );
                b += ( J.transpose() * weighting * r );
			} else {
				A += ( J.transpose() * J );
				b +=( J.transpose() * r );
			}
		}
		return costs;
	}
	
	template < typename T >
	inline T PointCorrespondences3d2d<T>::evaluateCosts( const CostFunction<T, MeasType> & costFunc )
	{
		T costs = 0;
		T weight;
		Eigen::Matrix<T, 2, 1> p;
		Eigen::Matrix<T, 3, 1> tp;

		for( size_t i = 0; i < _data.size(); i++ ){
			_pose.transform( tp, _data[ i ] );
			Vision::project( p, _KR, _Kt, tp );
			costs += costFunc.cost( _meas[ i ] - p, weight );
		}
		
		return costs;
	}
	
	template < typename T >
	inline void	PointCorrespondences3d2d<T>::apply( const Eigen::Matrix<T, 6, 1> & delta )
	{
		_pose.apply( delta );
	}
	
	template < typename T >
	inline void	PointCorrespondences3d2d<T>::add( const Eigen::Matrix<T, 3, 1> & p3d, const Eigen::Matrix<T, 2, 1> & p2d )
	{
		_data.push_back( p3d );
		_meas.push_back( p2d );
	}
}
#endif
