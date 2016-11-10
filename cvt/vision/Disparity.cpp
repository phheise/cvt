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

#include <cvt/vision/Disparity.h>
#include <cvt/gfx/IColorCode.h>

namespace cvt {
    void Disparity::interpolateBackground( Image& dst, const Image& disparity )
    {
        // TODO: SIMD candidate for the filling
        // check the formats
        if( disparity.format() != IFormat::GRAY_FLOAT )
            throw CVTException("Invalid format provided!");

        dst.reallocate( disparity.width(), disparity.height(), IFormat::GRAY_FLOAT, disparity.memType() );

        IMapScoped<const float> dispmap( disparity );
        IMapScoped<float>      dstmap( dst );

        for( ssize_t y = 0; y < dispmap.height(); y++ ) {
            const float* ptrdisp = dispmap.ptr();
            float*     ptrdst  = dstmap.ptr();
            ssize_t   count   = 0;
            for( ssize_t x = 0; x < dispmap.width(); x++ ) {
                if( ptrdisp[ x ] > 0.0f ) {
                    // valid
                    if( count ) {
                        // fill
                        if( x - count > 0 ) {
                            float fillval = Math::min( ptrdisp[ x - count - 1 ], ptrdisp[ x ] );
                            for( ssize_t fillx = x - count; fillx < x; fillx++ )
                                ptrdst[ fillx ] = fillval;
                        } else {
                            // this can only happen if the left values from the border are invalid
                            float fillval = ptrdisp[ x ];
                            for( ssize_t fillx = 0; fillx < x; fillx++ )
                                ptrdst[ fillx ] = fillval;
                        }
                        // reset
                        count = 0;
                    }
                    // reuse current value
                    ptrdst[ x ] = ptrdisp[ x ];
                } else {
                    // invalid
                    count++;
                }
            }
            // check the right border
            if( count && ( ( ssize_t ) dispmap.width() - count ) > 0 ) {
                // this can only happen if the right values to the border are invalid
                float fillval = ptrdisp[ dispmap.width() - 1 - count ];
                for( ssize_t fillx = dispmap.width() - count; fillx < dispmap.width(); fillx++ )
                    ptrdst[ fillx ] = fillval;
            }
            dispmap++;
            dstmap++;
        }

        // if the first pixel is invalid the whole line must be invalid
        // if everything is invalid we will never find a valid line
        // top to bottom
        dstmap.reset();
        for( ssize_t y = 0; y < dstmap.height(); y++ ) {
            if( *dstmap.ptr() > 0.0f ) {
                SIMD* simd = SIMD::instance();
                for( ssize_t filly = 0; filly < y; filly++ ) {
                    simd->Memcpy( ( uint8_t* ) dstmap.line( filly ), ( uint8_t* ) dstmap.line( y ), sizeof( float ) * dstmap.width() );
                }
                break;
            }
            dstmap++;
        }
        // bottom to top
        dstmap.setLine( dstmap.height() - 1 );
        for( ssize_t y = dstmap.height() - 1; y >= 0; y-- ) {
            if( *dstmap.ptr() > 0.0f ) {
                SIMD* simd = SIMD::instance();
                for( ssize_t filly = dstmap.height() - 1; filly > y; filly-- ) {
                    simd->Memcpy( ( uint8_t* ) dstmap.line( filly ), ( uint8_t* ) dstmap.line( y ), sizeof( float ) * dstmap.width() );
                }
                break;
            }
            dstmap--;
        }
        // TODO: completely invalid lines in the middle are not filled
    }

    void Disparity::errorImage( Image& dst, const Image& disparity, const Image& disparity_gt_occ /*, const Image& disparity_gt_noc*/ )
    {
        // we reuse the KITTI error color coding scheme
        struct IntervalColor {
            float min, max;
            Color color;
        };
        static IntervalColor coltable[] = {
            {.min = 0.0f   , .max = 0.0625f, Color( 0.192f, 0.212f, 0.584f, 1.0f ) },
            {.min = 0.0625f, .max = 0.125f,  Color( 0.270f, 0.459f, 0.706f, 1.0f ) },
            {.min = 0.125f , .max = 0.25f,   Color( 0.455f, 0.678f, 0.820f, 1.0f ) },
            {.min = 0.25f  , .max = 0.5f,   Color( 0.671f, 0.851f, 0.914f, 1.0f ) },
            {.min = 0.5f   , .max = 1.0f,   Color( 0.878f, 0.953f, 0.973f, 1.0f ) },
            {.min = 1.0f   , .max = 2.0f,   Color( 0.996f, 0.878f, 0.565f, 1.0f ) },
            {.min = 2.0f   , .max = 4.0f,   Color( 0.992f, 0.682f, 0.380f, 1.0f ) },
            {.min = 4.0f   , .max = 8.0f,   Color( 0.957f, 0.427f, 0.263f, 1.0f ) },
            {.min = 8.0f   , .max = 16.0f,   Color( 0.843,  0.188f, 0.153f, 1.0f ) },
            {.min = 16.0f  , .max = 1e10f,   Color( 0.647,  0.0f, 0.149f, 1.0f ) },
        };

        // check the formats
        if( disparity.format() != IFormat::GRAY_FLOAT ||
           //disparity_gt_noc.format() != IFormat::GRAY_FLOAT ||
           disparity_gt_occ.format() != IFormat::GRAY_FLOAT )
            throw CVTException("Invalid formats provided!");

        dst.reallocate( disparity.width(), disparity.height(), IFormat::RGBA_FLOAT, disparity.memType() );

//      IMapScoped<const float> gtnocmap( disparity_gt_noc );
        IMapScoped<const float> gtoccmap( disparity_gt_occ );
        IMapScoped<const float> dispmap( disparity );
        IMapScoped<float>      dstmap( dst );
        Color* c;

        for( size_t y = 0; y < dispmap.height(); y++ ) {
            const float* ptrocc  = gtoccmap.ptr();
//          const float* ptrnoc  = gtnocmap.ptr();
            const float* ptrdisp = dispmap.ptr();
            float*     ptrdst  = dstmap.ptr();
            for( size_t x = 0; x < dispmap.width(); x++ ) {
                if( ptrocc[ x ] > 0.0f ) {
                    float d_gt  = ptrocc[ x ];
                    float d_est = ptrdisp[ x ];
                    float d_err = Math::abs( d_gt - d_est );
                    float n_err = Math::min( d_err / 3.0f, 20.0f * d_err / Math::abs( d_gt ) );

                    c = &coltable[ 9 ].color;
                    for( int i = 0 ; i < 10; i++ ) {
                        if( n_err >= coltable[i].min && n_err < coltable[i].max ) c = &coltable[ i ].color;
                    }
                    //if( ptrnoc[ x ] > 0.0f ) {
                    //  ptrdst[ x * 4 + 0 ] = 0.5f * c->red();
                    //  ptrdst[ x * 4 + 1 ] = 0.5f * c->green();
                    //  ptrdst[ x * 4 + 2 ] = 0.5f * c->blue();
                    //  ptrdst[ x * 4 + 3 ] = 1.0f;
                    //} else {
                        ptrdst[ x * 4 + 0 ] = c->red();
                        ptrdst[ x * 4 + 1 ] = c->green();
                        ptrdst[ x * 4 + 2 ] = c->blue();
                        ptrdst[ x * 4 + 3 ] = 1.0f;
                    //}
                } else {
                    ptrdst[ x * 4 + 0 ] = 0.0f;
                    ptrdst[ x * 4 + 1 ] = 0.0f;
                    ptrdst[ x * 4 + 2 ] = 0.0f;
                    ptrdst[ x * 4 + 3 ] = 1.0f;
                }
            }
            //gtnocmap++;
            gtoccmap++;
            dispmap++;
            dstmap++;
        }
    }

    void Disparity::colorCode( Image& dst, const Image& disparity, float* max_disparity )
    {
        // check the formats
        if( disparity.format() != IFormat::GRAY_FLOAT )
            throw CVTException("Invalid formats provided!");
        dst.reallocate( disparity.width(), disparity.height(), IFormat::RGBA_FLOAT, disparity.memType() );


        float min, max;
        disparity.minmax( min, max );
        if( max_disparity )
            *max_disparity = max;
        disparity.colorCode( dst, ICOLORCODE_KITTI, 0.0f, max, false );
    }


    void Disparity::evaluateError( size_t& error, size_t& valid, size_t& all, const Image& disparity, const Image& dispgt, float abs_thres, float rel_thres )
    {
        // check the formats
        if( disparity.format() != IFormat::GRAY_FLOAT ||
           dispgt.format() != IFormat::GRAY_FLOAT )
            throw CVTException("Invalid formats provided!");

        error = 0;
        valid = 0;
        all = 0;

        IMapScoped<const float> gtmap( dispgt );
        IMapScoped<const float> dispmap( disparity );
        for( size_t y = 0; y < gtmap.height(); y++ ) {
            const float* ptrgt  = gtmap.ptr();
            const float* ptrest = dispmap.ptr();
            for( size_t x = 0; x < gtmap.width(); x++ ) {
                if( ptrgt[ x ] > 0.0f ) {
                    float d_gt  = ptrgt[ x ];
                    float d_est = ptrest[ x ];
                    bool  d_err = Math::abs( d_gt - d_est ) > abs_thres && ( Math::abs( d_gt - d_est ) / Math::abs( d_gt ) ) > rel_thres;

                    if( d_est > 0.0f )
                        valid++;
                    if( d_err )
                        error++;
                    all++;
                }
            }
            gtmap++;
            dispmap++;
        }
    }


    void Disparity::histogramErrorAbsolute( const Image& gt, const Image& test, std::vector< std::pair< float, float> >& histogram )
    {
        // check the formats
        if( gt.format() != IFormat::GRAY_FLOAT ||
           test.format() != IFormat::GRAY_FLOAT )
            throw CVTException("Invalid formats provided!");


        IMapScoped<const float> gtmap( gt );
        IMapScoped<const float> testmap( test );
        typedef std::vector< std::pair< float, float> >::iterator IT;

        size_t w = gt.width();
        size_t h = gt.height();
        size_t valid = 0;

        for( size_t y = 0; y < h; y++ ) {
            const float* gtptr = gtmap.ptr();
            const float* testptr = testmap.ptr();
            for( size_t x = 0; x < w; x++ ) {
                if( std::isfinite( gtptr[ x ] ) && gtptr[ x ] > 0.0f &&
                   std::isfinite( testptr[ x ] ) && testptr[ x ] > 0.0f ) {
                    valid++;
                    float lastvalue = 0.0f;
                    float diff = Math::abs( testptr[ x ] - gtptr[ x ] );
                    for( IT it = histogram.begin(); it < histogram.end(); it++ ) {
                        if( diff <= it->first && diff > lastvalue )
                            it->second += 1.0f;
                        lastvalue = it->first;
                    }
                }
            }
            gtmap++;
            testmap++;
        }


        for( IT it = histogram.begin(); it < histogram.end(); it++ ) {
            it->second /= ( float ) valid;
        }
    }

    void Disparity::histogramErrorRelative( const Image& gt, const Image& test, std::vector< std::pair< float, float> >& histogram )
    {
        // check the formats
        if( gt.format() != IFormat::GRAY_FLOAT ||
           test.format() != IFormat::GRAY_FLOAT )
            throw CVTException("Invalid formats provided!");


        IMapScoped<const float> gtmap( gt );
        IMapScoped<const float> testmap( test );
        typedef std::vector< std::pair< float, float> >::iterator IT;

        size_t w = gt.width();
        size_t h = gt.height();
        size_t valid = 0;

        for( size_t y = 0; y < h; y++ ) {
            const float* gtptr = gtmap.ptr();
            const float* testptr = testmap.ptr();
            for( size_t x = 0; x < w; x++ ) {
                if( std::isfinite( gtptr[ x ] ) && gtptr[ x ] > 0.0f &&
                   std::isfinite( testptr[ x ] ) && testptr[ x ] > 0.0f ) {
                    valid++;
                    float lastvalue = 0.0f;
                    float diff = Math::abs( testptr[ x ] - gtptr[ x ] ) / Math::abs( gtptr[ x ] );
                    for( IT it = histogram.begin(); it < histogram.end(); it++ ) {
                        if( diff <= it->first && diff > lastvalue )
                            it->second += 1.0f;
                        lastvalue = it->first;
                    }
                }
            }
            gtmap++;
            testmap++;
        }


        for( IT it = histogram.begin(); it < histogram.end(); it++ ) {
            it->second /= ( float ) valid;
        }
    }

    void Disparity::defaultHistogramAbsolute( std::vector< std::pair< float, float> >& histogram )
    {
        histogram.clear();
        histogram.push_back( std::pair<float,float>(  0.50f, 0.0f ) );
        histogram.push_back( std::pair<float,float>(  0.75f, 0.0f ) );
        histogram.push_back( std::pair<float,float>(  1.00f, 0.0f ) );
        histogram.push_back( std::pair<float,float>(  2.00f, 0.0f ) );
        histogram.push_back( std::pair<float,float>(  3.00f, 0.0f ) );
        histogram.push_back( std::pair<float,float>(  4.00f, 0.0f ) );
        histogram.push_back( std::pair<float,float>(  5.00f, 0.0f ) );
        histogram.push_back( std::pair<float,float>(  6.00f, 0.0f ) );
        histogram.push_back( std::pair<float,float>(  7.00f, 0.0f ) );
        histogram.push_back( std::pair<float,float>(  8.00f, 0.0f ) );
        histogram.push_back( std::pair<float,float>(  9.00f, 0.0f ) );
        histogram.push_back( std::pair<float,float>( 10.00f, 0.0f ) );
        histogram.push_back( std::pair<float,float>( 20.00f, 0.0f ) );
        histogram.push_back( std::pair<float,float>( 40.00f, 0.0f ) );
        histogram.push_back( std::pair<float,float>(  1e10f, 0.0f ) );
    }

    void Disparity::defaultHistogramRelative( std::vector< std::pair< float, float> >& histogram )
    {
        histogram.clear();
        histogram.push_back( std::pair<float,float>(  0.005f, 0.0f ) );
        histogram.push_back( std::pair<float,float>(  0.01f, 0.0f ) );
        histogram.push_back( std::pair<float,float>(  0.02f, 0.0f ) );
        histogram.push_back( std::pair<float,float>(  0.03f, 0.0f ) );
        histogram.push_back( std::pair<float,float>(  0.04f, 0.0f ) );
        histogram.push_back( std::pair<float,float>(  0.05f, 0.0f ) );
        histogram.push_back( std::pair<float,float>(  0.10f, 0.0f ) );
        histogram.push_back( std::pair<float,float>(  0.20f, 0.0f ) );
        histogram.push_back( std::pair<float,float>(  0.40f, 0.0f ) );
        histogram.push_back( std::pair<float,float>(  1e10f, 0.0f ) );
    }
}
