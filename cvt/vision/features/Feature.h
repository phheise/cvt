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

#ifndef CVT_FEATURE_H
#define CVT_FEATURE_H

#include <cvt/math/Vector.h>

namespace cvt {

    struct Feature {
        Feature( float x = 0, float y = 0, float angle = 0.0f, int octave = 0, float score = 0.0f );        
        Feature( const Feature& other );

        Vector2f pt;
        float angle;
        int octave;
        float score;
    };

    inline Feature::Feature( float x, float y, float a, int o, float sc ) : pt( x, y ), angle( a ), octave( o ), score( sc )
    {
    }

    inline Feature::Feature( const Feature& other ) :
        pt( other.pt ), angle( other.angle ), octave( other.octave ), score( other.score )
    {
    }

    inline std::ostream& operator<<( std::ostream& out, const Feature& f )
    {
        out << " | " << f.pt.x << " " << f.pt.y << " |  ( " << f.score << " , " << f.angle << " , " << f.octave <<  " ) ";
        return out;
    }

}

#endif
