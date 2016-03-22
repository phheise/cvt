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

#include <iostream>

#include <cvt/math/Math.h>
#include <cvt/util/CVTTest.h>
#include <cvt/util/Time.h>

#include <cvt/math/Vector.h>
#include <complex>

namespace cvt {

    namespace Math {

        static const uint8_t _table_invsqrt[ 512 ] =
        {
            106,  105,  104,  104,  103,  102,  102,  101,
            100,  100,   99,   98,   98,   97,   96,   96,
            95,   94,   94,   93,   92,   92,   91,   91,
            90,   89,   89,   88,   87,   87,   86,   86,
            85,   84,   84,   83,   83,   82,   82,   81,
            80,   80,   79,   79,   78,   78,   77,   77,
            76,   75,   75,   74,   74,   73,   73,   72,
            72,   71,   71,   70,   70,   69,   69,   68,
            68,   67,   67,   66,   66,   65,   65,   64,
            64,   63,   63,   62,   62,   61,   61,   60,
            60,   59,   59,   58,   58,   57,   57,   57,
            56,   56,   55,   55,   54,   54,   53,   53,
            52,   52,   52,   51,   51,   50,   50,   49,
            49,   49,   48,   48,   47,   47,   47,   46,
            46,   45,   45,   44,   44,   44,   43,   43,
            42,   42,   42,   41,   41,   41,   40,   40,
            39,   39,   39,   38,   38,   37,   37,   37,
            36,   36,   36,   35,   35,   34,   34,   34,
            33,   33,   33,   32,   32,   32,   31,   31,
            31,   30,   30,   29,   29,   29,   28,   28,
            28,   27,   27,   27,   26,   26,   26,   25,
            25,   25,   24,   24,   24,   23,   23,   23,
            22,   22,   22,   21,   21,   21,   21,   20,
            20,   20,   19,   19,   19,   18,   18,   18,
            17,   17,   17,   17,   16,   16,   16,   15,
            15,   15,   14,   14,   14,   14,   13,   13,
            13,   12,   12,   12,   12,   11,   11,   11,
            10,   10,   10,   10,    9,    9,    9,    8,
            8,    8,    8,    7,    7,    7,    7,    6,
            6,    6,    5,    5,    5,    5,    4,    4,
            4,    4,    3,    3,    3,    3,    2,    2,
            2,    2,    1,    1,    1,    1,    0,    0,
            255,  255,  254,  253,  252,  251,  250,  249,
            248,  247,  246,  245,  244,  243,  242,  241,
            240,  240,  239,  238,  237,  236,  235,  234,
            233,  232,  232,  231,  230,  229,  228,  227,
            226,  226,  225,  224,  223,  222,  222,  221,
            220,  219,  218,  218,  217,  216,  215,  214,
            214,  213,  212,  211,  211,  210,  209,  208,
            208,  207,  206,  205,  205,  204,  203,  202,
            202,  201,  200,  200,  199,  198,  197,  197,
            196,  195,  195,  194,  193,  193,  192,  191,
            191,  190,  189,  189,  188,  187,  187,  186,
            185,  185,  184,  184,  183,  182,  182,  181,
            180,  180,  179,  179,  178,  177,  177,  176,
            176,  175,  174,  174,  173,  173,  172,  171,
            171,  170,  170,  169,  168,  168,  167,  167,
            166,  166,  165,  165,  164,  163,  163,  162,
            162,  161,  161,  160,  160,  159,  159,  158,
            158,  157,  156,  156,  155,  155,  154,  154,
            153,  153,  152,  152,  151,  151,  150,  150,
            149,  149,  148,  148,  147,  147,  146,  146,
            145,  145,  144,  144,  143,  143,  143,  142,
            142,  141,  141,  140,  140,  139,  139,  138,
            138,  137,  137,  137,  136,  136,  135,  135,
            134,  134,  133,  133,  133,  132,  132,  131,
            131,  130,  130,  129,  129,  129,  128,  128,
            127,  127,  127,  126,  126,  125,  125,  124,
            124,  124,  123,  123,  122,  122,  122,  121,
            121,  120,  120,  120,  119,  119,  118,  118,
            118,  117,  117,  116,  116,  116,  115,  115,
            115,  114,  114,  113,  113,  113,  112,  112,
            112,  111,  111,  110,  110,  110,  109,  109,
            109,  108,  108,  108,  107,  107,  106,  106,
        };


        size_t solveQuadratic( float a, float b, float c, float (&result)[2] )
        {
            float d;

            if( a == 0.0f ) {
                if( b == 0.0f )
                    return 0;
                result[ 0 ] = -c / b;
                return 1;
            }

            d = b * b - 4.0f * a * c;

            if( d < 0.0f )
                return 0;

            if( d == 0.0f ) {
                result[ 0 ] = -b / ( 2.0f * a );
                return 1;
            }

            d = sqrt( d );
            result[ 0 ] = ( -b + d ) / ( 2.0f * a );
            result[ 1 ] = ( -b - d ) / ( 2.0f * a );
            return 2;
        }

        size_t solveQuadratic( double a, double b, double c, double (&result)[ 2 ] )
        {
            double d;

            if( a == 0.0 ) {
                if( b == 0.0 )
                    return 0;
                result[ 0 ] = -c / b;
                return 1;
            }

            d = b * b - 4.0 * a * c;

            if( d < 0.0 )
                return 0;

            if( d == 0.0 ) {
                result[ 0 ] = -b / ( 2.0 * a );
                return 1;
            }

            d = sqrt( d );
            result[ 0 ] = ( -b + d ) / ( 2.0 * a );
            result[ 1 ] = ( -b - d ) / ( 2.0 * a );
            return 2;
        }

        void solveQuarticReal( Vector4<double> &realRoots, double a, double b, double c, double d, double e )
        {
            double a_2 = Math::sqr( a );
            double a_3 = a_2 * a;
            double a_4 = a_3 * a;
            double b_2 = Math::sqr( b );
            double b_3 = b_2 * b;
            double b_4 = b_3 * b;

            double alpha = -3.0 * b_2 / ( 8.0 * a_2 ) + c / a;
            double beta  = b_3 / ( 8.0 * a_3 ) - b * c / ( 2.0 * a_2 ) + d / a;
            double gamma = -3.0 * b_4 / ( 256.0 * a_4 ) + b_2 * c / ( 16.0 * a_3 ) - b * d / ( 4.0 * a_2 ) + e / a;

            double alpha_2 = Math::sqr( alpha );
            double alpha_3 = alpha_2 * alpha;

            std::complex<double> P( -alpha_2 /  12.0 - gamma, 0.0 );
            std::complex<double> Q( -alpha_3 / 108.0 + alpha * gamma / 3.0 - Math::pow( beta, 2 ) / 8.0, 0.0 );
            std::complex<double> R = -Q / 2.0 + sqrt( pow( Q, 2.0 ) / 4.0 + pow( P,3.0 )/ 27.0 );

            std::complex<double> U = pow( R, ( 1.0 / 3.0 ) );
            std::complex<double> y;

            if( U.real() == 0 )
                y = -5.0 * alpha / 6.0 - pow( Q,( 1.0/3.0 ) );
            else
                y = -5.0 * alpha / 6.0 - P / ( 3.0 * U ) + U;

            std::complex<double> w = sqrt( alpha + 2.0 * y );

            std::complex<double> temp;

            temp = -b / ( 4.0 * a ) + 0.5 * ( w + sqrt( -( 3.0 * alpha + 2.0 * y + 2.0 * beta / w ) ) );
            realRoots[ 0 ] = temp.real();
            temp = -b / ( 4.0 * a ) + 0.5 * ( w - sqrt( -( 3.0 * alpha + 2.0 * y + 2.0 * beta / w ) ) );
            realRoots[ 1 ] = temp.real();
            temp = -b / ( 4.0 * a ) + 0.5 * (-w + sqrt( -( 3.0 * alpha + 2.0 * y - 2.0 * beta / w ) ) );
            realRoots[ 2 ] = temp.real();
            temp = -b / ( 4.0 * a ) + 0.5 * (-w - sqrt( -( 3.0 * alpha + 2.0 * y - 2.0 * beta / w ) ) );
            realRoots[ 3 ] = temp.real();
        }

#if 0
        /*
           Based on implementation of Ken Turkowski.
           Here with 8-bit lookup-table and standard IEEE floating point format.
           Exponent is halfed and mantissa inverse square root is looked-up in table
           Followed by some Newton iterations.
         */
        float invSqrt( float f )
        {
            float fhalf = 0.5f * f;
            _flint32 u;

            u.f = f;

            u.i = ( ( ( 0xBE000000 - ( u.i & 0x7F800000 ) ) >> 1 ) & 0x7F800000 ) |
                ( ( ( uint32_t ) _table_invsqrt[ ( u.i >> 15 ) & 0x1FF ] ) << 15 );

            f = u.f;

            /*Two Newton iterations*/
            f = f * ( 1.5f - fhalf * f * f );
            f = f * ( 1.5f - fhalf * f * f );
            return f;
        }

        double invSqrt( double d )
        {
            double dhalf = 0.5 * d;
            _flint64 u;

            u.d = d;
            u.i = ( ( ( 0xBFC0000000000000ULL - ( u.i & 0x7FF0000000000000ULL ) ) >> 1 ) & 0x7FF0000000000000ULL ) |
                ( ( ( uint64_t ) _table_invsqrt[ ( u.i >> 44 ) & 0x1FF ] ) << 44 );

            d = u.d;

            /*Three Newton iterations*/
            d = d * ( 1.5 - dhalf * d * d );
            d = d * ( 1.5 - dhalf * d * d );
            d = d * ( 1.5 - dhalf * d * d );
            return d;
        }
#endif


        BEGIN_CVTTEST( math )
            float f;
            double t1, t2, d;
            bool b;
            Time t;

            f = Math::mix( 0.0f, 1.0f, 0.5f );
            CVTTEST_PRINT( "Mix with float", f == 0.5f );
            f = Math::mix( 0.0f, 1.0f, 0.0f );
            CVTTEST_PRINT( "Mix with float", f == 0.0f );
            f = Math::mix( 0.0f, 1.0f, 1.0f );
            CVTTEST_PRINT( "Mix with float", f == 1.0f );

            /* SQRT */
            b = true;
            for( size_t i = 0; i <= 10000; i++ ) {
                 f = Math::rand( 0.0f, Math::MAXF );
                 bool b2 = ( Math::abs( Math::sqrt( f ) - ::sqrtf( f ) ) / ::sqrtf( f ) < 0.001f );
                 if( !b2 ) {
                    std::cerr << "Math::sqrt failed for " << f << " Math::sqrt = " << Math::sqrt( f ) << " sqrtf = " << ::sqrtf( f ) << std::endl;
                 }
                 b &= b2;
            }

            /*CVTTEST_PRINT( "Math::sqrt", b );
            t.reset();
            for( size_t i = 0; i <= 800000; i++ ){
                 f = Math::rand( 0.0f, Math::MAXF );
                 f = Math::invSqrt( f );
            }
            t1 = t.elapsedMilliSeconds();
            t.reset();
            for( size_t i = 0; i <= 800000; i++ ){
                 f = Math::rand( 0.0f, Math::MAXF );
                 f = 1.0f / ::sqrtf( f );
            }
            t2 = t.elapsedMilliSeconds();
            std::cerr << "Math::invSqrt " << t1 << " ms 1.0/::sqrtf " << t2 << " ms" << std::endl;
            CVTTEST_PRINT( "Math::abs performance", t1 < t2 );*/


            t.reset();
            for( size_t i = 0; i <= 800000; i++ ){
                 f = Math::rand( -1e5f, 1e5f );
                 f = Math::abs( f );
            }
            t1 = t.elapsedMilliSeconds();
            t.reset();
            for( size_t i = 0; i <= 800000; i++ ){
                 f = Math::rand( -1e5f, 1e5f );
                 f = ::fabsf( f );
            }
            t2 = t.elapsedMilliSeconds();
            std::cerr << "Math::abs " << t1 << " ms ::fabsf " << t2 << " ms" << std::endl;
            CVTTEST_PRINT( "Math::abs performance", t1 - 0.5f < t2 );

            /* ABS */
            for( size_t i = 0; i <= 20000; i++ ) {
                 f = Math::rand( -1e5f, 1e5f );
                 b &= ( Math::abs( f ) * ::fabsf( f ) > 0.0f );
            }
            CVTTEST_PRINT( "Math::abs", b );

            /* Copysign */
            f = copysign( 1.0f, -123.0f );
            b &= ( f < 0 );
            f = copysign( -1.0f, -123.0f );
            b &= ( f < 0 );
            f = copysign( 1.0f, 123.0f );
            b &= ( f > 0 );
            f = copysign( -1.0f, 123.0f );
            b &= ( f > 0 );
            d = copysign( 1.0, -123.0 );
            b &= ( d < 0 );
            d = copysign( -1.0, -123.0 );
            b &= ( d < 0 );
            d = copysign( 1.0, 123.0 );
            b &= ( d > 0 );
            d = copysign( -1.0, 123.0 );
            b &= ( d > 0 );


            CVTTEST_PRINT( "Math::copysign", b );

            return true;
        END_CVTTEST
    }
}
