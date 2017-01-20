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

#ifndef CVT_FEATURECL_H
#define CVT_FEATURECL_H

#include <algorithm>
#include <cvt/cl/OpenCL.h>

namespace cvt {


    typedef struct FeatureCL {
        cl_int2 pt;
        cl_float score;

        static void sortPosition( FeatureCL* features, size_t size );
        static void buildScanlineIndex( cl_int2* output, int height, const FeatureCL* features, size_t size );
        void print() const;

        private:
        class CmpPosi
        {
            public:
                bool operator()( const FeatureCL& f1, const FeatureCL& f2 )
                {
                    if( ( int ) f1.pt.y == ( int ) f2.pt.y )
                        return ( int ) f1.pt.x < ( int ) f2.pt.x;
                    return ( int ) f1.pt.y < ( int ) f2.pt.y;
                }
        };

        class CmpYi
        {
            public:
                bool operator()( const FeatureCL& f1, const FeatureCL& f2 )
                {
                    return ( int ) f1.pt.y < ( int ) f2.pt.y;
                }
        };

    } FeatureCL;

    inline void FeatureCL::print() const
    {
        std::cout << "( " << pt.x << " " << pt.y << " ) " << score << std::endl;
    }

    inline void FeatureCL::sortPosition( FeatureCL* features, size_t size )
    {
        FeatureCL::CmpPosi cmp;
        std::sort( features, features + size, cmp );
    }

    inline void FeatureCL::buildScanlineIndex( cl_int2* output, int height, const FeatureCL* features, size_t size )
    {
        int minY = ( int ) features[ 0 ].pt.y;
        int maxY = ( int ) features[ size - 1 ].pt.y;

        FeatureCL::CmpYi cmp;

        /* set everything to invalid before minY */
        int y = 0;
        for(; y < minY; y++ ) {
            output[ y ].x = -1;
            output[ y ].y = 0;
        }

        /* first entry has offset 0 */
        output[ minY ].x = 0;
        y++;

        /* iterate over all scanlines */
        for( ; y <= maxY; y++ ) {
            /* find next element with y bigger than current element*/
            int offset = std::upper_bound( features + output[ y - 1 ].x, features + size, features[ output[ y - 1 ].x ], cmp ) - features;
            /* if we hit the last element we stop */
            if( offset == output[ y - 1 ].x ) {
                break;
            }

            /* set the size of the previous element */
            output[ y - 1 ].y = offset - output[ y - 1 ].x;

            /* set the current y */
            int nexty = ( int ) features[ offset ].pt.y;
            for(; y < nexty; y++ ) {
                output[ y ].x = -1;
                output[ y ].y = 0;
            }
            output[ y ].x = offset;
        }

        output[ y - 1 ].y = size - output[ y - 1 ].x;

        /* set everything to invalid after maxY to height */
        for(; y <= height; y++ ) {
            output[ y ].x = -1;
            output[ y ].y = 0;
        }
    }

}

#endif
