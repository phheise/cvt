/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2013, Philipp Heise and Sebastian Klose
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
#ifndef VPMSTEREO_CL
#define VPMSTEREO_CL

#import "Matrix3.cl"

#define CENSUS_COLOR_WEIGHT 8.0f
#define CENSUS_COLOR_EXPONENT 1.0f

#define OVERSAMPLEX 1.0f
#define OVERSAMPLEY 1.0f

#define OVERSAMPLECUBEX 0.03f
#define OVERSAMPLECUBEY 0.03f

const sampler_t SAMPLER_NN       = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

typedef float3 VPMSState;

VPMSState vpmsstate_viewprop( const VPMSState state );

inline float vpmsstate_vanDerCorput2( uint value )
{
     value = ( value << 16 ) | ( value >> 16 );
     value = ( ( value & 0x55555555 ) << 1 ) | ( ( value & 0xAAAAAAAA ) >> 1 );
     value = ( ( value & 0x33333333 ) << 2 ) | ( ( value & 0xCCCCCCCC ) >> 2 );
     value = ( ( value & 0x0F0F0F0F ) << 4 ) | ( ( value & 0xF0F0F0F0 ) >> 4 );
     value = ( ( value & 0x00FF00FF ) << 8 ) | ( ( value & 0xFF00FF00 ) >> 8 );
     return 2.3283064365386963e-10f * ( float ) value;
}

inline VPMSState vpmsstate_from_disparity_normal( const float2 coord, const float disparity, const float3 normal )
{
    float3 ret = ( float3 ) ( 1.0f, 0.0f, 0.0f ) - ( float3 ) ( - normal.x / normal.z, - normal.y / normal.z, ( normal.x * coord.x + normal.y * coord.y ) / normal.z + disparity );
    return ret;
}

inline VPMSState vpmsstate_init_vandercorput2( const float2 coord, uint nth, const float disparityMax )
{
    return vpmsstate_from_disparity_normal( coord, vpmsstate_vanDerCorput2( nth ) * disparityMax, ( float3 ) ( 0.0f, 0.0f, 1.0f ) );
}

inline float4 vpmsstate_to_normal_depth( const VPMSState state, const float2 coord, const int lr )
{
    float4 ret;
    float3 _state = state;
    if( !lr )
        _state = vpmsstate_viewprop( state );
    _state =  ( float3 ) ( 1.0f, 0.0f, 0.0f ) - _state;
    ret.w = _state.x * coord.x + _state.y * coord.y + _state.z;
    ret.z = native_rsqrt( _state.x * _state.x + _state.y * _state.y + 1.0f );
    ret.x = -_state.x * ret.z;
    ret.y = -_state.y * ret.z;
    return ret;
}

inline VPMSState vpmsstate_viewprop( const VPMSState state )
{
    float3 n = state;
    float3 ret = ( float3 ) ( ( 1.0f / n.x ), -n.y / n.x, -n.z / n.x );
    return ret;
}

inline float2 vpmsstate_transform( const VPMSState state, const float2 coord )
{
    return ( float2 ) ( state.x * coord.x + state.y * coord.y + state.z, coord.y );
}

inline float3 vpmsstate_to_normal( const VPMSState _state )
{
    float3 n;
    float3 state =  ( float3 ) ( 1.0f, 0.0f, 0.0f ) - _state;
    n.z = native_rsqrt( state.x * state.x + state.y * state.y + 1.0f );
    n.x = -state.x * n.z;
    n.y = -state.y * n.z;
    n.z = native_sqrt( 1.0f - n.x * n.x - n.y * n.y );
    return n;
}

inline float4 vpmsstate_to_normal_color( const VPMSState _state )
{
    float4 n;
    float3 state =  ( float3 ) ( 1.0f, 0.0f, 0.0f ) - _state;
    n.z = native_rsqrt( state.x * state.x + state.y * state.y + 1.0f );
    n.x = -state.x * n.z;
    n.y = -state.y * n.z;
    n.z = native_sqrt( 1.0f - n.x * n.x - n.y * n.y );
    n.xyz = n.xyz * 0.5f + 0.5f;
    n.w = 1.0f;
    return n;
}

inline VPMSState patch_census_klt( read_only image2d_t colimg1, read_only image2d_t gradimg1,
                                             read_only image2d_t colimg2, read_only image2d_t gradimg2,
                                             const float2 coord, const VPMSState state, const int patchsize )
{
    int width = get_image_width( colimg2 );
    int height = get_image_height( colimg2 );
    float3 J = ( float3 ) 0.0f;
    //float3 H = ( float3 ) 0.0f;
    Mat3f H, mtmp;
    const float CENSUS_EPSILON = 0.001f;
    const float CENSUS_GRAD_EPSILON = 0.001f;
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
    const float3 grayWeight =  ( float3 ) ( 0.2126f, 0.7152f, 0.0722f );
    const float alpha = 0.05f;

    mat3f_set_zero( &H );

    float3 valc1 = read_imagef( colimg1, SAMPLER_BILINEAR, coord + ( float2 ) ( 0.5f, 0.5f) ).xyz;

    for( float dy = -patchsize; dy <= patchsize; dy+=1.0f ) {
        for( float dx = -patchsize; dx <= patchsize; dx+=1.0f ) {
            float2 displace = ( float2 ) ( dx * OVERSAMPLEX, dy * OVERSAMPLEY );
            //float2 displace = ( float2 ) ( ( float ) dx + OVERSAMPLECUBEX * pow( ( float ) dx, 3.0f ), ( float ) dy + OVERSAMPLECUBEY * pow( ( float ) dy, 3.0f ) );
            float2 pos = coord + displace;

            if( !( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height ) ) {

                float3 valin = read_imagef( colimg1, SAMPLER_BILINEAR, pos + OFFSETHALF ).xyz;
                float3 val1 = valin - valc1;
                float4 gval1 = read_imagef( gradimg1, SAMPLER_BILINEAR, pos  + OFFSETHALF );

                float w1 = exp( -pow( fabs( dot( val1, grayWeight ) ), CENSUS_COLOR_EXPONENT )  * CENSUS_COLOR_WEIGHT );
                float2 pos2 = vpmsstate_transform( state, pos );

                if( !( pos.x < 1 || pos.x >= width - 1 ) ) {

                    /* C */
                    float3 val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF ).xyz;
                    float3 c = fmin( fabs( val2 - valin ), ( float3 ) 0.05f ) / 0.05f;
                    float C = w1 * alpha * 0.33f * ( c.x + c.y + c.z );

                    float4 gval2 = read_imagef( gradimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF );
                    int4 ga = signbit(  gval1 );
                    int4 gb = signbit(  gval2 );
                    uint4 gc = abs( ga - gb );
                    C += w1 * ( 1.0f - alpha ) * 0.25 * ( float )( gc.x + gc.y + gc.z + gc.w );

                    /* Cxp */
                    val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + ( float2 ) ( 1.0f, 0.0f ) + OFFSETHALF ).xyz;
                    c = fmin( fabs( val2 - valin ), ( float3 ) 0.05f ) / 0.05f;
                    float Cxp = w1 * alpha * 0.33f * ( c.x + c.y + c.z );

                    gval2 = read_imagef( gradimg2, SAMPLER_BILINEAR, pos2 + ( float2 ) ( 1.0f, 0.0f ) + OFFSETHALF );
                    gb = signbit(  gval2 );
                    gc = abs( ga - gb );
                    Cxp += w1 * ( 1.0f - alpha ) * 0.25 * ( float )( gc.x + gc.y + gc.z + gc.w );

                    /* Cxm */
                    val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + ( float2 ) ( -1.0f, 0.0f ) + OFFSETHALF ).xyz;
                    c = fmin( fabs( val2 - valin ), ( float3 ) 0.05f ) / 0.05f;
                    float Cxm = w1 * alpha * 0.33f * ( c.x + c.y + c.z );

                    gval2 = read_imagef( gradimg2, SAMPLER_BILINEAR, pos2 + ( float2 ) ( -1.0f, 0.0f ) + OFFSETHALF );
                    gb = signbit(  gval2 );
                    gc = abs( ga - gb );
                    Cxm += w1 * ( 1.0f - alpha ) * 0.25 * ( float )( gc.x + gc.y + gc.z + gc.w );


                    float3 pos3 = ( float3 ) ( coord + displace, 1.0f );
                    J = J + w1 * ( Cxm - Cxp ) * 0.5f * pos3;
                    //H = H + w1 * ( Cxp + Cxm - 2.0f * C ) * pos3 * pos3;

                    mat3f_outer( &mtmp, pos3, pos3 );
                    mat3f_mul_scalar( &mtmp, w1 * ( Cxp + Cxm - 2.0f * C ), &mtmp );
                    mat3f_add( &H, &H, &mtmp );
                }
            }
        }
    }

    return state /*+ J / fmin( H, 0.1f );*/ - mat3f_lusolve( &H, J );
}

inline float patch_eval_census( read_only image2d_t colimg1, read_only image2d_t gradimg1,
                                read_only image2d_t colimg2, read_only image2d_t gradimg2,
                                const float2 coord, const VPMSState state, const int patchsize )
{
    float wsum = 0;
    float census = 0;
    int width = get_image_width( colimg2 );
    int height = get_image_height( colimg2 );
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
    const float3 grayWeight =  ( float3 ) ( 0.2126f, 0.7152f, 0.0722f );
    const float alpha = 0.05f;

    float2 coord2 = vpmsstate_transform( state, coord );

    if( coord2.x < 0 || coord2.x >= width )
        return 1.0f;

    float3 valc1 = read_imagef( colimg1, SAMPLER_BILINEAR, coord + OFFSETHALF ).xyz;
    float3 valc2 = read_imagef( colimg2, SAMPLER_BILINEAR, coord2 + OFFSETHALF ).xyz;

    for( int dy = -patchsize; dy <= patchsize; dy++ ) {
#pragma unroll
        for( int dx = -patchsize; dx <= patchsize; dx++ ) {
            float2 displace = ( float2 ) ( ( ( float ) dx ) * OVERSAMPLEX, ( ( float ) dy ) * OVERSAMPLEY );
            //float2 displace = ( float2 ) ( ( float ) dx + OVERSAMPLECUBEX * pow( ( float ) dx, 3.0f ), ( float ) dy + OVERSAMPLECUBEY * pow( ( float ) dy, 3.0f ) );
            float2 pos = coord + displace;

            float2 pos2 = vpmsstate_transform( state, pos );
            if( pos2.x < 0 || pos2.x >= width ||
                pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height ) {
                continue;
            }

            float3 valin = read_imagef( colimg1, SAMPLER_BILINEAR, pos + OFFSETHALF ).xyz;
            float3 val1 = valin - valc1;

            float w = exp( -pow( fabs( dot( val1, grayWeight ) ), CENSUS_COLOR_EXPONENT )  * CENSUS_COLOR_WEIGHT );
            //float w = exp( -pow( fabs( dot( val1, grayWeight ) ), CENSUS_COLOR_EXPONENT )  * CENSUS_COLOR_WEIGHT * ( 0.01 + smoothstep( 0.0f, length( ( float2 ) patchsize ), length( displace ) ) ) );

            float3 val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF ).xyz;// - valc2;
            float3 c = fmin( fabs( val2 - valin ), ( float3 ) 0.05f ) / 0.05f;

            census += w * alpha * 0.33f * ( float )( c.x + c.y + c.z );

            float4 gval1 = read_imagef( gradimg1, SAMPLER_BILINEAR, pos + OFFSETHALF );
            float4 gval2 = read_imagef( gradimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF );

            //census += 0.5f * w * ( 1.0f - alpha ) * 0.25f * dot( fmin( fabs( gval1 - gval2 ), 0.05f ) / 0.05f, ( float4 ) 0.25f );

            int4 ga = signbit(  gval1 );
            int4 gb = signbit(  gval2 );
            uint4 gc = abs( ga - gb );
            census += w * ( 1.0f - alpha ) * 0.25f * ( float )( gc.x + gc.y + gc.z + gc.w );
            wsum += w;
        }
    }

    return census / wsum;
}

kernel void vpmstereo_init( write_only image2d_t output,
                            read_only image2d_t colimg1, read_only image2d_t gradimg1,
                            read_only image2d_t colimg2, read_only image2d_t gradimg2,
                            const int patchsize, const float disparityMax )
{
    const int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const float2 coordf = ( float2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const int width = get_image_width( colimg1 );
    const int height = get_image_height( colimg1 );

    if( coord.x >= width || coord.y >= height )
        return;

    VPMSState ret = vpmsstate_init_vandercorput2( coordf, coord.x * width + coord.y, disparityMax );
    float cost = patch_eval_census( colimg1, gradimg1, colimg2, gradimg2, coordf, ret, patchsize );

    write_imagef( output, coord, ( float4 ) ( ret, cost ) );
}

kernel void vpmstereo_init_disparity( write_only image2d_t output,
                                     read_only image2d_t disparityimg,
                                     read_only image2d_t colimg1, read_only image2d_t gradimg1,
                                     read_only image2d_t colimg2, read_only image2d_t gradimg2,
                                     const int patchsize, const float disparityMax )
{
    const int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const float2 coordf = ( float2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const int width = get_image_width( colimg1 );
    const int height = get_image_height( colimg1 );

    if( coord.x >= width || coord.y >= height )
        return;

    float disparity = read_imagef( disparityimg, SAMPLER_NN, coord ).x;
    VPMSState ret;
    if( disparity >= 0 && disparity < disparityMax ) {
        float3 normal = ( float3 ) ( 0.0f, 0.0f, 1.0f );
        //float dleft = read_imagef( disparityimg, SAMPLER_NN, coord + ( int2 ) ( -1,  0 ) ).x;
        //float dup   = read_imagef( disparityimg, SAMPLER_NN, coord + ( int2 ) (  0, -1 ) ).x;
        //normal = ( float3 ) ( dleft - disparity, dup - disparity, 0.0f );
        //normal.xy = normal.xy / fmax( length( normal.xy ), 0.9f );
        //normal.z = sqrt( 1.0f - normal.x * normal.x - normal.y * normal.y );
        ret = vpmsstate_from_disparity_normal( coordf, disparity, normal );
    } else
        ret = vpmsstate_init_vandercorput2( coordf, coord.x * width + coord.y, disparityMax );

    float cost = patch_eval_census( colimg1, gradimg1, colimg2, gradimg2, coordf, ret, patchsize );

    write_imagef( output, coord, ( float4 ) (ret, cost ) );
}

kernel void vpmstereo_iter_simple( write_only image2d_t output, read_only image2d_t input,
                                   read_only image2d_t colimg1, read_only image2d_t gradimg1,
                                   read_only image2d_t colimg2, read_only image2d_t gradimg2,
                                   const int patchsize, const float disparityMax, int iter )
{
    const int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const float2 coordf = ( float2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const int width = get_image_width( colimg1 );
    const int height = get_image_height( colimg1 );
    float4 current, new;
    VPMSState state;
    float cost;
    int2 offsets[] = {
        ( int2 ) (  1,  0 ),
        ( int2 ) ( -1,  0 ),
        ( int2 ) (  0,  1 ),
        ( int2 ) (  0, -1 ),
        ( int2 ) (  16,   0 ),
        ( int2 ) ( -16,   0 ),
        ( int2 ) (   0,  16 ),
        ( int2 ) (   0, -16 )
    };

    if( coord.x >= width || coord.y >= height )
        return;

    current = read_imagef( input, SAMPLER_NN, coord );

    state   = vpmsstate_init_vandercorput2( coordf, ( coord.x & 0x07 ) * ( coord.y & 0x07 ) + iter, disparityMax );
    cost    = patch_eval_census( colimg1, gradimg1, colimg2, gradimg2, coordf, state, patchsize );
    current = cost < current.w ? ( float4 ) ( state, cost ) : current;

    for( int i = 0; i < 8; i++ ) {
        state = read_imagef( input, SAMPLER_NN, coord + offsets[ ( iter + i ) & 0x07 ] ).xyz;
        cost    = patch_eval_census( colimg1, gradimg1, colimg2, gradimg2, coordf, state, patchsize );
        current = cost < current.w ? ( float4 ) ( state, cost ) : current;
    }

#if 1
    // KLT step
    {
        VPMSState state = patch_census_klt( colimg1, gradimg1, colimg2, gradimg2, coordf, current.xyz, patchsize );
        for( int kltiter = 0; kltiter < 2; kltiter++ )
            state = patch_census_klt( colimg1, gradimg1, colimg2, gradimg2, coordf, state, patchsize );
        cost    = patch_eval_census( colimg1, gradimg1, colimg2, gradimg2, coordf, state, patchsize );
        current = cost < current.w ? ( float4 ) ( state, cost ) : current;
    }
#endif

    write_imagef( output, coord, current );
}

kernel void vpmstereo_disparity( write_only image2d_t output, read_only image2d_t input, float scale )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    float2 coord = ( float2 ) ( gx, gy );
    int2 coordi  = ( int2 ) ( gx, gy );
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );

    if( gx >= get_image_width( input ) || gy >= get_image_height( input ) )
        return;

    VPMSState state = read_imagef( input, SAMPLER_NN, coordi ).xyz;
    float val = fabs( vpmsstate_transform( state, coord ).x - coord.x ) * scale;
    write_imagef( output, coordi, ( float4 ) val );
}

kernel void vpmstereo_warp_image( write_only image2d_t output, read_only image2d_t input, read_only image2d_t source )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    float2 coord = ( float2 ) ( gx, gy );
    int2 coordi  = ( int2 ) ( gx, gy );
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );

    if( gx >= get_image_width( input ) || gy >= get_image_height( input ) )
        return;

    VPMSState state = read_imagef( input, SAMPLER_NN, coordi ).xyz;
    float4 val = read_imagef( source, SAMPLER_BILINEAR, vpmsstate_transform( state, coord ) + OFFSETHALF );
    write_imagef( output, coordi, val );
}

#endif
