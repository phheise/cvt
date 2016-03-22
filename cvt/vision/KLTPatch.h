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

#ifndef CVT_KLT_PATCH_H
#define CVT_KLT_PATCH_H

#include <Eigen/Dense>
#include <cvt/vision/ImagePyramid.h>
#include <cvt/gfx/IMapScoped.h>
#include <cvt/util/EigenBridge.h>
#include <cvt/util/CVTAssert.h>
#include <Eigen/StdVector>

namespace cvt
{
    template <size_t pSize, class PoseType>
    class KLTPatch
    {
        public:
            typedef Eigen::Matrix<float, PoseType::NPARAMS, PoseType::NPARAMS> HessType;
            typedef Eigen::Matrix<float, PoseType::NPARAMS, 1>                 JacType;
            typedef Eigen::Matrix<float, 2, PoseType::NPARAMS>                 ScreenJacType;
            static const size_t PatchSize = pSize;

            KLTPatch( size_t octaves = 1 );

            /**
             *	\brief update the internal patch data
             *	\param ptr		ptr to the image
             *	\param stride	stride of the image
             *	\param pos		position of the feature in the image
             * */
            bool            update( IMapScoped<const float> &iMap,
                                    IMapScoped<const float> &gxMap,
                                    IMapScoped<const float> &gyMap,
                                    const Vector2f &pos,
                                    size_t w, size_t h, size_t octave = 0 );

            bool            update( const ImagePyramid & pyrImg,
                                    const ImagePyramid & pyrGx,
                                    const ImagePyramid & pyrGy,
                                    const Vector2f &pos );

            void            currentCenter( Vector2f& center )  const;

            PoseType&       pose()	{ return _pose; }
            Matrix3f        poseMat() const;
            void            initPose( const Vector2f& pos );
            void            initPose( const Matrix3f& mat );

            const float*    pixels( size_t octave = 0 )         const { return _patchDataForScale[ octave ].patch; }
            const float*    transformed( size_t octave = 0 )    const { return _patchDataForScale[ octave ].transformed; }

            const HessType& inverseHessian( size_t octave = 0 ) const { return _patchDataForScale[ octave ].inverseHessian; }
            const JacType*  jacobians( size_t octave = 0 )      const { return _patchDataForScale[ octave ].jac; }

            bool            align( const float *current, size_t currStride,
                                   size_t width, size_t height, size_t maxIters = 2, size_t octave = 0 );

            /* track patch through pyramid */
            bool            align( const ImagePyramid& pyramid, size_t maxIters = 2 );

            /**
             *	\brief	number of scales that are stored with this patch
             * */
            size_t          numScales() const { return _patchDataForScale.size(); }


            static size_t   size() { return pSize; }
            static void     extractPatches( std::vector<KLTPatch<pSize, PoseType>* > & patches,
                                            const std::vector<Vector2f> & positions,
                                            const Image & img,
                                            const Image & gradX,
                                            const Image & gradY );

            /**
			 * \brief extract patches from an image pyramid
             * */
            static void extractPatches( std::vector<KLTPatch<pSize, PoseType>* > & patches,
                                        const std::vector<Vector2f> & positions,
                                        const ImagePyramid & pyramid,
                                        const ImagePyramid & gradX,
                                        const ImagePyramid & gradY );            

            static const Vector2f* patchPoints()    { return &PatchPoints[ 0 ]; }
            static size_t          numPatchPoints() { return PatchPoints.size(); }
            static void resample( float* dst, const IMapScoped<float>& iMap, const Matrix3f& mat, float fill = -1.0f );

            void toImage( Image& img, size_t octave = 0 ) const;

            EIGEN_MAKE_ALIGNED_OPERATOR_NEW
        private:
            /* this stores the transform from the
             * Template space to the image space*/
            PoseType	_pose;

            struct PatchData
            {
                /* the pixel original information */
                float		patch[ pSize * pSize ];

                /* the transformed information */
                float		transformed[ pSize * pSize ];

                HessType	inverseHessian;
                JacType		jac[ pSize * pSize ];

                EIGEN_MAKE_ALIGNED_OPERATOR_NEW
            };

            typedef std::vector<PatchData, Eigen::aligned_allocator<PatchData> > PatchDataVec;
            PatchDataVec	_patchDataForScale;

            KLTPatch( const KLTPatch& );
            KLTPatch& operator= (const KLTPatch& );

            static std::vector<Vector2f> PatchPoints;

            typedef std::vector<ScreenJacType, Eigen::aligned_allocator<ScreenJacType> > ScreenJacVec;
            static ScreenJacVec ScreenJacobiansAtIdentity;

            static std::vector<Vector2f> initPatchPoints()
            {
                std::vector<Vector2f> points;
                points.reserve( pSize * pSize );

                int half = pSize >> 1;
                Vector2f p( -half, -half );
                for( size_t rows = 0; rows < pSize; rows++ ){
                    p.x = -half;
                    for( size_t cols = 0; cols < pSize; cols++ ){
                        points.push_back( p );
                        p.x += 1.0f;
                    }
                    p.y += 1.0f;
                }
                return points;
            }

            static ScreenJacVec initScreenJacobians()
            {
                std::vector<Vector2f> points = initPatchPoints();
                PoseType pose;

                ScreenJacVec jacobians;
                Eigen::Vector2f p;
                jacobians.resize( points.size() );
                for( size_t i = 0; i < points.size(); ++i ){
                    EigenBridge::toEigen( p, points[ i ] );
                    pose.screenJacobian( jacobians[ i ], p );
                }
                return jacobians;
            }

            bool patchIsInImage( const Matrix3f& pose, size_t w, size_t h ) const;
            float buildSystem( JacType& jacSum, const JacType *J, const float *r );
    };

    template <size_t pSize, class PoseType>
    std::vector<Vector2f> KLTPatch<pSize, PoseType>::PatchPoints( KLTPatch<pSize, PoseType>::initPatchPoints() );

    template <size_t pSize, class PoseType>
    typename KLTPatch<pSize, PoseType>::ScreenJacVec KLTPatch<pSize, PoseType>::ScreenJacobiansAtIdentity( KLTPatch<pSize, PoseType>::initScreenJacobians() );

    template <size_t pSize, class PoseType>
    void KLTPatch<pSize, PoseType>::resample( float* dst, const IMapScoped<float>& iMap, const Matrix3f& mat, float fill )
    {
        size_t n = numPatchPoints();
        std::vector<Vector2f> warpedPts( n );
        SIMD* simd = SIMD::instance();
        simd->transformPoints( &warpedPts[ 0 ], mat, patchPoints(), warpedPts.size() );
        simd->warpBilinear1f( dst, &warpedPts[ 0 ].x, iMap.ptr(), iMap.stride(), iMap.width(), iMap.height(), fill, n );
    }

    template <size_t pSize, class PoseType>
    inline KLTPatch<pSize, PoseType>::KLTPatch( size_t octaves )
    {
        _patchDataForScale.resize( octaves );
    }

    template <size_t pSize, class PoseType>
    inline bool KLTPatch<pSize, PoseType>::update( IMapScoped<const float>& iMap,
                                                   IMapScoped<const float>& gxMap,
                                                   IMapScoped<const float>& gyMap,
                                                   const Vector2f& pos, size_t w, size_t h, size_t octave )
    {
        const float pHalf = ( pSize >> 1 );

        if( pos.x < pHalf || ( pos.x > w - pHalf - 1 ) ||
            pos.y < pHalf || ( pos.y > h - pHalf - 1 ) )
            return false;

        size_t stride = iMap.stride() / sizeof( float );
        size_t offset = ( int )( pos.y - pHalf ) * stride + ( int )( pos.x - pHalf );

        // images have same type and type -> same stride, get pointer to first pixel
        const float* iptr = iMap.ptr() + offset;
        const float* gxptr = gxMap.ptr() + offset;
        const float* gyptr = gyMap.ptr() + offset;

        size_t numLines = pSize;

        JacType* J = _patchDataForScale[ octave ].jac;
        HessType& invH = _patchDataForScale[ octave ].inverseHessian;
        float* p = _patchDataForScale[ octave ].patch;
        float* t = _patchDataForScale[ octave ].transformed;

        Eigen::Matrix<float, 2, 1> g;
        HessType hess( HessType::Zero() );

        ScreenJacType* sj = &ScreenJacobiansAtIdentity[ 0 ];
        while( numLines-- ){
            for( size_t i = 0; i < pSize; i++ ){
                *p = *t = iptr[ i ];
                g[ 0 ] = gxptr[ i ];
                g[ 1 ] = gyptr[ i ];

                *J =  sj->transpose() * g;
                hess.noalias() += ( *J ) * J->transpose();
                J++;
                p++;
                t++;
                sj++;
            }
            iptr  += stride;
            gxptr += stride;
            gyptr += stride;

        }

        // initialize the _pose if at upper most octave
        if( octave == 0 )
            initPose( pos );

        float det = hess.determinant();
        if( Math::abs( det ) > 1e-5 ){
            invH = hess.inverse();
            return true;
        }

        return false;
    }

    template <size_t pSize, class PoseType>
    inline bool KLTPatch<pSize, PoseType>::update( const ImagePyramid & pyrImg,
                                                   const ImagePyramid & pyrGx,
                                                   const ImagePyramid & pyrGy,
                                                   const Vector2f &pos )
    {
        Vector2f scalePos = pos;
        for( size_t i = 0; i < pyrImg.octaves(); i++ ){
            IMapScoped<const float> iMap( pyrImg[ i ] );
            IMapScoped<const float> gxMap( pyrGx[ i ] );
            IMapScoped<const float> gyMap( pyrGy[ i ] );

            if( !this->update( iMap, gxMap, gyMap, scalePos, pyrImg[ i ].width(), pyrImg[ i ].height(), i ) )
                return false;
            scalePos *= pyrImg.scaleFactor();
        }
        return true;
    }

    template <size_t pSize, class PoseType>
    inline void KLTPatch<pSize, PoseType>::extractPatches( std::vector<KLTPatch<pSize, PoseType>* > & patches,
                                                           const std::vector<Vector2f> & positions,
                                                           const Image & img,
                                                           const Image& gradX,
                                                           const Image& gradY )
    {
        IMapScoped<const float> iMap( img );
        IMapScoped<const float> gxMap( gradX );
        IMapScoped<const float> gyMap( gradY );

        size_t w = img.width();
        size_t h = img.height();
        size_t phalf = pSize >> 1;

        int x, y;

        KLTPatch<pSize, PoseType>* patch = 0;

        for( size_t i = 0; i < positions.size(); i++ ){
            x = positions[ i ].x;
            y = positions[ i ].y;

            if( x < ( int )phalf || ( x + phalf - 1 ) > w ||
                y < ( int )phalf || ( y + phalf - 2 ) > h )
                continue;

            if( patch == 0 )
                patch = new KLTPatch<pSize, PoseType>();

            if( patch->update( iMap, gxMap, gyMap, positions[ i ] ) ){
                patches.push_back( patch );
                patch = 0;
            }
        }

        if( patch )
            delete patch;
    }

    template <size_t pSize, class PoseType>
    inline void KLTPatch<pSize, PoseType>::extractPatches( std::vector<KLTPatch<pSize, PoseType>* > & patches,
                                                           const std::vector<Vector2f> & positions,
                                                           const ImagePyramid & pyr,
                                                           const ImagePyramid & gradX,
                                                           const ImagePyramid & gradY )
    {
        int x, y;

        KLTPatch<pSize, PoseType>* patch = 0;
        std::vector<float> scales;

        std::vector<IMapScoped<const float>*> iMaps;
        std::vector<IMapScoped<const float>*> gxMaps;
        std::vector<IMapScoped<const float>*> gyMaps;

        scales.resize( pyr.octaves() );

        float scale = 1.0f;
        for( size_t i = 0; i < pyr.octaves(); i++ ){
            iMaps.push_back( new IMapScoped<const float>( pyr[ i ] ) );
            gxMaps.push_back( new IMapScoped<const float>( gradX[ i ] ) );
            gyMaps.push_back( new IMapScoped<const float>( gradY[ i ] ) );
            scales[ i ] = scale;
            scale *= pyr.scaleFactor();
        }

        Vector2f octavePos;
        size_t phalf = pSize >> 1;
        for( size_t i = 0; i < positions.size(); i++ ){

            float fx = positions[ i ].x;
            float fy = positions[ i ].y;

            bool isGood = true;
            if( patch == 0 )
                patch = new KLTPatch<pSize, PoseType>( pyr.octaves() );
            for( int o = pyr.octaves()-1; o >= 0; o-- ){
                x = scales[ o ] * fx;
                y = scales[ o ] * fy;

                size_t w = pyr[ o ].width();
                size_t h = pyr[ o ].height();

                octavePos = positions[ i ] * scales[ o ];

                if( x < ( int )phalf + 1 || ( x + phalf + 1 ) >= w ||
                    y < ( int )phalf + 1 || ( y + phalf + 1 ) >= h ){
                    isGood = false;
                    break;
                }

                // update octave o of patch
                if( !patch->update( *iMaps[ o ], *gxMaps[ o ], *gyMaps[ o ], octavePos, w, h, o ) ){
                    isGood = false;
                    break;
                }
            }

            if( isGood ){
                patches.push_back( patch );
                patch = 0;
            }
        }

        if( patch )
            delete patch;

        // unmap
        for( size_t i = 0; i < iMaps.size(); i++ ){
            delete iMaps[ i ];
            delete gxMaps[ i ];
            delete gyMaps[ i ];
        }
    }

    template <size_t pSize, class PoseType>
    inline bool KLTPatch<pSize, PoseType>::align( const float *current, size_t currStride,
                                                  size_t width, size_t height,
                                                  size_t maxIters, size_t octave )
    {
        JacType jSum( JacType::Zero() );
        typename PoseType::ParameterVectorType delta;

        PatchData& data = _patchDataForScale[ octave ];

        size_t n = numPatchPoints();
        std::vector<Vector2f> warpedPts( n );
        std::vector<float> residuals( n );
        SIMD* simd = SIMD::instance();

        // first test if all points transform into the image
        Matrix3f pose, poseSave;
        EigenBridge::toCVT( pose, _pose.transformation() );
        poseSave = pose;
        if( !patchIsInImage( pose, width, height ) ){
            return false;
        }

        // warp points and interpolate pixel value
        simd->transformPoints( &warpedPts[ 0 ], pose, patchPoints(), n );
        simd->warpBilinear1f( data.transformed, &warpedPts[ 0 ].x, current, currStride, width, height, 2.0f, n );
        simd->Sub( &residuals[ 0 ], data.transformed, data.patch, n );
        float diffSum = buildSystem( jSum, data.jac, &residuals[ 0 ] );

        size_t iter = 0;
        while( iter < maxIters ){
            // solve for the delta:
            delta = data.inverseHessian * jSum;

            float newError = diffSum + 1.0f;
            while( newError > diffSum ){
                if( Math::abs( delta.array().maxCoeff() ) < 1e-6 ){
                    return true;
                }

                // set the backuped pose
                EigenBridge::toEigen( _pose.transformation(), poseSave );

                // apply pose change
                _pose.applyInverse( -delta );

                // evaluate the new costs
                EigenBridge::toCVT( pose, _pose.transformation() );

                if( patchIsInImage( pose, width, height ) ){
                    simd->transformPoints( &warpedPts[ 0 ], pose, patchPoints(), n );
                    simd->warpBilinear1f( data.transformed, &warpedPts[ 0 ].x, current, currStride, width, height, 2.0f, n );
                    simd->Sub( &residuals[ 0 ], data.transformed, data.patch, n );
                    newError = simd->sumSqr( &residuals[ 0 ], n );
                }

                // half the step
                delta *= 0.5f;
            }
            poseSave = pose;

            // evaluate the normal equations again
            jSum.setZero();
            diffSum = buildSystem( jSum, data.jac, &residuals[ 0 ] );
            iter++;
        }

        return true;
    }

    /* track patch through pyramid */
    template <size_t pSize, class PoseType>
    inline bool KLTPatch<pSize, PoseType>::align( const ImagePyramid& pyramid, size_t maxIters )
    {
        if( maxIters == 0 )
            return true;

        CVT_ASSERT( pyramid[ 0 ].format() == IFormat::GRAY_FLOAT, "Format must be GRAY_FLOAT!" );

        size_t nOctaves = pyramid.octaves();
        float scale = Math::pow( pyramid.scaleFactor(), nOctaves-1 );
        float invScale = 1.0f / pyramid.scaleFactor();

        // get the pose of the patch
        Matrix3f poseMat;
        EigenBridge::toCVT( poseMat, _pose.transformation() );

        poseMat[ 0 ][ 2 ] *= scale;
        poseMat[ 1 ][ 2 ] *= scale;

        // backup pose
        PoseType tmpPose( _pose );
        _pose.set( poseMat );

        bool ret = false;
        for( int oc = pyramid.octaves() - 1; oc >= 0; --oc ){
            IMapScoped<const float> map( pyramid[ oc ] );
            size_t w = pyramid[ oc ].width();
            size_t h = pyramid[ oc ].height();

            ret = align( map.ptr(), map.stride(), w, h, maxIters, oc );
            if( !ret ){
                _pose.transformation() = tmpPose.transformation();
            }

            if( oc != 0 ){
                // we need to scale up
                EigenBridge::toCVT( poseMat, _pose.transformation() );
                poseMat[ 0 ][ 2 ] *= invScale;
                poseMat[ 1 ][ 2 ] *= invScale;

                _pose.set( poseMat );
                // backup pose if alignment fails
                tmpPose.transformation() = _pose.transformation();
            }
        }
        return ret;
    }


    template <size_t pSize, class PoseType>
    inline float KLTPatch<pSize, PoseType>::buildSystem( JacType& jacSum, const JacType* J, const float* r )
    {
        size_t num = numPatchPoints();
        float error = 0.0f;
        while( num-- ){
            error += Math::sqr( *r );
            jacSum += ( *J *  *r );
            J++;
            r++;
        }
        return error;
    }


    template <size_t pSize, class PoseType>
    inline void KLTPatch<pSize, PoseType>::currentCenter( Vector2f& center ) const
    {
        const Eigen::Matrix3f& tmp = _pose.transformation();
        center.x = tmp( 0, 2 );
        center.y = tmp( 1, 2 );
    }

	template <size_t pSize, class PoseType>
	inline Matrix3f KLTPatch<pSize, PoseType>::poseMat() const
	{
		Matrix3f m;
		EigenBridge::toCVT( m, _pose.transformation() );
		return m;
	}

    template <size_t pSize, class PoseType>
    inline void KLTPatch<pSize, PoseType>::initPose( const Vector2f& pos )
    {
        Matrix3f m;
        m.setIdentity();
        m[ 0 ][ 2 ] = pos.x;
        m[ 1 ][ 2 ] = pos.y;
        _pose.set( m );
    }

	template <size_t pSize, class PoseType>
	inline void KLTPatch<pSize, PoseType>::initPose( const Matrix3f& mat )
	{
		_pose.set( mat );
	}

    template <size_t pSize, class PoseType>
    inline void KLTPatch<pSize, PoseType>::toImage( Image& img, size_t octave ) const
    {
        img.reallocate( pSize, pSize, IFormat::GRAY_FLOAT );
        IMapScoped<uint8_t> map( img );
        int r = pSize;
        const uint8_t* vals = ( const uint8_t* )pixels( octave );
        SIMD* simd = SIMD::instance();
        size_t stride = pSize * sizeof( float );
        while( r-- ){
            simd->Memcpy( map.ptr(), vals, stride );
            vals += stride;
            map++;
        }
    }

    template <size_t pSize, class PoseType>
    bool KLTPatch<pSize, PoseType>::patchIsInImage( const Matrix3f& pose, size_t w, size_t h ) const
    {
        static const float half = pSize >> 1;
        static const Vector2f a( -half, -half );
        static const Vector2f b(  half, -half );
        static const Vector2f c(  half,  half );
        static const Vector2f d( -half,  half );

        Vector2f pWarped;

        pWarped = pose * a;
        if( pWarped.x < 0.0f || pWarped.x >= w ||
            pWarped.y < 0.0f || pWarped.y >= h )
            return false;

        pWarped = pose * b;
        if( pWarped.x < 0.0f || pWarped.x >= w ||
            pWarped.y < 0.0f || pWarped.y >= h )
            return false;

        pWarped = pose * c;
        if( pWarped.x < 0.0f || pWarped.x >= w ||
            pWarped.y < 0.0f || pWarped.y >= h )
            return false;

        pWarped = pose * d;
        if( pWarped.x < 0.0f || pWarped.x >= w ||
            pWarped.y < 0.0f || pWarped.y >= h )
            return false;
        return true;
    }
}

#endif
