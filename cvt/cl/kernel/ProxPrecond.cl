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
#ifndef CL_PROXPRECOND_CL
#define CL_PROXPRECOND_CL

kernel void prox_precond_simple( write_only image2d_t output )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int width = get_image_width( output );
    const int height = get_image_height( output );
    float4 val;

    if( gx >= get_image_width( output ) || gy >= get_image_height( output ) )
        return;

    if( gy == 0 || gy == height - 1 ) {
        if( gx == 0 || gx == width - 1 )
            val = 0.5f;
        else
            val = 0.33f;
    } else if( gx == 0 || gx == width - 1 ) {
        val = 0.3f;
    } else
        val = 0.25f;

    write_imagef( output, ( int2 ) ( gx, gy ), ( float4 ) val );
}

kernel void prox_precond_edge_clamp_separable3( write_only image2d_t output, float3 kx, float3 ky )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int width = get_image_width( output );
    const int height = get_image_height( output );
    float val;

    if( gx >= get_image_width( output ) || gy >= get_image_height( output ) )
        return;

    if( gx == 0 ) {
      val  = fabs( kx.s0 + kx.s1 ); // spread from x-border - 1, with edge clamping s0,s1 are multiplied with the same value
      val += fabs( kx.s0 );
    } else if( gx == width - 1 ) {
      val  = fabs( kx.s1 + kx.s2 );
      val += fabs( kx.s2 );
    } else
      val = dot( fabs( kx ), ( float3 ) 1.0f );

    if( gy == 0 ) {
      val += fabs( ky.s0 + ky.s1 ); // spread from y-border - 1, with edge clamping s0,s1 are multiplied with the same value
      val += fabs( ky.s0 );
    } else if( gy == height - 1 ) {
      val += fabs( ky.s1 + ky.s2 );
      val += fabs( ky.s2 );
    } else
      val += dot( fabs( ky ), ( float3 ) 1.0f );

    val = 1.0f / val;
    if( !isfinite( val ) )
        val = 0.0f;

    write_imagef( output, ( int2 ) ( gx, gy ), ( float4 ) val );
}

kernel void prox_precond_edge_zero_separable3( write_only image2d_t output, float3 kx, float3 ky )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int width = get_image_width( output );
    const int height = get_image_height( output );
    float val;

    if( gx >= get_image_width( output ) || gy >= get_image_height( output ) )
        return;

    if( gx == 0 ) {
      val  = fabs( kx.s1 ); // spread from x-border - 1, with edge clamping s0,s1 are multiplied with the same value
      val += fabs( kx.s0 );
    } else if( gx == width - 1 ) {
      val  = fabs( kx.s1 );
      val += fabs( kx.s2 );
    } else
      val = dot( fabs( kx ), ( float3 ) 1.0f );

    if( gy == 0 ) {
      val += fabs( ky.s1 ); // spread from y-border - 1, with edge clamping s0,s1 are multiplied with the same value
      val += fabs( ky.s0 );
    } else if( gy == height - 1 ) {
      val += fabs( ky.s1 );
      val += fabs( ky.s2 );
    } else
      val += dot( fabs( ky ), ( float3 ) 1.0f );

    val = 1.0f / val;
    if( !isfinite( val ) )
        val = 0.0f;

    write_imagef( output, ( int2 ) ( gx, gy ), ( float4 ) val );
}

#endif
