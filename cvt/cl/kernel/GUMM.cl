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
#ifndef GUMM_CL
#define GUMM_CL

/* GAUSSIAN UNIFORM MIXTURE MODEL */

typedef struct {
    float mu;
    float mu2;
    float accumg;
    float accumall;
} GUMM;

#define GUMM_INIT_DEF(name,typeattrib) \
inline void GUMM_##name( typeattrib GUMM* gumm, float mu, float variance, float pi, float accum ) \
{ \
    gumm->mu        = mu; \
    gumm->mu2      = mu * mu + variance; \
    gumm->accumg    = accum * pi; \
    gumm->accumall  = accum; \
} \

GUMM_INIT_DEF(init,private)
GUMM_INIT_DEF(init_global,global)

inline void GUMM_init_float4( GUMM* gumm, float4 vec )
{
    gumm->mu        = vec.x;
    gumm->mu2      = vec.y;
    gumm->accumg    = vec.z;
    gumm->accumall  = vec.w;
}

inline float4 GUMM_to_float4( const GUMM* gumm )
{
    return ( float4 ) ( gumm->mu, gumm->mu2, gumm->accumg, gumm->accumall );
}

inline float GUMM_mean( const global GUMM* gumm )
{
    return gumm->mu;
}

inline float GUMM_variance( const global GUMM* gumm )
{
    return gumm->mu2 - gumm->mu * gumm->mu;
}

#define GUMM_INLIER_RATE_DEF(name,typeattrib) \
inline float GUMM_##name( const typeattrib GUMM* gumm ) \
{ \
    return  gumm->accumg / ( gumm->accumall + 1e-8f ); \
}

GUMM_INLIER_RATE_DEF(inlier_rate,private)
GUMM_INLIER_RATE_DEF(inlier_rate_global,global)

#if 0
//This fails with the NVidia compiler and produces an invalid binary

#define GUMM_UPDATE_DEF(name,typeattrib) \
float GUMM_##name( typeattrib GUMM* gumm, float value, float weight, float UNIFORMPROB )      \
{                                                                                            \
          float GAMMAMIN    = 1e-6f;                                                          \
    const float VARMIN    = 1e-6f;                                                        \
    const float PIMIN      = 0.001f;                                                         \
    const float PIMAX      = 1.0f;                                                         \
                                                                                              \
    /* the current inlier rate */                                                            \
    float inrate = gumm->accumg / ( gumm->accumall + 1e-8f );                                \
    float pi = clamp( inrate, PIMIN, PIMAX );                                                \
                                                                                              \
    if( pi <= 0.4f || pi >= 0.999f ) {                                                      \
        /* restart */                                                                        \
        gumm->mu = value;                                                                    \
        /* inverse uniform probability should give us the range and we take 10% of it */      \
        gumm->mu2 = value * value + ( 1.0f / UNIFORMPROB ) * 0.1f;                          \
        gumm->accumg = 0.8f;                                                                  \
        gumm->accumall = 1.0f;                                                              \
        return 0.9f;                                                                          \
    }                                                                                        \
                                                                                              \
    float diff = value - gumm->mu;                                                          \
    float sigma2 = fmax( gumm->mu2 - gumm->mu * gumm->mu, VARMIN );                        \
                                                                                              \
    /* evaluate responsibility */                                                            \
    float gamma = exp( -( diff * diff ) / ( 2.0f * sigma2 ) ) / sqrt( 2.0f * M_PI * sigma2 ); \
    gamma = ( gamma * pi ) / ( ( 1.0f - pi ) * UNIFORMPROB + gamma * pi );                  \
    /* set minimal weight */                                                                  \
    gamma = fmax( gamma, GAMMAMIN ) * weight;                                                \
                                                                                              \
    float value2 = value * value;                                                            \
    /* update mu, mu2 */                                                                      \
    gumm->mu  = ( gumm->mu  * gumm->accumg + gamma * value  ) / ( gumm->accumg + gamma );    \
    gumm->mu2 = ( gumm->mu2 * gumm->accumg + gamma * value2 ) / ( gumm->accumg + gamma );    \
    /* set minimal variance */                                                              \
    gumm->mu2 = fmax( gumm->mu2, gumm->mu * gumm->mu + VARMIN );                              \
                                                                                              \
    /* update weighted average */                                                            \
    gumm->accumg  = 0.99f * gumm->accumg + gamma;                                            \
    gumm->accumall= 0.99f * gumm->accumall + weight;                                          \
    return gamma;                                                                            \
}

//GUMM_UPDATE_DEF(_update,private)
//GUMM_UPDATE_DEF(_update_global,global)

#endif

float GUMM_update( private GUMM* gumm, float value, float weight, float UNIFORMPROB )
{
          float GAMMAMIN    = 1e-6f;
    const float VARMIN    = 1e-6f;
    const float PIMIN      = 0.001f;
    const float PIMAX      = 1.0f;

    /* the current inlier rate */
    float inrate = gumm->accumg / ( gumm->accumall + 1e-8f );
    float pi = clamp( inrate, PIMIN, PIMAX );

    if( pi <= 0.4f || pi >= 0.999f ) {
        /* restart */
        gumm->mu = value;
        /* inverse uniform probability should give us the range and we take 10% of it */
        gumm->mu2 = value * value + ( 1.0f / UNIFORMPROB ) * 0.1f;
        gumm->accumg = 0.8f;
        gumm->accumall = 1.0f;
        return 0.9f;
    }

    float diff = value - gumm->mu;
    float sigma2 = fmax( gumm->mu2 - gumm->mu * gumm->mu, VARMIN );

    /* evaluate responsibility */
    float gamma = exp( -( diff * diff ) / ( 2.0f * sigma2 ) ) / sqrt( 2.0f * M_PI * sigma2 );
    gamma = ( gamma * pi ) / ( ( 1.0f - pi ) * UNIFORMPROB + gamma * pi );
    /* set minimal weight */
    gamma = fmax( gamma, GAMMAMIN ) * weight;

    float value2 = value * value;
    /* update mu, mu2 */
    gumm->mu  = ( gumm->mu  * gumm->accumg + gamma * value  ) / ( gumm->accumg + gamma );
    gumm->mu2 = ( gumm->mu2 * gumm->accumg + gamma * value2 ) / ( gumm->accumg + gamma );
    /* set minimal variance */
    gumm->mu2 = fmax( gumm->mu2, gumm->mu * gumm->mu + VARMIN );

    /* update weighted average */
    gumm->accumg  = 0.999f * gumm->accumg + gamma;
    gumm->accumall= 0.999f * gumm->accumall + weight;
    return gamma;
}

float GUMM_update_global( global GUMM* gumm, float value, float weight, float UNIFORMPROB )
{
          float GAMMAMIN    = 1e-6f;
    const float VARMIN    = 1e-6f;
    const float PIMIN      = 0.001f;
    const float PIMAX      = 1.0f;

    /* the current inlier rate */
    float inrate = gumm->accumg / ( gumm->accumall + 1e-8f );
    float pi = clamp( inrate, PIMIN, PIMAX );

    if( pi <= 0.4f || pi >= 0.999f ) {
        /* restart */
        gumm->mu = value;
        /* inverse uniform probability should give us the range and we take 10% of it */
        gumm->mu2 = value * value + ( 1.0f / UNIFORMPROB ) * 0.1f;
        gumm->accumg = 0.8f;
        gumm->accumall = 1.0f;
        return 0.9f;
    }

    float diff = value - gumm->mu;
    float sigma2 = fmax( gumm->mu2 - gumm->mu * gumm->mu, VARMIN );

    /* evaluate responsibility */
    float gamma = exp( -( diff * diff ) / ( 2.0f * sigma2 ) ) / sqrt( 2.0f * M_PI * sigma2 );
    gamma = ( gamma * pi ) / ( ( 1.0f - pi ) * UNIFORMPROB + gamma * pi );
    /* set minimal weight */
    gamma = fmax( gamma, GAMMAMIN ) * weight;

    float value2 = value * value;
    /* update mu, mu2 */
    gumm->mu  = ( gumm->mu  * gumm->accumg + gamma * value  ) / ( gumm->accumg + gamma );
    gumm->mu2 = ( gumm->mu2 * gumm->accumg + gamma * value2 ) / ( gumm->accumg + gamma );
    /* set minimal variance */
    gumm->mu2 = fmax( gumm->mu2, gumm->mu * gumm->mu + VARMIN );

    /* update weighted average */
    gumm->accumg  = 0.999f * gumm->accumg + gamma;
    gumm->accumall= 0.999f * gumm->accumall + weight;
    return gamma;
}
 #endif
