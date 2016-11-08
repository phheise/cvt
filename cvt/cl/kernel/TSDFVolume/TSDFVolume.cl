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

#import "../Matrix4.cl"

const sampler_t SAMPLER_LIN = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;
const sampler_t SAMPLER_NN = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void TSDFVolume_clear( global float2* cv, int width, int height, int depth, float value, float weight  )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int gz = get_global_id( 2 );
    global float2* cvptr = cv + gz * width * height + gy * width + gx;

    if( gx >= width || gy >= height || gz >= depth )
        return;

    *cvptr = ( float2 ) ( value, weight );
}

__kernel void TSDFVolume_setWeight( global float2* cv, int width, int height, int depth, float weight  )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int gz = get_global_id( 2 );
    global float2* cvptr = cv + gz * width * height + gy * width + gx;

    if( gx >= width || gy >= height || gz >= depth )
        return;

    *cvptr = ( float2 ) ( ( *cvptr ).x, weight );
}

__kernel void TSDFVolume_scaleWeight( global float2* cv, int width, int height, int depth, float scale  )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int gz = get_global_id( 2 );
    global float2* cvptr = cv + gz * width * height + gy * width + gx;

    if( gx >= width || gy >= height || gz >= depth )
        return;

    *cvptr = ( float2 ) ( ( *cvptr ).x, ( *cvptr ).y * scale );
}

__kernel void TSDFVolume_add( global float2* cv, int width, int height, int depth, read_only image2d_t dmap, float dscale,
                              const Mat4f TG2CAM, float truncation )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int gz = get_global_id( 2 );
    const int iwidth = get_image_width( dmap );
    const int iheight = get_image_height( dmap );
    global float2* cvptr = cv + ( gz * height + gy ) * width + gx;
    float4 gridpos = ( float4 ) ( gx, gy, gz, 1.0f );
    float4 gpos;
    float2 ipos;
    float d, sdf, tsdf, w;

    if( gx >= width || gy >= height || gz >= depth )
        return;

    /* use the transformation to transform grid position to camera coordinates*/
    gpos = mat4f_transform( &TG2CAM, gridpos );

    /* project into camera */
    ipos = gpos.xy / gpos.z; //+ ( float ) 0.5f;

    if( ipos.x < iwidth && ipos.y < iheight && ipos.x >= 0 && ipos.y >= 0  && gpos.z > 0 ) {
        d = read_imagef( dmap, SAMPLER_LIN, ipos + ( float2 ) ( 0.5f, 0.5f ) ).x * dscale;
        if( d > 0 ) {
            sdf = d - gpos.z;
            w = 1.0f;
            tsdf = clamp( sdf / truncation, -1.0f, 1.0f );
            if( sdf >= -truncation ) {
                    float2 old = *cvptr;
                    *cvptr = ( float2 ) ( ( old.x * old.y + tsdf * w  ) / ( old.y + w ), old.y + w );
            }
        }
    }
}

__kernel void TSDFVolume_addWeighted( global float2* cv, int width, int height, int depth, read_only image2d_t dmap, float dscale,
                                      const Mat4f TG2CAM, read_only image2d_t weight, float truncation )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int gz = get_global_id( 2 );
    const int iwidth = get_image_width( dmap );
    const int iheight = get_image_height( dmap );
    global float2* cvptr = cv + ( gz * height + gy ) * width + gx;
    float4 gridpos = ( float4 ) ( gx, gy, gz, 1.0f );
    float4 gpos;
    float2 ipos;
    float d, sdf, tsdf, w;

    if( gx >= width || gy >= height || gz >= depth )
        return;

    /* use the transformation to transform grid position to camera coordinates*/
    gpos = mat4f_transform( &TG2CAM, gridpos );

    /* project into camera */
    ipos = gpos.xy / gpos.z; //+ ( float ) 0.5f;

    if( ipos.x < iwidth && ipos.y < iheight && ipos.x >= 0 && ipos.y >= 0  && gpos.z > 0 ) {
        d = read_imagef( dmap, SAMPLER_LIN, ipos + ( float2 ) ( 0.5f, 0.5f ) ).x * dscale;
        if( d > 0 ) {
            sdf = d - gpos.z;
            if( sdf >= -truncation ) {
                    float2 old = *cvptr;
                    //old.y *= 0.9f;
                    w = read_imagef( weight, SAMPLER_LIN, ipos + ( float2 ) ( 0.5f, 0.5f ) ).x;
                    tsdf = clamp( sdf * 8.0f / truncation, -8.0f, 8.0f );

                    float diff = ( tsdf - cvptr->x );
                    w *= exp( -diff * diff * 0.1f ) * 0.95f + 0.05f;
                    *cvptr = ( float2 ) ( ( old.x * old.y + tsdf * w  ) / ( old.y + w ), old.y + w );
            }
        }
    }
}

__kernel void TSDFVolume_addSilhouette( global float2* cv, int width, int height, int depth, read_only image2d_t shilouette, const Mat4f TG2CAM )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int gz = get_global_id( 2 );
    const int iwidth = get_image_width( shilouette );
    const int iheight = get_image_height( shilouette );
    global float2* cvptr = cv + ( gz * height + gy ) * width + gx;
    float4 gridpos = ( float4 ) ( gx, gy, gz, 1.0f );
    float4 gpos;
    float2 fpos;
    int2 ipos;
    float d, sdf, tsdf;

    if( gx >= width || gy >= height || gz >= depth )
        return;

    /* use the transformation to transform grid position to camera coordinates*/
    gpos = mat4f_transform( &TG2CAM, gridpos );

    /* project into camera */
    fpos = gpos.xy / gpos.z; //+ ( float ) 0.5f;
    ipos = ( int2 ) ( fpos.x, fpos.y );

    if( ipos.x < iwidth && ipos.y < iheight && ipos.x >= 0 && ipos.y >= 0 ) {
        d = read_imagef( shilouette, SAMPLER_LIN, fpos + ( float2 ) ( 0.5f, 0.5f ) ).x > 0.0f;
        float2 old = *cvptr;
        // geometric mean, gm = \prod_{i=0}{N} a_i -> a_k = 0 with k \in (0...N) -> gm = 0
        // GMT: ( 0...1 ) -> -1 ... 1 and GMTINV( GMT( x ) ) = x
#define GMT( x ) ( -2.0f * ( x ) + 1.0f )
#define GMTINV( x ) ( -0.5f * ( ( x ) - 1.0f ) )
        *cvptr = ( float2 ) ( GMT( pow( GMTINV( old.x ), ( old.y ) / ( old.y + 1.0f ) ) * pow( d, 1.0f / ( old.y + 1.0f ) ) ), old.y + 1.0f );
#undef GMT
#undef GMTINV
    }
}

__kernel void TSDFVolume_sliceX( write_only image2d_t out, int v, global float2* cv, int width, int height, int depth  )
{
    const int gx = v;
    const int gy = get_global_id( 0 );
    const int gz = get_global_id( 1 );
    global float2* cvptr = cv + gz * width * height + gy * width + gx;

    if( gx >= width || gy >= height || gz >= depth )
        return;

    float4 val;
    val.xyz = clamp( ( ( *cvptr ).x ) + 0.5f, 0.0f, 1.0f );
    val.w   = 1.0f;
    write_imagef( out, ( int2 ) ( gy, gz ), val );
}

__kernel void TSDFVolume_sliceY( write_only image2d_t out, int v, global float2* cv, int width, int height, int depth  )
{
    const int gx = get_global_id( 0 );
    const int gy = v;
    const int gz = get_global_id( 1 );
    global float2* cvptr = cv + gz * width * height + gy * width + gx;

    if( gx >= width || gy >= height || gz >= depth )
        return;

    float4 val;
    val.xyz = clamp( ( ( *cvptr ).x ) + 0.5f, 0.0f, 1.0f );
    val.w   = 1.0f;
    write_imagef( out, ( int2 ) ( gx, gz ), val );
}

__kernel void TSDFVolume_sliceZ( write_only image2d_t out, int v, global float2* cv, int width, int height, int depth  )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int gz = v;
    global float2* cvptr = cv + gz * width * height + gy * width + gx;

    if( gx >= width || gy >= height || gz >= depth )
        return;

    float4 val;
    val.xyz = clamp( ( ( *cvptr ).x ) + 0.5f, 0.0f, 1.0f );
    val.w   = 1.0f;
    write_imagef( out, ( int2 ) ( gx, gy ), val );
}

static inline float TSDFVolume_rayStart( const float3 origin, const float3 direction, int width, int height, int depth )
{
    float xmin = ( ( direction.x > 0.0f ? 0.0f : width )  - origin.x ) / direction.x;
    float ymin = ( ( direction.y > 0.0f ? 0.0f : height ) - origin.y ) / direction.y;
    float zmin = ( ( direction.z > 0.0f ? 0.0f : depth )  - origin.z ) / direction.z;

    return fmax( fmax( xmin, ymin ), zmin );
}

static inline float TSDFVolume_rayEnd( const float3 origin, const float3 direction, int width, int height, int depth )
{
    float xmin = ( ( direction.x > 0.0f ? width : 0.0f )  - origin.x ) / direction.x;
    float ymin = ( ( direction.y > 0.0f ? height : 0.0f ) - origin.y ) / direction.y;
    float zmin = ( ( direction.z > 0.0f ? depth : 0.0f )  - origin.z ) / direction.z;

    return fmin( fmin( xmin, ymin ), zmin );
}

static inline float TSDFVolume_trilinearValue( global float2* cv, int width, int height, int depth, float3 pos )
{
    pos = fmin( fmax( ( float3 ) 0.0f, pos ), ( float3 ) ( width - 2, height - 2, depth - 2 ) );

    float3 base  = floor( pos );
    float3 alpha = pos - base;
    int3   ibase = ( int3 )( base.x, base.y, base.z );

#define TSDFVALUE( _x, _y, _z ) ( *( cv + ( ( _z ) * height + ( _y ) ) * width + ( _x ) ) )

    float2 tmp;
    float8 values;
    tmp       = TSDFVALUE( ibase.x    , ibase.y    , ibase.z );
    if( tmp.y < 1.0f ) return 1e10f;
    values.s0 = tmp.x;
    tmp       = TSDFVALUE( ibase.x + 1, ibase.y    , ibase.z );
    if( tmp.y < 1.0f ) return 1e10f;
    values.s1 = tmp.x;
    tmp       = TSDFVALUE( ibase.x    , ibase.y + 1, ibase.z );
    if( tmp.y < 1.0f ) return 1e10f;
    values.s2 = tmp.x;
    tmp       = TSDFVALUE( ibase.x + 1, ibase.y + 1, ibase.z );
    if( tmp.y < 1.0f ) return 1e10f;
    values.s3 = tmp.x;

    tmp       = TSDFVALUE( ibase.x    , ibase.y    , ibase.z + 1 );
    if( tmp.y < 1.0f ) return 1e10f;
    values.s4 = tmp.x;
    tmp       = TSDFVALUE( ibase.x + 1, ibase.y    , ibase.z + 1 );
    if( tmp.y < 1.0f ) return 1e10f;
    values.s5 = tmp.x;
    tmp       = TSDFVALUE( ibase.x    , ibase.y + 1, ibase.z + 1 );
    if( tmp.y < 1.0f ) return 1e10f;
    values.s6 = tmp.x;
    tmp       = TSDFVALUE( ibase.x + 1, ibase.y + 1, ibase.z + 1 );
    if( tmp.y < 1.0f ) return 1e10f;
    values.s7 = tmp.x;

    float4 interpz = mix( values.s0123, values.s4567, alpha.z );
    float2 interpy = mix( interpz.s01, interpz.s23, alpha.y );
    float  interpx = mix( interpy.s0, interpy.s1, alpha.x );

    return interpx;

#undef TSDFVALUE
}

static inline float TSDFVolume_trilinearValueUnchecked( global float2* cv, int width, int height, int depth, float3 pos )
{
    pos = fmin( fmax( ( float3 ) 0.0f, pos ), ( float3 ) ( width - 2, height - 2, depth - 2 ) );

    float3 base  = floor( pos );
    float3 alpha = pos - base;
    int3   ibase = ( int3 )( base.x, base.y, base.z );

#define TSDFVALUE( _x, _y, _z ) ( *( cv + ( ( _z ) * height + ( _y ) ) * width + ( _x ) ) )

    float2 tmp;
    float8 values;
    tmp       = TSDFVALUE( ibase.x    , ibase.y    , ibase.z );
    values.s0 = tmp.x;
    tmp       = TSDFVALUE( ibase.x + 1, ibase.y    , ibase.z );
    values.s1 = tmp.x;
    tmp       = TSDFVALUE( ibase.x    , ibase.y + 1, ibase.z );
    values.s2 = tmp.x;
    tmp       = TSDFVALUE( ibase.x + 1, ibase.y + 1, ibase.z );
    values.s3 = tmp.x;

    tmp       = TSDFVALUE( ibase.x    , ibase.y    , ibase.z + 1 );
    values.s4 = tmp.x;
    tmp       = TSDFVALUE( ibase.x + 1, ibase.y    , ibase.z + 1 );
    values.s5 = tmp.x;
    tmp       = TSDFVALUE( ibase.x    , ibase.y + 1, ibase.z + 1 );
    values.s6 = tmp.x;
    tmp       = TSDFVALUE( ibase.x + 1, ibase.y + 1, ibase.z + 1 );
    values.s7 = tmp.x;

    float4 interpz = mix( values.s0123, values.s4567, alpha.z );
    float2 interpy = mix( interpz.s01, interpz.s23, alpha.y );
    float  interpx = mix( interpy.s0, interpy.s1, alpha.x );

    return interpx;

#undef TSDFVALUE
}

static inline float TSDFVolume_trilinearWeight( global float2* cv, int width, int height, int depth, float3 pos )
{
    pos = fmin( fmax( ( float3 ) 0.0f, pos ), ( float3 ) ( width - 2, height - 2, depth - 2 ) );

    float3 base  = floor( pos );
    float3 alpha = pos - base;
    int3   ibase = ( int3 )( base.x, base.y, base.z );

#define TSDFVALUE( _x, _y, _z ) ( *( cv + ( ( _z ) * height + ( _y ) ) * width + ( _x ) ) )

    float2 tmp;
    float8 values;
    tmp       = TSDFVALUE( ibase.x    , ibase.y    , ibase.z );
    if( tmp.y < 1.0f ) return 0.0f;
    values.s0 = tmp.y;
    tmp       = TSDFVALUE( ibase.x + 1, ibase.y    , ibase.z );
    if( tmp.y < 1.0f ) return 0.0f;
    values.s1 = tmp.y;
    tmp       = TSDFVALUE( ibase.x    , ibase.y + 1, ibase.z );
    if( tmp.y < 1.0f ) return 0.0f;
    values.s2 = tmp.y;
    tmp       = TSDFVALUE( ibase.x + 1, ibase.y + 1, ibase.z );
    if( tmp.y < 1.0f ) return 0.0f;
    values.s3 = tmp.y;

    tmp       = TSDFVALUE( ibase.x    , ibase.y    , ibase.z + 1 );
    if( tmp.y < 1.0f ) return 0.0f;
    values.s4 = tmp.y;
    tmp       = TSDFVALUE( ibase.x + 1, ibase.y    , ibase.z + 1 );
    if( tmp.y < 1.0f ) return 0.0f;
    values.s5 = tmp.y;
    tmp       = TSDFVALUE( ibase.x    , ibase.y + 1, ibase.z + 1 );
    if( tmp.y < 1.0f ) return 0.0f;
    values.s6 = tmp.y;
    tmp       = TSDFVALUE( ibase.x + 1, ibase.y + 1, ibase.z + 1 );
    if( tmp.y < 1.0f ) return 0.0f;
    values.s7 = tmp.y;

    float4 interpz = mix( values.s0123, values.s4567, alpha.z );
    float2 interpy = mix( interpz.s01, interpz.s23, alpha.y );
    float  interpx = mix( interpy.s0, interpy.s1, alpha.x );

    return interpx;

#undef TSDFVALUE
}

static inline float TSDFVolume_trilinearWeightUnchecked( global float2* cv, int width, int height, int depth, float3 pos )
{
    pos = fmin( fmax( ( float3 ) 0.0f, pos ), ( float3 ) ( width - 2, height - 2, depth - 2 ) );

    float3 base  = floor( pos );
    float3 alpha = pos - base;
    int3   ibase = ( int3 )( base.x, base.y, base.z );

#define TSDFVALUE( _x, _y, _z ) ( *( cv + ( ( _z ) * height + ( _y ) ) * width + ( _x ) ) )

    float2 tmp;
    float8 values;
    tmp       = TSDFVALUE( ibase.x    , ibase.y    , ibase.z );
    values.s0 = tmp.y;
    tmp       = TSDFVALUE( ibase.x + 1, ibase.y    , ibase.z );
    values.s1 = tmp.y;
    tmp       = TSDFVALUE( ibase.x    , ibase.y + 1, ibase.z );
    values.s2 = tmp.y;
    tmp       = TSDFVALUE( ibase.x + 1, ibase.y + 1, ibase.z );
    values.s3 = tmp.y;

    tmp       = TSDFVALUE( ibase.x    , ibase.y    , ibase.z + 1 );
    values.s4 = tmp.y;
    tmp       = TSDFVALUE( ibase.x + 1, ibase.y    , ibase.z + 1 );
    values.s5 = tmp.y;
    tmp       = TSDFVALUE( ibase.x    , ibase.y + 1, ibase.z + 1 );
    values.s6 = tmp.y;
    tmp       = TSDFVALUE( ibase.x + 1, ibase.y + 1, ibase.z + 1 );
    values.s7 = tmp.y;

    float4 interpz = mix( values.s0123, values.s4567, alpha.z );
    float2 interpy = mix( interpz.s01, interpz.s23, alpha.y );
    float  interpx = mix( interpy.s0, interpy.s1, alpha.x );

    return interpx;

#undef TSDFVALUE
}

__kernel void TSDFVolume_rayCastDepthmap( write_only image2d_t out, global float2* cv, int width, int height, int depth, const Mat4f TCAM2G, const Mat4f TG2CAM, float scale )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int iwidth = get_image_width( out );
    const int iheight = get_image_height( out );

    if( gx >= iwidth || gy >= iheight )
        return;

    float3 rayOrigin = mat4f_transform( &TCAM2G, ( float4 ) ( 0.0f, 0.0f, 0.0f, 1.0f ) ).xyz;
    float3 raytmp    = mat4f_transform( &TCAM2G, ( float4 ) ( ( float ) gx, ( float ) gy, 1.0f, 1.0f ) ).xyz;
    float3 rayDir    = normalize( raytmp - rayOrigin );

    float rayStart = TSDFVolume_rayStart( rayOrigin, rayDir, width, height, depth );
    float rayEnd   = TSDFVolume_rayEnd( rayOrigin, rayDir, width, height, depth );

    if( rayStart < rayEnd && isfinite( rayStart ) && isfinite( rayEnd ) && all(isfinite( rayDir )) ) {
        float val_prev, val, ret = 0;
        float3 pos_prev, pos;
        float3 rayVec = fabs( rayDir * ( rayEnd - rayStart ) );
        float lambda_step = 0.5f * length( rayVec ) / fmax( rayVec.x, fmax( rayVec.y, rayVec.z ) );

        pos_prev = rayOrigin + rayDir * rayStart;
        val_prev = TSDFVolume_trilinearValueUnchecked( cv, width, height, depth, pos_prev );
        ret = 1e10f;

        for( float lambda = rayStart + lambda_step; lambda <= rayEnd; lambda += lambda_step ) {
            pos = rayOrigin + rayDir * lambda;
            val = TSDFVolume_trilinearValueUnchecked( cv, width, height, depth, pos );

            if ( val_prev < 0.0f && val > 0.0f) {
                break;
            }

            if ( val_prev > 0.0f && val <= 0.0f) {
                float dopt = val / ( ( val_prev - val ) / lambda_step );
                float3 gpos = pos + dopt * rayDir;

                ret = mat4f_transform( &TG2CAM, ( float4 ) ( gpos , 1.0f ) ).z * scale;
                break;
            }
            val_prev = val;
            pos_prev = pos;
        }

        write_imagef( out, ( int2 ) ( gx, gy ), ( float4 ) ( ret, ret, ret, 1.0f ) );
    } else {
        write_imagef( out, ( int2 ) ( gx, gy ), ( float4 ) 0.0f );
    }
}

__kernel void TSDFVolume_rayCastDepthNormalMap( write_only image2d_t out, global float2* cv, int width, int height, int depth, const Mat4f TCAM2G, const Mat4f TG2CAM, float scale )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int iwidth = get_image_width( out );
    const int iheight = get_image_height( out );

    if( gx >= iwidth || gy >= iheight )
        return;

    float3 rayOrigin = mat4f_transform( &TCAM2G, ( float4 ) ( 0.0f, 0.0f, 0.0f, 1.0f ) ).xyz;
    float3 raytmp    = mat4f_transform( &TCAM2G, ( float4 ) ( ( float ) gx, ( float ) gy, 1.0f, 1.0f ) ).xyz;
    float3 rayDir    = normalize( raytmp - rayOrigin );

    float rayStart = TSDFVolume_rayStart( rayOrigin, rayDir, width, height, depth );
    float rayEnd   = TSDFVolume_rayEnd( rayOrigin, rayDir, width, height, depth );

    if( rayStart < rayEnd && isfinite( rayStart ) && isfinite( rayEnd ) && all(isfinite( rayDir )) ) {
        float val_prev, val;
        float4 ret = ( float4 ) 0.0f;
        float3 pos_prev, pos;
        float3 rayVec = fabs( rayDir * ( rayEnd - rayStart ) );
        float lambda_step = 0.5f * length( rayVec ) / fmax( rayVec.x, fmax( rayVec.y, rayVec.z ) );

        pos_prev = rayOrigin + rayDir * rayStart;
        val_prev = TSDFVolume_trilinearValueUnchecked( cv, width, height, depth, pos_prev );

        for( float lambda = rayStart + lambda_step; lambda <= rayEnd; lambda += lambda_step ) {
            pos = rayOrigin + rayDir * lambda;
            val = TSDFVolume_trilinearValueUnchecked( cv, width, height, depth, pos );

            if ( val_prev < 0.0f && val > 0.0f) {
                break;
            }

            if ( val_prev > 0.0f && val < 0.0f) {
                float dopt = val / ( ( val_prev - val ) / lambda_step );
                float3 gpos = pos + dopt * rayDir;
                float3 grad;
                const float delta = 0.5f;
                grad.x = TSDFVolume_trilinearValueUnchecked( cv, width, height, depth, gpos + ( float3 ) ( delta, 0.0f, 0.0f ) ) - TSDFVolume_trilinearValueUnchecked( cv, width, height, depth, gpos + ( float3 ) ( -delta, 0.0f, 0.0f ) ) / ( 2.0f * delta );
                grad.y = TSDFVolume_trilinearValueUnchecked( cv, width, height, depth, gpos + ( float3 ) ( 0.0f, delta, 0.0f ) ) - TSDFVolume_trilinearValueUnchecked( cv, width, height, depth, gpos + ( float3 ) ( 0.0f, -delta, 0.0f ) ) / ( 2.0f * delta );
                grad.z = TSDFVolume_trilinearValueUnchecked( cv, width, height, depth, gpos + ( float3 ) ( 0.0f, 0.0f, delta ) ) - TSDFVolume_trilinearValueUnchecked( cv, width, height, depth, gpos + ( float3 ) ( 0.0f, 0.0f, -delta ) ) / ( 2.0f * delta );

                ret.x = mat4f_transform( &TG2CAM, ( float4 ) ( gpos , 1.0f ) ).z * scale;
                ret.yzw = normalize( mat4f_transform( &TG2CAM, ( float4 ) ( ( grad ), 0.0f ) ).xyz );
                break;
            }
            val_prev = val;
            pos_prev = pos;
        }
        write_imagef( out, ( int2 ) ( gx, gy ), ( float4 ) ret );
    } else {
        write_imagef( out, ( int2 ) ( gx, gy ), ( float4 ) 0.0f );
    }
}

__kernel void TSDFVolume_rayCastDepthNormalMapSlope( write_only image2d_t out, write_only image2d_t slopemap, global float2* cv, int width, int height, int depth, const Mat4f TCAM2G, const Mat4f TG2CAM, float scale )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int iwidth = get_image_width( out );
    const int iheight = get_image_height( out );

    if( gx >= iwidth || gy >= iheight )
        return;

    float3 rayOrigin = mat4f_transform( &TCAM2G, ( float4 ) ( 0.0f, 0.0f, 0.0f, 1.0f ) ).xyz;
    float3 raytmp    = mat4f_transform( &TCAM2G, ( float4 ) ( ( float ) gx, ( float ) gy, 1.0f, 1.0f ) ).xyz;
    float3 rayDir    = normalize( raytmp - rayOrigin );

    float rayStart = TSDFVolume_rayStart( rayOrigin, rayDir, width, height, depth );
    float rayEnd   = TSDFVolume_rayEnd( rayOrigin, rayDir, width, height, depth );

    const float FAR = 1e4f;

    if( rayStart < rayEnd && isfinite( rayStart ) && isfinite( rayEnd ) && all(isfinite( rayDir )) ) {
        float val_prev, val, w = 0.0f, slope = 0.0f;
        float4 ret = ( float4 ) ( FAR, 0.0f, 0.0f, 1.0f );
        float3 pos_prev, pos;
        float3 rayVec = fabs( rayDir * ( rayEnd - rayStart ) );
        float lambda_step = 0.25f * length( rayVec ) / fmax( rayVec.x, fmax( rayVec.y, rayVec.z ) );

        pos_prev = rayOrigin + rayDir * rayStart;
        val_prev = TSDFVolume_trilinearValueUnchecked( cv, width, height, depth, pos_prev );

        for( float lambda = rayStart + lambda_step; lambda <= rayEnd; lambda += lambda_step ) {
            pos = rayOrigin + rayDir * lambda;
            val = TSDFVolume_trilinearValueUnchecked( cv, width, height, depth, pos );

            if ( val_prev < 0.0f && val > 0.0f) {
                break;
            }

            if ( val_prev > 0.0f && val < 0.0f) {
                const float delta = 1.0f;
                float dopt = lambda_step * val / ( ( val_prev - val ) );
                float3 gpos = pos + dopt * rayDir;
                // get the slope of the function values
                slope = TSDFVolume_trilinearValueUnchecked( cv, width, height, depth, gpos + delta * rayDir ) - TSDFVolume_trilinearValueUnchecked( cv, width, height, depth, gpos - delta * rayDir ) / ( 2.0f * delta );
                w = TSDFVolume_trilinearWeight( cv, width, height, depth, gpos );
                float3 grad;
                grad.x = TSDFVolume_trilinearValueUnchecked( cv, width, height, depth, gpos + ( float3 ) ( delta, 0.0f, 0.0f ) ) - TSDFVolume_trilinearValueUnchecked( cv, width, height, depth, gpos + ( float3 ) ( -delta, 0.0f, 0.0f ) ) / ( 2.0f * delta );
                grad.y = TSDFVolume_trilinearValueUnchecked( cv, width, height, depth, gpos + ( float3 ) ( 0.0f, delta, 0.0f ) ) - TSDFVolume_trilinearValueUnchecked( cv, width, height, depth, gpos + ( float3 ) ( 0.0f, -delta, 0.0f ) ) / ( 2.0f * delta );
                grad.z = TSDFVolume_trilinearValueUnchecked( cv, width, height, depth, gpos + ( float3 ) ( 0.0f, 0.0f, delta ) ) - TSDFVolume_trilinearValueUnchecked( cv, width, height, depth, gpos + ( float3 ) ( 0.0f, 0.0f, -delta ) ) / ( 2.0f * delta );

                ret.x = mat4f_transform( &TG2CAM, ( float4 ) ( gpos , 1.0f ) ).z * scale;
                ret.yzw = normalize( mat4f_transform( &TG2CAM, ( float4 ) ( ( grad ), 0.0f ) ).xyz );
                break;
            }
            val_prev = val;
            pos_prev = pos;
        }
        write_imagef( slopemap, ( int2 ) ( gx, gy ), ( float4 ) ( ( clamp( ( -slope ), 0.0f, 4.0f ) ) / ( 4.0f ) * ( 1.0f - exp( -0.1 * w * w ) ) ) );
        write_imagef( out, ( int2 ) ( gx, gy ), ( float4 ) ret );
    } else {
        write_imagef( slopemap, ( int2 ) ( gx, gy ), ( float4 ) 0.0f );
        write_imagef( out, ( int2 ) ( gx, gy ), ( float4 ) ( FAR, 0.0f, 0.0f, 1.0f ) );
    }
}

__kernel void TSDFVolume_normalToWeight( write_only image2d_t out, read_only image2d_t input )
{
    int w = get_image_width( out );
    int h = get_image_height( out );

    int2 coord;
    coord.x = get_global_id( 0 );
    coord.y = get_global_id( 1 );

    if( coord.x >= w || coord.y >= h )
        return;

    float4 value = 0.5f + 0.5f * ( float4 ) fabs( read_imagef( input, SAMPLER_NN, coord ).z ) + 1e-5f;
    write_imagef( out, coord, value );
}
