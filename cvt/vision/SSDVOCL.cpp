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

#include <cvt/vision/SSDVOCL.h>

#include <cvt/math/SE3.h>
#include <cvt/util/EigenBridge.h>
#include <cvt/gfx/GFX.h>
#include <cvt/util/Time.h>

#include <cvt/cl/CLMatrix.h>
#include <cvt/cl/CLLocalSpace.h>

#include <cvt/cl/kernel/SSDVO.h>

namespace cvt {

#define THRESHOLD 0.5f
#define THRESHOLD_STEREO 0.25f
#define DISPARITY_MAX 160.0f
#define DISPARITY_MIN 1.0f

#define KX 16
#define KY 16
#define K1D 256

    template<typename T>
    class Revertable {
        public:
            Revertable() {}
            Revertable( const T& val ) : _value( val ), _valueOld( val ) {}
            ~Revertable() {}

            const T& operator()() const { return _value; }
            T& operator()() { return _value; }

            T& update() { _valueOld = _value; return _value; }
            void revert() { _value = _valueOld; }
        private:
            T _value;
            T _valueOld;
    };

    SSDVOCL::SSDVOCL( const StereoCameraCalibration& stereocalib, int levels, float scale ) :
        _stereocalib( stereocalib ),
        _K( _stereocalib.firstCamera().intrinsics() ),
        _fBfactor( stereocalib.focalLength() * stereocalib.baseLine() /* * 0.98075f */ ),
        _pyrKeyLeft( scale, levels ),
        _pyrLeft( scale, levels ),
        _clentries( NULL ),
        _size( 0 ),
        _features0(sizeof( cl_int2 ) * 50000 ),
        _features1(sizeof( cl_int2 ) * 50000 ),
        _sclIdx( NULL ),
        _rgbdBuffer( NULL ),
        _mergeBuffer( NULL ),
        _briefDesc0( NULL ),
        _briefDesc1( NULL ),
        _stereoMatches( NULL ),
        _stereoMatchesSize( 0 ),
        _debugWarp( NULL ),
        _debugStereo( NULL ),
        _clssdvowarp( _SSDVO_source, "ssdvo_warp" ),
        _clssdvowarp2( _SSDVO_source, "ssdvo_warp2" ),
        _clssdvoinit( _SSDVO_source, "ssdvo_init_entries" ),
        _clssdvotransformentries( _SSDVO_source, "ssdvo_transform_entries" ),
        _clssdvostereomatch( _SSDVO_source, "ssdvo_stereo_match" ),
        _clssdvostereorefine( _SSDVO_source, "ssdvo_refine_stereo" ),
        _clssdvorefinedepth( _SSDVO_source, "ssdvo_refine_depth" )
    {
    }

    void SSDVOCL::updateReference( const Image& left, const Image& right, const Matrix4f* RT )
    {
        _pyrKeyLeft.update( left );

        int featuresize0, featuresize1;
        detectFeatures( featuresize0, featuresize1, left, right );
#if 1
        stereoMatchBRIEF( left, right,
                          _features0, featuresize0,
                          _features1, featuresize1, *_sclIdx, 50 );
#else
        stereoMatch( left, right,
                     _features0, featuresize0,
                     _features1, featuresize1, *_sclIdx, THRESHOLD_STEREO );

#endif

        refineStereoMatches( left, right, *_stereoMatches, _stereoMatchesSize );

        if( _debugStereo )
            debugStereoMatches( left, right,  *_stereoMatches, _stereoMatchesSize );

        // Keep current RGBDEntries and transform them according to the new pose
        if( RT ) {
            mergeInit( *RT, *_stereoMatches, _stereoMatchesSize );
        } else {
            /* use the matches to initialize the RGBDEntries ... */
            // only allocate if size is unsufficient
            clbufferAllocMinSize( &_clentries, _stereoMatchesSize * sizeof( RGBDEntryCL ) );
            initEntries( *_clentries, _K.inverse(), _fBfactor, *_stereoMatches, _stereoMatchesSize );
        }

    }

    SSDVOCL::~SSDVOCL()
    {
        if( _sclIdx ) delete _sclIdx;
        if( _rgbdBuffer ) delete _rgbdBuffer;
        if( _mergeBuffer ) delete _mergeBuffer;
        if( _debugStereo ) delete _debugStereo;
        if( _debugWarp ) delete _debugWarp;
        if( _clentries ) delete _clentries;
    }

    void SSDVOCL::debug( bool warp, bool stereo )
    {
        // toggle warp
        if( warp && !_debugWarp ) {
            _debugWarp = new Image();
        } else if( !warp && _debugWarp ) {
            delete _debugWarp;
            _debugWarp = NULL;
        }

        // toggle stereo
        if( stereo && !_debugStereo ) {
            _debugStereo = new Image();
        } else if( !stereo && _debugStereo ) {
            delete _debugStereo;
            _debugStereo = NULL;
        }
    }

    void SSDVOCL::detectFeatures( int& featuresize0, int& featuresize1, const Image& left, const Image& right )
    {
        // Perform stereo matching with FAST for initialization

        _fastcl.detect( _features0, featuresize0, left, 0.05f, 5 );
        _fastcl.detect( _features1, featuresize1, right, 0.05f, 5 );

        // Sort features1 and build scanline index for matching
        clbufferAllocMinSize( &_sclIdx, sizeof( cl_int2 ) * right.height() );

        void* ptr = _features1.map();
        void* ptr2 = _sclIdx->map();

        FeatureCL::sortPosition( ( FeatureCL* ) ptr, featuresize1 );
        FeatureCL::buildScanlineIndex( ( cl_int2* ) ptr2, right.height(), ( const FeatureCL* ) ptr, featuresize1 );

        _sclIdx->unmap( ptr2 );
        _features1.unmap( ptr );
    }

    void SSDVOCL::align( Matrix4f& RT, const Image& left, const Image& right, bool initialguess, size_t* sizeinliner, float* costout )
    {
        Revertable<SE3<float>> se3;
        Eigen::Matrix<float, 6, 1> delta;
        float cost;
        float L2reg                   = 0.01f;
        const float L2regMin          = 0.001f;
        const float L2regUp           = 4.0f;
        const float L2regDown         = 0.75f;
        const int innerMax            = 50;
        const int breakCountThreshold = 5;

        if( !_clentries || _size == 0 ) return;

        if( initialguess )
            se3.update().set( RT );

        //Time t;

        /* build image pyramid for 'left' image */
        _pyrLeft.update( left );

        //clbufferAllocMinSize( &_rgbdBuffer, sizeof( RGBDJacobianHessCL ) * _size, CL_MEM_READ_ONLY );
        clbufferAllocMinSize( &_rgbdBuffer, sizeof( CLRGBDWarpReduce::RGBDWarpEntry ) * _size, CL_MEM_READ_ONLY );

        size_t N;
        for( int lvl = _pyrLeft.levels() - 1; lvl >= 0; lvl-- ) {
            Matrix4f RTcurrent;
            EigenBridge::toCVT( RTcurrent, se3().transformation() );
            float costprev = 1e10f;
            int breakCount = 0;

            //t.reset();
            for( int inner = 0; inner < innerMax; inner++ ) {
                const float threshold = lvl ? THRESHOLD + lvl * 0.05f : 0.5 * THRESHOLD;
                EigenBridge::toCVT( RT, se3().transformation() );

                //warp( *_rgbdBuffer, lvl, RT, _pyrKeyLeft[ lvl ], _pyrLeft[ lvl ], _K, threshold, lvl < 3 );
                //cost = solveDelta( delta, N, *_rgbdBuffer, _size, L2reg );

                warp2( *_rgbdBuffer, lvl, RT, _pyrKeyLeft[ lvl ], _pyrLeft[ lvl ], _K, threshold, true );
                cost = solveDelta2( delta, N, *_rgbdBuffer, _size, _K, lvl, threshold, L2reg );

                //std::cout << "lvl: " << lvl << " inner: " << inner << " cost: " << cost << " inlier: " << N << std::endl;
                if( costprev * 1.01f < cost ) {
                    breakCount = 0;
                    se3.revert();
                    L2reg *= L2regUp;
                    continue;
                } else {
                    L2reg = Math::max( L2reg * L2regDown, L2regMin );
                }

                se3.update().apply( -delta );
                // TODO: add proper early step out scheme
                if( ( delta.norm() < 5e-4f || ( Math::abs( costprev - cost ) < 1e-4f ) ) ) {
                    breakCount++;
                    if( breakCount >= breakCountThreshold )
                        break;
                } else {
                    breakCount = 0;
                }
                costprev = cost;
            }
       }

        EigenBridge::toCVT( RT, se3().transformation() );
        if( _debugWarp )
            debugWarp( left, RT, _K );

        refineDepth( left, right, RT );

        // std::cerr << "Align took " << t.elapsedMilliSeconds() << std::endl;

        if( sizeinliner )
            *sizeinliner = N;

        if( costout )
            *costout = cost;
    }

    float SSDVOCL::solveDelta( Eigen::Matrix<float, 6, 1>& delta, size_t& numinlier, const CLBuffer& output, size_t size, float L2reg )
    {
        Eigen::Matrix<float, 6, 6> A;
        Eigen::Matrix<float, 6, 1> b;
        float costsum = 0.0f;

        A.setZero();
        b.setZero();

        const void* clptr = output.map();
        const RGBDJacobianHessCL* ptr = ( const RGBDJacobianHessCL* ) clptr;
        numinlier = 0;

        size_t n = size;
        size_t i = 0;
        while( n-- ) {
            if( ptr->valid ) {
                costsum += ptr->cost;
                // triangular upper part of the outer product
                int entry = 0;
                for( int y = 0; y < 6; y++ ) {
                    for( int x = y; x < 6; x++ ) {
                        A( y, x ) += ptr->hess[ entry++ ];
                    }
                    b( y ) += ptr->jac[ y ];
                }
                numinlier++;
            }
            i++;
            ptr++;
        }
        output.unmap( clptr );

        /* mirror upper triangular matrix */
        for( int y = 1; y < 6; y++ ) {
            for( int x = 0; x < y; x++ ) {
                A( y, x ) = A( x, y );
            }
        }

        if( ( ( ( float ) numinlier ) / ( ( float ) size ) ) < 0.02f ) {
            std::cerr << "Not enough inliers: " << ( ( ( float ) numinlier ) / ( ( float ) size ) ) << std::endl;
            delta.setZero();
            return 1e10f;
        }

        for( int y = 0; y < 6; y++ )
            A( y, y ) *= ( 1.0f + L2reg );

        delta = A.ldlt().solve( b );
        return costsum / ( float ) numinlier;
    }

     float SSDVOCL::solveDelta2( Eigen::Matrix<float, 6, 1>& delta, size_t& numinlier, const CLBuffer& output, size_t size, const Matrix3f& K, int lvl, float threshold, float L2reg )
    {
        Eigen::Matrix<float, 6, 6> A;
        Eigen::Matrix<float, 6, 1> b;
        float costsum;
        float count;

        Matrix3f diag, Ktmp;
        CLMatrix3f clk;

        diag.setIdentity();
        float scale = Math::pow( _pyrKeyLeft.scaleFactor(), ( float ) lvl );
        diag[ 0 ][ 0 ] = scale;
        diag[ 1 ][ 1 ] = scale;
        Ktmp = diag * K;
        clk.set( Ktmp );

        _rgbdReduce.reduce( A, b, costsum, count, output, size, clk, threshold );

        numinlier = count;

        if( ( ( ( float ) numinlier ) / ( ( float ) size ) ) < 0.02f ) {
            std::cerr << "Not enough inliers: " << ( ( ( float ) numinlier ) / ( ( float ) size ) ) << std::endl;
            delta.setZero();
            return 1e10f;
        }

        for( int y = 0; y < 6; y++ )
            A( y, y ) *= ( 1.0f + L2reg );

        delta = A.ldlt().solve( b );
        return costsum / count;
    }

    void SSDVOCL::warp( CLBuffer& output, int lvl, const Matrix4f& RT, const Image& frame0, const Image& frame1, const Matrix3f& Kdst, float threshold, int remove )
    {
        Matrix3f diag, K;
        CLMatrix3f clk;
        CLMatrix4f clRT;
        Matrix3f Kinv = _K.inverse();

        diag.setIdentity();
        float scale = Math::pow( _pyrKeyLeft.scaleFactor(), ( float ) lvl );
        diag[ 0 ][ 0 ] = scale;
        diag[ 1 ][ 1 ] = scale;
        K = diag * Kdst;
        clk.set( K );

        clRT.set( RT * Kinv.toMatrix4() );

        _clssdvowarp.setArg( 0, output );
        _clssdvowarp.setArg( 1, *_clentries );
        _clssdvowarp.setArg<cl_uint>( 2, _size );
        _clssdvowarp.setArg( 3, sizeof( CLMatrix3f ), &clk );
        _clssdvowarp.setArg( 4, sizeof( CLMatrix4f ), &clRT );
        _clssdvowarp.setArg( 5, scale );
        _clssdvowarp.setArg( 6, frame0 );
        _clssdvowarp.setArg( 7, frame1 );
        _clssdvowarp.setArg( 8, threshold );
        _clssdvowarp.setArg( 9, remove );
        _clssdvowarp.run( CLNDRange( Math::pad( _size, 256 ), 1 ), CLNDRange( 256, 1 ) );
    }

    void SSDVOCL::warp2( CLBuffer& output, int lvl, const Matrix4f& RT, const Image& frame0, const Image& frame1, const Matrix3f& Kdst, float threshold, int remove )
    {
        Matrix3f diag, K;
        CLMatrix3f clk;
        CLMatrix4f clRT;
        Matrix3f Kinv = _K.inverse();

        float scale = Math::pow( _pyrKeyLeft.scaleFactor(), ( float ) lvl );
        clk.set( Kdst );

        clRT.set( RT * Kinv.toMatrix4() );

        _clssdvowarp2.setArg( 0, output );
        _clssdvowarp2.setArg( 1, *_clentries );
        _clssdvowarp2.setArg<cl_uint>( 2, _size );
        _clssdvowarp2.setArg( 3, sizeof( CLMatrix3f ), &clk );
        _clssdvowarp2.setArg( 4, sizeof( CLMatrix4f ), &clRT );
        _clssdvowarp2.setArg( 5, scale );
        _clssdvowarp2.setArg( 6, frame0 );
        _clssdvowarp2.setArg( 7, frame1 );
        _clssdvowarp2.setArg( 8, threshold );
        _clssdvowarp2.setArg( 9, remove );
        _clssdvowarp2.run( CLNDRange( Math::pad( _size, 256 ), 1 ), CLNDRange( 256, 1 ) );
    }

    void SSDVOCL::mergeInit( const Matrix4f& RT, CLBuffer& stereomatches, int numstereomatches )
    {
        clbufferAllocMinSize( &_mergeBuffer, sizeof( RGBDEntryCL ) * ( _size + numstereomatches ) );

        Matrix3f Kinv = _K.inverse();

        CLMatrix3f clk( _K );
        CLMatrix4f clRT( RT * Kinv.toMatrix4() );

        _clssdvotransformentries.setArg( 0, *_clentries );
        _clssdvotransformentries.setArg<cl_uint>( 1, ( cl_uint ) _size );
        _clssdvotransformentries.setArg( 2, sizeof( CLMatrix3f ), &clk );
        _clssdvotransformentries.setArg( 3, sizeof( CLMatrix4f ), &clRT );
        _clssdvotransformentries.setArg( 4, ( cl_int ) _pyrKeyLeft[ 0 ].width() );
        _clssdvotransformentries.setArg( 5, ( cl_int ) _pyrKeyLeft[ 0 ].height() );
        _clssdvotransformentries.run( CLNDRange( Math::pad( _size, K1D ) ), CLNDRange( K1D ) );

        void* ptr = _clentries->map();
        void* ptrmatch = stereomatches.map();
        void* ptrnew = _mergeBuffer->map();

        /* sort the current entries */
        CmpRGBDEntryPos rgbdentrycmp;
        std::sort( ( RGBDEntryCL* ) ptr, ( ( RGBDEntryCL* ) ptr ) + _size, rgbdentrycmp );

        /* sort the matches */
        CmpMatchPos matchcmp;
        std::sort( ( cl_float3* ) ptrmatch, ( ( cl_float3* ) ptrmatch ) + numstereomatches, matchcmp );

        size_t idx1 = 0;
        size_t idx2 = 0;
        RGBDEntryCL* entry1 = ( RGBDEntryCL* ) ptr;
        cl_float3*   entry2 = ( cl_float3* ) ptrmatch;
        RGBDEntryCL* dst    = ( RGBDEntryCL* ) ptrnew;
        const float MAXWEIGHT = 20.0f;

        while( idx1 < _size && idx2 < numstereomatches ) {
            // skip invalid entries
            if( !entry1[ idx1 ].valid ) {
                idx1++;
            } else if( entry2[ idx2 ].z <= 0 || entry2[ idx2 ].z >= DISPARITY_MAX || entry2[ idx2 ].z < DISPARITY_MIN ) {
                idx2++;
            } else if( ( entry2[ idx2 ].y - entry1[ idx1 ].pt.y ) > 1.0f ||
                      ( Math::abs( entry2[ idx2 ].y - entry1[ idx1 ].pt.y ) <= 1.0f && entry2[ idx2 ].x - entry1[ idx1 ].pt.x > 1.0f ) ) {
                // the next stereo match is far away from entry1[ idx1 ]
                *dst = entry1[ idx1++ ];
                dst->weight = Math::min( dst->weight + 1.0f, MAXWEIGHT );
                dst++;
            } else if( Math::abs( entry2[ idx2 ].y - entry1[ idx1 ].pt.y ) <= 1.0f
                       && Math::abs( entry2[ idx2 ].x - entry1[ idx1 ].pt.x  ) <= 1.0f ) {

                // Calculate 3d point from stereo match
                Vector3f pt( Kinv * Vector3f( entry2[ idx2 ].x, entry2[ idx2 ].y, 1.0f ) );
                // Convert disparity to depth using focalLength * Baseline factor
                float depth = ( _fBfactor / entry2[ idx2 ].z );
                pt = pt * depth;

                // entry and the stereo match are close
                // take entry1 if the thery are close also in terms of the disparity
                if( Math::abs( entry2[ idx2 ].z - ( _fBfactor / entry1[ idx1 ].depth ) ) <= 3.0f ) {
                    // how close are the forwared warped point and the newly detected one
                    float newweight = exp( -( Vector2f( entry2[ idx2 ].x,  entry2[ idx2 ].y ).dot( Vector2f( entry1[ idx1 ].pt.x,  entry1[ idx1 ].pt.y ) ) ) * 2.0f );
                    // how far/close are the disparities away
                    newweight = newweight * 0.25f + 0.75f * exp( -Math::sqr( entry2[ idx2 ].z - ( _fBfactor / entry1[ idx1 ].depth ) ) * 0.5f );

                    float alpha = 1.0f - ( entry1[ idx1 ].weight * newweight ) / ( entry1[ idx1 ].weight * newweight + 1.0f );
                    *dst = entry1[ idx1 ];

#if 0
                    // we merge here by adding the 3d points from both measurements
                    pt = Math::mix( pt, entry1[ idx1 ].depth * ( Kinv * Vector3f( entry1[ idx1 ].pt.x, entry1[ idx1 ].pt.y, 1.0f ) ), alpha );
                    Vector2f pt2 = _K * pt;
                    // merge the 3d points and project back
                    dst->pt.x  = pt2.x;
                    dst->pt.y  = pt2.y;
                    dst->depth = pt.z; //Math::mix( dst->depth, depth, alpha );
#endif
                    // we keep the original detected 2d point
                    // only the disparity is adpated
                    dst->depth = Math::mix( dst->depth, depth, alpha );
                    dst->weight = Math::min( dst->weight * newweight + 1.0f, MAXWEIGHT );
                    dst++;
                } else {
                    // we add the new one
                    dst->valid  = 1;
                    dst->pt.x   = entry2[ idx2 ].x;
                    dst->pt.y   = entry2[ idx2 ].y;
                    dst->depth  = depth;
                    dst->weight = 1.0f;
                    dst++;
                }
                // advance both indices
                idx1++;
                idx2++;
            } else {
                // we add the stereo match
                //Vector3f pt( Kinv * Vector3f( entry2[ idx2 ].x, entry2[ idx2 ].y, 1.0f ) );
                // Convert disparity to depth using focalLength * Baseline factor
                float depth = ( _fBfactor / entry2[ idx2 ].z );
                dst->valid  = 1;
                dst->pt.x   = entry2[ idx2 ].x;
                dst->pt.y   = entry2[ idx2 ].y;
                dst->depth  = depth;
                dst->weight = 1.0f;
                dst++;
                idx2++;
            }
        }

        if( idx1 < _size ) {
            // copy the entries if they are valid
            while( idx1 < _size ) {
                if( entry1[ idx1 ].valid ) {
                    *dst = entry1[ idx1 ];
                    dst->weight = Math::min( dst->weight + 1.0f, MAXWEIGHT );
                    dst++;
                }
                idx1++;
            }
        } else if( idx2 < numstereomatches) {
            // we add the stereo match
            //Vector3f pt( Kinv * Vector3f( entry2[ idx2 ].x, entry2[ idx2 ].y, 1.0f ) );
            // Convert disparity to depth using focalLength * Baseline factor
            float depth = ( _fBfactor / entry2[ idx2 ].z );
            //pt = pt * depth;
            dst->valid  = 1;
            dst->pt.x   = entry2[ idx2 ].x;
            dst->pt.y   = entry2[ idx2 ].y;
            dst->depth  = depth;
            dst->weight = 1.0f;
            dst++;
            idx2++;
        }

        /* update the size */
        _size = dst - ( RGBDEntryCL* ) ptrnew;

        _clentries->unmap( ptr );
        stereomatches.unmap( ptrmatch );
        _mergeBuffer->unmap( ptrnew );

        std::swap( _mergeBuffer, _clentries );
    }

    void SSDVOCL::initEntries( CLBuffer& buf, const Matrix3f& Kinv, float fBfactor, const CLBuffer& stereomatches, int numstereomatches )
    {
        /* setup inverse matrix */
        CLMatrix3f clkinv;
        clkinv.set( Kinv );

        cl_uint clsize = 0;
        CLBuffer size( &clsize, sizeof( cl_uint ) );

        _clssdvoinit.setArg( 0, buf );
        _clssdvoinit.setArg( 1, size );
        _clssdvoinit.setArg( 2, ( int ) ( buf.size() / sizeof( RGBDEntryCL ) ) );
        _clssdvoinit.setArg( 3, sizeof( CLMatrix3f ), &clkinv );
        _clssdvoinit.setArg( 4, fBfactor );
        _clssdvoinit.setArg( 5, stereomatches );
        _clssdvoinit.setArg( 6, numstereomatches );
        _clssdvoinit.run( CLNDRange( Math::pad( numstereomatches, K1D ) ), CLNDRange( K1D ) );

        size.read( &_size );
    }

    void SSDVOCL::stereoMatchBRIEF( const Image& left, const Image& right,
                                    const CLBuffer& features0, int size_features0,
                                    const CLBuffer& features1, int size_features1, const CLBuffer& sclidx1, int threshold )
    {
        clbufferAllocMinSize( &_briefDesc0, sizeof( cl_uint8 ) * size_features0 );
        clbufferAllocMinSize( &_briefDesc1, sizeof( cl_uint8 ) * size_features1 );
        clbufferAllocMinSize( &_stereoMatches, sizeof( cl_float3 ) * size_features0 );

        /* extract brief descriptors */
        _briefcl.extract( *_briefDesc0, _features0, size_features0, left );
        _briefcl.extract( *_briefDesc1, _features1, size_features1, right );

        /* perform stereo matching using the brief descriptors and the scanline index and the features itself*/
        _briefcl.stereoMatch( *_stereoMatches, _stereoMatchesSize,
                             _features0, *_briefDesc0, size_features0,
                             _features1, *_briefDesc1, size_features1, *_sclIdx, threshold );

    }

    void SSDVOCL::stereoMatch( const Image& left, const Image& right,
                               const CLBuffer& features0, int size_features0,
                               const CLBuffer& features1, int size_features1, const CLBuffer& sclidx1, float threshold )
    {
        cl_int matches_size = 0;
        CLBuffer nummatches( &matches_size, sizeof( cl_int ) );

        clbufferAllocMinSize( &_stereoMatches, sizeof( cl_float3 ) * size_features0 );

        _clssdvostereomatch.setArg( 0, *_stereoMatches );
        _clssdvostereomatch.setArg( 1, nummatches );
        _clssdvostereomatch.setArg( 2, ( int ) ( _stereoMatches->size() / sizeof( cl_float3 ) ) );
        _clssdvostereomatch.setArg( 3, left );
        _clssdvostereomatch.setArg( 4, right );
        _clssdvostereomatch.setArg( 5, features0 );
        _clssdvostereomatch.setArg( 6, size_features0 );
        _clssdvostereomatch.setArg( 7, features1 );
        _clssdvostereomatch.setArg( 8, size_features1 );
        _clssdvostereomatch.setArg( 9, sclidx1 );
        _clssdvostereomatch.setArg( 10, threshold );
        _clssdvostereomatch.run( CLNDRange( Math::pad( size_features0, 256 ) ), CLNDRange( 256 ) );

        nummatches.read( &_stereoMatchesSize );
        _stereoMatchesSize = std::min<int>( _stereoMatchesSize, ( _stereoMatches->size() / sizeof( cl_float3 ) ) );
    }

    void SSDVOCL::refineStereoMatches( const Image& left, const Image& right, CLBuffer& stereomatches, int numstereomatches )
    {
        _clssdvostereorefine.setArg( 0, left );
        _clssdvostereorefine.setArg( 1, right );
        _clssdvostereorefine.setArg( 2, stereomatches );
        _clssdvostereorefine.setArg( 3, numstereomatches );
        _clssdvostereorefine.run( CLNDRange( Math::pad( numstereomatches, K1D ) ), CLNDRange( K1D ) );
    }

    void SSDVOCL::refineDepth( const Image& left, const Image& right, const Matrix4f& RT )
    {
        Matrix3f Kinv = _K.inverse();

        CLMatrix3f clK( _K );
        CLMatrix4f clRTKinv( RT * Kinv.toMatrix4() );
        CLMatrix4f clRTinvKinv( RT.inverse() * Kinv.toMatrix4() );

        _clssdvorefinedepth.setArg( 0, *_clentries );
        _clssdvorefinedepth.setArg( 1, _size );
        _clssdvorefinedepth.setArg( 2, sizeof( CLMatrix3f ), &clK );
        _clssdvorefinedepth.setArg( 3, sizeof( CLMatrix4f ), &clRTKinv );
        _clssdvorefinedepth.setArg( 4, sizeof( CLMatrix4f ), &clRTinvKinv );
        _clssdvorefinedepth.setArg( 5, _fBfactor );
        _clssdvorefinedepth.setArg( 6, left );
        _clssdvorefinedepth.setArg( 7, right );
        _clssdvorefinedepth.setArg( 8, THRESHOLD_STEREO );
        _clssdvorefinedepth.run( CLNDRange( Math::pad( _size, K1D ) ), CLNDRange( K1D ) );
    }

    void SSDVOCL::debugStereoMatches( const Image& left, const Image& right, const CLBuffer& stereomatches, int numstereomatches ) const
    {
        if( !_debugStereo )
            return;
        _debugStereo->reallocate( left.width(), left.height() + right.height(), IFormat::RGBA_FLOAT );

        _debugStereo->copyRect( 0, 0, left, left.rect() );
        _debugStereo->copyRect( 0, left.height(), right, right.rect() );

        {
            GFX g( *_debugStereo );

            const void* ptr = stereomatches.map();
            const cl_float3* matches = ( const cl_float3* ) ptr;
            g.setColor( Color::GREEN );
            for( int i = 0; i < numstereomatches; i++ ) {
                g.drawLine( matches[ i ].x, matches[ i ].y, matches[ i ].x - matches[ i ].z, matches[ i ].y + left.height() );
            }

            stereomatches.unmap( ptr );
        }
    }

    void SSDVOCL::debugWarp( const Image& left, const Matrix4f& RT, const Matrix3f& K ) const
    {
        if( !_debugWarp )
            return;

        _debugWarp->assign( left, IALLOCATOR_CL );
        {
            GFX g( *_debugWarp );

            const void* ptr = _clentries->map();
            const RGBDEntryCL* entry = ( const RGBDEntryCL* ) ptr;
            g.setColor( Color::GREEN );
            //            Matrix4f RTKinv = RT * _K.inverse().toMatrix4();
            Matrix3f Kinv = _K.inverse();
            for( size_t i = 0; i < _size; i++ ) {
                if( !entry[ i ].valid ) continue;
                Vector3f pt3d = entry[ i ].depth * ( Kinv * Vector3f( entry[ i ].pt.x, entry[ i ].pt.y, 1.0f ) );
                Vector4f pt = RT * Vector4f( pt3d.x, pt3d.y, pt3d.z, 1.0f );
                Vector3f pt2 = K * Vector3f( pt.x, pt.y, pt.z );
                if( pt2.z > 0.0f ) {
                    g.drawIcon( entry[ i ].pt.x, entry[ i ].pt.y, GFX::ICON_CROSS );
                    g.drawLine( entry[ i ].pt.x, entry[ i ].pt.y, pt2.x / pt2.z, pt2.y / pt2.z );
                }
            }
            _clentries->unmap( ptr );
        }
    }

    void SSDVOCL::toScenePoints( ScenePoints& pts ) const
    {
       pts.clear();

       IMapScoped<const float> map( _pyrKeyLeft[ 0 ] );

       Matrix3f Kinv( _K.inverse() );
       const void* ptr = _clentries->map();
       const RGBDEntryCL* entry = ( const RGBDEntryCL* ) ptr;
       size_t n = _size;
       while( n-- ) {
           if( entry->valid && entry->depth < 20.0f ) {
               Vector3f pt =  Kinv * Vector3f( entry->pt.x * entry->depth, entry->pt.y * entry->depth, entry->depth );
               Vector4f col;
               col.x = map( ( ( int ) ( entry->pt.x + 0.5f ) ), ( int ) ( entry->pt.y + 0.5f ), 0 );
               col.y = map( ( ( int ) ( entry->pt.x + 0.5f ) ), ( int ) ( entry->pt.y + 0.5f ), 1 );
               col.z = map( ( ( int ) ( entry->pt.x + 0.5f ) ), ( int ) ( entry->pt.y + 0.5f ), 2 );
               col.w = 1.0f;
               pts.add( pt, col );
           }
           entry++;
       }
       _clentries->unmap( ptr );
    }
}
