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
#ifndef CVT_GNSS_H
#define CVT_GNSS_H

#include <cvt/math/Vector.h>
#include <cvt/math/Matrix.h>

namespace cvt {

    class GNSS {
        public:
            static void LLAtoECEF( Vector3d& ecef, double latitude, double longitude, double altitude );
            static void LLAtoECEF( Vector3d& ecef, const Vector3d& lla );

            static void ECEFtoLLA( Vector3d& lla, const Vector3d& ecef );
            static void ECEFtoLLA( Vector3d& lla, double x, double y, double z );

            static void ENUTangentRotation( Matrix3d& rotation, const Vector3d& lla );

            static const double WGS84_SEMI_MAJOR_a    = 6378137.0;
            static const double WGS84_SEMI_MINOR_b    = 6356752.314245;
            static const double WGS84_INV_FATTENING_f = 298.257223563;
            static const double WGS84_E_SQR           = 0.0066943799901411239067;
            static const double WGS84_E_PRIME_SQR     = 0.00673949674227623894;

        private:
            GNSS();
            GNSS( const GNSS& );
            ~GNSS();
    };

    inline void GNSS::LLAtoECEF( Vector3d& ecef, const Vector3d& lla )
    {
        double latsin, latcos;
        double lonsin, loncos;

        latsin = Math::sin( Math::deg2Rad( lla.x ) );
        latcos = Math::cos( Math::deg2Rad( lla.x ) );

        lonsin = Math::sin( Math::deg2Rad( lla.y ) );
        loncos = Math::cos( Math::deg2Rad( lla.y ) );

        const double N = WGS84_SEMI_MAJOR_a / Math::sqrt( 1.0 - WGS84_E_SQR * Math::sqr( latsin ) );

        ecef.x = ( N + lla.z ) * latcos * loncos;
        ecef.y = ( N + lla.z ) * latcos * lonsin;
        // 1 - WGS84_E_SQR = 1 - ( ( a^2-b^2 ) / a^2  ) = ( a^2 - a^2 + b^2  ) / ( a^2 ) = b^2 / a^2
        ecef.z = ( N * ( 1 - WGS84_E_SQR ) + lla.z ) * latsin;
    }

    inline void GNSS::LLAtoECEF( Vector3d& ecef, double lat, double lon, double alt )
    {
        LLAtoECEF( ecef, Vector3d( lat, lon, alt ) );
    }

    inline void GNSS::ECEFtoLLA( Vector3d& lla, const Vector3d& ecef )
    {
        const int ITERMAX = 10;
        const double p = Math::sqrt( ecef.x * ecef.x + ecef.y * ecef.y );

        double lon = Math::atan2( ecef.y, ecef.x );
        double lat = Math::atan2( ecef.z, p * ( 1.0 - WGS84_E_SQR ) );
        double alt;

        // if the point is at the pole, we treat it different
        if( Math::abs( Math::cos( lat ) -  0 ) < Math::epsilon<double>() ) {
            alt = Math::abs( ecef.z ) - WGS84_SEMI_MINOR_b;
        } else {
            for( int iter = 0; iter < ITERMAX; iter++ ) {
                const double sLat = Math::sin( lat );
                const double cLat = Math::cos( lat );
                const double N =  WGS84_SEMI_MAJOR_a / Math::sqrt( 1.0 - WGS84_E_SQR * sLat * sLat );
                alt = ( p / cLat ) - N;
                lat = Math::atan2( ecef.z, p * ( 1.0 - WGS84_E_SQR * ( N / ( N + alt ) ) ) );
            }
        }

        lla.x = Math::rad2Deg( lat );
        lla.y = Math::rad2Deg( lon );
        lla.z = alt;
    }

    inline void GNSS::ECEFtoLLA( Vector3d& lla, double x, double y, double z )
    {
        ECEFtoLLA( lla, Vector3d( x, y, z ) );
    }

    inline void GNSS::ENUTangentRotation( Matrix3d& rotation, const Vector3d& lla )
    {
        double latsin, latcos;
        double lonsin, loncos;

        latsin = Math::sin( Math::deg2Rad( lla.x ) );
        latcos = Math::cos( Math::deg2Rad( lla.x ) );

        lonsin = Math::sin( Math::deg2Rad( lla.y ) );
        loncos = Math::cos( Math::deg2Rad( lla.y ) );

        rotation(0, 0) = -lonsin;
        rotation(0, 1) = loncos;
        rotation(0, 2) = 0;

        rotation(1, 0) = -latsin * loncos;
        rotation(1, 1) = -latsin * lonsin;
        rotation(1, 2) = latcos;

        rotation(2, 0) = latcos * loncos;
        rotation(2, 1) = latcos * lonsin;
        rotation(2, 2) = latsin;
    }
}
#endif
