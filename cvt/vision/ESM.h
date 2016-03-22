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
 *  ESM.h
 *
 *  Created by Sebastian Klose on 10.10.10.
 *  Copyright 2010. All rights reserved.
 *
 */
#ifndef CVT_ESM_H
#define CVT_ESM_H

#include <cvt/gfx/Image.h>
#include <cvt/vision/MeasurementModel.h>
#include <cvt/math/SL3.h>
#include <cvt/math/Matrix.h>
#include <cvt/math/CostFunction.h>
#include <cvt/gfx/ifilter/ITransform.h>

namespace cvt {
	template < typename T >
	class ESM : public MeasurementModel				
				< T, 
				  ESM, 
				  Eigen::Matrix<T, 8, 8>, 
				  Eigen::Matrix<T, 8, 1>, 
				  Eigen::Matrix<T, 8, 1>,
				  T >	
	{
	public:
		ESM();		
		~ESM(){}		
		
		void	updateInput( const Image * img ){ _currI = img; updateInputData(); };
		void	updateTemplate( const Image & img );
		void	setPose( T alpha, T phi, T sx, T sy, T tx, T ty, T v0, T v1 );
		
		
		const SL3<T>& pose() { return _pose; }
		
		void	apply( const Eigen::Matrix<T, 8, 1> & delta );
		T		buildLSSystem( Eigen::Matrix<T, 8, 8> & A, Eigen::Matrix<T, 8, 1> & b, const CostFunction<T, T> & costFunc );
		T		evaluateCosts( const CostFunction<T, T> & costFunc );
		
	private:		
		/* the template & warped */
		SL3<T>	_pose;
		Image	_temp;		
		Image	_warped;
		
		//Homography		_hFilter;
		cvt::Matrix3f	_warpMat;
		
		/* pointer to current input image */
		const Image	*_currI;	
		
		/* derivatives wrt the pose parameters */
		Eigen::Matrix<T, 8, 8> _jPose;
		T					   _costs;
		
		/* gradients of template & current warped */
		Image _tmpDx;
		Image _tmpDy;
		Image _warpedDx;
		Image _warpedDy;
		
		// gradient convolution kernels
		IKernel _kdx, _kdy;
		
		void updateInputData();		
		
	};
	
	template <typename T>
	inline ESM<T>::ESM() : _costs( 0.0 ), _kdx( IKernel::HAAR_HORIZONTAL_3 ), _kdy( IKernel::HAAR_VERTICAL_3 )
	{		
	}
	
	template < typename T >
	inline void ESM<T>::updateTemplate( const Image & img )
	{
		_temp.reallocate( img );
		_temp = img;
		
		_warped.reallocate( _temp );
		_tmpDx.reallocate( _temp );
		_tmpDy.reallocate( _temp );		
		_warpedDx.reallocate( _temp );
		_warpedDy.reallocate( _temp );
		
		// compute gradients of the template image
		_temp.convolve( _tmpDx, _kdx );
		_temp.convolve( _tmpDy, _kdy );		
	}	
	
	template <typename T>
	inline void ESM<T>::apply( const Eigen::Matrix<T, 8, 1> & delta )
	{
		_pose.applyInverse( delta );
		updateInputData();
	}
	
	template <typename T>
	inline T ESM<T>::buildLSSystem( Eigen::Matrix<T, 8, 8> & A, Eigen::Matrix<T, 8, 1> & b, const CostFunction<T, T> & costFunc )
	{		
		Eigen::Matrix<T, 2, 1> p;
		p.setZero();
		
		float* timg;
		float* tptr;
		size_t tstride;
		float* tdximg;
		float* tdxptr;
		size_t tdxstride;
		float* tdyimg;
		float* tdyptr;
		size_t tdystride;
		float* wimg;
		float* wptr;
		size_t wstride;
		float* wdximg;
		float* wdxptr;
		size_t wdxstride;
		float* wdyimg;
		float* wdyptr;
		size_t wdystride;
		
		timg = _temp.map<float>( &tstride );
		tptr = timg;
		
		tdximg = _tmpDx.map<float>( &tdxstride );
		tdxptr = tdximg;
		
		tdyimg = _tmpDy.map<float>( &tdystride );
		tdyptr = tdyimg;
		
		wimg = _warped.map<float>( &wstride );
		wptr = wimg;
		
		wdximg = _warpedDx.map<float>( &wdxstride );
		wdxptr = wdximg;
		
		wdyimg = _warpedDy.map<float>( &wdystride );
		wdyptr = wdyimg;
		
		Eigen::Matrix<T, 1, 2> grad;
		T intensityDelta;
		Eigen::Matrix<T, 2, 8> jScreen;
		
		A.setZero();
		b.setZero();
		
		Eigen::Matrix<T, 1, 8> j;
		
		T weight;
		
		_costs = 0.0;		
		size_t pointIdx = 0;
		for( size_t h = 0; h < _temp.height(); h++ ) {
			p[ 1 ] = ( T ) h;
			
			for( size_t x = 0; x < _temp.width(); x++ ){
				p[ 0 ] = ( T ) x;
				
				grad( 0, 0 ) = ( wdxptr[ x ] + tdxptr[ x ] );
				grad( 0, 1 ) = ( wdyptr[ x ] + tdyptr[ x ] );
								
				_pose.screenJacobian( jScreen, p );
				
				j = grad * jScreen;
				
				// compute delta between intensities + ssd				
				intensityDelta = ( wptr[ x ] - tptr[ x ] );
				
				_costs += costFunc.cost( intensityDelta, weight );
				
				if( costFunc.isRobust ){
					A += j.transpose() * weight * j;
					b += weight * intensityDelta * j.transpose();
				} else {
					A += j.transpose() * j;
					b += intensityDelta * j.transpose();
				}	
				
				pointIdx++;
			}
			
			tptr += tstride;
			tdxptr += tdxstride;
			tdyptr += tdystride;
			wptr += wstride;
			wdxptr += wdxstride;
			wdyptr += wdystride;
		}
		
		b *= 2.0;
		
		_costs /= ( T )( _temp.width() * _temp.height() );
		
		_temp.unmap<float>( timg );
		_tmpDx.unmap<float>( tdximg );
		_tmpDy.unmap<float>( tdyimg );
		_warped.unmap<float>( wimg );
		_warpedDx.unmap<float>( wdximg );
		_warpedDy.unmap<float>( wdyimg );
		
		return _costs;
	}
	
	template <typename T>
	inline T ESM<T>::evaluateCosts( const CostFunction<T, T> & costFunc )
	{
		size_t tstride, wstride;
		float * t;
		float *w;
		float * ti = _temp.map<float>( &tstride );
		float * wi = _warped.map<float>( &wstride );
		
		_costs = 0;
		
		t = ti; w = wi;
		T weight;
		int height = _temp.height();		
		while( height-- ){
			for( size_t x = 0; x < _temp.width(); x++ ){
				_costs+= costFunc.cost( w[ x ] - t[ x ], weight );
			}
			w += wstride;
			t += tstride;
		}
		
		_temp.unmap( ti );
		_warped.unmap( wi );
		
		return _costs;
	}
	
	template < typename T >
	inline void	ESM<T>::setPose( T alpha, T phi, T sx, T sy, T tx, T ty, T v0, T v1 )
	{
		_pose.set( alpha, phi, sx, sy, tx, ty, v0, v1 );
		updateInputData();
	}
	
	template < typename T >
	inline void ESM<T>::updateInputData()
	{
		
		// update the homography for the warping
		const Eigen::Matrix<T, 3, 3> & hMat = _pose.transformation();				
		_warpMat[ 0 ].x = ( float )hMat( 0, 0 ); _warpMat[ 0 ].y = ( float )hMat( 0, 1 ); _warpMat[ 0 ].z = ( float )hMat( 0, 2 );
		_warpMat[ 1 ].x = ( float )hMat( 1, 0 ); _warpMat[ 1 ].y = ( float )hMat( 1, 1 ); _warpMat[ 1 ].z = ( float )hMat( 1, 2 );
		_warpMat[ 2 ].x = ( float )hMat( 2, 0 ); _warpMat[ 2 ].y = ( float )hMat( 2, 1 ); _warpMat[ 2 ].z = ( float )hMat( 2, 2 );
		
		// get the pixels using the current warp
		_warped.fill( Color::WHITE );
		ITransform::apply( _warped, *_currI, _warpMat.inverse() );	
		
		// update the gradients of the warped input
		_warped.convolve( _warpedDx, _kdx );
		_warped.convolve( _warpedDy, _kdy );
	}
}


#endif

