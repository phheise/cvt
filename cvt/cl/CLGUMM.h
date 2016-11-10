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
#ifndef CVT_CLGUMM_H
#define CVT_CLGUMM_H

namespace cvt {

    typedef struct {
        cl_float mu;
        cl_float mu2;
        cl_float accumg;
        cl_float accumall;

        inline void init( float mu, float variance, float pi, float accum )
        {
            this->mu        = mu;
            this->mu2      = mu * mu + variance;
            this->accumg    = accum * pi;
            this->accumall  = accum;
        }

        inline float mean() const
        {
            return mu;
        }

        inline float variance() const
        {
            return mu2 - mu * mu;
        }

        inline float inlier_rate() const
        {
            return accumg / accumall;
        }

        inline float update( float value, float weight, float UNIFORMPROB )
        {
            float GAMMAMIN   = 1e-6f;
            const float VARMIN = 1e-6f;
            const float PIMIN  = 0.001f;
            const float PIMAX  = 1.0f;
            //  const float UNIFORMPROB = 0.5f; // -1 to 1

            // the current inlier rate
            float pi = Math::clamp( inlier_rate() , PIMIN, PIMAX );

            if( pi <= 0.4f || pi >= 0.999f /*|| variance() > 0.025f / UNIFORMPROB*/ ) {
                //GAMMAMIN = 0.5f + pi;
                // restart
                mu = value;
                // inverse uniform probability should give us the range and we take 10% of it
                mu2 = value * value + ( 1.0f / UNIFORMPROB ) * 0.1f;
                accumg = 0.8f;
                accumall = 1.0f;
                return 0.9f;
            }

            float diff = value - mu;
            float sigma2 = Math::max( mu2 - mu * mu, VARMIN );

            // evaluate responsibility
            float gamma = exp( -( diff * diff ) / ( 2.0f * sigma2 ) ) / sqrt( 2.0f * M_PI * sigma2 );
            gamma = ( gamma * pi ) / ( ( 1.0f - pi ) * UNIFORMPROB + gamma * pi );
            // set minimal weight
            gamma = Math::max( gamma, GAMMAMIN ) * weight;

            float value2 = value * value;
            // update mu, mu2
            mu  = ( mu  * accumg + gamma * value  ) / ( accumg + gamma );
            mu2 = ( mu2 * accumg + gamma * value2 ) / ( accumg + gamma );
            // set minimal variance
            mu2 = Math::max( mu2, mu * mu + VARMIN );

            // update weighted average
            accumg  = 0.99f * accumg + gamma;
            accumall= 0.99f * accumall + weight;

            return gamma;
        }

    } CLGUMM;

}

#endif
