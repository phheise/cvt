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

#ifndef CVTMATH_H
#define CVTMATH_H

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include <iostream>
#include <limits>

#include <cvt/util/Exception.h>

#include <Eigen/Core>
#include <Eigen/LU>

namespace cvt {
    template<class T> class Vector4;

    /**
      \ingroup Math
      \defgroup Math Math
     */
    namespace Math {

        typedef union {
            uint32_t i;
            float f;
        } _flint32;

        typedef union {
            uint64_t i;
            double d;
        } _flint64;

        static const float  PI	     = 3.14159265358979323846f;
        static const float  TWO_PI   = 2.0f * PI;
        static const float  HALF_PI  = 0.5f * PI;
        static const float  E	     = 2.71828182845904523536f;
        static const float  EPSILONF = FLT_EPSILON;
        static const float  MAXF	 = FLT_MAX;
        static const double EPSILOND = DBL_EPSILON;
        static const double MAXD	 = DBL_MAX;

        template<class T> static inline T max( T x, T y ) { return ( x > y ) ? x : y; }
        template<class T> static inline T min( T x, T y ) { return ( x < y ) ? x : y; }
        template<class T> static inline size_t maxIndex( T x, T y ) { return  ( x > y ) ? 0 : 1; }
        template<class T> static inline size_t minIndex( T x, T y ) { return ( x < y ) ? 0 : 1; }

        /**
         *	Mix two values such that
         *	return = ( 1 - alpha ) * a + b * alpha
         * */
        template<typename T> static inline T mix( T a, T b, float alpha ) { return a + ( b - a ) * alpha; }
        template<typename T> static inline T mix( T a, T b, double alpha ) { return a + ( b - a ) * alpha; }

        template<typename T> static inline T clamp( T v, T min, T max ) { return ( v < min ) ? min : ( v > max ) ? max : v; }

		template<typename T> static inline T smoothstep( T edge0, T edge1, T x )
		{
			T t = clamp<T>( ( x - edge0 ) / ( edge1 - edge0 ), 0, 1);
			return t * t * ( ( ( T ) 3 ) - ( ( T ) 2 ) * t);
		}

		template<typename T> static inline T smoothstep( T x )
		{
			T t = clamp<T>( x, 0, 1 );
			return t * t * ( ( ( T ) 3 ) - ( ( T ) 2 ) * t);
		}

        static inline size_t pad16( size_t v ) { return v & 0xf ?  ( ( v | 0xf ) + 1 ) : v; }
        static inline size_t pad32( size_t v ) { return v & 0x1f ?  ( ( v | 0x1f ) + 1 ) : v; }

        static inline size_t pad( size_t v, size_t multiple )
        {
            size_t r = v % multiple;
            if( !r )
                return v;
            return v + multiple - r;
        }

		static inline uint32_t popcount( uint32_t v )
		{
			/* Bit Twiddling Hacks
´			   By Sean Eron Anderson
			   Standford Graphics */
			v = v - ( ( v >> 1 ) & 0x55555555 );
			v = ( v & 0x33333333 ) + ( ( v >> 2 ) & 0x33333333 );
			return ( ( v + ( ( v >> 4 ) & 0xF0F0F0F ) ) * 0x1010101 ) >> 24;
		}

        template<typename T> static inline T sqr( T v ) { return v * v; }

        template<typename T> static inline bool isNaN( T v ) { return v != v; }
        template<typename T> static inline bool isInf( T v )
        {
            return ( std::numeric_limits<T>::has_infinity &&
                     v == std::numeric_limits<T>::infinity() );
        }

        size_t solveQuadratic( float a, float b, float c, float (&result)[ 2 ] );
        size_t solveQuadratic( double a, double b, double c, double (&result)[ 2 ] );
        void solveQuarticReal( Vector4<double>& realRoots, double a, double b, double c, double d, double e );

        static inline float floatPrev( float f )
        {
            _flint32 u;
            u.f = f;
            u.i -= 1;
            return u.f;
        }

        static inline float floatNext( float f )
        {
            _flint32 u;
            u.f = f;
            u.i += 1;
            return u.f;
        }

        template<typename T>
            static inline T epsilon()
            {
                return 1;
            }

        template<>
            inline float epsilon<float>()
            {
                return EPSILONF;
            }

        template<>
            inline double epsilon<double>()
            {
                return EPSILOND;
            }

        template <typename T>
            static inline T deg2Rad( T a )
            {
                return a * PI / ( T )180;
            }

        template <typename T>
            static inline T rad2Deg( T a )
            {
                return a * ( T )180 / PI;
            }

        static inline float sqrt( float f )
        {
            return ::sqrtf( f );
        }

        static inline double sqrt( double d )
        {
            return ::sqrt( d );
        }

        template <typename T>
            static inline float sqrt( T f )
            {
                return ::sqrtf( ( float )f );
            }

        static inline float cbrt( float f )
        {
            return ::cbrtf( f );
        }

        static inline double cbrt( double d )
        {
            return ::cbrt( d );
        }

        static inline float invSqrt( float f )
        {
            return 1.0f / ::sqrtf( f );
        }

        static inline double invSqrt( double d )
        {
            return 1.0 / ::sqrt( d );
        }

        static inline float abs( float f )
        {
            _flint32 u;
            u.f = f;
            u.i &= 0x7FFFFFFF;
            return u.f;
        }

        static inline double abs( double d )
        {
            _flint64 u;
            u.d = d;
            u.i &= 0x7FFFFFFFFFFFFFFFULL;
            return u.d;
        }

        static inline int abs( int i )
        {
            return ::abs( i );
        }

        static inline float cos( float x )
        {
            return ::cosf( x );
        }

        static inline double cos( double x )
        {
            return ::cos( x );
        }

        static inline float sin( float x )
        {
            return ::sinf( x );
        }

        static inline double sin( double x )
        {
            return ::sin( x );
        }

        static inline float acos( float x )
        {
            return ::acosf( x );
        }

        static inline double acos( double x )
        {
            return ::acos( x );
        }

        static inline float asin( float x )
        {
            return ::asinf( x );
        }

        static inline double asin( double x )
        {
            return ::asin( x );
        }
        /*
           static inline void sincos( float rad, float& sin, float& cos )
           {
           ::sincosf( rad, &sin, &cos );
           }

           static inline void sincos( double rad, double& sin, double& cos )
           {
           ::sincos( rad, &sin, &cos );
           }
         */
        static inline float round( float x )
        {
            return ::roundf( x );
        }

        static inline float round( double x )
        {
            return ::round( x );
        }

        static inline float ceil( float x )
        {
            return ::ceilf( x );
        }

        static inline double ceil( double x )
        {
            return ::ceil( x );
        }

        static inline float floor( float x )
        {
            return ::floorf( x );
        }

        static inline double floor( double x )
        {
            return ::floor( x );
        }

        static inline float exp( float x )
        {
            return ::expf( x );
        }

        static inline double exp( double x )
        {
            return ::exp( x );
        }

        static inline float log( float x )
        {
            return ::logf( x );
        }

        static inline double log( double x )
        {
            return ::log( x );
        }

        static inline float log2( float x )
        {
            return ::log2f( x );
        }

        static inline double log2( double x )
        {
            return ::log2( x );
        }

        static inline float tan( float x )
        {
            return ::tanf( x );
        }

        static inline double tan( double x )
        {
            return ::tan( x );
        }

        static inline float tanh( float x )
        {
            return ::tanhf( x );
        }

        static inline double tanh( double x )
        {
            return ::tanh( x );
        }

        static inline float atan2( float y, float x )
        {
            return ::atan2f( y, x );
        }

        static inline double atan2( double y, double x )
        {
            return ::atan2( y, x );
        }

        static inline double pow( double x, double y )
        {
            return ::pow( x, y );
        }

        static inline float pow( float x, float y )
        {
            return ::powf( x, y );
        }

        static inline void srand( unsigned int seed )
        {
            ::srandom( seed );
        }

        static inline ssize_t rand()
        {
            return ( ssize_t ) ::random();
        }

        static inline int rand( int min, int max )
        {
            return min + ( ( max - min ) * ( ( float ) ::random() / ( ( size_t ) RAND_MAX + 1 )  ) );
        }

        static inline float rand( float min, float max )
        {
            return min + ( ( max - min ) * ( ( float ) ::random() / ( ( float ) RAND_MAX + 1.0f )  ) );
        }

        static inline double rand( double min, double max )
        {
            return min + ( ( max - min ) * ( ( double ) ::random() / ( ( double ) RAND_MAX + 1.0 )  ) );
        }

        static inline float copysign( float dst, float src )
        {
            _flint32 _dst;
            _flint32 _src;

            _dst.f = dst;
            _src.f = src;
            _dst.i = ( _dst.i & ~( 1 << 31 ) ) | ( _src.i & ( 1 << 31 ) );
            return _dst.f;
        }

        static inline double copysign( double dst, double src )
        {
            _flint64 _dst;
            _flint64 _src;

            _dst.d = dst;
            _src.d = src;
            _dst.i = ( _dst.i & ~( ( uint64_t ) 1 << 63 ) ) | ( _src.i & ( ( uint64_t ) 1 << 63 ) );
            return _dst.d;
        }

        template<typename T>
            static inline T sgn( T x )
            {
                return ( x > 0 ) ? 1 : ( ( x < 0 ) ? -1 : 0 );
            }

        template<>
            inline float sgn<float>( float x )
            {
                return copysign( 1.0f, x );
            }

        template<>
            inline double sgn<double>( double x )
            {
                return copysign( 1.0, x );
            }

        static inline float givens( float& c, float& s, float a, float b )
        {
            float t, u;
            if( b == 0 ) {
                c = copysign( 1.0f, a );
                s = 0;
                return abs(a);
            } else if( a == 0) {
                c = 0;
                s = copysign( 1.0f, b );
                return abs(b);
            } else if( abs( b ) > abs( a ) ) {
                t = a / b;
                u = copysign( sqrt( 1.0f + sqr( t ) ), b );
                s = 1.0f / u;
                c = s * t;
                return b * u;
            } else {
                t = b / a;
                u = copysign( sqrt( 1.0f + sqr( t ) ), a );
                c = 1.0f / u;
                s = c * t;
                return a * u;
            }
        }

        static inline double givens( double& c, double& s, double a, double b )
        {
            double t, u;
            if( b == 0 ) {
                c = copysign( 1.0, a );
                s = 0;
                return abs(a);
            } else if( a == 0) {
                c = 0;
                s = copysign( 1.0, b );
                return abs(b);
            } else if( abs( b ) > abs( a ) ) {
                t = a / b;
                u = copysign( sqrt( 1.0 + sqr( t ) ), b );
                s = 1.0 / u;
                c = s * t;
                return b * u;
            } else {
                t = b / a;
                u = copysign( sqrt( 1.0 + sqr( t ) ), a );
                c = 1.0 / u;
                s = c * t;
                return a * u;
            }
        }

        static inline void jacobi( float& c, float& s, float x, float y, float z )
        {
            if( y == 0 ) {
                c = copysign( 1.0f, x );
                s = 0;
            } else {
                float t, u;
                float tau = ( z - x ) / ( 2.0f * y );
                t = copysign( 1.0f / ( abs( tau ) + sqrt( 1.0f + sqr( tau ) ) ) , tau );
                u = sqrt( 1.0f + sqr( t ) );
                c = 1.0f / u;
                s = c * t;
            }
        }


        static inline void jacobi( double& c, double& s, double x, double y, double z )
        {
            if( y == 0 ) {
                c = copysign( 1.0, x );
                s = 0;
            } else {
                double t, u;
                double tau = ( z - x ) / ( 2.0 * y );
                t = copysign( 1.0 / ( abs( tau ) + sqrt( 1.0 + sqr( tau ) ) ) , tau );
                u = sqrt( 1.0 + sqr( t ) );
                c = 1.0 / u;
                s = c * t;
            }
        }

        /*
           Inspired by Jose Fonsecas SSE variant
         */
        static inline float fastLog2( float f )
        {
            _flint32 fl;
            fl.f = f;
            float e = ( float ) (  ( int32_t ) ( ( fl.i & 0x7F800000 ) >> 23 ) - 127 );
            fl.i = ( fl.i & 0x7FFFFF ) | 0x3F800000;
            float p = ( ( ( ( ( ( -3.4436006e-2f ) * fl.f + 3.1821337e-1f ) * fl.f + -1.2315303f ) * fl.f + 2.5988452f ) * fl.f + -3.3241990f ) * fl.f + 3.1157899f );
            return p * ( fl.f - 1.0f ) + e;
        }

        /*
           Same as fastLog2, just scale by 1 / log2( e )
         */
        static inline float fastLog( float f )
        {
            _flint32 fl;
            fl.f = f;
            float e = ( float ) (  ( int32_t ) ( ( fl.i & 0x7F800000 ) >> 23 ) - 127 );
            fl.i = ( fl.i & 0x7FFFFF ) | 0x3F800000;
            float p = ( ( ( ( ( ( -3.4436006e-2f ) * fl.f + 3.1821337e-1f ) * fl.f + -1.2315303f ) * fl.f + 2.5988452f ) * fl.f + -3.3241990f ) * fl.f + 3.1157899f );
            return ( p * ( fl.f - 1.0f ) + e ) * 0.6931471805599453f;
        }


        /*
           The famous fast inverse square root approximation found in the code
           of id-tech 3/4.
         */
        static inline float fastInvSqrtf( float f )
        {
            _flint32 u;
            float fhalf;

            fhalf = 0.5f * f;
            u.f = f;
            u.i = 0x5F3759DF - ( u.i >> 1 );
            f = u.f;
            return f * ( 1.5f - fhalf * f * f );
        }

        /* Round to nearest integer */
        static inline int rint( float f ) {
            int i;
            __asm__ __volatile__ ("fistpl %0" : "=m" (i) : "t" (f) : "st") ;
            return i;
        }

        template<typename T>
            static inline size_t gcd( T x, T y )
            {
                T remainder;

                while ( ( remainder = x % y ) != 0 ) {
                    x = y;
                    y = remainder;
                }
                return y;
            }

        /*
         * Algorithm from Goloub, v. Loan p.573f
         * but with fixed number of pade approximants;
         * 6 approximants give precision 3.39452e-16
         */
        template<class Matrix>
            static inline void exponential( const Matrix & A, Matrix & result, size_t padeApprox = 6 )
            {
                double infNorm = 0.0;

                for( int r = 0; r < A.rows(); r++ ){
                    double rowSum = 0.0;
                    for( int c = 0; c < A.cols(); c++ ){
                        rowSum += A( r, c );
                    }
                    if( rowSum > infNorm )
                        infNorm = rowSum;
                }

                int j = max( 0, 1 + int( log( infNorm ) / log( 2.0 ) ) );

                // tmpA = A * 2^j
                Matrix tmpA = A / ( 1 << j ) ;

                Matrix D = Matrix::Identity();
                Matrix N = Matrix::Identity();
                Matrix X = Matrix::Identity();
                Matrix cX;

                double c = 1.0;
                double s = -1.0;
                size_t q = padeApprox;
                size_t twoq = (padeApprox << 1);

                for( size_t k = 1; k < padeApprox; ++k ){
                    c *= q / ( double )(twoq * k);
                    X = tmpA * X;
                    cX = X*c;
                    N += cX;
                    D += ( s*cX );
                    --q;
                    --twoq;
                    s *= -1;
                }

                result = D.inverse();
                result *= N;

                for( int k = 0; k < j; ++k )
                    result = result * result;
            }


        template<typename T, typename FUNC>
            static inline T lineSearchMinGolden( T a, T b, FUNC f, size_t maxiter = 200, T eps = Math::epsilon<T>() )
            {
                T phi = ( ( T ) -1 + Math::sqrt( ( T ) 5 ) ) / ( T ) 2; // golden ratio 0.61 ...
                T f1, f2, x0, x1, x2, x3;

                x0 = a;
                x3 = b;

                x1 = Math::mix( b, a, phi );
                x2 = Math::mix( a, b, phi );

                f1 = f( x1 );
                f2 = f( x2 );

                for( size_t i = 0; i < maxiter; i++ ) {
                    if( f2 < f1 ) {
                        T n = Math::mix( x1, x3, phi );
                        x0 = x1;
                        x1 = x2;
                        x2 = n;
                        f1 = f2;
                        f2 = f( x2 );
                    } else {
                        T n = Math::mix( x2, x0, phi );
                        x3 = x2;
                        x2 = x1;
                        x1 = n;
                        f2 = f1;
                        f1 = f( x1 );
                    }
                    if( Math::abs( x3 - x0 ) <= eps * ( Math::abs( x1 ) + Math::abs( x2 ) ) )
                        break;
                }

                if (f1 < f2) {
                    return x1;
                } else {
                    return x2;
                }
            }

        template<typename T, typename FUNC>
            static inline T lineSearchMaxGolden( T a, T b, FUNC f, size_t maxiter = 200, T eps = Math::epsilon<T>() )
            {
                T phi = ( ( T ) -1 + Math::sqrt( ( T ) 5 ) ) / ( T ) 2; // golden ratio 0.61 ...
                T f1, f2, x0, x1, x2, x3;

                x0 = a;
                x3 = b;

                x1 = Math::mix( b, a, phi );
                x2 = Math::mix( a, b, phi );

                f1 = f( x1 );
                f2 = f( x2 );

                for( size_t i = 0; i < maxiter; i++ ) {
                    if( f2 > f1 ) {
                        T n = Math::mix( x1, x3, phi );
                        x0 = x1;
                        x1 = x2;
                        x2 = n;
                        f1 = f2;
                        f2 = f( x2 );
                    } else {
                        T n = Math::mix( x2, x0, phi );
                        x3 = x2;
                        x2 = x1;
                        x1 = n;
                        f2 = f1;
                        f1 = f( x1 );
                    }
                    if( Math::abs( x3 - x0 ) <= eps * ( Math::abs( x1 ) + Math::abs( x2 ) ) )
                        break;
                }

                if (f1 > f2) {
                    return x1;
                } else {
                    return x2;
                }
            }


    }
}


#endif
