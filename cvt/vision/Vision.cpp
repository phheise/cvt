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

#include <cvt/vision/Vision.h>

#include <cvt/util/CVTTest.h>
#include <cvt/math/Matrix.h>

#include <cvt/gfx/IMapScoped.h>

#include <vector>


namespace cvt {

    void Vision::unprojectToScenePoints( ScenePoints& scenepts, const Image& texture, const Image& depthmap, const CameraCalibration& calibration, float dscale )
    {
        Matrix3f Kinv = calibration.intrinsics().inverse();
        std::vector<Vector3f> pts;
        std::vector<Vector4f> colors;

        scenepts.clear();

        if( texture.format() != IFormat::RGBA_FLOAT || depthmap.format() != IFormat::GRAY_FLOAT || texture.width() != depthmap.width() || texture.height() != depthmap.height() )
            throw CVTException( "unprojectToScenePoints: invalid texture or depth-map!" );

        IMapScoped<const float> tex( texture );
        IMapScoped<const float> dmap( depthmap );
        size_t w = depthmap.width();
        size_t h = depthmap.height();
        for( size_t y = 0; y < h; y++ ) {
            const float* dmapptr = dmap.ptr();
            const float* texptr = tex.ptr();

            for( size_t x = 0; x < w; x++ ) {
                if( dmapptr[ x ] > 0 ) {
                    Vector3f pt = Kinv * Vector3f( x, y, 1.0f );
                    pts.push_back( pt * dmapptr[ x ] * dscale );
                    colors.push_back( Vector4f( texptr[ x * 4 + 0 ], texptr[ x * 4 + 1 ], texptr[ x * 4 + 2 ], texptr[ x * 4 + 3 ] ) );
                }
            }

            dmap++;
            tex++;
        }

        scenepts.setVerticesWithColor( &pts[ 0 ], &colors[ 0 ], pts.size() );
        scenepts.transform( calibration.extrinsics().inverse() );
    }

    void Vision::unprojectToScenePoints( ScenePoints& scenepts, const Image& texture, const Image& depthmap, float dscale )
    {
        std::vector<Vector3f> pts;
        std::vector<Vector4f> colors;

        scenepts.clear();

        if( texture.format() != IFormat::RGBA_FLOAT || depthmap.format() != IFormat::GRAY_FLOAT || texture.width() != depthmap.width() || texture.height() != depthmap.height() )
            throw CVTException( "unprojectToScenePoints: invalid texture or depth-map!" );

        IMapScoped<const float> tex( texture );
        IMapScoped<const float> dmap( depthmap );
        size_t w = depthmap.width();
        size_t h = depthmap.height();
        float dx = 0.5f * depthmap.width();
        float dy = 0.5f * depthmap.height();

        for( size_t y = 0; y < h; y++ ) {
            const float* dmapptr = dmap.ptr();
            const float* texptr = tex.ptr();

            for( size_t x = 0; x < w; x++ ) {
                if( dmapptr[ x ] > 0 ) {
                    Vector3f pt = Vector3f( ( x - dx ) / ( float ) w , ( y - dy ) / ( float ) h, dmapptr[ x ] * dscale );
                    pts.push_back( pt );
                    colors.push_back( Vector4f( texptr[ x * 4 + 0 ], texptr[ x * 4 + 1 ], texptr[ x * 4 + 2 ], texptr[ x * 4 + 3 ] ) );
                }
            }

            dmap++;
            tex++;
        }

                scenepts.setVerticesWithColor( &pts[ 0 ], &colors[ 0 ], pts.size() );
    }

        void Vision::unprojectToXYZ( PointSet3f& pts, Image& depth, const Matrix3f& K, float depthScale )
        {
            if( depth.format() == IFormat::GRAY_UINT16 ){
                IMapScoped<const uint16_t> depthMap( depth );
                float invFx = 1.0f / K[ 0 ][ 0 ];
                float invFy = 1.0f / K[ 1 ][ 1 ];
                float cx    = K[ 0 ][ 2 ];
                float cy    = K[ 1 ][ 2 ];

                // temp vals
                std::vector<float> tmpx( depth.width() );
                std::vector<float> tmpy( depth.height() );

                for( size_t i = 0; i < tmpx.size(); i++ ){
                    tmpx[ i ] = ( i - cx ) * invFx;
                }
                for( size_t i = 0; i < tmpy.size(); i++ ){
                    tmpy[ i ] = ( i - cy ) * invFy;
                }

                Vector3f p3d;
                for( size_t y = 0; y < depth.height(); y++ ){
                    const uint16_t* dptr = depthMap.ptr();
                    for( size_t x = 0; x < depth.width(); x++ ){
                        float d = dptr[ x ] * depthScale;
                        p3d[ 0 ] = tmpx[ x ] * d;
                        p3d[ 1 ] = tmpy[ y ] * d;
                        p3d[ 2 ] = d;
                        pts.add( p3d );
                    }
                    // next line in depth image
                    depthMap++;
                }
            } else {
                throw CVTException( "Unproject not implemented for given format" );
            }
        }


		void Vision::disparityToDepthmap( Image& depthmap, const Image& disparity, const float dispscale, const float focallength, const float baseline, const float dispthres )
		{
			depthmap.reallocate( disparity.width(), disparity.height(), IFormat::GRAY_FLOAT );

            IMapScoped<const float> src( disparity );
            IMapScoped<float> dst( depthmap );
            float fb = focallength * baseline;

            size_t height = disparity.height();
            while( height-- ){
                float* dptr = dst.ptr();
                const float* sptr = src.ptr();
                for( size_t x = 0; x < disparity.width(); x++ ){
                    float disp = sptr[ x ] * dispscale;
                    if( disp > dispthres )
                        dptr[ x ] = fb / disp;
                    else
                        dptr[ x ] = 0;
                }
                src++;
                dst++;
            }
        }

    template <typename T>
    static bool _compare( const Matrix3<T> & a, const Matrix3<T> & b, T epsilon )
    {
        for( size_t i = 0; i < 3; i++ ){
            for( size_t k = 0; k < 3; k++ ){
                if( Math::abs( a[ i ][ k ] - b[ i ][ k ] ) > epsilon )
                    return false;
            }
        }

        return true;
    }

    template <typename T>
    static bool _compareVectors( const Vector3<T> & a, const Vector3<T> & b, T epsilon )
    {
        if( Math::abs( a[ 0 ] - b[ 0 ] ) > epsilon )
            return false;
        if( Math::abs( a[ 1 ] - b[ 1 ] ) > epsilon )
            return false;
        if( Math::abs( a[ 2 ] - b[ 2 ] ) > epsilon )
            return false;
        return true;
    }

    template <typename T>
    static bool _compareVectorsNormalized( const Vector3<T> & a, const Vector3<T> & b, T epsilon )
    {
        Vector3<T> a0 = a;
        a0.normalize();

        Vector3<T> b0 = b;
        b0.normalize();

        return _compareVectors( a0, b0, epsilon );
    }

    template <typename T>
    static bool _essentialTest()
    {
        bool result = true;

        Matrix3<T> K( 230.0,   0.0, 320.0,
                        0.0, 235.0, 240.0,
                        0.0,   0.0,   1.0 );

        Matrix3<T> R, tSkew;

        srandom( time( NULL ) );

        for( size_t i = 0; i < 100; i++ ){
            R.setRotationXYZ( Math::rand( ( T )-Math::PI/6.0, ( T )Math::PI/6.0 ),
                              Math::rand( ( T )-Math::PI/6.0, ( T )Math::PI/6.0 ),
                              Math::rand( ( T )-Math::PI/6.0, ( T )Math::PI/6.0 ));

            Vector3<T> t( Math::rand( ( T )-1000, ( T )1000 ),
                          Math::rand( ( T )-1000, ( T )1000 ),
                          Math::rand( ( T )-1000, ( T )1000 ) );

            tSkew.setSkewSymmetric( t );


            Matrix3<T> E = tSkew * R;
            E *= 1.0 / E[ 2 ][ 2 ];

            Matrix3<T> R0, R1, RR;
            Vector3<T> t0, t1, tt;
            Vision::decomposeEssential( R0, R1, t0, t1, E );

            bool b = true;

            b = _compare( R, R0, (T)0.0001 ) || _compare( R, R1, (T)0.0001 );
            result &= b;
            if( !b ){
                std::cout << "Ground Truth Rotation: \n" << R << std::endl;
                std::cout << "Decomposed R0: \n"            << R0 << std::endl;
                std::cout << "Decomposed R1: \n"            << R1 << std::endl;
            }

            b = _compareVectorsNormalized( t, t0, (T)0.0001 ) || _compareVectorsNormalized( t, t1, (T)0.0001 );
            result &= b;

            if( !b ){
                std::cout << "Ground Truth Translation: \n" << t << std::endl;
                std::cout << "Decomposed t0: \n" << t0 << std::endl;
                std::cout << "Decomposed t1: \n" << t1 << std::endl;
            }
        }

        return result;
    }

    template <typename T>
    static bool _triangulateMultiView()
    {
        bool ret = true;

        Matrix3<T> K( 600.0,   0.0, 320.0,
                        0.0, 600.0, 240.0,
                        0.0,   0.0,   1.0 );

        Matrix4<T> T0;
        T0.setIdentity();

        size_t ncams = 5;
        std::vector<Matrix4<T> > pmats( ncams );
        std::vector<Vector2<T> > ipts( ncams );

        for( size_t i = 0; i < 100; i++ ){
            Vector4<T> truePoint( Math::rand( ( T )-50, ( T )50 ),
                                  Math::rand( ( T )-50, ( T )50 ),
                                  Math::rand( ( T )5, ( T )50 ),
                                  ( T )1 );

            Vector4<T> tmp;
            Vector2<T> proj;
            for( size_t c = 0; c < ncams; c++ ){
                T0.setRotationXYZ( Math::rand( ( T )-Math::PI/12.0, ( T )Math::PI/12.0 ),
                               Math::rand( ( T )-Math::PI/12.0, ( T )Math::PI/12.0 ),
                               Math::rand( ( T )-Math::PI/12.0, ( T )Math::PI/12.0 ));
                T0.setTranslation( Math::rand( ( T )-0.5, ( T )0.5 ),
                               Math::rand( ( T )-0.5, ( T )0.5 ),
                               Math::rand( ( T )-0.5, ( T )0.5 ) );
                T0[ 3 ][ 3 ] = ( T )1;

                Matrix3<T> R = T0.toMatrix3();
                Matrix4<T> P0( K*R );
                Vector3<T> t( T0[ 0 ][ 3 ], T0[ 1 ][ 3 ], T0[ 2 ][ 3 ] );
                t = K * t;
                P0[ 0 ][ 3 ] = t[ 0 ];
                P0[ 1 ][ 3 ] = t[ 1 ];
                P0[ 2 ][ 3 ] = t[ 2 ];
                P0[ 3 ][ 3 ] = ( T )1;
                pmats[ c ] = P0;

                tmp = P0 * truePoint;
                proj[ 0 ] = tmp[ 0 ] / tmp[ 2 ];
                proj[ 1 ] = tmp[ 1 ] / tmp[ 2 ];

                proj[ 0 ] += Math::rand( ( T )-0.2, ( T )0.2 );
                proj[ 1 ] += Math::rand( ( T )-0.2, ( T )0.2 );

                ipts[ c ] = proj;
            }

            Vision::triangulate( tmp, &pmats[ 0 ], &ipts[ 0 ], ncams );

            // normalize
            tmp *= ( T )1 / tmp[ 3 ];

            bool b = ( ( tmp - truePoint ).length() < 3 );
            ret &= b;

            if( !b ){
                std::cout << "Ground Truth point:\t\t" << truePoint << std::endl;
                std::cout << "Estimated \t\t: " << tmp << std::endl;
                std::cout << "Distance \t\t: " << ( tmp - truePoint ).length()  << std::endl;
            }
        }

        return ret;
    }

    template <typename T>
    static bool _triangulate()
    {
        bool ret = true;

        Matrix3<T> K( 600.0,   0.0, 320.0,
                        0.0, 600.0, 240.0,
                        0.0,   0.0,   1.0 );

        Matrix4<T> T0;
        T0.setIdentity();
        Matrix3<T> fund;

        std::vector<Matrix4<T> > pmats( 2 );
        std::vector<Vector2<T> > ipts( 2 );

        for( size_t i = 0; i < 100; i++ ){
            Matrix4<T> T1;
            T1.setRotationXYZ( Math::rand( ( T )-Math::PI/12.0, ( T )Math::PI/12.0 ),
                               Math::rand( ( T )-Math::PI/12.0, ( T )Math::PI/12.0 ),
                               Math::rand( ( T )-Math::PI/12.0, ( T )Math::PI/12.0 ));
            T1.setTranslation( Math::rand( ( T )-0.5, ( T )0.5 ),
                               Math::rand( ( T )-0.5, ( T )0.5 ),
                               Math::rand( ( T )-0.5, ( T )0.5 ) );
            T1[ 3 ][ 3 ] = ( T )1;


            Vision::composeFundamental( fund, K, T0, K, T1 );

            Vector4<T> truePoint( Math::rand( ( T )-50, ( T )50 ),
                                  Math::rand( ( T )-50, ( T )50 ),
                                  Math::rand( ( T )5, ( T )50 ),
                                  ( T )1 );

            Matrix3<T> R = T1.toMatrix3();
            Matrix4<T> P0( K ), P1( K*R );
            Vector3<T> t( T1[ 0 ][ 3 ], T1[ 1 ][ 3 ], T1[ 2 ][ 3 ] );

            t = K * t;
            P1[ 0 ][ 3 ] = t[ 0 ];
            P1[ 1 ][ 3 ] = t[ 1 ];
            P1[ 2 ][ 3 ] = t[ 2 ];
            P1[ 3 ][ 3 ] = ( T )1;
            P0[ 3 ][ 3 ] = ( T )1;

            pmats[ 0 ] = P0;
            pmats[ 1 ] = P1;

            Vector4<T> tmp;
            Vector2<T> proj0, proj1;

            tmp = P0 * truePoint;
            proj0[ 0 ] = tmp[ 0 ] / tmp[ 2 ];
            proj0[ 1 ] = tmp[ 1 ] / tmp[ 2 ];
            tmp = P1 * truePoint;
            proj1[ 0 ] = tmp[ 0 ] / tmp[ 2 ];
            proj1[ 1 ] = tmp[ 1 ] / tmp[ 2 ];

            proj0[ 0 ] += Math::rand( ( T )-0.2, ( T )0.2 );
            proj0[ 1 ] += Math::rand( ( T )-0.2, ( T )0.2 );
            proj1[ 0 ] += Math::rand( ( T )-0.2, ( T )0.2 );
            proj1[ 1 ] += Math::rand( ( T )-0.2, ( T )0.2 );

            Vision::correctCorrespondencesSampson( proj0, proj1, fund );

            ipts[ 0 ] = proj0;
            ipts[ 1 ] = proj1;
            //Vision::triangulate( tmp, P0, P1, proj0, proj1 );
            Vision::triangulate( tmp, &pmats[ 0 ], &ipts[ 0 ], 2 );

            // normalize
            tmp *= ( T )1 / tmp[ 3 ];

            bool b = ( ( tmp - truePoint ).length() < 3 );
            ret &= b;

            if( !b ){
                std::cout << "Ground Truth point:\t\t" << truePoint << std::endl;
                std::cout << "Estimated \t\t: " << tmp << std::endl;
                std::cout << "Distance \t\t: " << ( tmp - truePoint ).length()  << std::endl;
            }
        }

        return ret;
    }

	template <typename T>
	static bool _poseFromHomography()
	{
		Matrix3<T> K( 600.0,   0.0, 320.0,
					 0.0, 600.0, 240.0,
					 0.0,   0.0,   1.0 );

		for( size_t i = 0; i < 100; i++ ){
			Matrix4<T> T1, T2;
			T1.setRotationXYZ( Math::rand( ( T )-Math::PI/4.0, ( T )Math::PI/4.0 ),
							  Math::rand( ( T )-Math::PI/4.0, ( T )Math::PI/4.0 ),
							  Math::rand( ( T )-Math::PI/4.0, ( T )Math::PI/4.0 ));
			T1.setTranslation( Math::rand( ( T )-5, ( T )5 ),
							  Math::rand( ( T )-5, ( T )5 ),
							  Math::rand( ( T )0.1, ( T )5 ) );

			Matrix3<T> RT = T1.toMatrix3();
			RT[ 0 ][ 2 ] = T1[ 0 ][ 3 ];
			RT[ 1 ][ 2 ] = T1[ 1 ][ 3 ];
			RT[ 2 ][ 2 ] = T1[ 2 ][ 3 ];

			Matrix3<T> H = K * RT;
			H *= ( T )1.0 / H[ 2 ][ 2 ];

			Vision::poseFromHomography( T2, K, H );

			T1 -= T2;
			T sum = 0;
			for( int y = 0; y < 4; y++ )
				for( int x = 0; x < 4; x++ )
					sum += T1[ y ][ x ];
			sum /= ( T ) 16;
			if( sum > (T)1e-3 ){
				std::cout << T1 << std::endl;
				return false;
			}
		}
		return true;
	}

	template<typename T>
	static bool _planeSweepHomography()
	{

		Matrix3<T> H25(
					   0.96062,     0.013129,       313.42,
					   -0.025338,       1.0067,      -32.779,
					   -2.4123e-05,   6.9148e-06,       1.0251 );


		Matrix3<T> H75(
					   0.96062,     0.013129,       257.86,
					   -0.025338,       1.0067,      -26.737,
					   -2.4123e-05,   6.9148e-06,       1.0269 );


		Matrix3<T> H100(
						0.96062,     0.013129,       250.92,
						-0.025338,       1.0067,      -25.982,
						-2.4123e-05,   6.9148e-06,       1.0271 );


		Matrix3<T> H150(
						0.96062,     0.013129,       243.97,
						-0.025338,       1.0067,      -25.227,
						-2.4123e-05,   6.9148e-06,       1.0273 );


		Matrix4<T> T0( 9.9776885e-01,  -1.5470000e-02,  -6.4946200e-02,  8.6385170e-01,
					  1.7084844e-02,   9.9955669e-01,   2.4383000e-02, -2.7568581e-01,
					  6.4540204e-02,  -2.5438193e-02,   9.9759083e-01, -1.0694305e+00,
					  0.00000000	,   0.00000000	 ,  0.00000000	  ,1.00000000 );

		Matrix3<T> K0( 2780.1700000000000728, 0, 1539.25,
					  0, 2773.5399999999999636, 1001.2699999999999818,
					  0, 0, 1 );

		Matrix4<T> T1( 9.9989024e-01,  -1.4662000e-02,   2.1287000e-03,   7.7862946e-02,
					  1.4650766e-02,   9.9987915e-01,   5.2004500e-03,  -2.1781616e-01,
					  -2.2046917e-03,  -5.1686921e-03,   9.9998421e-01,  -1.0034388e+00,
					  0.00000000,	   0.00000000,	   0.00000000,	   1.00000000 );

		Matrix3<T> K1( 2780.1700000000000728, 0, 1539.25,
					  0, 2773.5399999999999636, 1001.2699999999999818,
					  0, 0, 1 );
		Matrix3<T> H;
		Vision::planeSweepHomography<T>( H, K1, T1, K0, T0, Vector3<T>( 0, 0, 1 ), 25.0 );
		bool b = _compare( H25, H,  (T)0.01 );
		Vision::planeSweepHomography<T>( H, K1, T1, K0, T0, Vector3<T>( 0, 0, 1 ), 75.0 );
		b &= _compare( H75, H,  (T)0.01 );
		Vision::planeSweepHomography<T>( H, K1, T1, K0, T0, Vector3<T>( 0, 0, 1 ), 100.0 );
		b &= _compare( H100, H,  (T)0.01 );
		Vision::planeSweepHomography<T>( H, K1, T1, K0, T0, Vector3<T>( 0, 0, 1 ), 150.0 );
		b &= _compare( H150, H,  (T)0.01 );

		return b;
	}

BEGIN_CVTTEST( Vision )
    bool testResult = true;
    bool b;


    b = _essentialTest<float>();
    CVTTEST_PRINT( "decomposeEssential<float>()\t", b );
    testResult &= b;

    b = _essentialTest<double>();
    CVTTEST_PRINT( "decomposeEssential<double>()\t", b );
    testResult &= b;

    b = _triangulate<float>();
    CVTTEST_PRINT( "triangulate<float>()\t", b );
    testResult &= b;
    b = _triangulate<double>();
    CVTTEST_PRINT( "triangulate<double>()\t", b );
    testResult &= b;

    b = _triangulateMultiView<double>();
    CVTTEST_PRINT( "triangulateMultiView<double>()\t", b );
    testResult &= b;

    b = _poseFromHomography<float>();
    CVTTEST_PRINT( "poseFromHomography<float>()\t", b );
    testResult &= b;

    b = _poseFromHomography<double>();
    CVTTEST_PRINT( "poseFromHomography<double>()\t", b );
    testResult &= b;


    b = _planeSweepHomography<float>();
    CVTTEST_PRINT( "planeSweepHomography<float>()\t", b );
    testResult &= b;

    b = _planeSweepHomography<double>();
    CVTTEST_PRINT( "planeSweepHomography<double>()\t", b );
    testResult &= b;


    return testResult;
END_CVTTEST

}
