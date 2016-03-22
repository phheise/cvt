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
			void setWhitepointFromTemperature( float T );

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
										 float xw, float yw ) : r( xr, yr ), g( xg, yg ), b( xb, yb ), w( xw, yw )
	{
	}

	inline ColorspaceXYZ::ColorspaceXYZ( const ColorspaceXYZ& other ) : r( other.r ), g( other.g ), b( other.b ), w( other.w )
	{
	}

	/*
		This shit is patented, although it is just a cubic spline approximating the Planckian locus in xy space:

		Publication number:	US2003095138 (A1)
		Publication date:	2003-05-22
		Inventor(s):	KIM YOUNG-SUN [KR]; CHO BONG-HWAN [KE]; KANG BONG-SOON [KR]; HONG DOO-IL [KR]
					+ (KIM YOUNG-SUN, ; CHO BONG-HWAN, ; KANG BONG-SOON, ; HONG DOO-IL)
		Applicant(s):	SAMSUNG ELECTRONICS CO LTD [US] + (SAMSUNG ELECTRONICS CO., LTD)
		Classification:
		- international:	G06T11/00; H04N9/73; G06T11/00; H04N9/73; (IPC1-7): G09G5/02
		- European:			G06T11/00C; H04N9/73
		Application number:	US20020270390 20021015
		Priority number(s):	KR20010063418 20011015
	 */
	inline void ColorspaceXYZ::setWhitepointFromTemperature( float _T )
	{
		double T = Math::clamp( _T, 1667.0f, 25000.0f );
		double T2, T3, p, p2, p3;
		double xw, yw;

		T2 = T * T;
		T3 = T2 * T;

		if( T < 4000 )
			xw = -0.2661239 * 1e9 / T3 - 0.2343589 * 1e6 / T2 + 0.8776956 *1e3 / T + 0.179910;
		else
			xw = -3.0258469 * 1e9 / T3 + 2.1070379 * 1e6 / T2 + 0.2226347 *1e3 / T + 0.24039;

		p = xw;
		p2 = p * p;
		p3 = p2 * p;

		if( T < 2222 )
			yw = -1.1063814 * p3 - 1.34811020 * p2 + 2.18555832 * p - 0.20219683;
		else if( T < 4000 )
			yw = -0.9549476 * p3 - 1.37418593 * p2 + 2.09137015 * p - 0.16748867;
		else
			yw = 3.0817580 * p3 - 5.8733867 * p2 + 3.75112997 * p - 0.37001483;
		w.set( xw, yw );
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
