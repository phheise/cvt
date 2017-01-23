/*
   The MIT License (MIT)

   Copyright (c) 2016, BMW Car IT GmbH, Philipp Heise (philipp.heise@bmw.de)

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

#ifndef CL_CLRGBDWARPREDUCE_H
#define CL_CLRGBDWARPREDUCE_H

#include <cvt/cl/CLKernel.h>
#include <cvt/cl/CLMatrix.h>
#include <cvt/math/SE3.h>

#include <cvt/cl/kernel/RGBDWarpReduce.h>

namespace cvt {
    class CLRGBDWarpReduce {
        public:
            CLRGBDWarpReduce();
            ~CLRGBDWarpReduce();

            void reduce( Eigen::Matrix<float, 6, 6>& A, Eigen::Matrix<float, 6, 1>& b,
                         float& cost, float& count,
                         const CLBuffer& warpBuffer, size_t size, const CLMatrix3f& K, float threshold );

            struct RGBDWarpLS {
                cl_float  cost;
                cl_float  count;
                cl_float  b[ 6 ];
                cl_float  A[ 6 + 5 + 4 + 3 + 2 + 1 ];

                void setZero()
                {
                    cost  = 0;
                    count = 0;

                    for( int i = 0; i < 6; i++ )
                        b[ i ] = 0;
                    for( int i = 0; i < 21; i++ )
                        A[ i ] = 0;
                }

                void sumToUpperLS( Eigen::Matrix<float, 6, 6>& Aout, Eigen::Matrix<float, 6, 1>& bout ) const
                {
                    int l = 0;
                    for( int i = 0; i < 6; i++ ) {
                        bout( i ) += b[ i ];
                        for( int k = i; k < 6; k++ ) {
                            Aout( i, k ) += A[ l++ ];
                        }
                    }
                }

            };

            struct RGBDWarpEntry {
                cl_float  cost; // marked as invalid for values < 0
                cl_float3 pt;
                cl_float2 grad;
                cl_float2 hessDiag;

                bool isValid() const
                {
                    return cost >= 0.0;
                }

                void sumToRGBDWarpLS( RGBDWarpLS& warpls, const Matrix3f& K ) const
                {
                    if( !isValid() )
                        return;

                    SE3<float>::ScreenJacType jac;
                    Eigen::Matrix<float, 2, 1> g;
                    Eigen::Matrix<float, 6, 1> jall;
                    Eigen::Matrix<float, 6, 6> hall;

                    warpls.cost  += cost;
                    warpls.count += 1;

                    g( 0 ) = grad.x;
                    g( 1 ) = grad.y;

                    SE3<float>::screenJacobian( jac, Vector3f( pt.x, pt.y, pt.z ), K );
                    jall = jac.transpose() * g;

                    for( int i = 0; i < 6; i++ )
                        warpls.b[ i ] += jall( i );

                    hall = jac.transpose() * Eigen::DiagonalMatrix<float,2>( hessDiag.x, hessDiag.y ) * jac;

                    int l = 0;
                    for( int i = 0; i < 6; i++ ) {
                        for( int k = i; k < 6; k++ )
                            warpls.A[ l++ ] += hall( i, k );
                    }
                }
            };


        private:
            void allocateReduceBufferMinSizeforInput( size_t inputElements );
            void reallocateReduceBuffer( size_t size );

            static const size_t REDUCE_LOCAL_SIZE = 256;

            CLBuffer* _reduceBuffer;
            CLKernel  _rgbdWarpReduce;
    };

    inline CLRGBDWarpReduce::CLRGBDWarpReduce() :
        _reduceBuffer( NULL ),
        _rgbdWarpReduce( _RGBDWarpReduce_source, "rgbdwarp_reduce" )
    {
    }

    inline void CLRGBDWarpReduce::allocateReduceBufferMinSizeforInput( size_t outputSize )
    {
        if( !_reduceBuffer ||  _reduceBuffer->size() < ( sizeof( RGBDWarpLS ) * outputSize  ) )
            reallocateReduceBuffer( outputSize );
    }

    inline void CLRGBDWarpReduce::reallocateReduceBuffer( size_t size )
    {
        if( _reduceBuffer ) {
            if( _reduceBuffer->size() == ( sizeof( RGBDWarpLS ) * size ) )
                return;
            delete _reduceBuffer;
        }
        _reduceBuffer = new CLBuffer( sizeof( RGBDWarpLS ) * size );
    }

    inline CLRGBDWarpReduce::~CLRGBDWarpReduce()
    {
        delete _reduceBuffer;
    }

    inline void CLRGBDWarpReduce::reduce( Eigen::Matrix<float, 6, 6>& A, Eigen::Matrix<float, 6, 1>& b,
                                          float& cost, float& count,
                                          const CLBuffer& warpBuffer, size_t size, const CLMatrix3f& K, float threshold )
    {
        if( size == 0 )
            return;

        A.setZero();
        b.setZero();
        cost = 0;
        count = 0;

        const size_t outsize = Math::pad( ( size >> 1 ) + ( size & 1 ), REDUCE_LOCAL_SIZE ) / ( REDUCE_LOCAL_SIZE );
        allocateReduceBufferMinSizeforInput( outsize );

        _rgbdWarpReduce.setArg( 0, *_reduceBuffer );
        _rgbdWarpReduce.setArg( 1, warpBuffer );
        _rgbdWarpReduce.setArg<cl_uint>( 2, size );
        _rgbdWarpReduce.setArg( 3, sizeof( CLMatrix3f ), &K );
        _rgbdWarpReduce.setArg( 4, threshold );
        _rgbdWarpReduce.run( CLNDRange( Math::pad( ( size >> 1 ) + ( size & 1 ) , REDUCE_LOCAL_SIZE ) ), CLNDRange( REDUCE_LOCAL_SIZE ) );

        const void* ptr = _reduceBuffer->map();
        const RGBDWarpLS* outptr = ( const RGBDWarpLS* ) ptr;
        for( size_t i = 0; i < outsize; i++ ) {
            cost   += outptr[ i ].cost;
            count  += outptr[ i ].count;
            outptr[ i ].sumToUpperLS( A, b );
        }
        _reduceBuffer->unmap( ptr );

        /* mirror upper triangular matrix */
        for( int y = 1; y < 6; y++ ) {
            for( int x = 0; x < y; x++ ) {
                A( y, x ) = A( x, y );
            }
        }
    }

}
#endif
