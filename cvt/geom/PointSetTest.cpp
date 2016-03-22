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

#include <cvt/geom/PointSet.h>

#include <cvt/util/CVTTest.h>
#include <cvt/math/Matrix.h>
#include <cvt/math/Vector.h>
#include <cvt/math/Math.h>

namespace cvt
{
	template <typename T>
	static void generate3dPoints( std::vector<Vector3<T> > & pts, size_t n )
	{
		Vector3<T> p;
		srandom( time( NULL ) );
		while( n-- ){
			p.x = Math::rand( ( T )-1000, ( T )1000 );
			p.y = Math::rand( ( T )-1000, ( T )1000 );
			p.z = Math::rand( ( T )-1000, ( T )1000 );
			pts.push_back( p );
		}
	}

	template <typename T>
	static void generate2dPointSet( PointSet<2, T> & pts, size_t n )
	{
		Vector2<T> p;
		srandom( time( NULL ) );
		while( n-- ){
			p.x = Math::rand( ( T )-1000, ( T )1000 );
			p.y = Math::rand( ( T )-1000, ( T )1000 );
			pts.add( p );
		}
	}
	
	template <typename T>
	static void generate3dPointSet( PointSet<3, T> & set, size_t n )
	{
		Vector3<T> p;
		srandom( time( NULL ) );
		while( n-- ){
			p.x = Math::rand( ( T )-1000, ( T )1000 );
			p.y = Math::rand( ( T )-1000, ( T )1000 );
			p.z = Math::rand( ( T )-1000, ( T )1000 );
			set.add( p );
		}
	}

    template <typename T>
	static void fillPointSets( PointSet<2, T> & p0, PointSet<2, T> & p1, const Matrix3<T> & K, const Matrix4<T> & transform, std::vector<Vector3<T> > & pts )
	{
		Vector3<T> p;
		Vector3<T> pp;
        Vector2<T> p2;


		for( size_t i = 0; i < pts.size(); i++ ){
			p = K * pts[ i ];

            p2 = Vector2<T>( p );
            //p2.x += Math::rand( ( T )-0.1, ( T )0.1 );
            //p2.y += Math::rand( ( T )-0.1, ( T )0.1 );

			p0.add( p2 );


			pp = transform * pts[ i ];
			p = K * pp;

            p2 = Vector2<T>( p );
			//p2.x += Math::rand( ( T )-0.1, ( T )0.1 );
            //p2.y += Math::rand( ( T )-0.1, ( T )0.1 );
			p1.add( p2 );
		}
	}

	template <typename T>
	static bool _alignRigidTest()
	{
		bool ret = true;

		PointSet<3, T> set0, set1;

		generate3dPointSet( set0, 40 );

		srandom( time( NULL ) );

		for( size_t i = 0; i < 100; i++ ){
			set1 = set0;
			Matrix4<T> transformation;
			transformation.setRotationXYZ( Math::deg2Rad( Math::rand( (T)-40, (T)40 ) ), 
										   Math::deg2Rad( Math::rand( (T)-40, (T)40 ) ), 
										   Math::deg2Rad( Math::rand( (T)-40, (T)40 ) ) );

			transformation.setTranslation( Math::rand( (T)-100, (T)100 ),
										   Math::rand( (T)-100, (T)100 ),
										   Math::rand( (T)-100, (T)100 ) );
			set1.transform( transformation );

			Matrix4<T> estimated;
			estimated = set0.alignRigid( set1 );

			bool b =  estimated.isEqual( transformation, (T)0.001 );
			ret &= b;
			if( !b ){
				std::cout << "GT:\n" << transformation << std::endl;
				std::cout << "Estimated:\n" << estimated << std::endl;
			}
		}

		return ret;
	}

    template <typename T>
    static bool _alignSimilarityTest( T epsilon )
    {
        int numtrials = 20;

        while( numtrials-- ){
            PointSet<3, T> ptset;
            generate3dPointSet( ptset, 20 );

            Matrix4<T> m, ms;
            m.setIdentity();
            m[ 0 ][ 3 ] = Math::rand( (T)-100, (T)100 );
            m[ 1 ][ 3 ] = Math::rand( (T)-100, (T)100 );
            m[ 2 ][ 3 ] = Math::rand( (T)-100, (T)100 );

            Quaternion<T> qrot;
            qrot.setRotation( 1.0f, 0.0f, 0.0f, Math::rand( (T)-1.0, (T)1.0 ) );
            m *= qrot.toMatrix4();

            PointSet<3, T> ptset2( ptset );
            ptset2.transform( m );

            ms = ptset.alignSimilarity( ptset2 );
            if( !ms.isEqual( m, epsilon ) ){
                std::cout << "Diff\n" << (m - ms) << std::endl;
                return false;
            }
        }
        return true;
    }

    template <typename T>
    static bool _alignPerspectiveTest( T epsilon )
    {
        int trials = 20;
        while( trials-- ){
            PointSet<2, T> p0;
            generate2dPointSet( p0, 20 );
            PointSet<2, T> p1( p0 );

            Matrix3<T> H;
            T theta = Math::rand( ( T )-Math::PI / 8.0, ( T )-Math::PI / 8.0 );
            T phi   = Math::rand( ( T )-Math::PI / 10.0, ( T )-Math::PI / 10.0 );
            T sx    = Math::rand( ( T )0.8, ( T )1.4 );
            T sy    = Math::rand( ( T )0.8, ( T )1.4 );
            T tx    = Math::rand( ( T )-300, ( T )300 );
            T ty    = Math::rand( ( T )-300, ( T )300 );
            T v0    = Math::rand( ( T )-0.001, ( T )0.001 );
            T v1    = Math::rand( ( T )-0.001, ( T )0.001 );
            H.setHomography( theta, phi, sx, sy, tx, ty, v0, v1 );

            p1.transform( H );

            Matrix3<T> hEst = p0.alignPerspective( p1 );

            if( !hEst.isEqual( H, epsilon ) ){
                std::cout << __FUNCTION__ << "Error:\n" << H - hEst << std::endl;
                return false;
            }
        }
        return true;
    }

    template <typename T>
    static bool _essentialTest( T epsilon )
    {
        PointSet<2, T> p0, p1;
        std::vector<Vector3<T> > pts;

        Matrix4<T> trans;
        trans.setIdentity();

        Vector3<T> t( 10.0, 30.0, 50.0 );

        trans.setRotationXYZ( Math::deg2Rad<T>( 10 ), Math::deg2Rad<T>( 60 ), Math::deg2Rad<T>( 5 ) );
        trans.setTranslation( t.x, t.y, t.z );

        Matrix3<T> K( 500.0,   0.0, 320.0,
                      0.0,   505.0, 240.0,
                      0.0,     0.0,   1.0 );

        generate3dPoints( pts, 100 );
        fillPointSets( p0, p1, K, trans, pts );

        Matrix3<T> E;

        E = p0.essentialMatrix( p1, K );
        E *= 1.0 / E[ 2 ][ 2 ];

        // essential matrix = [t]_x R
        Matrix3<T> skew, Etrue;
        skew.setSkewSymmetric( t );

        Etrue = skew * trans.toMatrix3();
        Etrue *= 1.0 / Etrue[ 2 ][ 2 ];

        bool b = Etrue.isEqual( E, epsilon );
        if( !b ){
            std::cout << "True:\n" << Etrue << std::endl;
            std::cout << "Estimated:\n" << E << std::endl;
        }
        return b;
    }

}

BEGIN_CVTTEST( PointSet )

	bool result = true;

	using namespace cvt;

    bool b;
    
    b = _alignRigidTest<float>();
    CVTTEST_PRINT( "alignRigid<float>(): ", b );
	result &= b;
	b = _alignRigidTest<double>();
    CVTTEST_PRINT( "alignRigid<double>(): ", b );
	result &= b;

    b = _alignPerspectiveTest<float>( 0.01f );
    CVTTEST_PRINT( "alignPerspective<float>(): ", b );
    result &= b;
    b = _alignPerspectiveTest<double>( 0.01 );
    CVTTEST_PRINT( "alignPerspective<double>(): ", b );
    result &= b;

    b = _alignSimilarityTest<float>( 0.01f );
    CVTTEST_PRINT( "alignSimilarityTest<float>(): ", b );
    result &= b;
    b = _alignSimilarityTest<double>( 0.01 );
    CVTTEST_PRINT( "alignSimilarityTest<double>(): ", b );
    result &= b;

    b = _essentialTest<double>( 0.01 );
    CVTTEST_PRINT( "essentialMatrix<double>(): ", b );
	result &= b;

    b = _essentialTest<float>( 0.1f );
    CVTTEST_PRINT( "essentialMatrix<float>(): ", b );
	result &= b;

	return result;
END_CVTTEST
