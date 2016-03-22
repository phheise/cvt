/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2013, Philipp Heise and Sebastian Klose

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

#ifndef CVT_ROBUST_WEIGHTING_H
#define CVT_ROBUST_WEIGHTING_H

namespace cvt
{
    template <class T>
    struct RobustEstimator
    {
        public:
            virtual ~RobustEstimator(){}
            virtual T weight( T res ) const = 0;
            virtual void setScale( T sigma ) = 0;
            virtual bool isRobust() const { return true; }
    };

    template <typename T>
    struct NoWeighting : public RobustEstimator<T> {
        NoWeighting(){}
        T weight( T ) const { return (T)1; }

        void setThreshold( T /*thresh*/ ){}
        void setScale( T /*sigma*/ ){}
        bool isRobust() const { return false; }
    };

    template <typename T>
    struct Huber : public RobustEstimator<T>
    {
        Huber() : c( ( T )1.345 ), s( 1.0f ) {}

        T weight( T r ) const
        {
            T t = Math::abs( r/s );
            if ( t < c )
                return ( T )1;
            else
                return c / t;
        }

        void setThreshold( T thresh ){ c = thresh; }
        void setScale( T scale ){ s = scale; }

        T c;
        T s;
    };
    
    template <typename T>
    struct PseudoHuber : public RobustEstimator<T>
    {
        PseudoHuber() : c( ( T )1.345 ), s( 1.0f ) {}

        T weight( T r ) const
        {
            return ( T )1.0 / ( Math::sqrt( 1 + Math::sqr( r / ( s * c ) ) ) );
        }

        /* threshold is the slope here */
        void setThreshold( T thresh ){ c = thresh; }
        void setScale( T scale ){ s = scale; }

        T c;
        T s;
    };

    template <typename T>
    struct Tukey : public RobustEstimator<T>
    {
        /**
         *	\brief	cut-off at threshold (0 influence of outliers!)
         */
        Tukey() : c( ( T )4.685 ), s( 1 ) {}

        T weight( T r ) const
        {
            T rs = Math::abs( r / s );
            if ( rs > c )
                return (T)0;
            else
                return Math::sqr( 1 - Math::sqr( rs / c ) );
        }

        void setThreshold( T thresh ){ c = thresh; }
        void setScale( T sigma ){ s = sigma; }

        T c;
        T s;
    };

    template <typename T>
    struct BlakeZisserman : public RobustEstimator<T>
    {
        BlakeZisserman() : c( (T)0.3 ), s( (T)1.0 )
        {
        }

        T weight( T r ) const
        {
            T rs = Math::abs( r / s );
            return 2.0 / ( ( T )1.0 + c * Math::exp( rs * rs ) );
        }

        void setThreshold( T thresh ){ c = thresh; }
        void setScale( T sigma ){ s = sigma; }

        T c;
        T s;
    };

    typedef Huber<float> Huberf;
    typedef PseudoHuber<float> PseudoHuberf;
    typedef Tukey<float> Tukeyf;
    typedef BlakeZisserman<float> BlakeZissermanf;
    typedef NoWeighting<float> NoWeightingf;
}

#endif
