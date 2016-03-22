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
//  Created by Sebastian Klose on 08.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <cvt/math/SL3.h>
#include <cvt/math/Math.h>
#include <cvt/math/Matrix.h>
#include <cvt/util/CVTTest.h>

#include <Eigen/Core>

namespace cvt {

    static bool testJacobianWithGenerators()
    {
        Eigen::Matrix<double, 3, 8> jGenerated, jAnalytic, jDiff;

		SL3<double> pose;
		pose.set( Math::deg2Rad( 10 ), Math::deg2Rad( -25 ), 0.76, 0.89, 10, -10, 0.0001, 0.0002 );

		Eigen::Matrix<double, 3, 1> point;
		Eigen::Matrix<double, 3, 1> p, col;
		point[ 0 ] = 13; point[ 1 ] = 8; point[ 2 ] = 12;;

		pose.transform( p, point );

		for( size_t i = 0; i < SL3<double>::NPARAMS; i++ ){

			col = SL3<double>::generator( i ) * p;
			jGenerated.block<3, 1>( 0, i ) = col;
		}

		pose.jacobian( jAnalytic, point );

		bool b, ret = true;
		jDiff = jAnalytic - jGenerated;
		b = ( jDiff.array().abs().sum() / 24.0 ) < 0.00001;
		CVTTEST_PRINT( "Generated Pose Jacobian", b );
		if( !b ){
			std::cout << "Analytic:\n" << jAnalytic << std::endl;
			std::cout << "Generated:\n" << jGenerated << std::endl;
			std::cout << "Difference:\n" << jDiff << std::endl;
		}
		ret &= b;

        return ret;
    }
    
	static bool testJacobian()
	{
		Eigen::Matrix<double, 8, 1> delta = Eigen::Matrix<double, 8, 1>::Zero();
		Eigen::Matrix<double, 3, 8> jNumeric, jAnalytic, jDiff;
		Eigen::Matrix<double, 2, 8> sJN, sJA, sJDiff;

		SL3<double> pose;
		pose.set( Math::deg2Rad( 10 ), Math::deg2Rad( -25 ), 0.76, 0.89, 10, -10, 0.0001, 0.0002 );

		Eigen::Matrix<double, 3, 1> point;
		Eigen::Matrix<double, 3, 1> p, pp;
		Eigen::Matrix<double, 2, 1> sp, spp;
		point[ 0 ] = 13; point[ 1 ] = 8; point[ 2 ] = 12;;

		pose.transform( p, point );
		pose.project( sp, point );

		

		double h = 0.000000001;
		for( size_t i = 0; i < 8; i++ ){
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
		b = ( jDiff.array().abs().sum() / 24.0 ) < 0.00001;

		CVTTEST_PRINT( "Pose Jacobian", b );
		if( !b ){
			std::cout << "Analytic:\n" << jAnalytic << std::endl;
			std::cout << "Numeric:\n" << jNumeric << std::endl;
			std::cout << "Difference:\n" << jDiff << std::endl;
		}
		ret &= b;

		sJDiff = sJA - sJN;
		b = ( sJDiff.array().abs().sum() / 16.0 ) < 0.0001;
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
        Eigen::Matrix<double, 8, 1> delta = Eigen::Matrix<double, 8, 1>::Zero();        
		Eigen::Matrix<double, 24, 8> hN, hA;
        
		SL3<double> pose;
		pose.set( Math::deg2Rad( 10 ), Math::deg2Rad( -25 ), 0.76, 0.89, 10, -10, 0.0001, 0.0002 );
        
		Eigen::Matrix<double, 3, 1> point;
		Eigen::Matrix<double, 3, 1> p, ff, fb, bf, bb, xxf, xxb, hess;
		point[ 0 ] = 16; 
		point[ 1 ] = 80; 
		point[ 2 ] = 13;
        
		pose.transform( p, point );

		double h = 0.001;
		for( size_t i = 0; i < 8; i++ ){
            for( size_t j = 0; j < 8; j++ ){
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
        Eigen::Matrix<double, 24, 8> jDiff;
        jDiff = hN - hA;
		b = ( jDiff.array().abs().sum() / ( 24.0 * 8.0 ) ) < 0.00001;
        
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
        Eigen::Matrix<double, 8, 1> delta = Eigen::Matrix<double, 8, 1>::Zero();        
		Eigen::Matrix<double, 8, 8> shNumericX, shNumericY, shX, shY;
        
		SL3<double> pose;
		pose.set( Math::deg2Rad( 10 ), Math::deg2Rad( -25 ), 0.76, 0.89, 10, -10, 0.0001, 0.0002 );
        
		Eigen::Matrix<double, 3, 1> point, ptrans;
		Eigen::Matrix<double, 2, 1> sp, ff, fb, bf, bb, xxf, xxb, hess;
		point[ 0 ] = 13; point[ 1 ] = 8; point[ 2 ] = 13;
        
        // project the point with current parameters
		pose.project( sp, point );
		pose.transform( ptrans, point );

		double h = 0.0001;
		for( size_t i = 0; i < 8; i++ ){
            for( size_t j = 0; j < 8; j++ ){

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
        Eigen::Matrix<double, 8, 8> jDiff;
        jDiff = shNumericX - shX;
		b = ( jDiff.array().abs().sum() / 64.0 ) < 0.001;
        
		CVTTEST_PRINT( "Pose ScreenHessian X", b );
		if( !b ){
			std::cout << "Analytic:\n" << shX << std::endl;
			std::cout << "Numeric:\n" << shNumericX << std::endl;
			std::cout << "Difference:\n" << jDiff << std::endl;
		}
		ret &= b;
        
        jDiff = shNumericY - shY;
		b = ( jDiff.array().abs().sum() / 64.0 ) < 0.001;
        
		CVTTEST_PRINT( "Pose ScreenHessian Y", b );
		if( !b ){
			std::cout << "Analytic:\n" << shY << std::endl;
			std::cout << "Numeric:\n" << shNumericY << std::endl;
			std::cout << "Difference:\n" << jDiff << std::endl;
		}
		ret &= b;
        
        return ret;
    }

    static bool testMatrices( const Eigen::Matrix3d & m1, const Matrix3d & m2 )
    {
        for ( size_t i = 0; i < 3; i++ ) {
            for ( size_t k = 0; k < 3; k++ ) {
                if ( Math::abs( m1( i, k ) - m2[ i ][ k ] ) > 0.00001 ) {
                    return false;
                }
            }
        }
        return true;
    }

    static bool testSet()
    {
        bool result = true;

        SL3<double> pose;
        Matrix3d H;

        pose.set( 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0 );
        H.setHomography( 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0 );

        result &= testMatrices( pose.transformation(), H );

        return result;
    }

BEGIN_CVTTEST( SL3 )
	bool ret = true;

	SL3<double> pose;
	bool b = true;
	b = ( pose.transformation() == Eigen::Matrix<double, 3, 3>::Identity() );

	CVTTEST_PRINT( "Initialization", b );
	ret &= b;

    b = testSet();
    CVTTEST_PRINT( "SL3::set( ... )", b );
	ret &= b;

	ret &= testJacobian();
	ret &= testJacobianWithGenerators();
    ret &= testHessian();
    ret &= testScreenHessian();

	return ret;
END_CVTTEST

}
