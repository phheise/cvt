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

#import "RNG.cl"
#import "Matrix3.cl"

#define PROPSIZE 0
#define DEPTHREFINEMUL 2.0f
#define NORMALREFINEMUL 0.05f
#define NORMALCOMPMAX 0.95f
#define NUMRNDTRIES  2
#define NUMRNDSAMPLE 4
#define NUMRNDSAMPLERADIUS 5.0f

#define COLORWEIGHT 4.0f
#define COLORWEIGHTBIAS 8.0f
#define COLORWEIGHTBETA 0.8f
#define COLORGRADALPHA 0.05f
#define COLORMAXDIFF 0.05f
#define GRADMAXDIFF 0.05f
#define OVERSAMPLEX 1.0f
#define OVERSAMPLEY 1.0f
#define OVERSAMPLECUBE 0.03f
#define VIEWSAMPLES 2
#define CENSUS_COLOR_WEIGHT 12.0f
#define CENSUS_COLOR_EXPONENT 1.0f

#define NORMAL_Z_MIN 1e-4f

#define INVEPS 1.0f
#define INVSCALE 0.25f

//#define SMOOTHSCALE 2.0f
//#define TOSMOOTH( x ) ( log1p( fmax( x * SMOOTHSCALE, 0.0f ) ) / log1p( SMOOTHSCALE ) )
//#define FROMSMOOTH( x ) ( (  exp( fmax( x * log1p( SMOOTHSCALE ), 0.0f ) ) - 1.0f ) / SMOOTHSCALE )

//#define SMOOTHSCALE 0.01f
//#define TOSMOOTH( x ) clamp( x * SMOOTHSCALE, -4.0f, 4.0f )
//#define FROMSMOOTH( x ) ( x / SMOOTHSCALE )

#define SMOOTHSCALE 0.02f
#define TOSMOOTH( x ) ( SMOOTHSCALE / ( ( x ) + SMOOTHSCALE ) )
#define FROMSMOOTH( x ) ( ( ( x ) / SMOOTHSCALE ) - SMOOTHSCALE )


// #define USELOCALBUF  1

const sampler_t SAMPLER_NN       = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;


float4 nd_state_viewprop( const float4 state );
float2 nd_state_transform( const float4 state, const float2 coord );

float4 nd_state_init( RNG* rng, const float2 coord, int lr, const float normmul, const float disparitymax )
{
    float z = RNG_float( rng ) * disparitymax;
    float3 n;

    float azimuth = 2.0f * M_PI * RNG_float( rng );
    float u = RNG_float( rng ) * NORMALCOMPMAX;
    float w = sqrt( 1.0f - u * u );

    n.x = w * cos( azimuth );
    n.y = w * sin( azimuth );
    n.z = u;

    n.z = fmax( n.z, NORMAL_Z_MIN );
    n   = normalize( n );

    float4 ret = ( float4 ) ( 1.0f, 0.0f, 0.0f, 0.0f ) - ( float4 ) ( - n.x / n.z, - n.y / n.z, ( n.x * coord.x + n.y * coord.y ) / n.z + z, 0.0f );
    if( !lr )
        ret = nd_state_viewprop( ret );
    return ret;
}

float4 nd_state_from_disparity_normal( const float2 coord, const float disparity, const float3 _normal )
{
    float3 normal = _normal;
    normal.z = fmax( normal.z, NORMAL_Z_MIN );
    normal   = normalize( normal );
    float4 ret = ( float4 ) ( 1.0f, 0.0f, 0.0f, 0.0f ) - ( float4 ) ( - normal.x / normal.z, - normal.y / normal.z, ( normal.x * coord.x + normal.y * coord.y ) / normal.z + disparity, 0.0f );
    return ret;
}

float4 nd_state_to_ref_normal_depth( const float4 state, const float2 coord, const int lr )
{
    float4 ret;
    float4 _state = state;
    int sidesign = lr ? -1 : 1;

    if( !lr )
        _state = nd_state_viewprop( state );
    _state =  ( float4 ) ( 1.0f, 0.0f, 0.0f, 0.0f ) - _state;
    //ret.w = ( _state.x * coord.x + _state.y * coord.y + _state.z;
    ret.w = ( nd_state_transform( state, coord ).x - coord.x ) * ( float ) sidesign;
    ret.z = native_rsqrt( _state.x * _state.x + _state.y * _state.y + 1.0f );
    ret.x = -_state.x * ret.z;
    ret.y = -_state.y * ret.z;

    if( !all(isfinite(ret.xyz)))
        return (float4) (0.0f,0.0f,1.0f,0.0f);

    return ret;
}

float4 nd_state_refine( RNG* rng, const float4 _state, const float2 coord, const float disparitymax, int lr )
{
    float3 n, n2;
    float4 state = _state;
    if( !lr )
        state = nd_state_viewprop( state );
    state =  ( float4 ) ( 1.0f, 0.0f, 0.0f, 0.0f ) - state;
    float z = state.x * coord.x + state.y * coord.y + state.z;
    n.z = native_rsqrt( state.x * state.x + state.y * state.y + 1.0f );
    n.x = -state.x * n.z;
    n.y = -state.y * n.z;

    z += ( RNG_float( rng ) - 0.5f ) * 2.0f * DEPTHREFINEMUL;
    z = clamp( z, 0.0f, disparitymax );

    float azimuth = 2.0f * M_PI * RNG_float( rng );
    float u = RNG_float( rng ) * ( 1.0f - NORMALREFINEMUL );
    float w = sqrt( 1.0f - u * u );

    n2.x = w * cos( azimuth );
    n2.y = w * sin( azimuth );
    n2.z = u;

    n = ( n + n2 ) * 0.5f;
    //n *= sign( n.z );

    //n.x += ( RNG_float( rng ) - 0.5f ) * NORMALREFINEMUL;
    //n.y += ( RNG_float( rng ) - 0.5f ) * NORMALREFINEMUL;
    //n.x = clamp( n.x, -NORMALCOMPMAX, NORMALCOMPMAX );
    //n.y = clamp( n.y, -NORMALCOMPMAX, NORMALCOMPMAX );

    //float nfactor = fmax( length( n.xy ) + 0.001f, 1.0f );
    //n.xy = n.xy / nfactor;
    //n.z = native_sqrt( 1.0f - n.x * n.x - n.y * n.y );

    n.z = fmax( n.z, NORMAL_Z_MIN );
    n   = normalize( n );

    float4 ret = ( float4 ) ( - n.x / n.z, - n.y / n.z, ( n.x * coord.x + n.y * coord.y ) / n.z + z, 0.0f );
    ret = ( float4 ) ( 1.0f, 0.0f, 0.0f, 0.0f ) - ret;
    if( !lr )
        return nd_state_viewprop( state );
    return ret;
}

float4 nd_state_viewprop( const float4 state )
{
    float4 n = state;
    float4 ret = ( float4 ) ( ( 1.0f / n.x ), -n.y / n.x, -n.z / n.x, 0.0f );
    if( fabs( n.x ) < 1e-8f )
        ret = ( float4 ) ( 0.0f, -n.y, -n.z, 0.0f );
    return ret;
}

float2 nd_state_transform( const float4 state, const float2 coord )
{
    return ( float2 ) ( state.x * coord.x + state.y * coord.y + state.z, coord.y );
}

float4 nd_state_to_color( const float4 _state, const float2 coord, const float disparitymax )
{
    float4 n;
    float4 state =  ( float4 ) ( 1.0f, 0.0f, 0.0f, 0.0f ) - _state;
    n.z = native_rsqrt( state.x * state.x + state.y * state.y + 1.0f );
    n.x = -state.x * n.z;
    n.y = -state.y * n.z;
    n.xy = n.xy * 0.5f + 0.5f;
    n.z = ( state.x * coord.x + state.y * coord.y + state.z ) / disparitymax;
    n.w = 1.0f;
    return n;
}

float3 nd_state_to_normal( const float4 _state )
{
    float3 n;
    float4 state =  ( float4 ) ( 1.0f, 0.0f, 0.0f, 0.0f ) - _state;
    n.z = native_rsqrt( state.x * state.x + state.y * state.y + 1.0f );
    n.x = -state.x * n.z;
    n.y = -state.y * n.z;
    n.z = native_sqrt( 1.0f - n.x * n.x - n.y * n.y );
    return n;
}


float4 nd_state_to_normal_color( const float4 _state )
{
    float4 n;
    float4 state =  ( float4 ) ( 1.0f, 0.0f, 0.0f, 0.0f ) - _state;
    n.z = native_rsqrt( state.x * state.x + state.y * state.y + 1.0f );
    n.x = -state.x * n.z;
    n.y = -state.y * n.z;
    n.z = native_sqrt( 1.0f - n.x * n.x - n.y * n.y );
    n.xyz = n.xyz * 0.5f + 0.5f;
    n.w = 1.0f;
    return n;
}

inline float4 image_grad( read_only image2d_t img, float2 pos, float4 state )
{
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
    const float3 grayWeight =  ( float3 ) ( 0.2126f, 0.7152f, 0.0722f );

    float valxm = dot( read_imagef( img, SAMPLER_BILINEAR, nd_state_transform( state, pos + ( float2 ) ( -1.0f,  0.0f ) ) + OFFSETHALF ).xyz, grayWeight );
    float valxp = dot( read_imagef( img, SAMPLER_BILINEAR, nd_state_transform( state, pos + ( float2 ) (  1.0f,  0.0f ) ) + OFFSETHALF ).xyz, grayWeight );
    float valym = dot( read_imagef( img, SAMPLER_BILINEAR, nd_state_transform( state, pos + ( float2 ) (  0.0f, -1.0f ) ) + OFFSETHALF ).xyz, grayWeight );
    float valyp = dot( read_imagef( img, SAMPLER_BILINEAR, nd_state_transform( state, pos + ( float2 ) (  0.0f,  1.0f ) ) + OFFSETHALF ).xyz, grayWeight );
    float val   = dot( read_imagef( img, SAMPLER_BILINEAR, nd_state_transform( state, pos + ( float2 ) (  0.0f,  0.0f ) ) + OFFSETHALF ).xyz, grayWeight );

    return ( float4 ) ( valxp - val, valyp - val, valxp - valxm, valyp - valym );
}


inline float4 patch_color_grad_weighted_klt( read_only image2d_t colimg1, read_only image2d_t gradimg1,
                                             read_only image2d_t colimg2, read_only image2d_t gradimg2,
                                             const float2 coord, const float4 state, const int patchsize, const int lr )
{
    int width = get_image_width( colimg2 );
    int height = get_image_height( colimg2 );
    float3 J = ( float3 ) 0.0f;
    //float3 H = ( float3 ) 0.0f;
    Mat3f H, mtmp;
    const float4 grayWeight =  ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );

    if( !all(isfinite(state.xyz)))
        return 1e5f;

    mat3f_set_zero( &H );

//    const float OVERSAMPLEX = ( nd_state_transform( state, coord ).x / 120.0f ) * 1.75f + 0.25f;

//  const float4 grayWeight =  ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );

    float4 valcenter = read_imagef( colimg1, SAMPLER_BILINEAR, coord + ( float2 ) ( 0.5f, 0.5f) );

    for( float dy = -patchsize; dy <= patchsize; dy+=1.0f ) {
        for( float dx = -patchsize; dx <= patchsize; dx+=1.0f ) {
            float2 displace = ( float2 ) ( dx * OVERSAMPLEX, dy * OVERSAMPLEY );
            //float2 displace = ( float2 ) ( dx + OVERSAMPLECUBE * pow( dx, 3 ), dy + OVERSAMPLECUBE * pow( dy, 3 ) );
            float2 pos = coord + displace;

            if( !( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height ) ) {

                float4 val1 = read_imagef( colimg1, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f, 0.5f));
                float4 gval1 = read_imagef( gradimg1, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f, 0.5f));

                float w1 = native_exp( -pow( dot( fabs( valcenter.xyz - val1.xyz ), grayWeight.xyz ), 2.0f ) * 16.0f );// ( smoothstep( 0.0f, length( ( float2 ) patchsize ), length( displace ) ) * COLORWEIGHT + COLORWEIGHTBIAS ) );// * exp( -fast_length( displace ) * 0.05f );
                //float w1 = native_exp( -pow( dot( fabs( valcenter.xyz - val1.xyz ), grayWeight.xyz ), COLORWEIGHTBETA ) * ( smoothstep( 0.0f, length( ( float2 ) patchsize ), length( displace ) ) * COLORWEIGHT + COLORWEIGHTBIAS ) );// * exp( -fast_length( displace ) * 0.05f );

                //          float w1 = exp( -dot( fabs( valcenter.xyz - val1.xyz ), ( float3 ) 1.0f ) * COLORWEIGHT );// * exp( -fast_length( displace ) * 0.05f );

                // transform point
                //      float d = nd_state_transform( state, pos );
                //      pos.x += select( d, -d, lr );
                pos = nd_state_transform( state, pos );
                if( !( pos.x < 1 || pos.x >= width - 1 ) ) {

#define TOGRAY(x) dot( x, grayWeight )

                    float4 val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f, 0.5f));
                    float4 gval2 = read_imagef( gradimg2, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f, 0.5f));

                    float4 val2xp = read_imagef( colimg2, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f + 1.0f, 0.5f));
                    float4 gval2xp = read_imagef( gradimg2, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f + 1.0f, 0.5f));

                    float4 val2xm = read_imagef( colimg2, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f - 1.0f, 0.5f));
                    float4 gval2xm = read_imagef( gradimg2, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f - 1.0f, 0.5f));

                    float3 diffc = ( val1 - val2 ).xyz;
                    float4 diffg = gval1 - gval2;
                    float C = COLORGRADALPHA * dot( fmin( fabs( diffc ), COLORMAXDIFF ), ( float3 ) 1.0f ) + ( 1.0f - COLORGRADALPHA ) * dot( fmin( fabs( diffg ) , GRADMAXDIFF ), ( float4 ) 1.0f );

                    diffc = ( val1 - val2xp ).xyz;
                    diffg = gval1 - gval2xp;
                    float Cxp = COLORGRADALPHA * dot( fmin( fabs( diffc ), COLORMAXDIFF ), ( float3 ) 1.0f ) + ( 1.0f - COLORGRADALPHA ) * dot( fmin( fabs( diffg ) , GRADMAXDIFF ), ( float4 ) 1.0f );

                    diffc = ( val1 - val2xm ).xyz;
                    diffg = gval1 - gval2xm;
                    float Cxm = COLORGRADALPHA * dot( fmin( fabs( diffc ), COLORMAXDIFF ), ( float3 ) 1.0f ) + ( 1.0f - COLORGRADALPHA ) * dot( fmin( fabs( diffg ) , GRADMAXDIFF ), ( float4 ) 1.0f );

                    if( Cxp > C && Cxm > C ) {
                        float3 pos3 = ( float3 ) ( coord + displace, 1.0f );
                        J = J + w1 * ( -C + Cxp ) * 0.5f * pos3;
                        //H = H + w1 * ( Cxp + Cxm - 2.0f * C ) * pos3 * pos3;

                        mat3f_outer( &mtmp, pos3, pos3 );
                        mat3f_mul_scalar( &mtmp, w1 * ( Cxp + Cxm - 2.0f * C ), &mtmp );
                        mat3f_add( &H, &H, &mtmp );
                    }
                }
            }
        }
    }


    return state /*+ ( float4 ) ( J / H, 1e5f ); */ + ( float4 ) ( mat3f_lusolve( &H, J ), 1e5f );
}

inline float4 patch_census_klt2( read_only image2d_t colimg1, read_only image2d_t gradimg1,
                                             read_only image2d_t colimg2, read_only image2d_t gradimg2,
                                             const float2 coord, const float4 state, const int patchsize )
{
    int width = get_image_width( colimg2 );
    int height = get_image_height( colimg2 );
    float3 J = ( float3 ) 0.0f;
    //float3 H = ( float3 ) 0.0f;
    Mat3f H, mtmp;
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
    const float3 grayWeight =  ( float3 ) ( 0.2126f, 0.7152f, 0.0722f );
    const float alpha = 0.125f;

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
                float2 pos2 = nd_state_transform( state, pos );

                if( !( pos.x < 1 || pos.x >= width - 1 ) ) {

                    /* C */
                    float3 val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF ).xyz;
                    float4 gval2 = read_imagef( gradimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF );

                    float3 c = fmin( fabs( val2 - val1 ), ( float3 ) 0.05f ) / 0.05f;
                    float C = w1 * alpha * 0.33f * ( c.x + c.y + c.z );

                    int4 ga = signbit(  gval1 );
                    int4 gb = signbit(  gval2 );
                    uint4 gc = abs( ga - gb );
                    C += w1 * ( 1.0f - alpha ) * 0.25 * ( float )( gc.x + gc.y + gc.z + gc.w );

                    /* Cxp */
                    val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + ( float2 ) ( 1.0f, 0.0f ) + OFFSETHALF ).xyz;
                    gval2 = read_imagef( gradimg2, SAMPLER_BILINEAR, pos2 + ( float2 ) ( 1.0f, 0.0f ) + OFFSETHALF );

                    c = fmin( fabs( val2 - val1 ), ( float3 ) 0.05f ) / 0.05f;
                    float Cxp = w1 * alpha * 0.33f * ( c.x + c.y + c.z );

                    gb = signbit(  gval2 );
                    gc = abs( ga - gb );
                    Cxp += w1 * ( 1.0f - alpha ) * 0.25 * ( float )( gc.x + gc.y + gc.z + gc.w );

                    /* Cxm */
                    val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + ( float2 ) ( -1.0f, 0.0f ) + OFFSETHALF ).xyz;
                    gval2 = read_imagef( gradimg2, SAMPLER_BILINEAR, pos2 + ( float2 ) ( -1.0f, 0.0f ) + OFFSETHALF );

                    c = fmin( fabs( val2 - val1 ), ( float3 ) 0.05f ) / 0.05f;
                    float Cxm = w1 * alpha * 0.33f * ( c.x + c.y + c.z );

                    gb = signbit(  gval2 );
                    gc = abs( ga - gb );
                    Cxm += w1 * ( 1.0f - alpha ) * 0.25 * ( float )( gc.x + gc.y + gc.z + gc.w );

                    if( ( Cxp + Cxm - 2.0f * C ) > 0.0f ) {
                        float3 pos3 = ( float3 ) ( coord + displace, 1.0f );
                        J = J + w1 * ( Cxm - Cxp ) * 0.5f * pos3;
                        // H = H + w1 * ( Cxp + Cxm - 2.0f * C ) * pos3 * pos3;

                        mat3f_outer( &mtmp, pos3, pos3 );
                        mat3f_mul_scalar( &mtmp, w1 * ( Cxp + Cxm - 2.0f * C ), &mtmp );
                        mat3f_add( &H, &H, &mtmp );
                    }
                }
            }
        }
    }

    //return state - ( float4 ) ( J / H, 1e5f );
    return state - ( float4 ) ( mat3f_lusolve( &H, J ), 1e5f );
}


inline float4 patch_census_klt( read_only image2d_t colimg1, read_only image2d_t gradimg1,
                                             read_only image2d_t colimg2, read_only image2d_t gradimg2,
                                             const float2 coord, const float4 state, const int patchsize )
{
    int width = get_image_width( colimg2 );
    int height = get_image_height( colimg2 );
    float3 J = ( float3 ) 0.0f;
    //float3 H = ( float3 ) 0.0f;
    Mat3f H, mtmp;
    const float CENSUS_EPSILON = 0.001f;
    const float CENSUS_GRAD_EPSILON = 0.001f;
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
    const float4 grayWeight =  ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );

    if( !all(isfinite(state.xyz)))
        return 1e5f;

    mat3f_set_zero( &H );

    float3 valc1 = read_imagef( colimg1, SAMPLER_BILINEAR, coord + ( float2 ) ( 0.5f, 0.5f) ).xyz;
    float3 valc2 = read_imagef( colimg2, SAMPLER_BILINEAR, nd_state_transform( state, coord ) + ( float2 ) ( 0.5f, 0.5f) ).xyz;

    for( float dy = -patchsize; dy <= patchsize; dy+=1.0f ) {
        for( float dx = -patchsize; dx <= patchsize; dx+=1.0f ) {
            float2 displace = ( float2 ) ( dx * OVERSAMPLEX, dy * OVERSAMPLEY );
            //float2 displace = ( float2 ) ( dx + OVERSAMPLECUBE * pow( dx, 3 ), dy + OVERSAMPLECUBE * pow( dy, 3 ) );
            float2 pos = coord + displace;

            if( !( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height ) ) {

                float3 val1 = read_imagef( colimg1, SAMPLER_BILINEAR, pos + OFFSETHALF ).xyz - valc1;
                float4 gval1 = read_imagef( gradimg1, SAMPLER_BILINEAR, pos  + OFFSETHALF );

                float w1 = exp( -pow( fabs( dot( val1, grayWeight.xyz ) ), CENSUS_COLOR_EXPONENT ) * CENSUS_COLOR_WEIGHT );
                float2 pos2 = nd_state_transform( state, pos );

                if( !( pos.x < 1 || pos.x >= width - 1 ) ) {

#define TOGRAY(x) dot( x, grayWeight )

                    float3 val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF ).xyz - valc2;
                    int3 a = as_int3( as_uint3( isgreaterequal( fabs( val1 ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val1 ) >> 31 ) << 1 )  ) );
                    int3 b = as_int3( as_uint3( isgreaterequal( fabs( val2 ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val2 ) >> 31 ) << 1 )  ) );
                    uint3 c = abs( b - a );
                    float C = 0.5f * 0.33f * ( c.x + c.y + c.z );

                    //float4 gval2 = image_grad( colimg2, pos2, state );
                    float4 gval2 = read_imagef( gradimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF );
                    int4 ga = as_int4( as_uint4( isgreaterequal( fabs( gval1 ), CENSUS_GRAD_EPSILON ) ) & as_uint4( ( int4 ) 1 - as_int4( ( as_uint4( gval1 ) >> 31 ) << 1 )  ) );
                    int4 gb = as_int4( as_uint4( isgreaterequal( fabs( gval2 ), CENSUS_GRAD_EPSILON ) ) & as_uint4( ( int4 ) 1 - as_int4( ( as_uint4( gval2 ) >> 31 ) << 1 )  ) );
                    uint4 gc = abs( gb - ga );
                    C += 0.5f * 0.25 * ( float )( gc.x + gc.y + gc.z + gc.w );

                    val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + ( float2 ) ( 1.0f, 0.0f ) + OFFSETHALF ).xyz - valc2;
                    a = as_int3( as_uint3( isgreaterequal( fabs( val1 ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val1 ) >> 31 ) << 1 )  ) );
                    b = as_int3( as_uint3( isgreaterequal( fabs( val2 ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val2 ) >> 31 ) << 1 )  ) );
                    c = abs( b - a );
                    float Cxp = 0.5f * 0.33f * ( c.x + c.y + c.z );

                    //gval2 = image_grad( colimg2, pos2 + ( float2 ) ( 1.0f, 0.0f ), state );
                    gval2 = read_imagef( gradimg2, SAMPLER_BILINEAR, pos2 + ( float2 ) ( 1.0f, 0.0f ) + OFFSETHALF );
                    ga = as_int4( as_uint4( isgreaterequal( fabs( gval1 ), CENSUS_GRAD_EPSILON ) ) & as_uint4( ( int4 ) 1 - as_int4( ( as_uint4( gval1 ) >> 31 ) << 1 )  ) );
                    gb = as_int4( as_uint4( isgreaterequal( fabs( gval2 ), CENSUS_GRAD_EPSILON ) ) & as_uint4( ( int4 ) 1 - as_int4( ( as_uint4( gval2 ) >> 31 ) << 1 )  ) );
                    gc = abs( gb - ga );
                    Cxp += 0.5f * 0.25 * ( float )( gc.x + gc.y + gc.z + gc.w );

                    val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + ( float2 ) ( -1.0f, 0.0f ) + OFFSETHALF ).xyz - valc2;
                    a = as_int3( as_uint3( isgreaterequal( fabs( val1 ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val1 ) >> 31 ) << 1 )  ) );
                    b = as_int3( as_uint3( isgreaterequal( fabs( val2 ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val2 ) >> 31 ) << 1 )  ) );
                    c = abs( b - a );
                    float Cxm = 0.5f * 0.33f * ( c.x + c.y + c.z );

                    //gval2 = image_grad( colimg2, pos2 + ( float2 ) ( -1.0f, 0.0f ), state );
                    gval2 = read_imagef( gradimg2, SAMPLER_BILINEAR, pos2 + ( float2 ) ( -1.0f, 0.0f ) + OFFSETHALF );
                    ga = as_int4( as_uint4( isgreaterequal( fabs( gval1 ), CENSUS_GRAD_EPSILON ) ) & as_uint4( ( int4 ) 1 - as_int4( ( as_uint4( gval1 ) >> 31 ) << 1 )  ) );
                    gb = as_int4( as_uint4( isgreaterequal( fabs( gval2 ), CENSUS_GRAD_EPSILON ) ) & as_uint4( ( int4 ) 1 - as_int4( ( as_uint4( gval2 ) >> 31 ) << 1 )  ) );
                    gc = abs( gb - ga );
                    Cxm += 0.5f * 0.25 * ( float )( gc.x + gc.y + gc.z + gc.w );


                    float3 pos3 = ( float3 ) ( coord + displace, 1.0f );
                    J = J + w1 * ( -C + Cxp ) * 0.5f * pos3;
                    //H = H + w1 * ( Cxp + Cxm - 2.0f * C ) * pos3 * pos3;

                    mat3f_outer( &mtmp, pos3, pos3 );
                    mat3f_mul_scalar( &mtmp, w1 * ( Cxp + Cxm - 2.0f * C ), &mtmp );
                    mat3f_add( &H, &H, &mtmp );
                }
            }
        }
    }


    return state /*+ ( float4 ) ( J / H, 1e5f ); */ + ( float4 ) ( mat3f_lusolve( &H, J ), 1e5f );
}


inline float patch_eval_color_grad_weighted( read_only image2d_t colimg1, read_only image2d_t gradimg1,
                                      read_only image2d_t colimg2, read_only image2d_t gradimg2,
                                      const float2 coord, const float4 state, const int patchsize, const int lr )
{
    float wsum1 = 0;
    float ret1 = 0;
    int width = get_image_width( colimg2 );
    int height = get_image_height( colimg2 );
    const float4 grayWeight =  ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );

    float4 valcenter = read_imagef( colimg1, SAMPLER_BILINEAR, coord + ( float2 ) ( 0.5f, 0.5f) );

    for( float dy = -patchsize; dy <= patchsize; dy+=1.0f ) {
        for( float dx = -patchsize; dx <= patchsize; dx+=1.0f ) {
            float2 displace = ( float2 ) ( dx * OVERSAMPLEX, dy * OVERSAMPLEY );
            //float2 displace = ( float2 ) ( dx + OVERSAMPLECUBE * pow( dx, 3 ), dy + OVERSAMPLECUBE * pow( dy, 3 ) );
            float2 pos = coord + displace;

            if( !( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height ) ) {

                float4 val1 = read_imagef( colimg1, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f, 0.5f));
                float4 gval1 = read_imagef( gradimg1, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f, 0.5f));

                float w1 = native_exp( -pow( fabs( dot( valcenter.xyz - val1.xyz, grayWeight.xyz ) ), 1.0f ) * 16.0f );// ( smoothstep( 0.0f, length( ( float2 ) patchsize ), length( displace ) ) * COLORWEIGHT + COLORWEIGHTBIAS ) );// * exp( -fast_length( displace ) * 0.05f );
                //float w1 = native_exp( -pow( dot( fabs( valcenter.xyz - val1.xyz ), grayWeight.xyz ), COLORWEIGHTBETA ) * ( smoothstep( 0.0f, length( ( float2 ) patchsize ), length( displace ) ) * COLORWEIGHT + COLORWEIGHTBIAS ) );// * exp( -fast_length( displace ) * 0.05f );

                //          float w1 = exp( -dot( fabs( valcenter.xyz - val1.xyz ), ( float3 ) 1.0f ) * COLORWEIGHT );// * exp( -fast_length( displace ) * 0.05f );

                // transform point
                //      float d = nd_state_transform( state, pos );
                //      pos.x += select( d, -d, lr );
                pos = nd_state_transform( state, pos );
                if( !( pos.x < 0 || pos.x >= width ) ) {

                    wsum1 += w1;

#define TOGRAY(x) dot( x, grayWeight )

                    float4 val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f, 0.5f));
                    float4 gval2 = read_imagef( gradimg2, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f, 0.5f));

                    float3 diffc = ( val1 - val2 ).xyz;
                    float4 diffg = gval1 - gval2;
                    //float C = COLORGRADALPHA * dot( fmin( fabs( diffc ), COLORMAXDIFF ), ( float3 ) 1.0f ) + ( 1.0f - COLORGRADALPHA ) * dot( fmin( fabs( diffg ), GRADMAXDIFF ), ( float4 ) 1.0f );
                    float C = COLORGRADALPHA * dot( fmin( fabs( diffc ), COLORMAXDIFF ), ( float3 ) 1.0f ) + ( 1.0f - COLORGRADALPHA ) * dot( fmin( fabs( diffg ) , GRADMAXDIFF ), ( float4 ) 1.0f );

                    //          float C = log( 0.25f * exp(-1.0f * fmin( fast_length( ( val1 - val2 ).xyz ), COLORMAXDIFF  ) ) + 0.75f * exp( -1.0f * fmin( fast_length( ( gval1 - gval2 ) ), GRADMAXDIFF ) ) ) / -1.0f;

                    ret1 += w1 * C;
                }
            }
        }
    }

    return ( ret1 / wsum1 );// * 0.9f + 0.1f / ( 1 + log( wsum1 ) );
}

inline float patch_eval_ncc_weighted( read_only image2d_t colimg1, read_only image2d_t gradimg1,
                                      read_only image2d_t colimg2, read_only image2d_t gradimg2,
                                      const float2 coord, const float4 state, const int patchsize, float mincost )
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

//  if( !all(isfinite(state.xy)))
//      return 1e5f;

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

            float w1 = native_exp( -pow( fabs( dot( valcenter.xyz - val1.xyz, grayWeight.xyz ) ), 1.0f ) * 16.0f );
            //float w1 = native_exp( -dot( fabs( valcenter.xyz - val1.xyz ), grayWeight.xyz ) * ( smoothstep( 0.0f, length( ( float2 ) ( patchsize, patchsize ) ), length( displace ) ) * 1.5f * COLORWEIGHT + COLORWEIGHTBIAS ) );// * exp( -fast_length( displace ) * 0.05f );
            pos = nd_state_transform( state, pos );
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

//  if( wsum1 <= 1.1f )
//      return 1e5f;

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
                          const float2 coord, const float4 state, const int patchsize, float mincost )
{
    float wsum = 0;
    float census = 0;
    int width = get_image_width( colimg2 );
    int height = get_image_height( colimg2 );
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
    const float3 grayWeight =  ( float3 ) ( 0.2126f, 0.7152f, 0.0722f );
    const float alpha = 0.125f;
    float m2 = 0;
    float m2sum = 0;

    float2 coord2 = nd_state_transform( state, coord );

    if( !all( isfinite( state.xyz ) ) || nd_state_to_normal( state ).z < NORMAL_Z_MIN || coord2.x < 0 || coord2.x >= width )
        return 1e5f;

    float4 in = read_imagef( colimg1, SAMPLER_BILINEAR, coord + OFFSETHALF );
    float bilw = in.w;
    float3 valc1 = in.xyz;
    float3 valc2 = read_imagef( colimg2, SAMPLER_BILINEAR, coord2 + OFFSETHALF ).xyz;

    for( int dy = -patchsize; dy <= patchsize; dy++ ) {
#pragma unroll
        for( int dx = -patchsize; dx <= patchsize; dx++ ) {
            float2 displace = ( float2 ) ( ( ( float ) dx ) * OVERSAMPLEX, ( ( float ) dy ) * OVERSAMPLEY );
            //float2 displace = ( float2 ) ( dx + OVERSAMPLECUBE * pow( ( float ) dx, 3 ), dy + OVERSAMPLECUBE * pow( ( float ) dy, 3 ) );
            float2 pos = coord + displace;

            float2 pos2 = nd_state_transform( state, pos );
            if( pos2.x < 0 || pos2.x >= width ||
                pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height ) {
                continue;
            }

            float3 valin = read_imagef( colimg1, SAMPLER_BILINEAR, pos + OFFSETHALF ).xyz;
            float3 val1 = valin - valc1;

            float w = exp( -pow( fabs( dot( val1, grayWeight ) ), CENSUS_COLOR_EXPONENT ) * CENSUS_COLOR_WEIGHT );

            float4 gval1 = read_imagef( gradimg1, SAMPLER_BILINEAR, pos + OFFSETHALF );
            float4 gval2 = read_imagef( gradimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF );

            float3 val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF ).xyz;
            float3 c = fmin( fabs( val2 - valin ), ( float3 ) 0.05f ) / 0.05f;

            float cad = 0.33f * ( float )( c.x + c.y + c.z );
            float cost = alpha * cad;

            int4 ga = signbit(  gval1 );
            int4 gb = signbit(  gval2 );
            uint4 gc = abs( ga - gb );
            float cc =  0.25f * ( float )( gc.x + gc.y + gc.z + gc.w );
            cost += ( 1.0f - alpha ) * cc;

    //        m2 += dot( displace, displace ) * exp( -cost * w * 8.0f );
  //          m2sum += exp( -cost * w * 8.0f );

            census += w * cost;
            wsum += w;

            if( census / bilw >= mincost ) {
                return 1.0f;
            }
        }
    }

    if( wsum <= 1.0 )
        return 1.1f;

//    m2 /= m2sum;

    return ( census / wsum );// * 0.9f + 0.1f * ( 1.0f - exp( -pow( m2, 2.0f ) * 200.0f ) );
}


inline float patch_eval_softmin( read_only image2d_t colimg1, read_only image2d_t gradimg1,
                                 read_only image2d_t colimg2, read_only image2d_t gradimg2,
                                 const float2 coord, const float4 state, const int patchsize, float mincost )
{
    float c1 = patch_eval_color_grad_weighted( colimg1, gradimg1, colimg2, gradimg2, coord, state, patchsize, mincost );
    float c2 = patch_eval_census( colimg1, gradimg1, colimg2, gradimg2, coord, state, patchsize, mincost );
//    float c3 = patch_eval_color_grad_weighted( colimg1, gradimg1, colimg2, gradimg2, coord, state, 2, mincost );

    return -native_log( native_exp( -c1 ) + native_exp( -( c2 ) ) );
//    return -native_log( native_exp( -c1 ) + native_exp( -c2 ) + native_exp( -( c3 + 0.01f ) ) );
}



#define patch_eval patch_eval_census

kernel void pmhstereo_init( write_only image2d_t output, read_only image2d_t img1, read_only image2d_t img2, read_only image2d_t gimg1, read_only image2d_t gimg2, const int patchsize, const float disparitymax, const int lr )
{
    RNG rng;
    const int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const float2 coordf = ( float2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const int width = get_image_width( img1 );
    const int height = get_image_height( img1 );

    if( coord.x >= width || coord.y >= height )
        return;

    RNG_init( &rng, coord.y * width + coord.x, 3 );

    float4 ret = nd_state_init( &rng, coordf, lr, 1.0f, disparitymax );

    ret.w  = patch_eval( img1, gimg1, img2, gimg2, coordf, ret, patchsize, 1e10f );

    write_imagef( output, coord, ret );
}

kernel void pmhstereo_init_disparity( write_only image2d_t output, read_only image2d_t img1, read_only image2d_t img2, read_only image2d_t gimg1, read_only image2d_t gimg2, const int patchsize, const float disparitymax, const int lr, read_only image2d_t disparityimg )
{
    RNG rng;
    const int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const float2 coordf = ( float2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const int width = get_image_width( img1 );
    const int height = get_image_height( img1 );

    if( coord.x >= width || coord.y >= height )
        return;

    RNG_init( &rng, coord.y * width + coord.x, 3 );

    float disparity = read_imagef( disparityimg, SAMPLER_NN, coord ).x;
    float4 ret;
    if( disparity > 0 && lr )
        ret = nd_state_from_disparity_normal( coordf, disparity, ( float3 ) (0.0f, 0.0f, 1.0f ) );
    else if( disparity > 0 && !lr ) {
        ret = nd_state_from_disparity_normal( coordf, -disparity, ( float3 ) (0.0f, 0.0f, 1.0f ) );
    } else
        ret = nd_state_init( &rng, coordf, lr, 1.0f, disparitymax );

    ret.w  = patch_eval( img1, gimg1, img2, gimg2, coordf, ret, patchsize, 1e10f );

    write_imagef( output, coord, ret );
}

kernel void pmhstereo_init_disparity_normal( write_only image2d_t output, read_only image2d_t img1, read_only image2d_t img2, read_only image2d_t gimg1, read_only image2d_t gimg2, const int patchsize, const float disparitymax, const int lr, read_only image2d_t disparityimg, read_only image2d_t normalimg )
{
    RNG rng;
    const int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const float2 coordf = ( float2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const int width = get_image_width( img1 );
    const int height = get_image_height( img1 );

    if( coord.x >= width || coord.y >= height )
        return;

    RNG_init( &rng, coord.y * width + coord.x, 3 );

    float disparity = read_imagef( disparityimg, SAMPLER_NN, coord ).x;
    float3 normal   = read_imagef( normalimg, SAMPLER_NN, coord ).xyz;
    float4 ret;
    if( disparity > 0 && lr )
        ret = nd_state_from_disparity_normal( coordf, disparity, normalize( normal ) );
    else if( disparity > 0 && !lr ) {
        ret = nd_state_from_disparity_normal( coordf, -disparity, normalize( normal ) );
    } else
        ret = nd_state_init( &rng, coordf, lr, 1.0f, disparitymax );

    ret.w  = patch_eval( img1, gimg1, img2, gimg2, coordf, ret, patchsize, 1e10f );

    write_imagef( output, coord, ret );
}

typedef struct {
    int n;
    float4 value[ VIEWSAMPLES ];// __attribute__ ((packed));
} VIEWPROP_t;

kernel void pmhstereo_viewbuf_clear( global VIEWPROP_t* vbuf, const int width, const int height )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );

    if( gx >= width || gy >= height )
        return;

    vbuf[ width * gy + gx ].n = 0;
}

inline float2 smoothDistance( const float4 statea, const float4 stateb, const float4 smooth, const float2 coord, const float disparitymax, int lr )
{
  float2 ret;
  float4 a = nd_state_to_ref_normal_depth( statea, coord, lr );
  float4 b = nd_state_to_ref_normal_depth( stateb, coord, lr );
//  float scale = fabs( smooth.w / disparitymax ) + 0.5f;
  const float4 diag = ( float4 ) ( 0.1f, 0.1f, 0.1f, 1.0f / ( disparitymax * disparitymax ) ) / ( 1.0f / ( disparitymax * disparitymax ) + 0.3f );

//  if( !all( isfinite( statea.xyz ) ) || !all( isfinite( stateb.xyz ) ) )
//    return ( float2 ) 0.0f;

//    a.z = TOSMOOTH( a.z );
//    b.z = TOSMOOTH( b.z );
//  a.w /= disparitymax;
//  b.w /= disparitymax;
//  a.w =  1.0f / ( ( a.w * INVSCALE ) + INVEPS );
//  b.w =  1.0f / ( ( b.w * INVSCALE ) + INVEPS );

  float4 da = a - smooth;
  ret.x = dot( da, diag * da );

  float4 db = b - smooth;
  ret.y = dot( db, diag * db );

  return ret;
}

inline float minimalCost( const float4 neighbour, const float4 self, const float4 smooth, const float2 coordf, float theta, float disparitymax, int lr )
{
    float2 sdist = smoothDistance( neighbour, self, smooth, coordf, disparitymax, lr );
    return self.w + theta * sdist.y - theta * sdist.x;
}


kernel void pmhstereo_propagate_view( write_only image2d_t output, read_only image2d_t old,
                                read_only image2d_t img1, read_only image2d_t img2,
                                read_only image2d_t gimg1, read_only image2d_t gimg2, read_only image2d_t imsmoooth, const float theta,
                                const int patchsize, const float disparitymax, const int lr, const int iter,
                                global VIEWPROP_t* viewin, global VIEWPROP_t* viewout /*, write_only image2d_t kltimg*/ )
{
    RNG rng;
    const int width = get_image_width( img1 );
    const int height = get_image_height( img1 );
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
    const int2 base = ( int2 )( get_group_id( 0 ) * lw - PROPSIZE, get_group_id( 1 ) * lh - PROPSIZE );
    const int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const float2 coordf = ( float2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    float4 self, neighbour, smooth;
    float mincost;
    const int2 offsets[] = {
        ( int2 ) (  1,  0 ),
        ( int2 ) ( -16,   0 ),
        ( int2 ) ( -1,  0 ),
        ( int2 ) (  16,   0 ),
        ( int2 ) (  0,  1 ),
        ( int2 ) (   0, -16 ),
        ( int2 ) (  0, -1 ),
        ( int2 ) (   0,  16 ),
    };

#if 0
    local float4 buf[ 16 + 2 * PROPSIZE ][ 16 + 2 * PROPSIZE ];
//  float theta = 0.1f * _theta;

    for( int y = ly; y < lh + 2 * PROPSIZE; y += lh ) {
        for( int x = lx; x < lw + 2 * PROPSIZE; x += lw ) {
            buf[ y ][ x ] = read_imagef( old, SAMPLER_NN, base + ( int2 )( x, y ) );
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );
#endif

    if( gx >= width || gy >= height )
        return;

    smooth = read_imagef( imsmoooth, SAMPLER_NN, coord );
    smooth.w = clamp( smooth.w, 0.0f, 1.0f ) * disparitymax;
    //smooth = nd_state_from_disparity_normal( coordf,  ( clamp( smooth.w, 0.0f, 1.1f ) * disparitymax ), normalize( smooth.xyz ) );
    //if( !lr )
    //    smooth = nd_state_viewprop( smooth );

    //smooth.w = 0;
    //float nfactor = fmax( length( smooth.xy ), 1.0f );
    //smooth.xy = smooth.xy / nfactor;
    ////smooth.z = ( ( 1.0f / smooth.z ) - 0.001f );// * disparitymax;
    //smooth = ( float4 ) ( smooth.x, smooth.y, native_sqrt( 1.0f - smooth.x * smooth.x - smooth.y * smooth.y ), smooth.z );

    self =  read_imagef( old, SAMPLER_NN, coord ); //buf[ ly + PROPSIZE ][ lx + PROPSIZE ];
    self.w  = patch_eval( img1, gimg1, img2, gimg2, coordf, self, patchsize, 1e10f );

    RNG_init( &rng, ( coord.y * width + coord.x ) + iter, ( ( ( 2 * PROPSIZE + 1 ) * ( 2 * PROPSIZE + 1 ) - 1 ) + NUMRNDTRIES ) * 3 + 2 * NUMRNDSAMPLE );

    // sample the nd_state of the neighbours
#if 0
    for( int py = -PROPSIZE; py <= PROPSIZE; py++ ) {
        for( int px = -PROPSIZE; px <= PROPSIZE; px++ ) {

            if( px == 0 && py == 0 )
                continue;

            neighbour = buf[ ly + PROPSIZE + py ][ lx + PROPSIZE + px ];
            neighbour.w  = patch_eval( img1, gimg1, img2, gimg2, coordf, neighbour, patchsize, lr );

            float2 sdist = smoothDistance( neighbour, self, smooth, coordf, disparitymax, lr );
            if( neighbour.w + theta * sdist.x <= self.w + theta * sdist.y ) self = neighbour;
//          if( neighbour.w  <= self.w ) self = neighbour;

        }
    }
#endif

    // try smooth
#if 1
    float2 sdist;
//  neighbour = ( float4 ) ( 1.0f, 0.0f, 0.0f, 0.0f ) - ( float4 ) ( - smooth.x / smooth.z, - smooth.y / smooth.z, ( smooth.x * coordf.x + smooth.y * coordf.y ) / smooth.z + smooth.w, 0.0f );
//    neighbour = select( neighbour, nd_state_viewprop( neighbour ), ( int4 ) -(!lr) );
    neighbour = nd_state_from_disparity_normal( coordf, smooth.w * ( lr ? -1.0f : 1.0f ), normalize( smooth.xyz ) );
    if( !lr )
        neighbour = nd_state_viewprop( neighbour );

    mincost = minimalCost( neighbour, self, smooth, coordf, theta, disparitymax, lr );
    neighbour.w  = patch_eval( img1, gimg1, img2, gimg2, coordf, neighbour, patchsize, mincost );

    sdist = smoothDistance( neighbour, self, smooth, coordf, disparitymax, lr );
    self = select( self, neighbour, ( int4 ) -( neighbour.w + theta * sdist.x <= self.w + theta * sdist.y ) );
#endif

    // rand neighbourhood tries
#if 1
#pragma unroll
    for( int i = 0; i < NUMRNDSAMPLE; i++ ) {
            neighbour = read_imagef( old, SAMPLER_NN, (int2)(gx,gy) + ( int2 )( ( RNG_float(&rng) - 0.5f  ) * 2.0f * NUMRNDSAMPLERADIUS + 0.5f, ( RNG_float(&rng) - 0.5f ) * 2.0f * NUMRNDSAMPLERADIUS + 0.5f ) );
            //neighbour = read_imagef( old, SAMPLER_NN, coord + offsets[ ( iter + i ) & 0x07 ] );

            mincost = minimalCost( neighbour, self, smooth, coordf, theta, disparitymax, lr );
            neighbour.w  = patch_eval( img1, gimg1, img2, gimg2, coordf, neighbour, patchsize, mincost );

            float2 sdist = smoothDistance( neighbour, self, smooth, coordf, disparitymax, lr );
            self = select( self, neighbour, ( int4 ) -( neighbour.w + theta * sdist.x <= self.w + theta * sdist.y ) );
    }
#endif

    // random try
#if 1
    neighbour = nd_state_init( &rng, coordf, lr, 1.0f, disparitymax );

    mincost = minimalCost( neighbour, self, smooth, coordf, theta, disparitymax, lr );
    neighbour.w  = patch_eval( img1, gimg1, img2, gimg2, coordf, neighbour, patchsize, mincost );

    sdist = smoothDistance( neighbour, self, smooth, coordf, disparitymax, lr );
    self = select( self, neighbour, ( int4 ) -( neighbour.w + theta * sdist.x <= self.w + theta * sdist.y ) );
#endif

    // try other view
#if 1
    int nview = min( viewin[ width * gy + gx ].n, ( int ) VIEWSAMPLES );
#pragma unroll
    for( int i = 0; i < nview; i++ ) {
        neighbour = viewin[ width * gy + gx ].value[ i ];
        mincost = minimalCost( neighbour, self, smooth, coordf, theta, disparitymax, lr );
        neighbour.w  = patch_eval( img1, gimg1, img2, gimg2, coordf, neighbour, patchsize, mincost );

        float2 sdist = smoothDistance( neighbour, self, smooth, coordf, disparitymax, lr );
        self = select( self, neighbour, ( int4 ) -( neighbour.w + theta * sdist.x <= self.w + theta * sdist.y ) );
    }
#endif

#if 1
    // randomized refinement
    for( int i = 0; i < NUMRNDTRIES - 1; i++ ) {
        neighbour = nd_state_refine( &rng, self, coordf, disparitymax, lr );
        mincost = minimalCost( neighbour, self, smooth, coordf, theta, disparitymax, lr );
        neighbour.w  = patch_eval( img1, gimg1, img2, gimg2, coordf, neighbour, patchsize, mincost );

        float2 sdist = smoothDistance( neighbour, self, smooth, coordf, disparitymax, lr );
        self = select( self, neighbour, ( int4 ) -( neighbour.w + theta * sdist.x <= self.w + theta * sdist.y ) );
    }
#endif


#if 1
    // KLT step
    {
        const int klt_patchsize = patchsize;
        neighbour = patch_census_klt2( img1, gimg1, img2, gimg2, coordf, self, klt_patchsize );
        for( int kltiter = 0; kltiter < 5; kltiter++ )
            neighbour = patch_census_klt2( img1, gimg1, img2, gimg2, coordf, neighbour, klt_patchsize );
        mincost = minimalCost( neighbour, self, smooth, coordf, theta, disparitymax, lr );
        neighbour.w  = patch_eval( img1, gimg1, img2, gimg2, coordf, neighbour, klt_patchsize, mincost );


        float2 sdist = smoothDistance( neighbour, self, smooth, coordf, disparitymax, lr );
//        if( neighbour.w + theta * sdist.x <= self.w + theta * sdist.y )
//            write_imagef( kltimg, coord, ( float4 ) 1.0f );
//        else
//            write_imagef( kltimg, coord, ( float4 ) 0.0f );

//        if( neighbour.w + theta * sdist.x <= self.w + theta * sdist.y )
//            printf("%d %d: KLT step\n", get_global_id( 0 ), get_global_id( 1 ) );

        self = select( self, neighbour, ( int4 ) -( neighbour.w + theta * sdist.x <= self.w + theta * sdist.y ) );
    }
#endif


    // store view prop result
    // maybe inconsistent r/w access - but random anyway
#if 1
    int sidesign = lr ? -1 : 1;
    int xposview = convert_int_rte(  nd_state_transform( self, coordf ).x );
    int disp = convert_int( ( nd_state_transform( self, ( float2 ) ( coord.x, coord.y ) ).x - coord.x ) * ( float ) sidesign );
    if( xposview >= 0 && xposview < width && disp > 0 ) {
        int nold = atomic_inc( &viewout[ gy * width + xposview ].n );
        if( nold < VIEWSAMPLES )
            viewout[ gy * width + xposview ].value[ nold ] = nd_state_viewprop( self );
    }
#endif

    write_imagef( output, coord, self );
}

kernel void pmhstereo_depthmap( write_only image2d_t depthmap, read_only image2d_t old, const float scale )
{
    int2 coord;
    const int width = get_image_width( depthmap );
    const int height = get_image_height( depthmap );

    coord.x = get_global_id( 0 );
    coord.y = get_global_id( 1 );

    if( coord.x >= width || coord.y >= height )
        return;

    float4 state = read_imagef( old, SAMPLER_NN, coord );
    float4 val;
    val.xyz = ( float3 ) fabs( nd_state_transform( state, ( float2 ) ( coord.x, coord.y ) ).x - ( float ) coord.x );
    if( !all(isfinite(state.xyz)))
        val.xyz = ( float3 ) ( 0.0f, 0.0f, 0.0f );
    val.xyz *= scale;
    val.w = 1.0f;
    write_imagef( depthmap, coord, val );
}

kernel void pmhstereo_consistency( write_only image2d_t output, read_only image2d_t left, read_only image2d_t right, const float maxdispdiff, const float maxnormaldegdiff, int lr )
{
    int2 coord;
    const int width = get_image_width( output );
    const int height = get_image_height( output );

    coord.x = get_global_id( 0 );
    coord.y = get_global_id( 1 );

    if( coord.x >= width || coord.y >= height )
        return;

    float4 statel = read_imagef( left, SAMPLER_NN, coord );
    float2 coord2 = nd_state_transform( statel, ( float2 ) ( coord.x, coord.y ) );
    float4 stater;
    float4 stater2;
    float4 val = ( float4 ) ( 0.0f, 0.0f, 0.0f, -1.0f );

    if( ceil( coord2.x ) >= 0 && floor( coord2.x ) < width ) {
        stater = read_imagef( right, SAMPLER_NN, ( int2 ) ( convert_int_rte( floor( coord2.x ) ), coord.y ) );
        stater2 = read_imagef( right, SAMPLER_NN, ( int2 ) ( convert_int_rte( ceil( coord2.x ) ), coord.y ) );

        float ndiff = dot( nd_state_to_normal( statel ), nd_state_to_normal( nd_state_viewprop( stater ) ) );
        float ndiff2 = dot( nd_state_to_normal( statel ), nd_state_to_normal( nd_state_viewprop( stater2 ) ) );

        float dmax = fabs( ( float ) coord.x - nd_state_transform( stater, coord2 ).x );
        float dmax2 = fabs( ( float ) coord.x - nd_state_transform( stater2, coord2 ).x );

        if( !( dmax  >= maxdispdiff || acospi( clamp( ndiff,  -1.0f, 1.0f ) ) >= maxnormaldegdiff / 180.0f ) ||
            !( dmax2 >= maxdispdiff || acospi( clamp( ndiff2, -1.0f, 1.0f ) ) >= maxnormaldegdiff / 180.0f ) )
            val = statel;
    }

    write_imagef( output, coord, val );
}

kernel void pmhstereo_fill_state( write_only image2d_t output, read_only image2d_t input, read_only image2d_t otherimg, const float dispscale, int lr )
{
    const int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const int width = get_image_width( output );
    const int height = get_image_height( output );
    const float3 grayWeight =  ( float3 ) ( 0.2126f, 0.7152f, 0.0722f );
    int sidesign = lr ? -1 : 1;

    if( coord.x >= width || coord.y >= height )
        return;

    float4 val;
    float4 state = read_imagef( input, SAMPLER_NN, coord );
    float4 other = read_imagef( otherimg, SAMPLER_NN, coord );

    float disp = ( nd_state_transform( state, ( float2 ) ( coord.x, coord.y ) ).x - coord.x ) * ( float ) sidesign;
    if( state.w < 0 || disp < 0 || disp > width ) {
        float4 left = ( float4 ) -1.0f;
        int lx, rx;

        int x = coord.x - 1;
        while( left.w < 0 && x >= 0 ) {
            left = read_imagef( input, SAMPLER_NN, ( int2 ) ( x, coord.y ) );
            lx = x;
            x--;
        }

        float4 right = ( float4 ) -1.0f;
        x = coord.x + 1;
        while( right.w < 0 && x < width ) {
            right = read_imagef( input, SAMPLER_NN, ( int2 ) ( x, coord.y ) );
            rx = x;
            x++;
        }

        if( left.w >= 0 && right.w >= 0 ) {
            //int dl = sidesign * ( nd_state_transform( left, ( float2 ) ( lx, coord.y ) ).x - lx );
            //int dr = sidesign * ( nd_state_transform( right, ( float2 ) ( rx, coord.y ) ).x - rx );
            int dl = sidesign * ( nd_state_transform( left, ( float2 ) ( coord.x, coord.y ) ).x - ( float ) coord.x );
            int dr = sidesign * ( nd_state_transform( right, ( float2 ) ( coord.x, coord.y ) ).x - ( float ) coord.x );
            dl = select( dl, width, dl <= 0 );
            dr = select( dr, width, dr <= 0 );
            val = select( right, left, ( uint4 ) -( dl <= dr ) );
        } else
            val = select( right, left, ( uint4 ) -( right.w < 0 ) );
    } else {
        val = state;
    }

    if( !all(isfinite( val.xy )) ) val.xy = ( float2 ) 0.0f;

    val = nd_state_to_ref_normal_depth( val, ( float2 ) ( coord.x, coord.y ), lr );
    val.w = ( val.w ) * dispscale;
    write_imagef( output, coord, val );
}

kernel void pmhstereo_fill_depthmap( write_only image2d_t output, read_only image2d_t input, const float scale )
{
    const int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const int width = get_image_width( output );
    const int height = get_image_height( output );

    if( coord.x >= width || coord.y >= height )
        return;

    float4 val;
    float4 state = read_imagef( input, SAMPLER_NN, coord );

    if( length( state.xyz ) < 1e-4f ) {
        float4 left = ( float4 ) 0;
        int x = coord.x - 1;
        while( length( left.xyz ) < 1e-1f && x >= 0 ) {
            left = read_imagef( input, SAMPLER_NN, ( int2 ) ( x, coord.y ) );
            x--;
        }

        float4 right = ( float4 ) 0;
        x = coord.x + 1;
        while( length( right.xyz ) < 1e-1f && x < width ) {
            right = read_imagef( input, SAMPLER_NN, ( int2 ) ( x, coord.y ) );
            x++;
        }

        if( length( left.xyz ) < 1e-1f ) left.xyz = ( float3 ) -1e5f;
        if( length( right.xyz ) < 1e-1f ) right.xyz = ( float3 ) -1e5f;

        left.w = -( nd_state_transform( left, ( float2 ) ( coord.x, coord.y ) ).x - ( float ) coord.x );
        right.w = -( nd_state_transform( right, ( float2 ) ( coord.x, coord.y ) ).x - ( float ) coord.x );

        val.xyz = ( float3 ) fmin( left.w, right.w );
        val.w = 0.0f;

    } else {
        val.xyz = fabs( nd_state_transform( state, ( float2 ) ( coord.x, coord.y ) ).x - ( float ) coord.x );
        val.w = 0.0f;
    }

    val.xyz *= scale;
    write_imagef( output, coord, val );
}

kernel void pmhstereo_to_disparity( write_only image2d_t output, read_only image2d_t input, const float scale, int lr )
{
    const int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const int width = get_image_width( output );
    const int height = get_image_height( output );
    int sidesign = lr ? -1 : 1;

    if( coord.x >= width || coord.y >= height )
        return;

    float4 val;
    float4 state = read_imagef( input, SAMPLER_NN, coord );

    float disp = ( nd_state_transform( state, ( float2 ) ( coord.x, coord.y ) ).x - coord.x ) * ( float ) sidesign;

    if( length( state.xyz ) < 1e-4f || disp < 0 || disp > width ) {
        val.xyz = 0.0f;
        val.w   = 0.0f;
    } else {
        val.xyz = fmax( disp, 0.0f );
        val.w   = 0.0f;
    }

    val.xyz *= scale;
    write_imagef( output, coord, val );
}

kernel void pmhstereo_fill_normalmap( write_only image2d_t output, read_only image2d_t input, int lr )
{
    const int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const int width = get_image_width( output );
    const int height = get_image_height( output );

    if( coord.x >= width || coord.y >= height )
        return;

    float4 val;
    float4 state = read_imagef( input, SAMPLER_NN, coord );

    if( length( state.xyz ) < 1e-1f ) {
        float4 left = ( float4 ) 0;
        int x = coord.x - 1;
        while( length( left.xyz ) < 1e-1f && x >= 0 ) {
            left = read_imagef( input, SAMPLER_NN, ( int2 ) ( x, coord.y ) );
            x--;
        }

        float4 right = ( float4 ) 0;
        x = coord.x + 1;
        while( length( right.xyz ) < 1e-1f && x < width ) {
            right = read_imagef( input, SAMPLER_NN, ( int2 ) ( x, coord.y ) );
            x++;
        }

        if( length( left.xyz ) < 1e-1f ) left.xyz = ( float3 ) -1e5f;
        if( length( right.xyz ) < 1e-1f ) right.xyz = ( float3 ) -1e5f;

        left.w = -( nd_state_transform( left, ( float2 ) ( coord.x, coord.y ) ).x - ( float ) coord.x );
        right.w = -( nd_state_transform( right, ( float2 ) ( coord.x, coord.y ) ).x - ( float ) coord.x );

        if( left.w < right.w )
            val.xyz = nd_state_to_normal( left );
        else
            val.xyz = nd_state_to_normal( right );

        val.w = 0.0f;

    } else {
        val.xyz = nd_state_to_normal( state );
        val.w = 0.0f;
    }

    write_imagef( output, coord, val );
}



kernel void pmhstereo_normalmap( write_only image2d_t normalmap, read_only image2d_t old )

{
    int2 coord;
    const int width = get_image_width( normalmap );
    const int height = get_image_height( normalmap );

    coord.x = get_global_id( 0 );
    coord.y = get_global_id( 1 );

    if( coord.x >= width || coord.y >= height )
        return;

    float4 self = read_imagef( old, SAMPLER_NN, coord );
    float4 val;
    val.xyz = nd_state_to_normal( self );
    val.w = 1.0f;
    write_imagef( normalmap, coord, val );
}

kernel void pmhstereo_colormap( write_only image2d_t normalmap, read_only image2d_t old, const float disparitymax )

{
    int2 coord;
    const int width = get_image_width( normalmap );
    const int height = get_image_height( normalmap );

    coord.x = get_global_id( 0 );
    coord.y = get_global_id( 1 );

    if( coord.x >= width || coord.y >= height )
        return;

    float4 self = read_imagef( old, SAMPLER_NN, coord );
    float4 val = nd_state_to_color( self, ( float2 ) ( coord.x, coord.y ), disparitymax );
    write_imagef( normalmap, coord, val );
}

kernel void pmhstereo_visualize_depth_normal( write_only image2d_t depth, write_only image2d_t normal, read_only image2d_t input, float dscale )
{
    int2 coord;
    const int width = get_image_width( input );
    const int height = get_image_height( input );

    coord.x = get_global_id( 0 );
    coord.y = get_global_id( 1 );

    if( coord.x >= width || coord.y >= height )
        return;

    float4 in = read_imagef( input, SAMPLER_NN, coord );
    write_imagef( depth, coord, ( float4 ) ( ( in.w ) * dscale ) );
    float3 n;
    n.xyz = in.xyz;
    write_imagef( normal, coord, ( float4 ) ( n * 0.5f + 0.5f, 1.0f ) );
}

kernel void pmhstereo_clear( write_only image2d_t output )

{
    int2 coord;
    const int width = get_image_width( output );
    const int height = get_image_height( output );

    coord.x = get_global_id( 0 );
    coord.y = get_global_id( 1 );

    if( coord.x >= width || coord.y >= height )
        return;

    write_imagef( output, coord, ( float4 ) 0.0f );
}



kernel void pmhstereo_normal_depth( write_only image2d_t output, read_only image2d_t input, const float dispscale, int lr )

{
    int2 coord;
    const int width = get_image_width( output );
    const int height = get_image_height( output );

    coord.x = get_global_id( 0 );
    coord.y = get_global_id( 1 );

    if( coord.x >= width || coord.y >= height )
        return;

    float4 self = read_imagef( input, SAMPLER_NN, coord );
    float4 val = nd_state_to_ref_normal_depth( self, ( float2 ) ( coord.x, coord.y ), lr );
    val.w = val.w * dispscale;

    write_imagef( output, coord, val );
}

kernel void pmhstereo_occmap( write_only image2d_t output, read_only image2d_t left, read_only image2d_t right, const float maxdispdiff, const float maxnormaldegdiff, int lr )
{
    int2 coord;
    const int width = get_image_width( output );
    const int height = get_image_height( output );

    coord.x = get_global_id( 0 );
    coord.y = get_global_id( 1 );

    if( coord.x >= width || coord.y >= height )
        return;

    float4 statel = read_imagef( left, SAMPLER_NN, coord );
    float2 coord2 = nd_state_transform( statel, ( float2 ) ( coord.x, coord.y ) );
    float4 stater;
    float4 stater2;
    float4 val = ( float4 ) ( 0.0f, 0.0f, 0.0f, 0.0f );

    if( ceil( coord2.x ) >= 0 && floor( coord2.x ) < width ) {
        stater = read_imagef( right, SAMPLER_NN, ( int2 ) ( convert_int_rte( floor( coord2.x ) ), coord.y ) );
        stater2 = read_imagef( right, SAMPLER_NN, ( int2 ) ( convert_int_rte( ceil( coord2.x ) ), coord.y ) );

        float ndiff = dot( nd_state_to_normal( statel ), nd_state_to_normal( nd_state_viewprop( stater ) ) );
        float ndiff2 = dot( nd_state_to_normal( statel ), nd_state_to_normal( nd_state_viewprop( stater2 ) ) );

        float dmax = fabs( ( float ) coord.x - nd_state_transform( stater, coord2 ).x );
        float dmax2 = fabs( ( float ) coord.x - nd_state_transform( stater2, coord2 ).x );

        if( !( dmax  >= maxdispdiff || acospi( clamp( ndiff,  -1.0f, 1.0f ) ) >= maxnormaldegdiff / 180.0f ) ||
            !( dmax2 >= maxdispdiff || acospi( clamp( ndiff2, -1.0f, 1.0f ) ) >= maxnormaldegdiff / 180.0f ) ) val = statel;
    }

    write_imagef( output, coord, select( ( float4 ) ( 0.0f, 0.0f, 0.0f, 1.0f ), ( float4 ) 1.0f, ( uint4 ) -( length( val ) > 1e-5f ) ) );
}

kernel void pmhstereo_lr_check( write_only image2d_t output, read_only image2d_t input1, read_only image2d_t input2, const float maxdiff, const float disparitymax, int lr )
{
    int2 coord;
    const int width = get_image_width( input1 );
    const int height = get_image_height( input1 );

    coord.x = get_global_id( 0 );
    coord.y = get_global_id( 1 );

    if( coord.x >= width || coord.y >= height )
        return;

    float dorig = read_imagef( input1, SAMPLER_NN, coord ).x;
    float d = disparitymax * dorig;

    float2 coord2 = ( float2 ) ( coord.x, coord.y );
    coord2.x += select( d, -d, lr );
    float d2 = disparitymax * read_imagef( input2, SAMPLER_BILINEAR, coord2 ).x;
    float4 out = ( float4 ) select( 0.0f, dorig /* * ( disparitymax * 4.0f / 256.0f )*/, fabs( d - d2) < maxdiff );
    out.w = 1.0f;
    write_imagef( output, coord, out );
}

__kernel void pmhstereo_bilateral_weight_to_alpha( __write_only image2d_t out, __read_only image2d_t src, int patchsize  )
{
    const int width = get_image_width( out );
    const int height = get_image_height( out );
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const float2 coord = ( float2 ) ( gx, gy );
    const float3 grayWeight =  ( float3 ) ( 0.2126f, 0.7152f, 0.0722f );
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
    float wsum = 0.0f;

    if( gx >= width || gy >= height )
        return;

    float3 valc1 = read_imagef( src, SAMPLER_BILINEAR, coord + OFFSETHALF ).xyz;

    for( int dy = -patchsize; dy <= patchsize; dy++ ) {
#pragma unroll
        for( int dx = -patchsize; dx <= patchsize; dx++ ) {
            float2 displace = ( float2 ) ( ( ( float ) dx ) * OVERSAMPLEX, ( ( float ) dy ) * OVERSAMPLEY );
            float2 pos = coord + displace;

            if( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height ) {
                continue;
            }

            float3 valin = read_imagef( src, SAMPLER_BILINEAR, pos + OFFSETHALF ).xyz;
            float3 val1 = valin - valc1;

            float w = exp( -pow( fabs( dot( val1, grayWeight ) ), CENSUS_COLOR_EXPONENT ) * CENSUS_COLOR_WEIGHT );

            wsum += w;
        }
    }

    write_imagef( out, ( int2 ) ( gx, gy ), ( float4 ) ( valc1, wsum ) );
}


__kernel void pmhstereo_weight( __write_only image2d_t out, __read_only image2d_t src  )
{
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
    const int width = get_image_width( out );
    const int height = get_image_height( out );
    const int2 base = ( int2 )( get_group_id( 0 ) * lw - 1, get_group_id( 1 ) * lh - 1 );
    const int bstride = lw + 2;
    local float buf[ 18 ][ 18 ];
    const float4 grayWeight =  ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );
//  const float4 grayWeight =  ( float4 ) ( 0.333f, 0.333f, 0.333f, 0.0f );

//  float4 c = read_imagef( src, sampler, ( int2 ) ( gx, gy ) );

    for( int y = ly; y < lh + 2; y += lh ) {
        for( int x = lx; x < lw + 2; x += lw ) {
            buf[ y ][ x ] = dot( read_imagef( src, sampler, base + ( int2 )( x, y ) ), grayWeight );
        }
    }
    barrier( CLK_LOCAL_MEM_FENCE );

#define BUF( x, y ) buf[ y + 1 ][ x + 1 ]

    if( gx >= width || gy >= height )
        return;

    float dx = ( BUF( lx + 1 , ly ) - BUF( lx - 1 , ly ) );// * 0.5f + ( BUF( lx + 1, ly - 1 ) - BUF( lx - 1, ly - 1  ) ) * 0.25f + ( BUF( lx + 1, ly + 1 ) - BUF( lx - 1, ly + 1 ) ) * 0.25f;
    float dy = ( BUF( lx, ly - 1 ) - BUF( lx, ly - 1 ) );// * 0.5f + ( BUF( lx - 1, ly + 1 ) - BUF( lx - 1, ly - 1 ) ) * 0.25f + ( BUF( lx + 1, ly + 1 ) - BUF( lx + 1, ly - 1 ) ) * 0.25f ;
//  float dxx = 0.125f * ( - BUF( lx + 1, ly ) * 0.5f - BUF( lx, ly + 1 ) * 0.5f + BUF( lx, ly ) );
//  float dyy = 0.125f * ( - BUF( lx - 1, ly ) * 0.5f - BUF( lx, ly - 1 ) * 0.5f +  BUF( lx, ly ) );
//  float dxy = BUF( lx + 1, ly + 1 ) - BUF( lx - 1, ly - 1 );
//  float dyx = BUF( lx - 1, ly + 1 ) - BUF( lx + 1, ly - 1 );

//  float lap = - BUF( lx, ly + 1 ) * 0.5f
//              - BUF( lx, ly - 1 ) * 0.5f
//              - BUF( lx + 1, ly ) * 0.25f
//              - BUF( lx - 1, ly ) * 0.25f
//              +  BUF( lx, ly );

    float w = exp(-6.0f * pow( sqrt(  dx * dx + dy * dy ), 0.8f ) ) + 0.0001f;
    write_imagef( out,( int2 )( gx, gy ), ( float4 ) ( w ) );
}

__kernel void pmhstereo_gradxy( __write_only image2d_t out, __read_only image2d_t src  )
{
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
    const int width = get_image_width( out );
    const int height = get_image_height( out );
    const int2 base = ( int2 )( get_group_id( 0 ) * lw - 1, get_group_id( 1 ) * lh - 1 );
//  const int bstride = lw + 2;
    float dx, dy;
    local float buf[ 18 ][ 18 ];
    const float4 grayWeight =  ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );
//  const float4 grayWeight =  ( float4 ) ( 0.333f, 0.333f, 0.333f, 0.0f );

//  float4 c = read_imagef( src, sampler, ( int2 ) ( gx, gy ) );

    for( int y = ly; y < lh + 2; y += lh ) {
        for( int x = lx; x < lw + 2; x += lw ) {
            buf[ y ][ x ] = dot( read_imagef( src, sampler, base + ( int2 )( x, y ) ), grayWeight );
        }
    }
    barrier( CLK_LOCAL_MEM_FENCE );

#define BUF( x, y ) buf[ y + 1 ][ x + 1 ]

    if( gx >= width || gy >= height )
        return;

//  dx = ( BUF( lx + 1, ly ) - BUF( lx - 1 , ly ) ) * 0.5f + ( BUF( lx + 1, ly - 1 ) - BUF( lx - 1, ly - 1  ) ) * 0.25f + ( BUF( lx + 1, ly + 1 ) - BUF( lx - 1, ly + 1 ) ) * 0.25f;
//  dy = ( BUF( lx, ly + 1 ) - BUF( lx, ly - 1 ) ) * 0.5f + ( BUF( lx - 1, ly + 1 ) - BUF( lx - 1, ly - 1 ) ) * 0.25f + ( BUF( lx + 1, ly + 1 ) - BUF( lx + 1, ly - 1 ) ) * 0.25f ;

    dx = ( BUF( lx + 1 , ly ) - BUF( lx , ly ) );
    dy = ( BUF( lx, ly + 1 ) - BUF( lx, ly  ) );

//  float dxx = ( - BUF( lx + 1, ly ) * 0.5f - BUF( lx - 1, ly ) * 0.5f + BUF( lx, ly ) );
//    float dyy = ( - BUF( lx, ly + 1 ) * 0.5f - BUF( lx, ly - 1 ) * 0.5f + BUF( lx, ly ) );

    float dxy = BUF( lx + 1, ly  ) - BUF( lx - 1 , ly  );
    float dyx = BUF( lx, ly + 1 ) - BUF( lx , ly - 1 );


//  float lap = - BUF( lx, ly + 1 ) * 0.5f
//              - BUF( lx, ly - 1 ) * 0.5f
//              - BUF( lx + 1, ly ) * 0.25f
//              - BUF( lx - 1, ly ) * 0.25f
//              +  BUF( lx, ly );

//  write_imagef( out,( int2 )( gx, gy ), ( float4 ) ( fmax( dx, 0.0f ), fmax( dy, 0.0f ), fmax( -dx, 0.0f ), fmax( -dy, 0.0f ) ) );
    write_imagef( out,( int2 )( gx, gy ), ( float4 ) ( dx, dy, dxy, dyx ) );
}

__kernel void pmhstereo_disparity_to_normal( __write_only image2d_t out, __read_only image2d_t src  )
{
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
    const int width = get_image_width( out );
    const int height = get_image_height( out );
    const int2 base = ( int2 )( get_group_id( 0 ) * lw - 1, get_group_id( 1 ) * lh - 1 );
    float dx, dy;
    local float buf[ 18 ][ 18 ];

    for( int y = ly; y < lh + 2; y += lh ) {
        for( int x = lx; x < lw + 2; x += lw ) {
            buf[ y ][ x ] = read_imagef( src, sampler, base + ( int2 )( x, y ) ).x;
        }
    }
    barrier( CLK_LOCAL_MEM_FENCE );

#define BUF( x, y ) buf[ y + 1 ][ x + 1 ]

    if( gx >= width || gy >= height )
        return;

    dx =  ( BUF( lx + 1 , ly ) - BUF( lx, ly ) );
    dy =  ( BUF( lx, ly + 1 ) - BUF( lx, ly ) );

    float4 normal = cross( ( float4 ) ( 1.0f, 0.0f, dx, 0.0f ), ( float4 ) ( 0.0f, 1.0f, dy, 0.0f ) );

    write_imagef( out,( int2 )( gx, gy ), ( float4 ) ( normalize( normal.xyz ), 0.0 ) );
}
