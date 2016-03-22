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

#include <cvt/util/SIMD.h>
#include <cvt/util/Time.h>
#include <cvt/util/CVTTest.h>
#include <cvt/math/Math.h>
#include <sstream>

using namespace cvt;

#define AATEST( op, p1, p2, p3, n, op2 ) \
	for( size_t i = 0; i < n; i++ ) {																\
		p2[ i ] = Math::rand( -1e3f, 1e3f );														\
		p3[ i ] = Math::rand( -1e3f, 1e3f );														\
	}																								\
	for( int st = SIMD_BASE; st <= bestType; st++ ) {												\
		std::stringstream ss;																		\
		SIMD* simd = SIMD::get( ( SIMDType ) st );													\
		bool fail = false;																			\
		for( size_t i = 0; i < n; i++ )																\
			p1[ i ] = 0.0f;																			\
		simd->op( p1, p2, p3, n );																	\
		for( size_t i = 0; i < TESTSIZE; i++ ) {                                                    \
			if( Math::abs( p1[ i ] - ( p2[ i ] op2 p3[ i ] ) ) > Math::EPSILONF ) {					\
				fail = true;																		\
				break;																				\
			}																						\
		}																							\
		ss << simd->name();																			\
		ss << " "#op" array array" ;																\
		delete simd;																				\
		CVTTEST_PRINT( ss.str(), !fail );															\
	}

#define AC1TEST( op, p1, p2, c, n, op2 ) \
	for( size_t i = 0; i < n; i++ ) {																\
		p2[ i ] = Math::rand( -1e3f, 1e3f );														\
		c = Math::rand( -1e3f, 1e3f );																\
	}																								\
	for( int st = SIMD_BASE; st <= bestType; st++ ) {												\
		std::stringstream ss;																		\
		SIMD* simd = SIMD::get( ( SIMDType ) st );													\
		bool fail = false;																			\
		for( size_t i = 0; i < n; i++ )																\
			p1[ i ] = 0.0f;																			\
		simd->op( p1, p2, c, n );																	\
		for( size_t i = 0; i < TESTSIZE; i++ ) {                                                    \
			if( Math::abs( p1[ i ] - ( p2[ i ] op2 c ) ) > Math::EPSILONF ) {						\
				std::cout << "[ "<< i << " ] : " << p1[ i ] << " <-> " << ( p2[ i ] op2 c ) << std::endl;					\
				fail = true;																		\
				break;																				\
			}																						\
		}																							\
		ss << simd->name();																			\
		ss << " "#op" array const" ;																\
		delete simd;																				\
		CVTTEST_PRINT( ss.str(), !fail );															\
	}

static bool _hammingTest()
{
    bool result = true;
    
    const size_t num = 15684;
    uint8_t vecA[ num ], vecB[ num ];
    srand( time( NULL ) );
    
    // initialize the arrays:
    for( size_t i = 0; i < num; i++ ){
        vecA[ i ] = ( uint8_t )rand();        
    }
    
      
    SIMDType bestType = SIMD::bestSupportedType();
    for (int st = SIMD_BASE; st <= bestType; st++) {        
        
        SIMD* simd = SIMD::get((SIMDType) st);

        for( size_t i = 0; i < num; i++ ){
            vecB[ i ] = vecA[ i ];
        }
        
        
        // first test should return 0:
        uint64_t expectedResult = 0;
        
        bool tRes = true;
        
        size_t bitdistance = simd->hammingDistance( vecA, vecB, num );            
        tRes &= ( bitdistance == expectedResult );
        
        if( tRes == false ){
            std::cout << "Expected Result: " << expectedResult << ", Bit distance: " << bitdistance << std::endl;
        }
        
        for( size_t i = 0; i < num * 8; i++ ){
            // invert the bit bits
            size_t bucket = i >> 3;
            size_t mask = 1 << ( i & 0x7 );
            vecB[ bucket ] ^= mask;
            expectedResult += 1;
            
            size_t bitdistance = simd->hammingDistance( vecA, vecB, num );            
            tRes &= ( bitdistance == expectedResult );           
            
            if( tRes == false ){
                std::cout << "Expected Result: " << expectedResult << ", Bit distance: " << bitdistance << std::endl;
            }
        }
        
        result &= tRes;
        CVTTEST_PRINT( "HammingDistance " + simd->name() + ": ", tRes );
        
        delete simd;
    }
    
    
    
    return result;
}

static bool _projectTest()
{
	std::vector<Vector2f> gtProjected;
	std::vector<Vector3f> pts3d;
	Matrix4f projectionMat; projectionMat.setIdentity();
	Matrix3f K, R, tmp;
	Vector3f t;
	t.x = 20.0f, t.y = 30.0f, t.z = 40.0f;
	R.setRotationXYZ( Math::HALF_PI, Math::PI / 2.0f, Math::PI / 10.0f );
	K.setIdentity();
	K[ 0 ][ 0 ] = 640.0f;
	K[ 0 ][ 2 ] = 321.0f;
	K[ 1 ][ 1 ] = 641.0f;
	K[ 1 ][ 2 ] = 241.0f;
	tmp = K * R;
	t = K * t;

    projectionMat.setIdentity();
	for( size_t r = 0; r < 3; r++ ){
		for( size_t c = 0; c < 3; c++ ){
			projectionMat[ r ][ c ] = tmp[ r ][ c ];
		}
		projectionMat[ r ][ 3 ] = t[ r ];
    }

	Vector2f res;
	Vector3f p, pp;
	for( size_t i = 0; i < 205; i++ ){
		p.x = Math::rand( -100.0f, 100.0f );
		p.y = Math::rand( -100.0f, 100.0f );
		p.z = Math::rand( -100.0f, 100.0f );
		pp = projectionMat * p;
		res.x = pp.x / pp.z;
		res.y = pp.y / pp.z;
		gtProjected.push_back( res );
		pts3d.push_back( p );
	}

	SIMD* simd = SIMD::instance();
	std::vector<Vector2f> result;

	result.resize( pts3d.size() );
	simd->projectPoints( &result[ 0 ], projectionMat, &pts3d[ 0 ], pts3d.size() );

	bool testResult = true;
	for( size_t i = 0; i < result.size(); i++ ){
		if( result[ i ] != gtProjected[ i ] ){
			testResult = false;
			std::cout << "Error: 3D: " << pts3d[ i ] << ", True: " << gtProjected[ i ] << " Simd: " << result[ i ] << std::endl;			
		}
	}

	return testResult;
}

static void _SADTest( float* src1, float* src2, size_t n )
{
	float reference = 0.0f;
	for( size_t i = 0; i < n; i++ ) {
		src1[ i ] = Math::rand( -100.0f, 100.0f );
		src2[ i ] = Math::rand( -100.0f, 100.0f );
		reference += Math::abs( src1[ i ] - src2[ i ] );
	}

	SIMDType bestType = SIMD::bestSupportedType( );
	for( int st = SIMD_BASE; st <= bestType; st++ ) {
		SIMD* simd = SIMD::get( ( SIMDType ) st );
		float result = simd->SAD( &src1[ 0 ], &src2[ 0 ], n );
		bool fail = false;
		if( Math::abs( reference - result ) / reference > 0.0001 ) {
			fail = true;
			std::cout << "Error: SAD: Reference: " << reference << ", " << simd->name( ) << ": " << result << std::endl;
		}
		std::stringstream ss;
		ss << simd->name( );
		ss << " SAD (float)";
		CVTTEST_PRINT( ss.str( ), !fail );
		delete simd;
	}
}

static void _SSDTest( float* src1, float* src2, size_t n )
{
	float reference = 0.0f;
	for( size_t i = 0; i < n; i++ ) {
		src1[ i ] = Math::rand( -100.0f, 100.0f );
		src2[ i ] = Math::rand( -100.0f, 100.0f );
		reference += Math::sqr( src1[ i ] - src2[ i ] );
	}

	SIMDType bestType = SIMD::bestSupportedType( );
	for( int st = SIMD_BASE; st <= bestType; st++ ) {
		SIMD* simd = SIMD::get( ( SIMDType ) st );
		float result = simd->SSD( &src1[ 0 ], &src2[ 0 ], n );
		bool fail = false;
		if( Math::abs( reference - result ) / reference > 0.0001 ) {
			fail = true;
			std::cout << "Error: SSD: Reference: " << reference << ", " << simd->name( ) << ": " << result << std::endl;
		}
		std::stringstream ss;
		ss << simd->name( );
		ss << " SSD (float)";
		CVTTEST_PRINT( ss.str( ), !fail );
		delete simd;
	}
}

static void _NCCTest( float* src1, float* src2, size_t n )
{
	float *constval = new float[ n ];
	float mean1 = 0.0f;
	float mean2 = 0.0f;
	for( size_t i = 0; i < n; i++ ) {
		src1[ i ] = Math::rand( -100.0f, 100.0f );
		src2[ i ] = Math::rand( -100.0f, 100.0f );
		constval[ i ] = 1.0f;
		mean1 += src1[ i ];
		mean2 += src2[ i ];
	}
	mean1 /= (float) n;
	mean2 /= (float) n;
	
	float var1 = 0.0f;
	float var2 = 0.0f;
	float sum = 0.0f;
	for( size_t i = 0; i < n; i++ ) {
		sum += ( src1[ i ] - mean1 ) * ( src2[ i ] - mean2 );
		var1 += Math::sqr( src1[ i ] - mean1 );
		var2 += Math::sqr( src2[ i ] - mean2 );
	}
	
	var1 /= (float) n;
	var2 /= (float) n;
	
	float reference = sum / ( (float) n * Math::sqrt( var1 * var2 ) );
    
	SIMDType bestType = SIMD::bestSupportedType( );
	for( int st = SIMD_BASE; st <= bestType; st++ ) {
		SIMD* simd = SIMD::get( ( SIMDType ) st );
		float result = simd->NCC( &src1[ 0 ], &src2[ 0 ], n );
		bool fail = false;
		if( Math::abs( result - reference ) > Math::EPSILONF ) {
			fail = true;
			std::cout << "Error: NCC: Reference: " << reference << ", " << simd->name( ) << ": " << result << std::endl;
		}
		std::stringstream ss;
		ss << simd->name( );
		ss << " NCC (float)";
		CVTTEST_PRINT( ss.str( ), !fail );

		// Assure that zero variance results in a non-NaN value
		ss << " (zero variance)";
		CVTTEST_PRINT( ss.str( ), !Math::isNaN( simd->NCC( &src1[ 0 ], &constval[ 0 ], n ) ) );
		delete simd;
	}
	delete[] constval;
}

BEGIN_CVTTEST( simd )
		float* fdst;
		float* fsrc1;
		float* fsrc2;
		float val1;
		Time tmr;
		double t;

        SIMDType bestType = SIMD::bestSupportedType();

#define TESTSIZE ( 32 + 3 )
		fdst = new float[ TESTSIZE ];
		fsrc1 = new float[ TESTSIZE ];
		fsrc2 = new float[ TESTSIZE ];

		AATEST( Add, fdst, fsrc1, fsrc2, TESTSIZE, + )
		AATEST( Sub, fdst, fsrc1, fsrc2, TESTSIZE, - )
		AATEST( Mul, fdst, fsrc1, fsrc2, TESTSIZE, * )
		AATEST( Div, fdst, fsrc1, fsrc2, TESTSIZE, / )

		AC1TEST( AddValue1f, fdst, fsrc1, val1, TESTSIZE, + )
		AC1TEST( SubValue1f, fdst, fsrc1, val1, TESTSIZE, - )
		AC1TEST( MulValue1f, fdst, fsrc1, val1, TESTSIZE, * )
		AC1TEST( DivValue1f, fdst, fsrc1, val1, TESTSIZE, / )

		_SADTest( fsrc1, fsrc2, TESTSIZE );
		_SSDTest( fsrc1, fsrc2, TESTSIZE );
		_NCCTest( fsrc1, fsrc2, TESTSIZE );

		delete[] fdst;
		delete[] fsrc1;
		delete[] fsrc2;
#undef TESTSIZE
                
        bool testResult = _hammingTest();
        CVTTEST_PRINT( "HammingDistance", testResult );
        
		testResult = _projectTest();
        CVTTEST_PRINT( "Project Points 3d->2d", testResult );

#define TESTSIZE ( 2048 * 2048 )
		fdst = new float[ TESTSIZE ];
		fsrc1 = new float[ TESTSIZE ];
		fsrc2 = new float[ TESTSIZE ];

		for( int st = SIMD_BASE; st <= bestType; st++ ) {
			SIMD* simd = SIMD::get( ( SIMDType ) st );
			t = 0;
			for( int iter = 0; iter < 100; iter++ ) {
				tmr.reset();
				simd->Add( fdst, fsrc1, fsrc2, TESTSIZE );
				t += tmr.elapsedMilliSeconds();
			}
			t /= 100.0;
			std::cout << simd->name() << " Add "  << t  << " ms" << std::endl;
			delete simd;
		}

		for( int st = SIMD_BASE; st <= bestType; st++ ) {
			SIMD* simd = SIMD::get( ( SIMDType ) st );
			t = 0;
			for( int iter = 0; iter < 100; iter++ ) {
				tmr.reset();
				simd->Sub( fdst, fsrc1, fsrc2, TESTSIZE );
				t += tmr.elapsedMilliSeconds();
			}
			t /= 100.0;
			std::cout << simd->name() << " Sub "  << t  << " ms" << std::endl;
			delete simd;
		}

		for( int st = SIMD_BASE; st <= bestType; st++ ) {
			SIMD* simd = SIMD::get( ( SIMDType ) st );
			t = 0;
			for( int iter = 0; iter < 100; iter++ ) {
				tmr.reset();
				simd->Mul( fdst, fsrc1, fsrc2, TESTSIZE );
				t += tmr.elapsedMilliSeconds();
			}
			t /= 100.0;
			std::cout << simd->name() << " Mul "  << t  << " ms" << std::endl;
			delete simd;
		}
            
        for( int st = SIMD_BASE; st <= bestType; st++ ) {
            SIMD* simd = SIMD::get( ( SIMDType ) st );
            t = 0;
            for( int iter = 0; iter < 100; iter++ ) {
                tmr.reset();
                simd->MulAddValue1f( fdst, fsrc1, 200.0f, TESTSIZE );
                t += tmr.elapsedMilliSeconds();
            }
            t /= 100.0;
            std::cout << simd->name() << " MulAdd "  << t  << " ms" << std::endl;
            delete simd;
        }
            
        for( int st = SIMD_BASE; st <= bestType; st++ ) {
            SIMD* simd = SIMD::get( ( SIMDType ) st );
            t = 0;
            for( int iter = 0; iter < 100; iter++ ) {
                tmr.reset();
                simd->MulSubValue1f( fdst, fsrc1, 200.0f, TESTSIZE );
                t += tmr.elapsedMilliSeconds();
            }
            t /= 100.0;
            std::cout << simd->name() << " MulSub "  << t  << " ms" << std::endl;
            delete simd;
        }

		for( int st = SIMD_BASE; st <= bestType; st++ ) {
			SIMD* simd = SIMD::get( ( SIMDType ) st );
			t = 0;
			for( int iter = 0; iter < 100; iter++ ) {
				tmr.reset();
				simd->Div( fdst, fsrc1, fsrc2, TESTSIZE );
				t += tmr.elapsedMilliSeconds();
			}
			t /= 100.0;
			std::cout << simd->name() << " Div "  << t  << " ms" << std::endl;
			delete simd;
		}

		for( int st = SIMD_BASE; st <= bestType; st++ ) {
			SIMD* simd = SIMD::get( ( SIMDType ) st );
			t = 0;
			for( int iter = 0; iter < 100; iter++ ) {
				tmr.reset();
				simd->AddValue1f( fdst, fsrc1, 10.0f, TESTSIZE );
				t += tmr.elapsedMilliSeconds();
			}
			t /= 100.0;
			std::cout << simd->name() << " Add Const "  << t  << " ms" << std::endl;
			delete simd;
		}

		for( int st = SIMD_BASE; st <= bestType; st++ ) {
			SIMD* simd = SIMD::get( ( SIMDType ) st );
			t = 0;
			for( int iter = 0; iter < 100; iter++ ) {
				tmr.reset();
				simd->SubValue1f( fdst, fsrc1, 10.0f, TESTSIZE );
				t += tmr.elapsedMilliSeconds();
			}
			t /= 100.0;
			std::cout << simd->name() << " Sub Const "  << t  << " ms" << std::endl;
			delete simd;
		}

		for( int st = SIMD_BASE; st <= bestType; st++ ) {
			SIMD* simd = SIMD::get( ( SIMDType ) st );
			t = 0;
			for( int iter = 0; iter < 100; iter++ ) {
				tmr.reset();
				simd->MulValue1f( fdst, fsrc1, 10.0f, TESTSIZE );
				t += tmr.elapsedMilliSeconds();
			}
			t /= 100.0;
			std::cout << simd->name() << " Mul Const "  << t  << " ms" << std::endl;
			delete simd;
		}

		for( int st = SIMD_BASE; st <= bestType; st++ ) {
			SIMD* simd = SIMD::get( ( SIMDType ) st );
			t = 0;
			for( int iter = 0; iter < 100; iter++ ) {
				tmr.reset();
				simd->DivValue1f( fdst, fsrc1, 10.0f, TESTSIZE );
				t += tmr.elapsedMilliSeconds();
			}
			t /= 100.0;
			std::cout << simd->name() << " Div Const "  << t  << " ms" << std::endl;
			delete simd;
		}

		for( int st = SIMD_BASE; st <= bestType; st++ ) {
			SIMD* simd = SIMD::get( ( SIMDType ) st );
			t = 0;
			for( int iter = 0; iter < 100; iter++ ) {
				tmr.reset();
				simd->SSD( fsrc1, fsrc2, TESTSIZE );
				t += tmr.elapsedMilliSeconds();
			}
			t /= 100.0;
			std::cout << simd->name() << " SSD float "  << t  << " ms" << std::endl;
			delete simd;
		}

		for( int st = SIMD_BASE; st <= bestType; st++ ) {
			SIMD* simd = SIMD::get( ( SIMDType ) st );
			t = 0;
			for( int iter = 0; iter < 100; iter++ ) {
				tmr.reset();
				simd->SAD( fsrc1, fsrc2, TESTSIZE );
				t += tmr.elapsedMilliSeconds();
			}
			t /= 100.0;
			std::cout << simd->name() << " SAD float "  << t  << " ms" << std::endl;
			delete simd;
		}
		for( int st = SIMD_BASE; st <= bestType; st++ ) {
			SIMD* simd = SIMD::get( ( SIMDType ) st );
			t = 0;
			for( int iter = 0; iter < 100; iter++ ) {
				tmr.reset();
				simd->NCC( fsrc1, fsrc2, TESTSIZE );
				t += tmr.elapsedMilliSeconds();
			}
			t /= 100.0;
			std::cout << simd->name() << " NCC float "  << t  << " ms" << std::endl;
			delete simd;
		}

        uint8_t * usrc0 = new uint8_t[ TESTSIZE ];
        for( int x = 0; x < TESTSIZE; x++ )
            usrc0[ x ] = Math::max( x, 255 );

        for( int st = SIMD_BASE; st <= bestType; st++ ) {
            SIMD* simd = SIMD::get( ( SIMDType ) st );
            t = 0;
            for( int iter = 0; iter < 100; iter++ ) {
                tmr.reset();
                simd->prefixSum1_u8_to_f( fdst, 0, usrc0, 0, TESTSIZE, 1 );
                t += tmr.elapsedMilliSeconds();
            }
            t /= 100.0;
            std::cout << simd->name() << " prefixSum1 Const "  << t  << " ms" << std::endl;


            delete simd;
        }

		uint8_t * udst = new uint8_t[ TESTSIZE ];
		for( int st = SIMD_BASE; st <= bestType; st++ ) {
			SIMD* simd = SIMD::get( ( SIMDType ) st );
			t = 0;
			tmr.reset();
			for( int iter = 0; iter < 100; iter++ ) {				
				simd->SAD( udst, usrc0, TESTSIZE );
			}
			t += tmr.elapsedMilliSeconds();
			t /= 100.0;
			std::cout << simd->name() << " SAD uint8_t "  << t  << " ms" << std::endl;
			delete simd;
		}
		delete[] usrc0;
		delete[] udst;	
			
		delete[] fdst;
		delete[] fsrc1;
		delete[] fsrc2;            
            
        uint8_t * ham0 = new uint8_t[ TESTSIZE ];
        uint8_t * ham1 = new uint8_t[ TESTSIZE ];
        size_t ref = 0;
        for( int st = SIMD_BASE; st <= bestType; st++ ) {
            SIMD* simd = SIMD::get( ( SIMDType ) st );
            t = 0;
            tmr.reset();
            size_t pcount;
            for( int iter = 0; iter < 100; iter++ ) {				
                pcount = simd->hammingDistance( ham0, ham1, TESTSIZE );
            }
            t += tmr.elapsedMilliSeconds();
            t /= 100.0;
            std::cout << simd->name() << " HAMMING DISTANCE "  << t  << " ms" << std::endl;
            
            if( st == SIMD_BASE ) {
                ref = pcount;
            } else {
                if( pcount != ref ){
                    std::cout << simd->name() << " pcount = " << pcount << ", reference = " << ref << std::endl;
                }
            }

            
            delete simd;
        }
        delete[] ham0;
        delete[] ham1;
        
#undef TESTSIZE       

		return true;
	END_CVTTEST
