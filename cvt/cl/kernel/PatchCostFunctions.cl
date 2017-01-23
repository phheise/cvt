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
#ifndef CVT_PATCHCOSTFUNCTION_CL
#define CVT_PATCHCOSTFUNCTION_CL

#import "Matrix3.cl"

inline float eval_census_trunc_ad( read_only image2d_t colimg1, read_only image2d_t colimg2,
                       const float2 coord, const float2 coord2, const int patchradius )
{
    const float ALPHA                 = 0.1f;
    const float TRUNCATION            = 0.05f;
    const float CENSUS_EPSILON        = 0.04f;

    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
    const int width = get_image_width( colimg2 );
    const int height = get_image_height( colimg2 );

    float ret = 0;
    float sum = 0;

    float3 valc1 = read_imagef( colimg1, SAMPLER_BILINEAR, coord + OFFSETHALF ).xyz;
    float3 valc2 = read_imagef( colimg2, SAMPLER_BILINEAR, coord2 + OFFSETHALF ).xyz;

    for( int dy = -patchradius; dy <= patchradius; dy++ ) {
        for( int dx = -patchradius; dx <= patchradius; dx++ ) {

            const float2 pos = coord + ( float2 ) ( dx, dy );
            const float2 pos2 = coord2 + ( float2 ) ( dx, dy );

            if( pos2.x < 0 || pos2.x >= width || pos2.y < 0 || pos2.y >= height )
                continue;

            const float3 inval1 = read_imagef( colimg1, SAMPLER_BILINEAR, pos + OFFSETHALF ).xyz;
            const float3 inval2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF ).xyz;

            const float3 val1 = inval1 - valc1;
            const float3 val2 = inval2 - valc2;

            const int3 a = as_int3( as_uint3( isgreaterequal( fabs( val1 ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val1 ) >> 31 ) << 1 )  ) );
            const int3 b = as_int3( as_uint3( isgreaterequal( fabs( val2 ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val2 ) >> 31 ) << 1 )  ) );
            //            int3 a = signbit( val1 );
            //            int3 b = signbit( val2 );
            const uint3 c = abs( b - a );
            float census = 0.333f * 0.5f * ( float )( c.x + c.y + c.z );

            ret += census * ( 1.0f - ALPHA ) + ALPHA * dot( fmin( fabs( inval1 - inval2 ), TRUNCATION ), ( float3 ) 1.0f ) / ( 3.0f * TRUNCATION);
            sum += 1.0f;
        }
    }

    if( sum < 1.0f ) return 1.0f;
    return ret / sum;
    //return ret / ( ( float ) ( patchradius * 2 + 1 ) * ( patchradius * 2 + 1 ) );
}

inline float eval_census_trunc_ad_affine2d( read_only image2d_t colimg1, read_only image2d_t colimg2,
                       const float2 coord, const float2 coord2, const float4 affine, const int patchradius )
{
    const float ALPHA                 = 0.1f;
    const float TRUNCATION            = 0.05f;
    const float CENSUS_EPSILON        = 0.04f;

    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
    const int width = get_image_width( colimg2 );
    const int height = get_image_height( colimg2 );

    float ret = 0;
    float sum = 0;

    float3 valc1 = read_imagef( colimg1, SAMPLER_BILINEAR, coord + OFFSETHALF ).xyz;
    float3 valc2 = read_imagef( colimg2, SAMPLER_BILINEAR, coord2 + OFFSETHALF ).xyz;

    for( int dy = -patchradius; dy <= patchradius; dy++ ) {
        for( int dx = -patchradius; dx <= patchradius; dx++ ) {

            const float2 pos = coord + ( float2 ) ( dx, dy );
            const float2 pos2 = coord2 + ( float2 ) ( dot( affine.xy, ( float2 )( dx, dy ) ),
                                                      dot( affine.zw, ( float2 )( dx, dy ) ) );

            if( pos2.x < 0 || pos2.x >= width || pos2.y < 0 || pos2.y >= height )
                continue;

            const float3 inval1 = read_imagef( colimg1, SAMPLER_BILINEAR, pos + OFFSETHALF ).xyz;
            const float3 inval2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF ).xyz;

            const float3 val1 = inval1 - valc1;
            const float3 val2 = inval2 - valc2;

            const int3 a = as_int3( as_uint3( isgreaterequal( fabs( val1 ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val1 ) >> 31 ) << 1 )  ) );
            const int3 b = as_int3( as_uint3( isgreaterequal( fabs( val2 ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val2 ) >> 31 ) << 1 )  ) );
            //            int3 a = signbit( val1 );
            //            int3 b = signbit( val2 );
            const uint3 c = abs( b - a );
            float census = 0.333f * 0.5f * ( float )( c.x + c.y + c.z );

            ret += census * ( 1.0f - ALPHA ) + ALPHA * dot( fmin( fabs( inval1 - inval2 ), TRUNCATION ), ( float3 ) 1.0f ) / ( 3.0f * TRUNCATION);
            sum += 1.0f;
        }
    }

    if( sum < 1.0f ) return 1.0f;
    return ret / sum;
    //return ret / ( ( float ) ( patchradius * 2 + 1 ) * ( patchradius * 2 + 1 ) );
}

inline float eval_bilateral_census_trunc_ad( read_only image2d_t colimg1, read_only image2d_t colimg2,
                             const float2 coord, const float2 coord2, const int patchradius )
{
    const float CENSUS_COLOR_WEIGHT   = 10.0f;
    const float CENSUS_COLOR_EXPONENT = 1.0f;
    const float ALPHA                 = 0.1f;
    const float TRUNCATION            = 0.05f;

    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
    const float2 OFFSETHALF  = ( float2 ) ( 0.5f, 0.5f );
    const float3 GRAY_WEIGHT =  ( float3 ) ( 0.2126f, 0.7152f, 0.0722f );
    const int width = get_image_width( colimg2 );
    const int height = get_image_height( colimg2 );

    float wsum = 0;
    float census = 0;
    float m2 = 0;
    float m2sum = 0;

    // central pixels
    float3 valc1 = read_imagef( colimg1, SAMPLER_BILINEAR, coord + OFFSETHALF ).xyz;
    float3 valc2 = read_imagef( colimg2, SAMPLER_BILINEAR, coord2 + OFFSETHALF ).xyz;

    for( int dy = -patchradius; dy <= patchradius; dy++ ) {
        for( int dx = -patchradius; dx <= patchradius; dx++ ) {
            float2 displace = ( float2 ) ( ( ( float ) dx ) , ( ( float ) dy ) );
            float2 pos = coord + displace;
            float2 pos2 = coord2 + displace;

            if( pos2.x < 0 || pos2.x >= width || pos2.y < 0 || pos2.y >= height )
                continue;

            float3 valin1 = read_imagef( colimg1, SAMPLER_BILINEAR, pos + OFFSETHALF ).xyz;
            float3 valin2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF ).xyz;

            float3 cvaldiff1 = valin1 - valc1;
            float3 cvaldiff2 = valin2 - valc2;

            // weight based on difference to central pixel
            float w = exp( -pow( fabs( dot( cvaldiff1, GRAY_WEIGHT ) ), CENSUS_COLOR_EXPONENT ) * CENSUS_COLOR_WEIGHT );

            // truncated absolute diff weighted by alpha
            float3 c = fmin( fabs( valin2 - valin1 ), ( float3 ) TRUNCATION ) / TRUNCATION;
            float cad = 0.333f * ( float )( c.x + c.y + c.z );
            float cost = ALPHA * cad;

            // census based difference weighted by 1 - ALPHA
            int3 ga = signbit(  cvaldiff1 );
            int3 gb = signbit(  cvaldiff2 );
            uint3 gc = abs( ga - gb );
            float cc =  0.333f * ( float )( gc.x + gc.y + gc.z );
            cost += ( 1.0f - ALPHA ) * cc;

            census += w * cost;
            wsum += w;
        }
    }

    // central pixel is isolated, or everything outside of second image
    if( wsum < 1.0f )
        return 1.0f;

    return ( census / wsum );
}

inline float eval_ncc_gray( read_only image2d_t colimg1, read_only image2d_t colimg2,
                       const float2 coord, const float2 coord2, const int patchradius )
{
    const float4 grayWeight =  ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );
#define TOGRAY(x) dot( x, grayWeight )

    float sum = 0;
    float mean1 = ( float ) 0;
    float mean2 = ( float ) 0;
    float mean12 = ( float ) 0;
    float msqr1 = ( float ) 0;
    float msqr2 = ( float ) 0;
    int width = get_image_width( colimg2 );
    int height = get_image_height( colimg2 );

    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
    const float COLORWEIGHT = 2.6f;

    float4 valcenter = TOGRAY( read_imagef( colimg1, SAMPLER_BILINEAR, coord + OFFSETHALF ) );

    for( int dy = -patchradius; dy <= patchradius; dy++ ) {
        for( int dx = -patchradius; dx <= patchradius; dx++ ) {

            float2 pos = coord + ( float2 ) ( dx, dy );
            float2 pos2 = coord2 + ( float2 ) ( dx, dy );

            // if( pos2.x < 0 || pos2.x >= width || pos2.y < 0 || pos2.y >= height )
            //     continue;

            float val1 = TOGRAY( read_imagef( colimg1, SAMPLER_BILINEAR, pos + OFFSETHALF ) );
            float val2 = TOGRAY( read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF ) );
            //float w = native_exp( -dot( fabs( valcenter.xyz - val1.xyz ), ( float3 ) 1.0f ) * COLORWEIGHT );
            float w = 1.0f;
            sum += w;
            mean1 += val1 * w;
            mean2 += val2 * w;
            mean12 += val1 * val2 * w;
            msqr1 += val1 * val1 * w;
            msqr2 += val2 * val2 * w;
        }
    }

    if( sum < 1.0f ) return 1.0f;

    float iwsum = 1.0f / sum;
    mean1  = mean1 * iwsum;
    mean2  = mean2 * iwsum;
    mean12 = mean12 * iwsum;
    msqr1  = msqr1 * iwsum;
    msqr2  = msqr2 * iwsum;
    float nom = mean12 - mean1 * mean2;
    float denom1 = msqr1 - mean1 * mean1;
    float denom2 = msqr2 - mean2 * mean2;
    float denom = sqrt( select( denom1, 1.0f, denom1 <= 1e-4f ) * select( denom2, 1.0f, denom2 <= 1e-4f ) );

    return acos( nom / denom ) / ( float ) M_PI;
#undef TOGRAY
}

inline float eval_ncc( read_only image2d_t colimg1, read_only image2d_t colimg2,
                       const float2 coord, const float2 coord2, const int patchradius )
{
    float3 mean1 = ( float3 ) 0;
    float3 mean2 = ( float3 ) 0;
    float3 mean12 = ( float3 ) 0;
    float3 msqr1 = ( float3 ) 0;
    float3 msqr2 = ( float3 ) 0;
    float sum = 0;
    int width = get_image_width( colimg2 );
    int height = get_image_height( colimg2 );
    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );

    for( int dy = -patchradius; dy <= patchradius; dy++ ) {
        for( int dx = -patchradius; dx <= patchradius; dx++ ) {
            const float2 pos = coord + ( float2 ) ( dx, dy );
            const float2 pos2 = coord2 + ( float2 ) ( dx, dy );

            if( pos2.x < 0 || pos2.x >= width || pos2.y < 0 || pos2.y >= height )
                continue;

            const float3 val1 = read_imagef( colimg1, SAMPLER_BILINEAR, pos + OFFSETHALF ).xyz;
            const float3 val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF ).xyz;

            mean1 += val1;
            mean2 += val2;
            mean12 += val1 * val2;
            msqr1 += val1 * val1;
            msqr2 += val2 * val2;
            sum += 1.0f;
        }
    }

    const float3 iwsum = 1.0f / sum;
    mean1  = mean1 * iwsum;
    mean2  = mean2 * iwsum;
    mean12 = mean12 * iwsum;
    msqr1  = msqr1 * iwsum;
    msqr2  = msqr2 * iwsum;
    const float3 nom = mean12 - mean1 * mean2;
    const float3 denom1 = msqr1 - mean1 * mean1;
    const float3 denom2 = msqr2 - mean2 * mean2;
    const float3 denom = sqrt( select( denom1, 1.0f, denom1 <= 1e-4f ) * select( denom2, 1.0f, denom2 <= 1e-4f ) );

    return dot( acos( nom / denom ),( float3 ) ( 1.0f / ( M_PI * 3.0f ) ) );
}

inline float eval_ncc_affine2d( read_only image2d_t colimg1, read_only image2d_t colimg2,
                             const float2 coord, const float2 coord2, const float4 affine, const int patchradius )
{
    float3 mean1 = ( float3 ) 0;
    float3 mean2 = ( float3 ) 0;
    float3 mean12 = ( float3 ) 0;
    float3 msqr1 = ( float3 ) 0;
    float3 msqr2 = ( float3 ) 0;
    float sum = 0;
    int width = get_image_width( colimg2 );
    int height = get_image_height( colimg2 );
    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );

    for( int dy = -patchradius; dy <= patchradius; dy++ ) {
        for( int dx = -patchradius; dx <= patchradius; dx++ ) {
            const float2 pos = coord + ( float2 ) ( dx, dy );
            const float2 pos2 = coord2 + ( float2 ) ( dot( affine.xy, ( float2 )( dx, dy ) ),
                                                      dot( affine.zw, ( float2 )( dx, dy ) ) );
            if( pos2.x < 0 || pos2.x >= width || pos2.y < 0 || pos2.y >= height )
                continue;
            const float3 val1 = read_imagef( colimg1, SAMPLER_BILINEAR, pos + OFFSETHALF ).xyz;
            const float3 val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF ).xyz;

            mean1 += val1;
            mean2 += val2;
            mean12 += val1 * val2;
            msqr1 += val1 * val1;
            msqr2 += val2 * val2;
            sum += 1.0f;
        }
    }

    const float3 iwsum = 1.0f / sum;
    mean1  = mean1 * iwsum;
    mean2  = mean2 * iwsum;
    mean12 = mean12 * iwsum;
    msqr1  = msqr1 * iwsum;
    msqr2  = msqr2 * iwsum;
    const float3 nom = mean12 - mean1 * mean2;
    const float3 denom1 = msqr1 - mean1 * mean1;
    const float3 denom2 = msqr2 - mean2 * mean2;
    const float3 denom = sqrt( select( denom1, 1.0f, denom1 <= 1e-4f ) * select( denom2, 1.0f, denom2 <= 1e-4f ) );

    return dot( acos( nom / denom ),( float3 ) ( 1.0f / ( M_PI * 3.0f ) ) );
}

inline float eval_bilateral_ncc( read_only image2d_t colimg1, read_only image2d_t colimg2,
                       const float2 coord, const float2 coord2, const int patchradius )
{
    float sum = 0;
    float3 mean1 = ( float3 ) 0;
    float3 mean2 = ( float3 ) 0;
    float3 mean12 = ( float3 ) 0;
    float3 msqr1 = ( float3 ) 0;
    float3 msqr2 = ( float3 ) 0;
    const int width = get_image_width( colimg2 );
    const int height = get_image_height( colimg2 );

    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
    const float COLORWEIGHT = 6.0f;

    const float4 valcenter = read_imagef( colimg1, SAMPLER_BILINEAR, coord + OFFSETHALF );

    for( int dy = -patchradius; dy <= patchradius; dy++ ) {
        for( int dx = -patchradius; dx <= patchradius; dx++ ) {

            float2 pos = coord + ( float2 ) ( dx, dy );
            float2 pos2 = coord2 + ( float2 ) ( dx, dy );

            if( !( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height ||
                pos2.x < 0 || pos2.x >= width || pos2.y < 0 || pos2.y >= height ) ) {
                float3 val1 = read_imagef( colimg1, SAMPLER_BILINEAR, pos + OFFSETHALF ).xyz;
                float3 val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF ).xyz;
                float w = native_exp( -dot( fabs( valcenter.xyz - val1.xyz ), ( float3 ) 1.0f ) * COLORWEIGHT );
                sum += w;
                mean1 += val1 * w;
                mean2 += val2 * w;
                mean12 += val1 * val2 * w;
                msqr1 += val1 * val1 * w;
                msqr2 += val2 * val2 * w;

            }
        }
    }

    if( sum < 1.0f ) return 1.0f;
    const float3 iwsum = 1.0f / sum;
    mean1  = mean1 * iwsum;
    mean2  = mean2 * iwsum;
    mean12 = mean12 * iwsum;
    msqr1  = msqr1 * iwsum;
    msqr2  = msqr2 * iwsum;
    const float3 nom = mean12 - mean1 * mean2;
    const float3 denom1 = msqr1 - mean1 * mean1;
    const float3 denom2 = msqr2 - mean2 * mean2;
    const float3 denom = sqrt( select( denom1, 1.0f, denom1 <= 1e-4f ) * select( denom2, 1.0f, denom2 <= 1e-4f ) );

    return dot( acos( nom / denom ),( float3 ) ( 1.0f / ( M_PI * 3.0f ) ) );
}

inline float eval_trunc_sad( read_only image2d_t colimg1, read_only image2d_t colimg2,
                             const float2 coord, const float2 coord2, const int patchradius )
{
    float ret = 0;
    float3 mean1 = ( float3 ) 0;
    float3 mean2 = ( float3 ) 0;
    float3 mean12 = ( float3 ) 0;
    float3 msqr1 = ( float3 ) 0;
    float3 msqr2 = ( float3 ) 0;
    int width = get_image_width( colimg2 );
    int height = get_image_height( colimg2 );
    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
    const float THRESHOLD = 0.05f;
    float wsum = 0.0f;

    for( int dy = -patchradius; dy <= patchradius; dy++ ) {
        for( int dx = -patchradius; dx <= patchradius; dx++ ) {

            float2 pos = coord + ( float2 ) ( dx, dy );
            float2 pos2 = coord2 + ( float2 ) ( dx, dy );
            //if( !( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height ||
            //      pos2.x < 0 || pos2.x >= width || pos2.y < 0 || pos2.y >= height ) ) {
            float4 val1 = read_imagef( colimg1, SAMPLER_BILINEAR, pos + OFFSETHALF );
            float4 val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF );
            ret += dot( fmin( fabs( val1.xyz - val2.xyz ), ( float3 ) THRESHOLD ), ( float3 ) 1.0f );
            wsum += 1.0f;
            //}
        }
    }

    if( wsum < 1.0f ) return 1.0f;
    return ret / wsum;
}

inline float eval_bilateral_trunc_sad( read_only image2d_t colimg1, read_only image2d_t colimg2,
                             const float2 coord, const float2 coord2, const int patchradius )
{
    const float COLORWEIGHT = 10.0f;
    float ret = 0;
    const int width = get_image_width( colimg2 );
    const int height = get_image_height( colimg2 );
    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
    const float THRESHOLD = 0.5f;
    float wsum = 0.0f;

    const float4 valcenter = read_imagef( colimg1, SAMPLER_BILINEAR, coord + OFFSETHALF );

    for( int dy = -patchradius; dy <= patchradius; dy++ ) {
        for( int dx = -patchradius; dx <= patchradius; dx++ ) {

            float2 pos = coord + ( float2 ) ( dx, dy );
            float2 pos2 = coord2 + ( float2 ) ( dx, dy );

            if( !( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height ||
                pos2.x < 0 || pos2.x >= width || pos2.y < 0 || pos2.y >= height ) ) {
                const float4 val1 = read_imagef( colimg1, SAMPLER_BILINEAR, pos + OFFSETHALF );
                const float4 val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF );
                const float w = native_exp( -dot( fabs( valcenter.xyz - val1.xyz ), ( float3 ) 1.0f ) * COLORWEIGHT );
                wsum += w;
                ret += w * dot( fmin( fabs( val1.xyz - val2.xyz ), ( float3 ) THRESHOLD ), ( float3 ) 1.0f ) / ( THRESHOLD * 3.0f );
            }
        }
    }

    if( wsum < 1.0f ) return 1.0f;
    return ret / wsum;
}

#endif
