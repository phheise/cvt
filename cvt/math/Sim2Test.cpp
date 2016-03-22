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

//
//  SL3Test.cpp
//  CVTools
//
//  Created by Sebastian Klose 
//  Copyright 2011. All rights reserved.
//

#include <cvt/math/Sim2.h>
#include <cvt/math/Math.h>
#include <cvt/math/Matrix.h>
#include <cvt/util/CVTTest.h>

#include <Eigen/Core>

namespace cvt {
    
	static bool testJacobian()
	{
		Eigen::Matrix<double, 4, 1> delta = Eigen::Matrix<double, 4, 1>::Zero();
		Eigen::Matrix<double, 3, 4> jNumeric, jAnalytic, jDiff;
		Eigen::Matrix<double, 2, 4> sJN, sJA, sJDiff;

		Sim2<double> pose;
		pose.set( 0.2, Math::deg2Rad( 10 ), 10, -10 );

		Eigen::Matrix<double, 3, 1> point;
		Eigen::Matrix<double, 3, 1> p, pp;
		Eigen::Matrix<double, 2, 1> sp, spp;
		point[ 0 ] = 13; point[ 1 ] = 8; point[ 2 ] = 12;;

		pose.transform( p, point );
		pose.project( sp, point );

		double h = 0.0000001;
		for( size_t i = 0; i < 4; i++ ){
			delta[ i ] = h;

			pose.apply( delta );

			pose.transform( pp, point );
			jNumeric.col( i ) = ( pp - p ) / h;

			pose.project( spp, point );
			sJN.col( i ) = ( spp - sp ) / h;

			delta[ i ] = -h;
			pose.apply( delta );
			delta[ i ] = 0;
		}

		pose.jacobian( jAnalytic, point );
		pose.project( spp, sJA, point );

		bool b, ret = true;
		jDiff = jAnalytic - jNumeric;
		b = ( jDiff.array().abs().sum() / 12.0 ) < 0.001;

		CVTTEST_PRINT( "Pose Jacobian", b );
		if( !b ){
			std::cout << "Analytic:\n" << jAnalytic << std::endl;
			std::cout << "Numeric:\n" << jNumeric << std::endl;
			std::cout << "Difference:\n" << jDiff << std::endl;
		}
		ret &= b;

		sJDiff = sJA - sJN;
		b = ( sJDiff.array().abs().sum() / 8.0 ) < 0.001;
		CVTTEST_PRINT( "Screen Jacobian", b );
		if( !b ){
			std::cout << "Analytic:\n" << sJA << std::endl;
			std::cout << "Numeric:\n" << sJN << std::endl;
			std::cout << "Difference:\n" << sJDiff << std::endl;
			std::cout << "Avg. Diff: " << sJDiff.array().abs().sum() / 16.0 << std::endl;
		}
		ret &= b;

		return ret;
	}

	static bool testHessian()
	{
        Eigen::Matrix<double, 4, 1> delta = Eigen::Matrix<double, 4, 1>::Zero();        
		Eigen::Matrix<double, 12, 4> hN, hA;
        
		Sim2<double> pose;
		pose.set( 0.2, Math::deg2Rad( 10 ), 10, -10 );
        
		Eigen::Matrix<double, 3, 1> point;
		Eigen::Matrix<double, 3, 1> p, ff, fb, bf, bb, xxf, xxb, hess;
		point[ 0 ] = 16; 
		point[ 1 ] = 80; 
		point[ 2 ] = 13;
        
		pose.transform( p, point );

		double h = 0.001;
		for( size_t i = 0; i < 4; i++ ){
            for( size_t j = 0; j < 4; j++ ){
				delta.setZero();
				if( i == j ){
					// +
					delta[ j ] = h;
					pose.apply( delta );
					pose.transform( xxf, point );
					pose.apply( -delta );

					delta[ j ] = -h;
					pose.apply( delta );
					pose.transform( xxb, point );
					pose.apply( -delta );

					hess = ( xxb - 2 * p + xxf ) / ( h*h );
				} else {
					delta[ i ] = h;
					delta[ j ] = h;
					pose.apply( delta );
					pose.transform( ff, point );
					pose.apply( -delta );

					delta[ i ] = h;
					delta[ j ] = -h;
					pose.apply( delta );
					pose.transform( fb, point );
					pose.apply( -delta );

					delta[ i ] = -h;
					delta[ j ] =  h;
					pose.apply( delta );
					pose.transform( bf, point );
					pose.apply( -delta );
					
					delta[ i ] = -h;
					delta[ j ] = -h;
					pose.apply( delta );
					pose.transform( bb, point );
					pose.apply( -delta );

					hess = ( ff - bf - fb + bb ) / ( 4 * h * h );
				}

                hN( 3 * i , j ) = hess[ 0 ];
                hN( 3 * i + 1 , j ) = hess[ 1 ];
                hN( 3 * i + 2 , j ) = hess[ 2 ];
            }
		}
		
		pose.hessian( hA, p );
        
		bool b, ret = true;
        Eigen::Matrix<double, 12, 4> jDiff;
        jDiff = hN - hA;
		b = ( jDiff.array().abs().sum() / ( 12.0 * 4.0 ) ) < 0.001;
        
		CVTTEST_PRINT( "Pose Hessian", b );
		if( !b ){
			std::cout << "Analytic:\n" << hA << std::endl;
			std::cout << "Numeric:\n" << hN << std::endl;
			std::cout << "Difference:\n" << jDiff << std::endl;
		}
		ret &= b;
        
		return ret;
	}
    
    static bool testScreenHessian()
    {        
        Eigen::Matrix<double, 4, 1> delta = Eigen::Matrix<double, 4, 1>::Zero();        
		Eigen::Matrix<double, 4, 4> shNumericX, shNumericY, shX, shY;
        
		Sim2<double> pose;
		pose.set( 0.2, Math::deg2Rad( 10 ), 10, -10 );
        
		Eigen::Matrix<double, 3, 1> point, ptrans;
		Eigen::Matrix<double, 2, 1> sp, ff, fb, bf, bb, xxf, xxb, hess;
		point[ 0 ] = 13; point[ 1 ] = 8; point[ 2 ] = 13;
        
        // project the point with current parameters
		pose.project( sp, point );
		pose.transform( ptrans, point );

		double h = 0.0001;
		for( size_t i = 0; i < 4; i++ ){
            for( size_t j = 0; j < 4; j++ ){

				if( i == j ){
					// +
					delta[ j ] = h;
					pose.apply( delta );
					pose.project( xxf, point );
					delta[ j ] = -2 * h;
					pose.apply( delta );
					pose.project( xxb, point );

					hess = ( xxb - 2 * sp + xxf ) / ( h*h );
					
					// back to start
					delta[ j ] = h;
					pose.apply( delta );
					delta[ j ] = 0;
				} else {
					delta[ i ] = h;
					delta[ j ] = h;
					pose.apply( delta );
					pose.project( ff, point );
					pose.apply( -delta );

					delta[ i ] = h;
					delta[ j ] = -h;
					pose.apply( delta );
					pose.project( fb, point );
					pose.apply( -delta );

					delta[ i ] = -h;
					delta[ j ] =  h;
					pose.apply( delta );
					pose.project( bf, point );
					pose.apply( -delta );
					
					delta[ i ] = -h;
					delta[ j ] = -h;
					pose.apply( delta );
					pose.project( bb, point );
					pose.apply( -delta );

					hess = ( ff - bf - fb + bb ) / ( 4 * h * h );
					delta.setZero();
				}

                shNumericX( i, j ) = hess[ 0 ];
                shNumericY( i, j ) = hess[ 1 ];
                
            }
		}
        
		pose.screenHessian( shX, shY, sp );
        
		bool b, ret = true;
        Eigen::Matrix<double, 4, 4> jDiff;
        jDiff = shNumericX - shX;
		b = ( jDiff.array().abs().sum() / 16.0 ) < 0.001;
        
		CVTTEST_PRINT( "Pose ScreenHessian X", b );
		if( !b ){
			std::cout << "Analytic:\n" << shX << std::endl;
			std::cout << "Numeric:\n" << shNumericX << std::endl;
			std::cout << "Difference:\n" << jDiff << std::endl;
		}
		ret &= b;
        
        jDiff = shNumericY - shY;
		b = ( jDiff.array().abs().sum() / 16.0 ) < 0.001;
        
		CVTTEST_PRINT( "Pose ScreenHessian Y", b );
		if( !b ){
			std::cout << "Analytic:\n" << shY << std::endl;
			std::cout << "Numeric:\n" << shNumericY << std::endl;
			std::cout << "Difference:\n" << jDiff << std::endl;
		}
		ret &= b;
        
        return ret;
    }

BEGIN_CVTTEST( Sim2 )
	bool ret = true;

	Sim2<double> pose;
	bool b = true;
	b = ( pose.transformation() == Eigen::Matrix<double, 3, 3>::Identity() );

	CVTTEST_PRINT( "Initialization", b );
	ret &= b;

	ret &= testJacobian();
    ret &= testHessian();
    ret &= testScreenHessian();

	return ret;
END_CVTTEST

}
