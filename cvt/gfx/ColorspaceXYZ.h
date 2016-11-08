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

#ifndef CVT_COLORSPACEXYZ_H
#define CVT_COLORSPACEXYZ_H

#include <cvt/math/Matrix.h>
#include <cvt/math/Vector.h>

namespace cvt {
    class ColorspaceXYZ
    {
        public:
            ColorspaceXYZ( float xr, float yr,
                          float xg, float yg,
                          float xb, float yb,
                          float xw, float yw );
            ColorspaceXYZ( const ColorspaceXYZ& csxyz );

            Matrix3f matrixXYZtoRGB() const;
            Matrix3f matrixRGBtoXYZ() const;

            static const ColorspaceXYZ AdobeRGB;
            static const ColorspaceXYZ sRGB;
            static const ColorspaceXYZ PALSECAM;

        private:
            Vector2f r, g, b, w; // xy primaries and white-point
    };

    inline ColorspaceXYZ::ColorspaceXYZ( float xr, float yr,
                                        float xg, float yg,
                                        float xb, float yb,
                                        float xw, float yw ) :
        r( xr, yr ),
        g( xg, yg ),
        b( xb, yb ),
        w( xw, yw )
    {
    }

    inline ColorspaceXYZ::ColorspaceXYZ( const ColorspaceXYZ& other ) :
        r( other.r ),
        g( other.g ),
        b( other.b ),
        w( other.w )
    {
    }

    inline Matrix3f ColorspaceXYZ::matrixXYZtoRGB() const
    {
        Matrix3f m = matrixRGBtoXYZ();
        m.inverseSelf();
        return m;
    }

    inline Matrix3f ColorspaceXYZ::matrixRGBtoXYZ() const
    {
        // x, y primaries
        Matrix3f mat( r[ 0 ], g[ 0 ], b[ 0 ],
                     r[ 1 ], g[ 1 ], b[ 1 ],
                     1.0f - r[ 0 ] - r[ 1 ], 1.0f - g[ 0 ] - g[ 1 ], 1.0f - b[ 0 ] - b[ 1 ] );
        // white-point
        Vector3f wp( w[ 0 ], w[ 1 ], 1.0f - w[ 0 ] - w[ 1 ] );
        // Assume 1.0 Luminance
        wp /= w[ 1 ];

        Matrix3f inv = mat.inverse();
        Vector3f s = inv * wp;

        mat[ 0 ] = mat[ 0 ].cmul( s );
        mat[ 1 ] = mat[ 1 ].cmul( s );
        mat[ 2 ] = mat[ 2 ].cmul( s );
        return mat;
    }

}

#endif
