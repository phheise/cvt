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
#ifndef CL_PROXSTEREO_CL
#define CL_PROXSTEREO_CL

#import "Proximal.cl"

#define CENSUS_COLOR_WEIGHT 12.0f
#define CENSUS_COLOR_EXPONENT 1.0f

#define COLORWEIGHT 4.0f
#define COLORWEIGHTBIAS 8.0f

#define TAU 0.25f
#define SIGMA 0.25f
#define OVERSAMPLEX 1.0f
#define OVERSAMPLEY 1.0f

const sampler_t SAMPLER_NN       = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
const sampler_t SAMPLER_NN_ZERO  = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

inline float2 patch_sad( read_only image2d_t colimg1, read_only image2d_t gradimg1,
                         read_only image2d_t colimg2, read_only image2d_t gradimg2,
                         const float2 coord, const float disparity, const int patchsize )
{
    int width = get_image_width( colimg2 );
    int height = get_image_height( colimg2 );
    float2 ret = ( float2 ) ( 0.0f, 0.0f );
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
    const float4 grayWeight =  ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );
    float sum = 0.0f;

    float3 valc1 = read_imagef( colimg1, SAMPLER_BILINEAR, coord + OFFSETHALF ).xyz;
    float3 valc2 = read_imagef( colimg2, SAMPLER_BILINEAR, ( float2 ) ( coord.x - disparity, coord.y ) + OFFSETHALF ).xyz;

    for( float dy = -patchsize; dy <= patchsize; dy+=1.0f ) {
        for( float dx = -patchsize; dx <= patchsize; dx+=1.0f ) {
            float2 displace = ( float2 ) ( dx * OVERSAMPLEX, dy * OVERSAMPLEY );
            float2 pos = coord + displace;

            if( !( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height ) ) {

                float3 val1 = read_imagef( colimg1, SAMPLER_BILINEAR, pos + OFFSETHALF ).xyz;

                float w1 = native_exp( -pow( dot( fabs( val1 - valc1 ), grayWeight.xyz ), CENSUS_COLOR_EXPONENT ) * CENSUS_COLOR_WEIGHT );
                float2 pos2 =  ( float2 ) ( pos.x - disparity, pos.y );

                if( !( pos.x < 1 || pos.x >= width - 1 ) ) {

                    float3 val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF ).xyz;
                    float3 c = fabs( val2 - val1 );
                    float C = 0.33f * ( c.x + c.y + c.z );
                    C = clamp( C, 0.0f, 0.1f ) / 0.1f;

                    val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + ( float2 ) ( 1.0f, 0.0f ) + OFFSETHALF ).xyz;
                    c = fabs( val2 - val1 );
                    float Cxp = 0.33f * ( c.x + c.y + c.z );
                    Cxp = clamp( Cxp, 0.0f, 0.1f ) / 0.1f;

                    val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + ( float2 ) ( -1.0f, 0.0f ) + OFFSETHALF ).xyz;
                    c = fabs( val2 - val1 );
                    float Cxm = 0.33f * ( c.x + c.y + c.z );
                    Cxm = clamp( Cxm, 0.0f, 0.1f ) / 0.1f;


                    ret.x = ret.x + w1 * ( Cxm - Cxp ) * 0.5f;
                    ret.y = ret.y + w1 * ( Cxp + Cxm - 2.0f * C );
                    sum  += w1;
                }
            }
        }
    }

    if( fabs( sum ) < 1e-6f )
        return ( float2 ) 0.0f;

    ret.y = fmax( ret.y, 0.0f );
    return ret / sum;
}

inline float2 patch_census( read_only image2d_t colimg1, read_only image2d_t gradimg1,
                               read_only image2d_t colimg2, read_only image2d_t gradimg2,
                               const float2 coord, const float disparity, const int patchsize )
{
    int width = get_image_width( colimg2 );
    int height = get_image_height( colimg2 );
    float2 ret = ( float2 ) ( 0.0f, 0.0f );
    const float CENSUS_EPSILON = 0.001f;
    const float CENSUS_GRAD_EPSILON = 0.001f;
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
    const float4 grayWeight =  ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );
    const float delta = 1.0f;
    float sum = 0.0f;

    float3 valc1 = read_imagef( colimg1, SAMPLER_BILINEAR, coord + OFFSETHALF ).xyz;
    float3 valc2 = read_imagef( colimg2, SAMPLER_BILINEAR, ( float2 ) ( coord.x - disparity, coord.y ) + OFFSETHALF ).xyz;

    for( float dy = -patchsize; dy <= patchsize; dy+=1.0f ) {
        for( float dx = -patchsize; dx <= patchsize; dx+=1.0f ) {
            float2 displace = ( float2 ) ( dx * OVERSAMPLEX, dy * OVERSAMPLEY );
            //float2 displace = ( float2 ) ( dx + OVERSAMPLECUBE * pow( dx, 3 ), dy + OVERSAMPLECUBE * pow( dy, 3 ) );
            float2 pos = coord + displace;

            if( !( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height ) ) {

                float3 val1 = read_imagef( colimg1, SAMPLER_BILINEAR, pos + OFFSETHALF ).xyz - valc1;
                float4 gval1 = read_imagef( gradimg1, SAMPLER_BILINEAR, pos  + OFFSETHALF );

                float w1 = native_exp( -pow( dot( fabs( val1 ), grayWeight.xyz ), CENSUS_COLOR_EXPONENT ) * CENSUS_COLOR_WEIGHT );
                float2 pos2 =  ( float2 ) ( pos.x - disparity, pos.y );

                if( !( pos.x < 1 || pos.x >= width - 1 ) ) {

#define TOGRAY(x) dot( x, grayWeight )

                    float3 val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF ).xyz - valc2;
                    int3 a = as_int3( as_uint3( isgreaterequal( fabs( val1 ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val1 ) >> 31 ) << 1 )  ) );
                    int3 b = as_int3( as_uint3( isgreaterequal( fabs( val2 ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val2 ) >> 31 ) << 1 )  ) );
                    uint3 c = abs( b - a );
                    float C = 0.5f * 0.33f * ( c.x + c.y + c.z );

                    float4 gval2 = read_imagef( gradimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF );
                    int4 ga = as_int4( as_uint4( isgreaterequal( fabs( gval1 ), CENSUS_GRAD_EPSILON ) ) & as_uint4( ( int4 ) 1 - as_int4( ( as_uint4( gval1 ) >> 31 ) << 1 )  ) );
                    int4 gb = as_int4( as_uint4( isgreaterequal( fabs( gval2 ), CENSUS_GRAD_EPSILON ) ) & as_uint4( ( int4 ) 1 - as_int4( ( as_uint4( gval2 ) >> 31 ) << 1 )  ) );
                    uint4 gc = abs( gb - ga );
                    C += 0.5f * 0.25 * ( float )( gc.x + gc.y + gc.z + gc.w );

                    val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + ( float2 ) ( delta, 0.0f ) + OFFSETHALF ).xyz - valc2;
                    a = as_int3( as_uint3( isgreaterequal( fabs( val1 ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val1 ) >> 31 ) << 1 )  ) );
                    b = as_int3( as_uint3( isgreaterequal( fabs( val2 ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val2 ) >> 31 ) << 1 )  ) );
                    c = abs( b - a );
                    float Cxp = 0.5f * 0.33f * ( c.x + c.y + c.z );

                    gval2 = read_imagef( gradimg2, SAMPLER_BILINEAR, pos2 + ( float2 ) ( delta, 0.0f ) + OFFSETHALF );
                    ga = as_int4( as_uint4( isgreaterequal( fabs( gval1 ), CENSUS_GRAD_EPSILON ) ) & as_uint4( ( int4 ) 1 - as_int4( ( as_uint4( gval1 ) >> 31 ) << 1 )  ) );
                    gb = as_int4( as_uint4( isgreaterequal( fabs( gval2 ), CENSUS_GRAD_EPSILON ) ) & as_uint4( ( int4 ) 1 - as_int4( ( as_uint4( gval2 ) >> 31 ) << 1 )  ) );
                    gc = abs( gb - ga );
                    Cxp += 0.5f * 0.25 * ( float )( gc.x + gc.y + gc.z + gc.w );

                    val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + ( float2 ) ( -delta, 0.0f ) + OFFSETHALF ).xyz - valc2;
                    a = as_int3( as_uint3( isgreaterequal( fabs( val1 ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val1 ) >> 31 ) << 1 )  ) );
                    b = as_int3( as_uint3( isgreaterequal( fabs( val2 ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val2 ) >> 31 ) << 1 )  ) );
                    c = abs( b - a );
                    float Cxm = 0.5f * 0.33f * ( c.x + c.y + c.z );

                    gval2 = read_imagef( gradimg2, SAMPLER_BILINEAR, pos2 + ( float2 ) ( -delta, 0.0f ) + OFFSETHALF );
                    ga = as_int4( as_uint4( isgreaterequal( fabs( gval1 ), CENSUS_GRAD_EPSILON ) ) & as_uint4( ( int4 ) 1 - as_int4( ( as_uint4( gval1 ) >> 31 ) << 1 )  ) );
                    gb = as_int4( as_uint4( isgreaterequal( fabs( gval2 ), CENSUS_GRAD_EPSILON ) ) & as_uint4( ( int4 ) 1 - as_int4( ( as_uint4( gval2 ) >> 31 ) << 1 )  ) );
                    gc = abs( gb - ga );
                    Cxm += 0.5f * 0.25 * ( float )( gc.x + gc.y + gc.z + gc.w );

                    ret.x = ret.x + w1 * ( Cxm - Cxp ) / ( 2.0f * delta );
                    ret.y = ret.y + w1 * ( Cxp + Cxm - 2.0f * C ) / ( 4.0f * delta * delta );
                    sum += w1;
                }
            }
        }
    }

    if( sum <= 1.1f )
        return ( float2 ) 0.0f;

    return ret / sum;
}

inline float patch_eval_ncc_weighted( read_only image2d_t colimg1, read_only image2d_t gradimg1,
                                      read_only image2d_t colimg2, read_only image2d_t gradimg2,
                                      const float2 coord, const float disparity, const int patchsize )
{
    float wsum1 = 0;
    float ret1 = 0;
    int width = get_image_width( colimg2 );
    int height = get_image_height( colimg2 );
    float4 m12 = 0;
    float4 m1 = 0;
    float4 m2 = 0;
    float4 m1sqr = 0;
    float4 m2sqr = 0;
    int out = 0;
    const float4 grayWeight =  ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );

    float4 valcenter = read_imagef( colimg1, SAMPLER_BILINEAR, coord + ( float2 ) ( 0.5f, 0.5f) );

    for( float dy = -patchsize; dy <= patchsize; dy+=1.0f ) {
        for( float dx = -patchsize; dx <= patchsize; dx+=1.0f ) {
            float2 displace = ( float2 ) ( dx * OVERSAMPLEX, dy * OVERSAMPLEY );
            float2 pos = coord + displace;


            if( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height ) {
                continue;
            }

            float4 val1 = read_imagef( colimg1, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f, 0.5f));
            float4 gval1 = read_imagef( gradimg1, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f, 0.5f));

            float w1 = native_exp( -dot( fabs( valcenter.xyz - val1.xyz ), grayWeight.xyz ) * ( smoothstep( 0.0f, length( ( float2 ) ( patchsize, patchsize ) ), length( displace ) ) * 1.5f * COLORWEIGHT + COLORWEIGHTBIAS ) );// * exp( -fast_length( displace ) * 0.05f );

            pos.x -= disparity;
            if( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height ) {
                out++;
                continue;
            }


            float4 val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f, 0.5f));
            float4 gval2 = read_imagef( gradimg2, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f, 0.5f));

            //          float C = COLORGRADALPHA * dot( fmin( fabs( ( val1 - val2 ).xyz ), COLORMAXDIFF ), ( float3 ) 1.0f ) + ( 1.0f - COLORGRADALPHA ) * dot( fmin( fabs( ( gval1 - gval2 ) ), GRADMAXDIFF ), ( float4 ) 1.0f );

#define TOGRAY(x) dot( x, grayWeight )
            float4 v1 = ( float4 ) ( TOGRAY( val1 ), gval1.xyz );
            m1 += w1 * v1;
            m1sqr += w1 * v1 * v1;
            float4 v2 = ( float4 ) ( TOGRAY( val2 ), gval2.xyz );
            m2 += w1 * v2;
            m2sqr += w1 * v2 * v2;
            m12 += w1 * v1 * v2;
            wsum1 += w1;
#undef TOGRAY
        }
    }

    if( wsum1 <= 1.1f )
        return 1.0f;

   m12 = m12 / wsum1;
   m1 = m1 / wsum1;
   m2 = m2 / wsum1;
   m1sqr = m1sqr / wsum1;
   m2sqr = m2sqr / wsum1;

   float4 nom = ( m12 - m1 * m2 );
   //float4 denom = sqrt( fmax( ( m1sqr - m1 * m1  ), ( float4 ) 1e-5f ) * fmax( ( m2sqr - m2 * m2  ),  ( float4 ) 1e-5f ) );

   float4 denom1 = m1sqr - m1 * m1;
   float4 denom2 = m2sqr - m2 * m2;
//   float4 denom = sqrt( select( denom1, 1e-6f, denom1 <= 1e-6f ) * select( denom2, 1e-6f, denom2 <= 1e-6f ) );
   float4 denom = sqrt( denom1 * denom2 + 1.0f );

   return dot( fabs( 1.0f - ( nom / denom ) ), ( float4 ) ( 0.1f, 0.35f, 0.35f, 0.2f ) );

//   float4 acpi = acospi( clamp( nom / denom, -1.0f, 1.0f ) );
//   return dot( fmin( acpi * 4.0f, ( float4 ) 1.0f ), ( float4 ) ( 0.1f, 0.35f, 0.35f, 0.2f ) );

//   if( fabs( denom ) < 1e-8 ) denom = 1e-8f;
#if 0
    float a = nom / ( ( m1sqr - m1 * m1  ) + 1e-6f );
    float b = m2 - a * m1;

    if( fabs( a - 1.0f ) > 0.5f || fabs( b ) > 0.75f ) return 1e5f;

    return ( acospi( nom  / ( denom ) ) );
#endif
}

inline float patch_eval_census( read_only image2d_t colimg1, read_only image2d_t gradimg1,
                          read_only image2d_t colimg2, read_only image2d_t gradimg2,
                          const float2 coord, const float disparity, const int patchsize )
{
    float wsum = 0;
    float census = 0;
    int width = get_image_width( colimg2 );
    int height = get_image_height( colimg2 );
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
    const float CENSUS_EPSILON = 0.0001f;
    const float CENSUS_GRAD_EPSILON = 0.0001f;
//    const float COLORWEIGHT = 2.6f;
    const float3 grayWeight =  ( float3 ) ( 0.2126f, 0.7152f, 0.0722f );

    float3 valc1 = read_imagef( colimg1, SAMPLER_BILINEAR, coord + OFFSETHALF ).xyz;
    float3 valc2 = read_imagef( colimg2, SAMPLER_BILINEAR, coord + ( float2 ) ( -disparity, 0.0f ) + OFFSETHALF ).xyz;

    for( int dy = -patchsize; dy <= patchsize; dy++ ) {
#pragma unroll
        for( int dx = -patchsize; dx <= patchsize; dx++ ) {
            float2 displace = ( float2 ) ( ( ( float ) dx ) * OVERSAMPLEX, ( ( float ) dy ) * OVERSAMPLEY );
            //float2 displace = ( float2 ) ( dx + OVERSAMPLECUBE * pow( ( float ) dx, 3 ), dy + OVERSAMPLECUBE * pow( ( float ) dy, 3 ) );
            float2 pos = coord + displace;

            float2 pos2 = pos + ( float2 ) ( -disparity, 0.0f );
            if( pos2.x < 0 || pos2.x >= width ||
                pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height ) {
                continue;
            }

            //if( !( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height ||
            //      pos2.x < 0 || pos2.x >= width || pos2.y < 0 || pos2.y >= height ) ) {

            float3 valin = read_imagef( colimg1, SAMPLER_BILINEAR, pos + OFFSETHALF ).xyz;
            float3 val1 = valin - valc1;

            float w = native_exp( -pow( dot( fabs( val1 ), grayWeight ), CENSUS_COLOR_EXPONENT ) /* * ( 5.0f / ( fabs( 1.0f - dot( valc1, grayWeight ) ) + 0.5f ) )*/ * CENSUS_COLOR_WEIGHT );// ( smoothstep( 0.0f, length( ( float2 ) patchsize ), length( displace ) ) * COLORWEIGHT + COLORWEIGHTBIAS ) );// * exp( -fast_length( displace ) * 0.05f );

            float3 val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF ).xyz;// - valc2;

            //int3 a = as_int3( as_uint3( isgreaterequal( fabs( val1 ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val1 ) >> 31 ) << 1 )  ) );
            //int3 b = as_int3( as_uint3( isgreaterequal( fabs( val2 ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val2 ) >> 31 ) << 1 )  ) );
            //uint3 c = abs( b - a );
            float3 c = fmin( fabs( val2 - valin ), ( float3 ) 0.1f ) / 0.1f;

            //float w = native_exp( -pow( dot( fabs( valc1.xyz - val1.xyz ), ( float3 ) 1.0f ), COLORWEIGHTBETA ) * ( smoothstep( 0.0f, length( ( float2 ) patchsize ), length( displace ) ) * COLORWEIGHT + COLORWEIGHTBIAS ) );// * exp( -fast_length( displace ) * 0.05f );

            const float alpha = 0.125f;
            census += w * alpha * 0.33f * ( float )( c.x + c.y + c.z );

            float4 gval1 = read_imagef( gradimg1, SAMPLER_BILINEAR, pos + OFFSETHALF );
            float4 gval2 = read_imagef( gradimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF );
//            float4 gval2 = image_grad( colimg2, pos, state );


            int4 ga = as_int4( as_uint4( isgreaterequal( fabs( gval1 ), CENSUS_GRAD_EPSILON ) ) & as_uint4( ( int4 ) 1 - as_int4( ( as_uint4( gval1 ) >> 31 ) << 1 )  ) );
            int4 gb = as_int4( as_uint4( isgreaterequal( fabs( gval2 ), CENSUS_GRAD_EPSILON ) ) & as_uint4( ( int4 ) 1 - as_int4( ( as_uint4( gval2 ) >> 31 ) << 1 )  ) );
            uint4 gc = abs( gb - ga );
            census += w * ( 1.0f - alpha ) * 0.5f * 0.25f * ( float )( gc.x + gc.y + gc.z + gc.w );
            wsum += w;



            //val1 = select( sign( val1 ), ( float3 ) 0.0f, fabs( val1 ) < CENSUS_EPSILON );
            //val2 = select( sign( val2 ), ( float3 ) 0.0f, fabs( val2 ) < CENSUS_EPSILON );
            //census += dot( fabs( val2 - val1 ), ( float3 ) 0.333f ); //dot( select( ( float3 ) 0.0f, ( float3 ) 1.0f,  fabs( val2 - val1 ) > 0.1f ), ( float3 ) 0.33f );


            //   float w = 1.0f;
            //float w = native_exp( -dot( fabs( valcenter.xyz - val1.xyz ), ( float3 ) 1.0f ) * COLORWEIGHT );
            //    sum += w;
            //}
        }
    }

    if( wsum <= 1.1f )
        return 1.0f;

    return census / wsum;
}


float2 patch_grad_hess( read_only image2d_t colimg1, read_only image2d_t gradimg1,
                  read_only image2d_t colimg2, read_only image2d_t gradimg2,
                  const float2 coord, const float disparity, const int patchsize )
{
    float2 ret;

    float C = patch_eval_census( colimg1, gradimg1,
                                 colimg2, gradimg2, coord, disparity, patchsize );
    float Cxm = patch_eval_census( colimg1, gradimg1,
                                 colimg2, gradimg2, coord, disparity + 1.0f, patchsize );
    float Cxp = patch_eval_census( colimg1, gradimg1,
                                 colimg2, gradimg2, coord, disparity - 1.0f, patchsize );


    ret.x = ( Cxm - Cxp ) * 0.5f;
    ret.y = fmax( Cxp + Cxm - 2.0f * C, 0.0f );
    return ret;
}


kernel void prox_stereo_warp( write_only image2d_t output, read_only image2d_t input,
                             read_only image2d_t colimage0, read_only image2d_t gradimage0,
                             read_only image2d_t colimage1, read_only image2d_t gradimage1,
                             int patchsize )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    float2 coord = ( float2 ) ( gx, gy );
    int2 coordi  = ( int2 ) ( gx, gy );

    if( gx >= get_image_width( colimage0 ) || gy >= get_image_height( colimage0 ) )
        return;

    float d = read_imagef( input, SAMPLER_NN, coordi ).x;
    float2 gradhess = patch_grad_hess( colimage0, gradimage0, colimage1, gradimage1, coord, d, patchsize );
    write_imagef( output, coordi, ( float4 ) ( d, gradhess.x, gradhess.y, 0.0f ) );
}


kernel void prox_stereo( write_only image2d_t output, read_only image2d_t input,
                         write_only image2d_t outputp, read_only image2d_t inputp,
                         read_only image2d_t d0gradhessimg, float lambda, float depthmax, image2d_t precond )
{
#define BORDER 1
    const int width = get_image_width( input );
    const int height = get_image_height( input );
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
    const int bstride = lw + 2 * BORDER;
    const int2 base = ( int2 )( get_group_id( 0 ) * lw - BORDER, get_group_id( 1 ) * lh - BORDER );
    local float4 buf[ ( 32 + 2 * BORDER ) * ( 32 + 2 * BORDER ) ];
    int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    int yoffset;

    for( int y = ly; y < lh + 2 * BORDER; y += lh ) {
        yoffset = y * bstride;
        for( int x = lx; x < lw + 2 * BORDER; x += lw ) {
            // read disparity
            buf[ yoffset + x ].xy = read_imagef( input, SAMPLER_NN,  base + ( int2 ) ( x, y ) ).xy;
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    for( int y = ly; y < lh + 1; y += lh ) {
        yoffset = y * bstride;
        for( int x = lx; x < lw + 1; x += lw ) {
            // calculate the gradient
            float4 value = buf[ yoffset + x ];
            float2 grad;
            grad.x = value.x - buf[ yoffset + x + 1 ].x;
            grad.y = value.x - buf[ yoffset + x + bstride ].x;

            // p = p + sigma * \nabla disparity
            float sigma = read_imagef( precond, SAMPLER_NN, base + ( int2 ) ( x, y ) ).x;
            float2 p =  read_imagef( inputp, SAMPLER_NN_ZERO, base + ( int2 ) ( x, y ) ).xy;
            p = p + sigma * grad;
#if 0
            // Huber
            p = ( p + sigma * grad ) / ( 1.0f + sigma * 0.5f );
#endif


#if 1
            // project onto the unit sphere for TV
            buf[ yoffset + x ].zw = prox_project_unitsphere_2f( p );
#elif 0
            float len = length( p );
            len = select( len, 1.0f, len == 0.0f );
            float2 pn = p / len;
            p = p - sigma * pn * prox_log1p_1f( len / sigma, 1.0f / sigma, 0.56 );
#else
            p = p - sigma * prox_hl_2f( p / sigma, 1.0f / sigma, 0.8, 5 );
            buf[ yoffset + x ].zw = p;
#endif
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    if( gx >= width || gy >= height )
        return;

    float tau = read_imagef( precond, SAMPLER_NN_ZERO, ( int2 ) ( gx, gy ) ).x;
    float3 d0gradhess = read_imagef( d0gradhessimg, SAMPLER_NN, coord ).xyz;

    // calculate the divergence
    yoffset      = ( ly + 1 ) * bstride;
    float4 value = buf[ yoffset + lx + 1 ];
    float div    = value.z - buf[ yoffset + lx ].z + value.w - buf[ yoffset - bstride + lx + 1 ].w;
    float d      = value.x - tau * div;

    const float t = tau * lambda;
    float dnew = ( d / t - d0gradhess.y + d0gradhess.z * d0gradhess.x )  / ( d0gradhess.z + 1.0f / t );
    //float dnew = d - t * d0gradhess.y;

    if( !isfinite( dnew ) || fabs( dnew - d0gradhess.x ) > depthmax * 0.15f )
        dnew = d;

    dnew = clamp( dnew, 0.0f, depthmax );
    float dnew2 = dnew + 0.5 * ( dnew - value.y );

    write_imagef( output, coord , ( float4 ) ( clamp( dnew2, 0.0f, depthmax ), dnew, 0.0f, 0.0f ) );
    write_imagef( outputp, coord , ( float4 ) ( value.zw, 0.0f, 0.0f ) );
}

kernel void prox_stereo_disparity( write_only image2d_t output, read_only image2d_t input, float scale )
{
    int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const int width = get_image_width( input );
    const int height = get_image_height( input );

    if( coord.x >= width || coord.y >= height )
        return;

    float val = read_imagef( input, SAMPLER_NN, coord ).x * scale;
    write_imagef( output, coord, ( float4 ) val );
}

kernel void prox_stereo_warp_image( write_only image2d_t output, read_only image2d_t input, read_only image2d_t source )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    float2 coord = ( float2 ) ( gx, gy );
    int2 coordi  = ( int2 ) ( gx, gy );
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );

    if( gx >= get_image_width( input ) || gy >= get_image_height( input ) )
        return;

    float d = read_imagef( input, SAMPLER_NN, coordi ).x;
    float4 val = read_imagef( source, SAMPLER_BILINEAR, coord + ( float2 ) ( -d, 0.0f ) + OFFSETHALF );
    write_imagef( output, coordi, val );
}

#endif
