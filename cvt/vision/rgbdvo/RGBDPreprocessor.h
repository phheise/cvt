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


#ifndef CVT_RGBDPREPROCESSOR_H
#define CVT_RGBDPREPROCESSOR_H

#include <cvt/gfx/IKernel.h>
#include <cvt/gfx/Image.h>
#include <cvt/gfx/IMapScoped.h>
#include <cvt/vision/ImagePyramid.h>

namespace cvt {
    class RGBDPreprocessor {
        public:
            static RGBDPreprocessor& instance()
            {
                static RGBDPreprocessor _instance;
                return _instance;
            }

            ~RGBDPreprocessor()
            {}

            void gradient( Image& gx, Image& gy, const Image& src ) const
            {
                gx.reallocate( src.width(), src.height(), IFormat::GRAY_FLOAT );
                gy.reallocate( src.width(), src.height(), IFormat::GRAY_FLOAT );
                src.convolve( gx, _dx );
                src.convolve( gy, _dy );
            }

            void gradient( ImagePyramid& gx, ImagePyramid& gy, const ImagePyramid& src ) const
            {
                src.convolve( gx, _dx );
                src.convolve( gy, _dy );
            }

            void depthGradient( Image& gx, Image& gy, const Image& src ) const;

            void setGradientKernels( const IKernel& dx, const IKernel& dy )
            {
                _dx = dx;
                _dy = dy;
            }

            const IKernel& dxKernel() const { return _dx; }
            const IKernel& dyKernel() const { return _dy; }

            // TODO: handle this more nicely: the problem is, that the Image has type float and is normalized between 0.0f-1.0f
            // for uint32_t the max value is 0xFFFF, we want to convert to meters, therefore we need to define the scaling
            // #val pixvals corresponds to 1m --> scale by (uint16_t max) * 1/val
            void setDepthScale( float scale )
            {
                _depthScaling = ( float )( 0xFFFF ) / scale;
                _depthToPixel = 1.0f / _depthScaling;
            }
            void setMinDepth( float minDepth ){ _minDepth = minDepth; }
            void setMaxDepth( float maxDepth ){ _maxDepth = maxDepth; }

            float interpolateDepth( const Vector2f& p, const float* ptr, size_t stride ) const
            {
                // get the fractions:
                int xi = p.x;
                int yi = p.y;

                const float* curr = ptr + yi * stride + xi ;

                float wx = p.x - xi;
                float wy = p.y - yi;

                float z0 = 0.0f;
                if( curr[ 0 ] > 0.0f ){
                    if( curr[ 1 ] > 0.0f ){
                        // both values valid
                        z0 = Math::mix( curr[ 0 ], curr[ 1 ], wx );
                    }  else {
                        z0 = curr[ 0 ];
                    }
                } else {
                    if( curr[ 1 ] ){
                        z0 = curr[ 1 ];
                    } else {
                        // no value valid in this line
                        wy = 1.0f;
                    }
                }

                float z1 = 0.0f;
                if( curr[ stride ] > 0.0f ){
                    if( curr[ stride + 1 ] > 0.0f ){
                        // both values valid
                        z1 = Math::mix( curr[ stride ], curr[ stride + 1 ], wx );
                    }  else {
                        z1 = curr[ stride ];
                    }
                } else {
                    if( curr[ stride + 1 ] > 0.0f ){
                        z1 = curr[ stride + 1 ];
                    } else {
                        // no value valid in this line
                        wy = 0.0f;
                    }
                }
                float zall = depthPixelToZ( Math::mix( z0, z1, wy ) );
                if( zall > _minDepth && zall < _maxDepth )
                    return zall;
                return -1.0f;
            }

            float interpolateDepth( const Vector2f& p, const IMapScoped<const float>& map )
            {
                return interpolateDepth( p, map.ptr(), map.stride() / sizeof( float ) );
            }

            float depthPixelToZ( float pixel ) const { return pixel * _depthScaling; }

            float depthToPixelFactor() const { return _depthToPixel; }
            float pixelToDepthFactor() const { return _depthScaling; }

            void interpolateNeareast( float *dstD, float* dstX, float* dstY,
                                      const Vector2f* pts,
                                      IMapScoped<const float>& mapD,
                                      IMapScoped<const float>& mapX,
                                      IMapScoped<const float>& mapY,
                                      float placeHolder, size_t n  );

            void computeDepthGradient( Vector2f& grad,
                                       const Vector2f& pos,
                                       float val,
                                       const IMapScoped<const float>& depthMap ) const;

            void depthPyramidNearestNeighbor();

        private:
            RGBDPreprocessor() :
                _dx( IKernel::FIVEPOINT_DERIVATIVE_HORIZONTAL ),
                _dy( IKernel::FIVEPOINT_DERIVATIVE_VERTICAL ),
                _depthScaling( 1.0f ),
                _depthToPixel( 1.0f / _depthScaling ),
                _minDepth( 0.05f ),
                _maxDepth( 10.0f )
            {
                float s = -1.0f;
                _dx.scale( s );
                _dy.scale( s );
            }

            /* don't allow copies */
            RGBDPreprocessor( const RGBDPreprocessor& );

            IKernel _dx;
            IKernel _dy;

            /* params for unprojection */
            float   _depthScaling;
            float   _depthToPixel;
            float   _minDepth;
            float   _maxDepth;

    };

    inline void RGBDPreprocessor::depthGradient( Image &grx, Image &gry, const Image &src ) const
    {
        grx.reallocate( src.width(), src.height(), IFormat::GRAY_FLOAT );
        gry.reallocate( src.width(), src.height(), IFormat::GRAY_FLOAT );

        IMapScoped<const float> d( src );
        IMapScoped<float> gX( grx );
        IMapScoped<float> gY( gry );

        size_t hEnd = d.height() - 1; // do not process last line
        size_t xEnd = d.width() - 1;
        float v0, v1, v2;
        for( size_t y = 0; y < hEnd; y++ ){
            const float* dLine = d.ptr();
            d++;
            const float* dLineNext = d.ptr();

            float* gxLine = gX.ptr();
            float* gyLine = gY.ptr();
            for( size_t x = 0; x < xEnd; ++x ){
                float vx = 0.0f;
                float vy = 0.0f;
                if( ( v0 = dLine[ x ] ) > 0 ){
                    if( ( v1 = dLine[ x + 1 ] ) > 0.0f ){
                        vx = v1 - v0;
                    }
                    if( ( v2 = dLineNext[ x ] ) > 0.0f ){
                        vy = v2 - v0;
                    }
                }
                *gxLine++ = vx;
                *gyLine++ = vy;
            }
            *gxLine = 0.0f;

            // next lines
            gX++;
            gY++;
        }

        // no next line
        float* gyLine = gY.ptr();
        for( size_t x = 0; x < xEnd; ++x ){
            gyLine[ x ] = 0.0f;
        }
    }

    inline void RGBDPreprocessor::interpolateNeareast( float* dstD, float* dstX, float* dstY,
                                                       const Vector2f* pts,
                                                       IMapScoped<const float>& mapD,
                                                       IMapScoped<const float>& mapX,
                                                       IMapScoped<const float>& mapY,
                                                       float placeHolder, size_t n )
    {
        size_t w = mapX.width();
        size_t h = mapX.height();
        while( n-- ){
            size_t y = ( int )( pts->y + 0.5f );
            *dstX = *dstY = *dstD = placeHolder;
            if( y < h ){
                const float* lineX = mapX.line( y );
                const float* lineY = mapY.line( y );
                const float* lineD = mapD.line( y );
                size_t x = ( int )( pts->x + 0.5f );
                if( x < w ){
                    *dstX = lineX[ x ];
                    *dstY = lineY[ x ];
                    *dstD = lineD[ x ];
                }
            }
            dstX++;
            dstY++;
            dstD++;
            pts++;
        }
    }

    inline void RGBDPreprocessor::computeDepthGradient( Vector2f& grad,
                                                        const Vector2f& pos,
                                                        float val,
                                                        const IMapScoped<const float>& depthMap ) const
    {
        // x+1
        Vector2f tmp( pos );
        tmp.x += 1.0f;
        if( tmp.x < depthMap.width() - 1 ){
            grad.x = interpolateDepth( tmp, depthMap.ptr(), depthMap.stride() / sizeof( float ) ) - val;
        } else {
            grad.x = 0.0f;
        }

        tmp.x = pos.x;
        tmp.y += 1.0f;
        if( tmp.y < depthMap.height() - 1 ){
            grad.y = interpolateDepth( tmp, depthMap.ptr(), depthMap.stride() / sizeof( float ) ) - val;
        } else {
            grad.y = 0.0f;
        }
    }

}

#endif // RGBDPREPROCESSOR_H
