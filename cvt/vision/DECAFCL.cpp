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

#include <cvt/vision/DECAFCL.h>
#include <cvt/math/SE3.h>
#include <cvt/util/EigenBridge.h>
#include <cvt/util/Time.h>
#include <cvt/cl/CLMatrix.h>
#include <cvt/gfx/IMapScoped.h>

#include <cvt/cl/kernel/fill.h>
#include <cvt/cl/kernel/DECAF.h>
#include <cvt/cl/kernel/pyr/pyrdownchecked.h>

#include <algorithm>

namespace cvt {

    const char* _CL_CF_DEFINE[] = {
        "-D CLASSIC_L2",
        "-D CLASSIC_HUBER",
        "-D CLASSIC_TUKEY",
        "-D AD_CENSUS_LOCAL"
    };

#define GUMM 1
#define GUMM_PROCNOISE 0.02f

#define REDUCE_LOCAL_SIZE 256

    struct __attribute__ ((packed)) DECAFGN_CL {
        cl_float  cost;
        cl_float  weight;
        cl_float2 grad;
        cl_float2 hessdiag;
        cl_float  jacx[ 6 ];
        cl_float  jacy[ 6 ];

        void buildLS( Eigen::Matrix<float, 6, 6>& A, Eigen::Matrix<float, 6, 1>& b )
        {
            for( int i = 0; i < 6; i++ ) {
                b( i ) = weight * ( grad.x * jacx[ i ] + grad.y * jacy[ i ] );
            }

            for( int i = 0; i < 6; i++ ) {
                for( int k = 0; k < 6; k++ ) {
                    A( i, k ) = weight * ( jacx[ i ] * hessdiag.x * jacx[ k ] + jacy[ i ] * hessdiag.y * jacy[ k ] );
                }
            }
        }

    };

    struct DECAFGNLS_CL {
        // we only store the uppper triangular part
        cl_float cost;
        cl_float weight;
        cl_float b[ 6 ];
        cl_float A[ 6 + 5 + 4 + 3 + 2 + 1 ];

        void toLS( Eigen::Matrix<float, 6, 6>& Aout, Eigen::Matrix<float, 6, 1>& bout )
        {
            int l = 0;
            for( int i = 0; i < 6; i++ ) {
                bout( i ) = b[ i ];
                for( int k = i; k < 6; k++ ) {
                    Aout( i, k ) = A[ l++ ];
                    if( k != i )
                        Aout( k, i ) = Aout( i, k );
                }
            }
        }

        void sumToLS( Eigen::Matrix<float, 6, 6>& Aout, Eigen::Matrix<float, 6, 1>& bout )
        {
            int l = 0;
            for( int i = 0; i < 6; i++ ) {
                bout( i ) += b[ i ];
                for( int k = i; k < 6; k++ ) {
                    Aout( i, k ) += A[ l ];
                    if( k != i )
                        Aout( k, i ) += A[ l ];
                    l++;
                }
            }
        }

    } __attribute__((packed));

    template<typename T, void (T::*m)( Image& dst, const Image& src )>
    class PyrdownFunctor
    {
        public:
            PyrdownFunctor(T& x) : _x( x ) {}
            void operator()( Image& dst, const Image& src ) const { (_x.*m)( dst, src ); }

        private:
            T& _x;
    };

    DECAFCL::DECAFCL( const Matrix3f& K, float focalLength, float baseLine, int levels, DECAF_COSTFUNC cf ) :
        _K( K ),
        _Kinv( K.inverse() ),
        _fB( focalLength * baseLine ),
        _levels( levels ),
        _rgb( 0.5f, levels ),
        _dispVar( 0.5f, levels ),
        _rgbDest( 0.5f, levels ),
        _dispVarDest( 0.5f, levels ),
        _warpBuffer( NULL ),
        _reduceBuffer( NULL ),
        _warpStride( 0 ),
        _warpcl( _DECAF_source, "decaf_warp", String("-cl-denorms-are-zero -cl-mad-enable -cl-fast-relaxed-math ") + _CL_CF_DEFINE[ cf ] ),
        _reducecl( _DECAF_source, "decaf_reduce", "-cl-denorms-are-zero -cl-mad-enable -cl-fast-relaxed-math" ),
        _warpdispvarcl( _DECAF_source, "decaf_warp_disparity_variance", "-cl-denorms-are-zero -cl-mad-enable -cl-fast-relaxed-math" ),
        _fusecl( _DECAF_source, "decaf_fuse_disparity_variance" ),
        _drawwarpcl( _DECAF_source, "decaf_draw_warp" ),
        _fillcl( _fill_source, "fill" ),
        _pyrdowncheckcl( _pyrdownchecked_source, "pyrdownchecked" ),
        _univariancecl( _DECAF_source, "decaf_uniform_variance" ),
        _depthunivarcl( _DECAF_source, "decaf_depth_uniform_variance" ),
        _drawdispcl( _DECAF_source, "decaf_draw_disparity" ),
        _drawvarcl( _DECAF_source, "decaf_draw_variance" ),
        _gumminit( _DECAF_source, "decaf_gumm_init"),
        _gumminitdispvar( _DECAF_source, "decaf_gumm_init_disparity_variance"),
        _gummfuse( _DECAF_source, "decaf_gumm_fuse")
    {
         //testReduce();
    }

    DECAFCL::~DECAFCL()
    {
        delete _warpBuffer;
        delete _reduceBuffer;
    }

    void DECAFCL::init( const Image& rgb, const Image& disparityVariance )
    {
        _rgb.update( rgb, IFormat::RGBA_FLOAT );
#ifdef GUMM
        Image tmp;
        gummInit( tmp, disparityVariance, 1.0f );
        _dispVar.update( tmp, IFormat::RGBA_FLOAT, PyrdownFunctor<DECAFCL, &DECAFCL::pyrDownChecked>( *this ) );
#else
        _dispVar.update( disparityVariance, IFormat::GRAYALPHA_FLOAT, PyrdownFunctor<DECAFCL, &DECAFCL::pyrDownChecked>( *this ) );
#endif

        delete _warpBuffer;
        delete _reduceBuffer;

        _warpStride = _rgb[ 0 ].width() * _rgb[ 0 ].height();
        _warpBuffer = new CLBuffer( sizeof( DECAFGN_CL ) * _rgb[ 0 ].width() * _rgb[ 0 ].height() );
        const int maxOutsize = Math::pad( _rgb[ 0 ].width() * _rgb[ 0 ].height(), REDUCE_LOCAL_SIZE ) / REDUCE_LOCAL_SIZE;
        _reduceBuffer = new CLBuffer( sizeof( DECAFGNLS_CL ) * maxOutsize );
    }

    void DECAFCL::initPropagate( const Image& rgb, const Image& disparityVariance, const Matrix4f& RT )
    {
        if( _rgb[ 0 ].width() != rgb.width() || _rgb[ 0 ].height() != rgb.height() ) {
            delete _warpBuffer;
            delete _reduceBuffer;

            _warpStride = rgb.width() * rgb.height();
            _warpBuffer = new CLBuffer( sizeof( DECAFGN_CL ) * rgb.width() * rgb.height() );
            const int maxOutsize = Math::pad( rgb.width() * rgb.height(), REDUCE_LOCAL_SIZE ) / REDUCE_LOCAL_SIZE;
            _reduceBuffer = new CLBuffer( sizeof( DECAFGNLS_CL ) * maxOutsize );
        }
        _rgb.update( rgb, IFormat::RGBA_FLOAT );

        // forward warp current keyframe disparity
        Image warpimg( _dispVar[ 0 ].width(), _dispVar[ 0 ].height(), IFormat::GRAYALPHA_FLOAT, IALLOCATOR_CL );
        warpDisparityVariance( warpimg, _dispVar[ 0 ], RT );

#ifdef GUMM
        Image tmp, fused;
        gummInitDisparityVariance( tmp, disparityVariance );
        gummFuse( fused, tmp, warpimg, GUMM_PROCNOISE );
        _dispVar.update( fused, IFormat::RGBA_FLOAT, PyrdownFunctor<DECAFCL, &DECAFCL::pyrDownChecked>( *this ) );
#else
        Image fused( _dispVar[ 0 ].width(), _dispVar[ 0 ].height(), IFormat::GRAYALPHA_FLOAT, IALLOCATOR_CL );
        fuse( fused, disparityVariance, warpimg, 0.5f, 0.025f );
        _dispVar.update( fused, PyrdownFunctor<DECAFCL, &DECAFCL::pyrDownChecked>( *this ) );
#endif
    }

    void DECAFCL::alignFuse( Matrix4f& RT, const Image& rgb, const Image& disparityVariance,
                             bool initialGuess, bool noFusion, float* costout, unsigned int* sizeout )
    {
        const int INNER_MAX = 50;
        SE3<float> se3;
        Eigen::Matrix<float, 6, 1> delta, delta2;
        int inner;
        float cost;
        unsigned int size;

        if( initialGuess )
            se3.set( RT );

        _rgbDest.update( rgb, IFormat::RGBA_FLOAT );
        _dispVarDest.update( disparityVariance, IFormat::GRAYALPHA_FLOAT, PyrdownFunctor<DECAFCL, &DECAFCL::pyrDownChecked>( *this ) );

        // iterate of the scale and perform warping followed by the reduction
        // apply the delta from the solved linear system
        for( int l = _levels - 1; l >= 0; l-- ) {
#if 0
            {
                Image tmp( _dispVar[ l ].width(), _dispVar[ l ].height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );
                drawDisparity( tmp, _dispVar[ l ], _fB, 1.0f / 4.0f );
                tmp.save("depth.png");
                getchar();
            }
#endif
            Time t;
            double twarp = 0, treduce = 0, dt;
            int breakcount = 0;
            float alpha = 0.05f;
            float costprev = 1e7f;

            delta.setZero();
            delta2.setZero();
            for( inner = 0; inner < INNER_MAX; inner++ ) {
                EigenBridge::toCVT( RT, se3.transformation() );
                Time it;

                size = warp( RT, l );
                dt = it.elapsedMilliSeconds();
                twarp += dt;

                // std::cout << "Warp " << l << " " << dt << " ms" << std::endl;
#if 0
                {
                    Image tmp( _dispVar[ l ].width(), _dispVar[ l ].height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
                    drawWarp( tmp, RT, l, true );
                    tmp.save("warp.png");
                    _rgbDest[ l ].save("dst.png");
                    getchar();
                }
#endif

                delta2 = delta;
                it.reset();
                cost = reduceSolve( delta, size, alpha );
                dt = it.elapsedMilliSeconds(),
                treduce += dt;
                //std::cout << "Reduce " << l << " " << dt << " ms" << std::endl;
                //std::cout << "Level " << l << " size " << size << std::endl;
                //std::cerr << "Cost("<< inner <<") : " << cost << " alpha: " << alpha << std::endl;
                // momentum
                //delta = delta + 0.25 * ( delta2 - delta);
                //delta = delta + 0.1f * delta2;

                se3.apply( -delta );
                if( delta.norm() < 5e-4f && cost < costprev * 1.1f ) {
                    breakcount++;
                    if( breakcount > 5 )
                        break;
                } else {
                    breakcount = 0;
                }

                if( cost < costprev ) {
                    costprev = cost;
                    alpha = Math::max(  alpha * 0.95f, 0.022f );
                } else {
                    alpha = Math::min(  alpha * 1.1f, 0.25f );
                }
            }
            // std::cout << "-------------------------\nLevel " << l << " " << t.elapsedMilliSeconds() << "ms inner:" << inner << " sum-warp: " << twarp << "ms sum-reduce: " << treduce << "ms\n-------------------------\n" << std::endl;
        }
        EigenBridge::toCVT( RT, se3.transformation() );
#if 0
        {
            Image tmp( _dispVar[ 0 ].width(), _dispVar[ 0 ].height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
            drawWarp( tmp, RT, 0, true );
            tmp.save("warp.png");
            _rgbDest[ 0 ].save("dst.png");
            getchar();
        }
#endif

        // do the fusion
        if( !noFusion  /*&& cost < 0.30f && size > 0.6f * _rgb[ 0 ].width() * _rgb[ 0 ].height()*/ ) {
            Image warpimg( _dispVar[ 0 ].width(), _dispVar[ 0 ].height(), IFormat::GRAYALPHA_FLOAT, IALLOCATOR_CL );
            Matrix4f RTinv = RT.inverse();
            warpDisparityVariance( warpimg, disparityVariance, RTinv );
#ifdef GUMM
            Image fused;
            gummFuse( fused, _dispVar[ 0 ], warpimg, GUMM_PROCNOISE );
            _dispVar.update( fused, IFormat::RGBA_FLOAT, PyrdownFunctor<DECAFCL, &DECAFCL::pyrDownChecked>( *this ) );
#else
            Image fused( _dispVar[ 0 ].width(), _dispVar[ 0 ].height(), IFormat::GRAYALPHA_FLOAT, IALLOCATOR_CL );
            fuse( fused, _dispVar[ 0 ], warpimg, 0.5f, 0.025f );
            _dispVar.update( fused, PyrdownFunctor<DECAFCL, &DECAFCL::pyrDownChecked>( *this ) );
#endif

#if 0
            {
                Image tmp( _dispVar[ 0 ].width(), _dispVar[ 0 ].height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );
                drawDisparity( tmp, fused, _fB, 0.25f );
                tmp.save("decaf_warp_disp.png");
            }
#endif
        }
        if( costout )
            *costout = cost;
        if( sizeout )
            *sizeout = size;
    }

    unsigned int DECAFCL::warp( const Matrix4f& RT, int level )
    {
        Matrix3f diag, K;
        Matrix3f Kinv;
        CLMatrix3f clk, clkinv;
        CLMatrix4f clRT( RT );

        diag.setIdentity();
        float scale = Math::pow( 0.5f, ( float ) level );
        diag[ 0 ][ 0 ] = scale;
        diag[ 1 ][ 1 ] = scale;
        K = diag * _K;
        Kinv = K.inverse();

        clk.set( K );
        clkinv.set( Kinv );

        unsigned int size = 0;
        CLBuffer sizecl( sizeof( cl_uint ) );
        sizecl.write( &size );

        _warpcl.setArg( 0, *_warpBuffer );
        _warpcl.setArg<cl_uint>( 1, _warpStride );
        _warpcl.setArg( 2, sizecl );
        _warpcl.setArg( 3, _rgb[ level ] );
        _warpcl.setArg( 4, _dispVar[ level ] );
        _warpcl.setArg( 5, _rgbDest[ level ] );
        _warpcl.setArg( 6, _dispVarDest[ level ] );
        _warpcl.setArg( 7, sizeof( CLMatrix3f ), &clk );
        _warpcl.setArg( 8, sizeof( CLMatrix3f ), &clkinv );
        _warpcl.setArg( 9, sizeof( CLMatrix4f ), &clRT );
        _warpcl.setArg( 10, _fB);
        _warpcl.run( CLNDRange( Math::pad( _rgb[ level ].width(), 16 ), Math::pad( _rgb[ level ].height(), 16 ) ), CLNDRange( 16, 16 ) );

        sizecl.read( ( void* ) &size );
        //std::cout << "Warp level: " << level << " " << size << std::endl;
        return size;
    }

    float DECAFCL::reduceSolve( Eigen::Matrix<float, 6, 1>& delta, unsigned int size, float alpha )
    {
        Eigen::Matrix<float, 6, 6> A;
        Eigen::Matrix<float, 6, 1> b;
        float cost = 0;
        float weight = 0;
        const int outsize = Math::pad( ( size >> 1 ) + ( size & 1 ), REDUCE_LOCAL_SIZE ) / ( REDUCE_LOCAL_SIZE );

        A.setZero();
        b.setZero();

       _reducecl.setArg( 0, *_reduceBuffer );
       _reducecl.setArg( 1, *_warpBuffer );
       _reducecl.setArg( 2, size );
       _reducecl.setArg<cl_uint>( 3, _warpStride );
       _reducecl.run( CLNDRange( Math::pad( ( size >> 1 ) + ( size & 1 ) , REDUCE_LOCAL_SIZE ) ), CLNDRange( REDUCE_LOCAL_SIZE ) );

       void* ptr = _reduceBuffer->map();
       DECAFGNLS_CL* outptr = ( DECAFGNLS_CL* ) ptr;
       for( int i = 0; i < outsize; i++ ) {
           cost += outptr[ i ].cost;
           weight += outptr[ i ].weight;
           outptr[ i ].sumToLS( A, b );
       }
       _reduceBuffer->unmap( ptr );

       // std::cout << "--------------------- cost: " << cost / weight << " ------------------------" << std::endl;

       // std::cout << "A:\n" << A << std::endl;
       // std::cout << "b:\n" << b << std::endl;

       for( int y = 0; y < 6; y++ )
           A( y, y ) *= ( 1.0f + alpha );

       delta = A.ldlt().solve( b );
       return cost / weight;
    }

    void DECAFCL::warpDisparityVariance( Image& dst, const Image& varDispSrc, const Matrix4f& RT )
    {
        CLMatrix3f clk, clkinv, clkrkinv;
        CLMatrix4f clRT;

        clk.set( _K );
        clkinv.set( _Kinv );
        clRT.set( RT );
        clkrkinv.set( _K * RT.toMatrix3() * _Kinv );

        fill( dst, Vector4f( 0.0f, 0.0f, 0.0f, 0.0f ) );

        _warpdispvarcl.setArg( 0, dst );
        _warpdispvarcl.setArg( 1, varDispSrc );
        _warpdispvarcl.setArg( 2, sizeof( CLMatrix3f ), &clk );
        _warpdispvarcl.setArg( 3, sizeof( CLMatrix3f ), &clkinv );
        _warpdispvarcl.setArg( 4, sizeof( CLMatrix4f ), &clRT );
        _warpdispvarcl.setArg( 5, sizeof( CLMatrix3f ), &clkrkinv );
        _warpdispvarcl.setArg( 6, _fB );
        _warpdispvarcl.run( CLNDRange( Math::pad( varDispSrc.width(), 32 ), Math::pad( varDispSrc.height(), 32 ) ), CLNDRange( 32, 32 ) );
    }

    static inline float tukey( float x, float c = 4.6851f ) {
        if( Math::abs( x ) > c )
            return 0.0f;
        float tmp = 1.0f - Math::sqr( x / c );
        return  Math::sqr( tmp );
    }

    static inline float huber( float x, float c = 1.345f )
    {
        float absx = Math::abs( x );
        if( absx > c )
            return 1.0f;
        return Math::min( c / absx, 10000.0f );
    }

    float DECAFCL::reduceSolveCPU( Eigen::Matrix<float, 6, 1>& delta, unsigned int size )
    {
        Eigen::Matrix<float, 6, 6> A;
        Eigen::Matrix<float, 6, 1> b;
        float cost = 0;
        float weight = 0;

        A.setZero();
        b.setZero();

       void* ptr = _warpBuffer->map();
       float* outptr = ( float* ) ptr;

#if 1
       SIMD* simd = SIMD::instance();
       float* demedian = new float[ size ];
       simd->Memcpy( ( uint8_t* ) demedian, ( uint8_t* ) outptr, sizeof( float ) * size );

//       std::nth_element( demedian, demedian + size / 2, demedian + size );
//       float median = demedian[ size / 2 ];
       float median = 0.0f;

       //float median = simd->medianBinApproximate( outptr, size );
       // std::cout << "approximate median value: " << median << std::endl;
       for( int i = 0; i < size; i++ ) {
           demedian[ i ] = Math::abs( outptr[ i ] - median );
       }
       //float meddev = simd->medianBinApproximate( demedian, size ) * 1.4826f;
       std::nth_element( demedian, demedian + size / 2, demedian + size );
       float meddev = demedian[ size / 2 ] * 1.4826f;

       meddev = Math::max( meddev, 1e-4f );
       // std::cout << "approximate median deviation: " << meddev << std::endl;
       delete[] demedian;
#endif

       //FILE* f = fopen("data.txt","w+");

       for( int i = 0; i < size; i++ ) {
            DECAFGN_CL gn;
            Eigen::Matrix<float, 6, 6> Atmp;
            Eigen::Matrix<float, 6, 1> btmp;

            gn.cost       = outptr[ 0 * _warpStride + i ];
            //gn.weight     = tukey( ( gn.cost - median ) / meddev, 0.5f * 4.6851f );  //outptr[ 1 * _warpStride + i ];
            gn.weight     = huber( ( gn.cost - median ) / meddev, 1.345f );  //outptr[ 1 * _warpStride + i ];
            // gn.weight     = tukey( gn.cost, 0.75f );  //outptr[ 1 * _warpStride + i ];
            gn.grad.x     = outptr[ 2 * _warpStride + i ];
            gn.grad.y     = outptr[ 3 * _warpStride + i ];
            gn.hessdiag.x = outptr[ 4 * _warpStride + i ];
            gn.hessdiag.y = outptr[ 5 * _warpStride + i ];
            for( int k = 0; k < 6; k++ ) {
                gn.jacx[ k ] = outptr[ ( 6 + k ) * _warpStride + i ];
                gn.jacy[ k ] = outptr[ ( 12 + k ) * _warpStride + i ];
            }
            gn.buildLS( Atmp, btmp ); 

            //fprintf(f,"%f %f\n", gn.cost, ( gn.cost - median ) / meddev );

            //std::cout << Atmp << std::endl;
            //std::cout << btmp << std::endl;
            //getchar();

            A += Atmp;
            b += btmp;

            cost += gn.cost * gn.weight;
            weight += gn.weight;
       }
       _warpBuffer->unmap( ptr );
       //std::cout << "A:\n" << A << std::endl;
       //std::cout << "b:\n" << b << std::endl;

       //fclose( f );
       //getchar();

       for( int y = 0; y < 6; y++ )
           A( y, y ) *= ( 1.0f + 0.025 );

       delta = A.ldlt().solve( b );

       return cost / weight;
    }

    void DECAFCL::fuse( Image& varDispDst, const Image& varDispSrc0, const Image& varDispSrc1, float procNoise, float maxDiffRel )
    {
        _fusecl.setArg( 0, varDispDst );
        _fusecl.setArg( 1, varDispSrc0 );
        _fusecl.setArg( 2, varDispSrc1 );
        _fusecl.setArg( 3, procNoise );
        _fusecl.setArg( 4, maxDiffRel );
        _fusecl.run( CLNDRange( Math::pad( varDispDst.width(), 32 ), Math::pad( varDispDst.height(), 32 ) ), CLNDRange( 32, 32 ) );
    }

     void DECAFCL::gummInit( Image& dst, const Image& input, float varianace )
     {
         dst.reallocate( input.width(), input.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
         _gumminit.setArg( 0, dst );
         _gumminit.setArg( 1, input );
         _gumminit.setArg( 2, varianace );
         _gumminit.run( CLNDRange( Math::pad( input.width(), 32 ), Math::pad( input.height(), 32 ) ), CLNDRange( 32, 32 ) );
     }

     void DECAFCL::gummInitDisparityVariance( Image& dst, const Image& input )
     {
         dst.reallocate( input.width(), input.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
         _gumminitdispvar.setArg( 0, dst );
         _gumminitdispvar.setArg( 1, input );
         _gumminitdispvar.run( CLNDRange( Math::pad( input.width(), 32 ), Math::pad( input.height(), 32 ) ), CLNDRange( 32, 32 ) );
     }

     void DECAFCL::gummFuse( Image& dst, const Image& src, const Image& other, float procNoise )
     {
         dst.reallocate( src.width(), src.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
         _gummfuse.setArg( 0, dst );
         _gummfuse.setArg( 1, src );
         _gummfuse.setArg( 2, other );
         _gummfuse.setArg( 3, procNoise );
         _gummfuse.run( CLNDRange( Math::pad( dst.width(), 32 ), Math::pad( dst.height(), 32 ) ), CLNDRange( 32, 32 ) );
     }

    void DECAFCL::drawWarp( Image& dst, const Matrix4f& RT, int level, bool clear )
    {
        Matrix3f diag, K;
        Matrix3f Kinv;
        CLMatrix3f clk, clkinv;
        CLMatrix4f clRT;

        diag.setIdentity();
        float scale = Math::pow( 0.5f, ( float ) level );
        diag[ 0 ][ 0 ] = scale;
        diag[ 1 ][ 1 ] = scale;
        K = diag * _K;
        Kinv = K.inverse();

        clk.set( K );
        clkinv.set( Kinv );
        clRT.set( RT );

        if( clear )
            fill( dst, Vector4f( 0.0f, 0.0f, 0.0f, 1.0f ) );

        _drawwarpcl.setArg( 0, dst );
        _drawwarpcl.setArg( 1, _rgb[ level ] );
        _drawwarpcl.setArg( 2, _dispVar[ level ] );
        _drawwarpcl.setArg( 3, sizeof( CLMatrix3f ), &clk );
        _drawwarpcl.setArg( 4, sizeof( CLMatrix3f ), &clkinv );
        _drawwarpcl.setArg( 5, sizeof( CLMatrix4f ), &clRT );
        _drawwarpcl.setArg( 6, _fB);
        _drawwarpcl.run( CLNDRange( Math::pad( _rgb[ level ].width(), 32 ), Math::pad( _rgb[ level ].height(), 32 ) ), CLNDRange( 32, 32 ) );
    }

    void DECAFCL::fill( Image& output, const Vector4f& value )
    {
        cl_float4 val;
        val.x = value.x;
        val.y = value.y;
        val.z = value.z;
        val.w = value.w;

        _fillcl.setArg( 0, output );
        _fillcl.setArg( 1, val );
        _fillcl.run( CLNDRange( Math::pad( output.width(), 32 ), Math::pad( output.height(), 32 ) ), CLNDRange( 32, 32 ) );
    }

    void DECAFCL::pyrDownChecked( Image& output, const Image& input )
    {
        _pyrdowncheckcl.setArg( 0, output );
        _pyrdowncheckcl.setArg( 1, input );
        _pyrdowncheckcl.run( CLNDRange( Math::pad( output.width(), 32 ), Math::pad( output.height(), 32 ) ), CLNDRange( 32, 32 ) );
    }

    void DECAFCL::uniformDisparityVariance( Image& dst, const Image& src, float variance )
    {
        dst.reallocate( src.width(), src.height(), IFormat::GRAYALPHA_FLOAT, IALLOCATOR_CL );

        _univariancecl.setArg( 0, dst );
        _univariancecl.setArg( 1, src );
        _univariancecl.setArg( 2, variance );
        _univariancecl.run( CLNDRange( Math::pad( dst.width(), 32 ), Math::pad( dst.height(), 32 ) ), CLNDRange( 32, 32 ) );
    }

    void DECAFCL::depthTouniformDisparityVariance( Image& dst, const Image& src, float scale, float variance )
    {
        dst.reallocate( src.width(), src.height(), IFormat::GRAYALPHA_FLOAT, IALLOCATOR_CL );

        _depthunivarcl.setArg( 0, dst );
        _depthunivarcl.setArg( 1, src );
        _depthunivarcl.setArg( 2, _fB );
        _depthunivarcl.setArg( 3, scale );
        _depthunivarcl.setArg( 4, variance );
        _depthunivarcl.run( CLNDRange( Math::pad( dst.width(), 32 ), Math::pad( dst.height(), 32 ) ), CLNDRange( 32, 32 ) );
    }

    void DECAFCL::drawDisparity( Image& dst, const Image& src, float fB, float scale )
    {
        dst.reallocate( src.width(), src.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );

        _drawdispcl.setArg( 0, dst );
        _drawdispcl.setArg( 1, src );
        _drawdispcl.setArg( 2, fB );
        _drawdispcl.setArg( 3, scale );
        _drawdispcl.run( CLNDRange( Math::pad( dst.width(), 32 ), Math::pad( dst.height(), 32 ) ), CLNDRange( 32, 32 ) );
    }

    void DECAFCL::drawDisparityVariance( Image& dst, const Image& src, float scale )
    {
        _drawvarcl.setArg( 0, dst );
        _drawvarcl.setArg( 1, src );
        _drawvarcl.setArg( 2, scale );
        _drawvarcl.run( CLNDRange( Math::pad( dst.width(), 32 ), Math::pad( dst.height(), 32 ) ), CLNDRange( 32, 32 ) );
    }

    void DECAFCL::toScenePoints( ScenePoints& pts ) const
    {
        pts.clear();

        IMapScoped<const float> maprgb( _rgb[ 0 ] );
        IMapScoped<const float> mapdispvar( _dispVar[ 0 ] );

        int channels = _dispVar[ 0 ].channels();

        size_t width = _rgb[ 0 ].width();
        size_t height = _rgb[ 0 ].height();

        for( size_t y = 0; y < height; y++ ) {
            const float* rgbptr = maprgb.ptr();
            const float* dispvarptr = mapdispvar.ptr();
            for( size_t x = 0; x < width; x++ ) {
                if( dispvarptr[ x * channels + 0 ] > 0 && std::isfinite( dispvarptr[ x * channels + 0 ] ) ) {
                    float depth = _fB / dispvarptr[ x * channels + 0 ];
                    Vector3f pt =  _Kinv * ( depth * Vector3f( x, y, 1.0f ) );
                    Vector4f col;
                    col.x = rgbptr[ x * 4 + 0 ];
                    col.y = rgbptr[ x * 4 + 1 ];
                    col.z = rgbptr[ x * 4 + 2 ];
                    col.w = 1.0f;
                    pts.add( pt, col );
                }
            }
            maprgb++;
            mapdispvar++;
        }
    }

    void DECAFCL::testReduce()
    {
        int size = 1200*375;
        int outsize = Math::pad( ( size >> 1 ) + ( size & 1 ), REDUCE_LOCAL_SIZE ) / REDUCE_LOCAL_SIZE;
        Eigen::Matrix<float, 6, 6> A, Acl;
        Eigen::Matrix<float, 6, 1> b, bcl;

        A.setZero();
        b.setZero();

        CLBuffer input( sizeof( DECAFGN_CL ) * size );
        CLBuffer output( sizeof( DECAFGNLS_CL ) * outsize  );

        std::cout << "sizeof( DECAFGN_CL ) " << sizeof( DECAFGN_CL ) << " vs sizeof( float ) * 18 " << sizeof( float ) * 18 << std::endl;
        std::cout << "sizeof( DECAFGNLS_CL ) " << sizeof( DECAFGNLS_CL ) << " vs sizeof( float ) * 29 " << sizeof( float ) * 29 << std::endl;

        Math::srand( time( NULL ) );

        void* ptr = input.map();
        float* fptr = ( float* ) ptr;
        Time t;
        for( int i = 0; i < size; i++ ) {
            DECAFGN_CL gn;
            Eigen::Matrix<float, 6, 6> Atmp;
            Eigen::Matrix<float, 6, 1> btmp;

            gn.cost       = Math::rand( 0.1f, 2.0f );
            gn.weight     = Math::rand( 0.1f, 2.0f );
            gn.grad.x     = Math::rand( -1.0f, 1.0f );
            gn.grad.y     = Math::rand( -1.0f, 1.0f );
            gn.hessdiag.x = Math::rand( -1.0f, 1.0f );
            gn.hessdiag.y = Math::rand( -1.0f, 1.0f );
            for( int k = 0; k < 6; k++ ) {
                gn.jacx[ k ] = Math::rand( -1.0f, 1.0f );
                gn.jacy[ k ] = Math::rand( -1.0f, 1.0f );
            }

            gn.buildLS( Atmp, btmp );

            A += Atmp;
            b += btmp;

            fptr[ 0 * size + i ] = gn.cost;
            fptr[ 1 * size + i ] = gn.weight;
            fptr[ 2 * size + i ] = gn.grad.x;
            fptr[ 3 * size + i ] = gn.grad.y;
            fptr[ 4 * size + i ] = gn.hessdiag.x;
            fptr[ 5 * size + i ] = gn.hessdiag.y;
            for( int k = 0; k < 6; k++ ) {
                fptr[ ( 6 + k ) * size + i ] = gn.jacx[ k ];
                fptr[ ( 12 + k ) * size + i ] = gn.jacy[ k ];
            }
        }
        std::cout << t.elapsedMilliSeconds() << " ms" << std::endl;
        input.unmap( ptr );

        t.reset();
        _reducecl.setArg( 0, output );
        _reducecl.setArg( 1, input );
        _reducecl.setArg( 2, size );
        _reducecl.setArg( 3, size );
        _reducecl.runWait( CLNDRange( Math::pad( ( size >> 1 ) + ( size & 1 ) , REDUCE_LOCAL_SIZE ) ), CLNDRange( REDUCE_LOCAL_SIZE ) );

        ptr = output.map();
        DECAFGNLS_CL* outptr = ( DECAFGNLS_CL* ) ptr;
        Acl.setZero();
        bcl.setZero();
        for( int i = 0; i < outsize; i++ )
            outptr[ i ].sumToLS( Acl, bcl );
        output.unmap( ptr );

        std::cout << t.elapsedMilliSeconds() << " ms" << std::endl;

        std::cout << A << std::endl;
        std::cout << b << std::endl;
        std::cout << Acl << std::endl;
        std::cout << bcl << std::endl;

        const float MAXDELTA = 1e-1f;
        for( int i = 0; i < 6; i++ ) {
            if( Math::abs( b[ i ] - bcl[ i ] ) > MAXDELTA ) {
                std::cout << "reduce est failed (b[ " << i << " ]):" << b[ i ] << " <> " << bcl[  i ] << std::endl;
                return;
            }
            for( int k = 0; k < 6; k++ ) {
                if( Math::abs( A( i, k ) - Acl( i, k ) ) > MAXDELTA ) {
                    std::cout << "reduce est failed (A( " << i << "," << k << " )) : " << A(i,k) << " <>" << Acl(i,k) << std::endl;
                    return;
                }
            }
        }
        std::cout << "Reduce CPU/GPU test successful" << std::endl;
    }
}
