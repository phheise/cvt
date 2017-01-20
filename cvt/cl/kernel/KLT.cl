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
#ifndef CL_KLT_CL
#define CL_KLT_CL

#import "cvt/cl/kernel/Homography.cl"
#import "cvt/cl/kernel/SL3.cl"
#import "cvt/cl/kernel/Matrix8.cl"
#import "cvt/cl/kernel/Plane.cl"

#define COLORWEIGHTKLT 16.0f

inline float2 gradient( read_only image2d_t img, const float2 pt )
{
    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
    const float2 offsethalf = ( float2 ) ( 0.5f, 0.5f );
    const float stepsize = 1.0f;
    float2 posx1 = pt + ( float2 ) (  stepsize, 0.0f );
    float2 posx2 = pt + ( float2 ) ( -stepsize, 0.0f );
    float2 posy1 = pt + ( float2 ) ( 0.0f,  stepsize );
    float2 posy2 = pt + ( float2 ) ( 0.0f, -stepsize );

    float valx1 = read_imagef( img, SAMPLER_BILINEAR, posx1 + offsethalf ).x;
    float valx2 = read_imagef( img, SAMPLER_BILINEAR, posx2 + offsethalf ).x;
    float valy1 = read_imagef( img, SAMPLER_BILINEAR, posy1 + offsethalf ).x;
    float valy2 = read_imagef( img, SAMPLER_BILINEAR, posy2 + offsethalf ).x;

    return ( ( float2 ) ( valx1 - valx2, valy1 - valy2  ) ) / ( 2.0f * stepsize);
}

inline float2 homography_gradient( read_only image2d_t img, const Homographyf h, const float2 pt )
{
    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
    const float4 grayWeight = ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );
    const float2 offsethalf = ( float2 ) ( 0.5f, 0.5f );
    const float stepsize = 1.0f;
    float2 posx1 = homographyf_transform( h, pt ) + ( float2 ) (  stepsize, 0.0f );
    float2 posx2 = homographyf_transform( h, pt ) + ( float2 ) ( -stepsize, 0.0f );
    float2 posy1 = homographyf_transform( h, pt ) + ( float2 ) ( 0.0f,  stepsize );
    float2 posy2 = homographyf_transform( h, pt ) + ( float2 ) ( 0.0f, -stepsize );

    float valx1 = dot( read_imagef( img, SAMPLER_BILINEAR, posx1 + offsethalf ), grayWeight );
    float valx2 = dot( read_imagef( img, SAMPLER_BILINEAR, posx2 + offsethalf ), grayWeight );
    float valy1 = dot( read_imagef( img, SAMPLER_BILINEAR, posy1 + offsethalf ), grayWeight );
    float valy2 = dot( read_imagef( img, SAMPLER_BILINEAR, posy2 + offsethalf ), grayWeight );

    return ( float2 ) ( valx1 - valx2, valy1 - valy2 ) / ( 2.0f * stepsize );
}

inline float2 mat3f_gradient( read_only image2d_t img, const Mat3f* h, const float2 pt )
{
    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
    const float4 grayWeight = ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );
    const float2 offsethalf = ( float2 ) ( 0.5f, 0.5f );
    const float stepsize = 1.0f;
    float2 posx1 = mat3f_transform_proj2( h, pt ) + ( float2 ) (  stepsize, 0.0f );
    float2 posx2 = mat3f_transform_proj2( h, pt ) + ( float2 ) ( -stepsize, 0.0f );
    float2 posy1 = mat3f_transform_proj2( h, pt ) + ( float2 ) ( 0.0f,  stepsize );
    float2 posy2 = mat3f_transform_proj2( h, pt ) + ( float2 ) ( 0.0f, -stepsize );

    float valx1 = dot( read_imagef( img, SAMPLER_BILINEAR, posx1 + offsethalf ), grayWeight );
    float valx2 = dot( read_imagef( img, SAMPLER_BILINEAR, posx2 + offsethalf ), grayWeight );
    float valy1 = dot( read_imagef( img, SAMPLER_BILINEAR, posy1 + offsethalf ), grayWeight );
    float valy2 = dot( read_imagef( img, SAMPLER_BILINEAR, posy2 + offsethalf ), grayWeight );

    return ( ( float2 ) ( valx1 - valx2, valy1 - valy2 ) ) / ( 2.0f * stepsize);
}

inline float2 klt_transform_grad( read_only image2d_t img, const Mat3f* T, const float2 coord )
{
    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
    const float2 offsethalf = ( float2 ) ( 0.5f, 0.5f );
    const float stepsize = 1.0f;
    float2 posx1 = mat3f_transform_proj2( T, coord )  + ( ( float2 ) ( stepsize, 0.0f) );
    float2 posx2 = mat3f_transform_proj2( T, coord )  + ( ( float2 ) ( -stepsize, 0.0f) );
    float2 posy1 = mat3f_transform_proj2( T, coord )  + ( ( float2 ) ( 0.0f, stepsize) );
    float2 posy2 = mat3f_transform_proj2( T, coord )  + ( ( float2 ) ( 0.0f, -stepsize) );

    float valx1 = read_imagef( img, SAMPLER_BILINEAR, posx1 + offsethalf ).x;
    float valx2 = read_imagef( img, SAMPLER_BILINEAR, posx2 + offsethalf ).x;
    float valy1 = read_imagef( img, SAMPLER_BILINEAR, posy1 + offsethalf ).x;
    float valy2 = read_imagef( img, SAMPLER_BILINEAR, posy2 + offsethalf ).x;

    return ( float2 ) ( valx1 - valx2, valy1 - valy2 ) / ( 2.0f * stepsize );
}

inline Planef klt_plane_iteration( Planef p,
                                   read_only image2d_t img1, read_only image2d_t img2, const float2 pt,
                                   const Mat3f* KSrcInv, const Mat4f* TSrc,
                                   const Mat3f* KDst, const Mat4f* TDst,
                                   const int patchsize, float* ssd )
{
    const float2 offsethalf = ( float2 ) ( 0.5f, 0.5f );
    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;
    Mat3f A, JH, H;
    float3 b, J;
    float2 pos;

    planef_homography( &H, p, KDst, TDst, KSrcInv, TSrc );
    mat3f_set_zero( &A );
    b = ( float3 ) 0.0f;

    *ssd = 0;

    float valcenter = read_imagef( img1, SAMPLER_BILINEAR, pt + offsethalf ).x;
    for( float dy = -patchsize; dy <= patchsize; dy+= 1.0f ) {
        for( float dx = -patchsize; dx <= patchsize; dx+= 1.0f ) {
            float2 displace = ( float2 ) ( dx, dy );
            pos = pt + displace;

            // transform point 1
            float3 pos1 = mat3f_transform( &H, ( float3 ) ( pos, 1.0f ) );
            float2 ppos1 = pos1.xy / pos1.z;

            float4 val1 = read_imagef( img1, SAMPLER_BILINEAR, pos + offsethalf );

            //float w = 1.0f;
            float w = native_exp( -fabs( valcenter - val1.x ) * ( 8.0f ) );
            //if( w < 1e-4f ) continue;

            float4 val2 = read_imagef( img2, SAMPLER_BILINEAR, ppos1 + offsethalf );

//          float2 grad = -0.5f * ( 0.5f * gradient( img1, pos ) + 0.5f * klt_transform_grad( img2, &H, pos ) );
            float2 grad = -0.25f * val1.yz - 0.25f * val2.yz;

            planef_homography_jacobian( &JH, pos, KDst, TDst, KSrcInv, TSrc );

            // 1 x 2 gradient times 2 x 3 jacobian of the projection times 3 x 3 homography jacobian
            J.x = ( grad.x * ( JH.m[ 0 ].x - ppos1.x * JH.m[ 2 ].x ) + grad.y * ( JH.m[ 1 ].x - ppos1.y * JH.m[ 2 ].x ) ) / pos1.z;
            J.y = ( grad.x * ( JH.m[ 0 ].y - ppos1.x * JH.m[ 2 ].y ) + grad.y * ( JH.m[ 1 ].y - ppos1.y * JH.m[ 2 ].y ) ) / pos1.z;
            J.z = ( grad.x * ( JH.m[ 0 ].z - ppos1.x * JH.m[ 2 ].z ) + grad.y * ( JH.m[ 1 ].z - ppos1.y * JH.m[ 2 ].z ) ) / pos1.z;

            float diff = val2.x - val1.x;

            *ssd += diff * diff;

            b += J * w * diff;
            Mat3f mattmp;
            mat3f_outer( &mattmp, J, w * J );
            mat3f_add( &A, &A, &mattmp );
        }
    }

//    Mat3f mattmp;
//    mat3f_inverse( &mattmp, &A );
//    float3 pdelta = mat3f_transform( &mattmp, b );
    A.m[ 0 ].x += 0.25f;
    A.m[ 1 ].y += 0.25f;
    A.m[ 2 ].z += 0.25f;
    float3 pdelta = mat3f_lusolve( &A, b );
    float4 ret;
    pdelta = p.xyz / p.w + pdelta;
    ret.w = sign( p.w ) / length( pdelta.xyz );
    ret.xyz = pdelta.xyz * ret.w;
    return ret;
}

inline Planef klt_ai_plane_iteration( Planef p,
                                   read_only image2d_t img1, read_only image2d_t img2, const float2 pt, const float2 ai,
                                   const Mat3f* KSrcInv, const Mat4f* TSrc,
                                   const Mat3f* KDst, const Mat4f* TDst,
                                   const int patchsize, float* ssd )
{
    const float2 offsethalf = ( float2 ) ( 0.5f, 0.5f );
    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;
    Mat3f A, JH, H;
    float3 b, J;
    float2 pos;

    planef_homography( &H, p, KDst, TDst, KSrcInv, TSrc );
    mat3f_set_zero( &A );
    b = ( float3 ) 0.0f;

    *ssd = 0;

    float valcenter = read_imagef( img1, SAMPLER_BILINEAR, pt + offsethalf ).x;
    for( float dy = -patchsize; dy <= patchsize; dy+= 1.0f ) {
        for( float dx = -patchsize; dx <= patchsize; dx+= 1.0f ) {
            float2 displace = ( float2 ) ( dx, dy );
            pos = pt + displace;

            // transform point 1
            float3 pos1 = mat3f_transform( &H, ( float3 ) ( pos, 1.0f ) );
            float2 ppos1 = pos1.xy / pos1.z;

            float4 val1 = read_imagef( img1, SAMPLER_BILINEAR, pos + offsethalf );
            val1.x = ( ( 1.0f + ai.x ) * val1.x + ai.y );

            //float w = 1.0f;
            float w = native_exp( -fabs( valcenter - val1.x ) * ( 8.0f ) );
            //if( w < 1e-4f ) continue;

            float4 val2 = read_imagef( img2, SAMPLER_BILINEAR, ppos1 + offsethalf );

//          float2 grad = -0.5f * ( 0.5f * gradient( img1, pos ) + 0.5f * klt_transform_grad( img2, &H, pos ) );
            float2 grad = -mix( val1.yz * ( 1.0f + ai.x ), val2.yz, 0.5f );

            planef_homography_jacobian( &JH, pos, KDst, TDst, KSrcInv, TSrc );

            // 1 x 2 gradient times 2 x 3 jacobian of the projection times 3 x 3 homography jacobian
            J.x = ( grad.x * ( JH.m[ 0 ].x - ppos1.x * JH.m[ 2 ].x ) + grad.y * ( JH.m[ 1 ].x - ppos1.y * JH.m[ 2 ].x ) ) / pos1.z;
            J.y = ( grad.x * ( JH.m[ 0 ].y - ppos1.x * JH.m[ 2 ].y ) + grad.y * ( JH.m[ 1 ].y - ppos1.y * JH.m[ 2 ].y ) ) / pos1.z;
            J.z = ( grad.x * ( JH.m[ 0 ].z - ppos1.x * JH.m[ 2 ].z ) + grad.y * ( JH.m[ 1 ].z - ppos1.y * JH.m[ 2 ].z ) ) / pos1.z;

            float diff = val2.x - val1.x;

            *ssd += diff * diff;

            b += J * w * diff;
            Mat3f mattmp;
            mat3f_outer( &mattmp, J, w * J );
            mat3f_add( &A, &A, &mattmp );
        }
    }

//    Mat3f mattmp;
//    mat3f_inverse( &mattmp, &A );
//    float3 pdelta = mat3f_transform( &mattmp, b );

    A.m[ 0 ].x *= 1.25f;
    A.m[ 1 ].y *= 1.25f;
    A.m[ 2 ].z *= 1.25f;
    float3 pdelta = mat3f_lusolve( &A, b );
    float4 ret;
    pdelta = p.xyz / p.w + pdelta;
    ret.w = sign( p.w ) / length( pdelta.xyz );
    ret.xyz = pdelta.xyz * ret.w;
    return ret;
}

inline float klt_homography_iteration( Homographyf* delta, read_only image2d_t img1, read_only image2d_t img2, const float2 pt, const Homographyf h, const int patchsize )
{
    const float4 grayWeight =  ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );
    const float2 offsethalf = ( float2 ) ( 0.5f, 0.5f );
    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;
    float8 jacx, jacy, b;
    Mat8f A, mattmp;
    float ssd = 0;

    mat8f_set_zero( &A );
    b = ( float8 ) 0.0f;

    for( float dy = -patchsize; dy <= patchsize; dy+= 1.0f ) {
        for( float dx = -patchsize; dx <= patchsize; dx+= 1.0f ) {
            float2 displace = ( float2 ) ( dx, dy );
            float2 pos1 = pt + displace;
            float2 pos2 = homographyf_transform( h, pos1 );

            float val1 = dot( read_imagef( img1, SAMPLER_BILINEAR, pos1 + offsethalf ), grayWeight );
            float val2 = dot( read_imagef( img2, SAMPLER_BILINEAR, pos2 + offsethalf ), grayWeight );
            float diff = val2 - val1;

            ssd += diff * diff;

            float2 grad = homography_gradient( img2, h, pos1 );
            //grad = gradient( img1, pos1 );
            //grad = 0.5f *grad + gradient( img1, pos1 );
            homographyf_jacobian( &jacx, &jacy, h, pos1 );
            float8 jac = grad.x * jacx + grad.y * jacy;

            mat8f_outer( &mattmp, jac, jac );
            mat8f_add( &A, &A, &mattmp );

            b += jac * diff;
        }
    }

//    b /= ( ( patchsize * 2 + 1 ) * ( patchsize * 2 + 1 ) );
//    mat8f_mul_scalar( &A, &A, 1.0f / ( ( patchsize * 2 + 1 ) * ( patchsize * 2 + 1 ) ) );

//    Mat8f bla;
//    mat8f_set_zero( &bla );
//    mat8f_set_diag( &bla, ( ( ( patchsize * 2 + 1 ) * ( patchsize * 2 + 1 ) ) ) * ( float8 ) ( 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f ) );
//    mat8f_add( &A, &A, &bla );

    *delta = mat8f_lusolve( &A, b );
    return ssd;
}

inline float klt_sl3_iteration( SL3* delta, read_only image2d_t img1, read_only image2d_t img2, const float2 pt, const Mat3f* sl3, const int patchsize )
{
    const float4 grayWeight =  ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );
    const float2 offsethalf = ( float2 ) ( 0.5f, 0.5f );
    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;
    float8 jacx, jacy, b;
    Mat8f A, mattmp;
//    Mat3f h, hi;
    float ssd = 0;

//    SL3_to_matrix( &h, sl3 );
  //  SL3_to_matrix( &hi, -sl3 );

    mat8f_set_zero( &A );
    b = ( float8 ) 0.0f;

    for( float dy = -patchsize; dy <= patchsize; dy+= 1.0f ) {
        for( float dx = -patchsize; dx <= patchsize; dx+= 1.0f ) {
            float2 displace = ( float2 ) ( dx, dy );
            float2 pos1 = pt + displace;
            float2 pos2 = mat3f_transform_proj2( sl3, pos1 );

            float val1 = dot( read_imagef( img1, SAMPLER_BILINEAR, pos1 + offsethalf ), grayWeight );
            float val2 = dot( read_imagef( img2, SAMPLER_BILINEAR, pos2 + offsethalf ), grayWeight );
            float diff = val1 - val2;

            ssd += diff * diff;

            float2 grad = mat3f_gradient( img1, sl3, pos1 );
            //grad = gradient( img1, pos1 );
            //grad = 0.5f * grad + 0.5f * gradient( img2, pos1 );
            SL3_jacobian( &jacx, &jacy, sl3, pos1 );
            float8 jac = grad.x * jacx + grad.y * jacy;

            mat8f_outer( &mattmp, jac, jac );
            mat8f_add( &A, &A, &mattmp );

            b += jac * diff;
        }
    }

    //Mat8f bla;
    //mat8f_set_zero( &bla );
    //mat8f_set_diag( &bla, ( ( ( patchsize * 2 + 1 ) * ( patchsize * 2 + 1 ) ) ) * ( float8 ) ( 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f ) );
    //mat8f_add( &A, &A, &bla );

    *delta = mat8f_lusolve( &A, b );
    return ssd;
}

inline float klt_weighted_homography_iteration( Homographyf* delta, read_only image2d_t img1, read_only image2d_t img2, const float2 pt, const Homographyf h, const int patchsize )
{
    // int width = get_image_width( img2 );
    // int height = get_image_height( img2 );
    const float4 grayWeight =  ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );
    const float2 offsethalf = ( float2 ) ( 0.5f, 0.5f );
    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;
    float8 jacx, jacy, b;
    Mat8f A, mattmp;
    float ssd = 0;

    mat8f_set_zero( &A );
    b = ( float8 ) 0.0f;

    float4 valcenter = read_imagef( img1, SAMPLER_BILINEAR, pt + offsethalf );
    for( float dy = -patchsize; dy <= patchsize; dy+= 1.0f ) {
        for( float dx = -patchsize; dx <= patchsize; dx+= 1.0f ) {
            float2 displace = ( float2 ) ( dx, dy );
            float2 pos1 = pt + displace;
            float2 pos2 = homographyf_transform( h, pos1 );

            float4 val1c = read_imagef( img1, SAMPLER_BILINEAR, pos1 + offsethalf );
            float val1 = dot( val1c, grayWeight );
            float val2 = dot( read_imagef( img2, SAMPLER_BILINEAR, pos2 + offsethalf ), grayWeight );
            float diff = val2 - val1;

            ssd += diff * diff;

            float2 grad = homography_gradient( img2, h, pos1 );
            grad = 0.5f * grad +  0.5f * gradient( img1, pos1 );
            homographyf_jacobian( &jacx, &jacy, h, pos1 );

            float8 jac = grad.x * jacx + grad.y * jacy;

            float weight = native_exp( -dot( fabs( valcenter.xyz - val1c.xyz ), ( float3 ) 1.0f ) * ( smoothstep( 0.0f, length( ( float2 ) ( patchsize, patchsize ) ), length( displace ) ) * 2.5f * COLORWEIGHTKLT + 5.0f ) );
            mat8f_outer( &mattmp, jac, weight * jac );
            mat8f_add( &A, &A, &mattmp );

            b += weight * jac * diff;
        }
    }

    *delta = mat8f_lusolve( &A, b );
    return ssd;
}

inline float klt_weighted_homography_iteration_gradient( Homographyf* delta, read_only image2d_t img1, read_only image2d_t img2,
                                                  read_only image2d_t gimg1, read_only image2d_t gimg2,
                                                  const float2 pt, const Homographyf h, const int patchsize )
{
    // int width = get_image_width( img2 );
    // int height = get_image_height( img2 );
    const float4 grayWeight =  ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );
    const float2 offsethalf = ( float2 ) ( 0.5f, 0.5f );
    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;
    float8 jacx, jacy, b;
    Mat8f A, mattmp;
    float ssd = 0;

    mat8f_set_zero( &A );
    b = ( float8 ) 0.0f;

    float4 valcenter = read_imagef( img1, SAMPLER_BILINEAR, pt + offsethalf );
    for( float dy = -patchsize; dy <= patchsize; dy+= 1.0f ) {
        for( float dx = -patchsize; dx <= patchsize; dx+= 1.0f ) {
            float2 displace = ( float2 ) ( dx, dy );
            float2 pos1 = pt + displace;
            float2 pos2 = homographyf_transform( h, pos1 );

            float4 val1c = read_imagef( img1, SAMPLER_BILINEAR, pos1 + offsethalf );
            float val1 = dot( val1c, grayWeight );
            float val2 = dot( read_imagef( img2, SAMPLER_BILINEAR, pos2 + offsethalf ), grayWeight );
            float diff = val2 - val1;

            ssd += diff * diff;

//            float2 grad = homography_gradient( img2, h, pos1 );
//          grad = 0.5f * grad +  0.5f * gradient( img1, pos1 );
            homographyf_jacobian( &jacx, &jacy, h, pos1 );
            float2 grad = 0.5f * read_imagef( gimg1, SAMPLER_BILINEAR, pos1 + offsethalf ).xy;
            grad += 0.5f * read_imagef( gimg2, SAMPLER_BILINEAR, pos2 + offsethalf ).xy;
            float8 jac = grad.x * jacx + grad.y * jacy;

            float weight = native_exp( -dot( fabs( valcenter.xyz - val1c.xyz ), ( float3 ) 1.0f ) * COLORWEIGHTKLT + 5.0f );
            //float weight = native_exp( -dot( fabs( valcenter.xyz - val1c.xyz ), ( float3 ) 1.0f ) * ( smoothstep( 0.0f, length( ( float2 ) ( patchsize, patchsize ) ), length( displace ) ) * 2.5f * COLORWEIGHTKLT + 5.0f ) );
            mat8f_outer( &mattmp, jac, weight * jac );
            mat8f_add( &A, &A, &mattmp );

            b += weight * jac * diff;
        }
    }

    *delta = mat8f_lusolve( &A, b );
    return ssd;
}

kernel void klt_test( global float8* output, global float* outssd, read_only image2d_t img1, read_only image2d_t img2, int iter )
{
    SL3 sl3 = SL3_identity();
    SL3 delta;
    Mat3f h, hdelta, tmp;

    // float ssdprev = 1e10f;
    float ssd;

    SL3_to_matrix( &h, sl3 );

    for( int i = 0; i < iter; i++ ) {

        ssd = klt_sl3_iteration( &delta, img1, img2, ( float2 ) ( 255.0f, 255.0f ), &h, 230 );
        outssd[ i ] = ssd;
//        if( ssd < ssdprev ) {
        SL3_to_matrix( &hdelta, -delta );
        mat3f_mul( &tmp, &h, &hdelta );
        mat3f_assign( &h, &tmp );
        //sl3 = SL3_compose( sl3, -delta );
//            ssdprev = ssd;
        output[ i + 1 ].s0 = h.m[ 0 ].s0 / h.m[ 2 ].s2;
        output[ i + 1 ].s1 = h.m[ 0 ].s1 / h.m[ 2 ].s2;
        output[ i + 1 ].s2 = h.m[ 0 ].s2 / h.m[ 2 ].s2;
        output[ i + 1 ].s3 = h.m[ 1 ].s0 / h.m[ 2 ].s2;
        output[ i + 1 ].s4 = h.m[ 1 ].s1 / h.m[ 2 ].s2;
        output[ i + 1 ].s5 = h.m[ 1 ].s2 / h.m[ 2 ].s2;
        output[ i + 1 ].s6 = h.m[ 2 ].s0 / h.m[ 2 ].s2;
        output[ i + 1 ].s7 = h.m[ 2 ].s1 / h.m[ 2 ].s2;
  //      } else
    //        break;
    }
    output[ 0 ] = sl3;

}

kernel void klt_test_homography( global float8* output, global float* outssd, read_only image2d_t img1, read_only image2d_t img2, int iter )
{
    SL3 delta;
    Homographyf h;
    float ssd;

    h = homographyf_identity();

    for( int i = 0; i < iter; i++ ) {
        ssd = klt_homography_iteration( &delta, img1, img2, ( float2 ) ( 255.0f, 255.0f ), h, 50 );
        outssd[ i ] = ssd;
        h = homographyf_compose( homographyf_inverse( delta ), h );

        output[ i ].s0 = h.s0 + 1.0f;
        output[ i ].s1 = h.s2;
        output[ i ].s2 = h.s4;
        output[ i ].s3 = h.s1;
        output[ i ].s4 = h.s3 + 1.0f;
        output[ i ].s5 = h.s5;
        output[ i ].s6 = h.s6;
        output[ i ].s7 = h.s7;
    }

}

kernel void klt_test_plane( global float4* output, global float* outssd,
                            read_only image2d_t img1, read_only image2d_t img2,
                            const float4 pinit, const Mat3f K, const Mat3f Kinv, const Mat4f RT,
                            int iter )
{
    float ssd;

    Planef p = pinit;
    Mat4f I;
    mat4f_set_identity( &I );
    for( int i = 0; i < iter; i++ ) {
        p = klt_plane_iteration( p, img1, img2, ( float2 ) ( 255.0f, 255.0f ),
                                 &Kinv, &I, &K, &RT, 50, &ssd );

        outssd[ i ] = ssd;
        output[ i ] = p;
    }

}

kernel void mat3f_warp_image( write_only image2d_t output, read_only image2d_t image, const Mat3f h )
{
    const float2 offsethalf = ( float2 ) ( 0.5f, 0.5f );
    const int width = get_image_width( image );
    const int height = get_image_height( image );
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
    const float2 coordf = ( float2 ) ( get_global_id( 0 ), get_global_id( 1 ) );

    if( gx >= width || gy >= height )
        return;

    float2 pos2 = mat3f_transform_proj2( &h, coordf );
    float4 val = read_imagef( image, SAMPLER_BILINEAR, pos2 + offsethalf );
    write_imagef( output, ( int2 ) ( gx, gy ), val );
}
#endif
