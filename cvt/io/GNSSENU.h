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
#ifndef CVT_GNSSENU_H
#define CVT_GNSSENU_H

#include <cvt/io/GNSS.h>

namespace cvt {

    class GNSSENU {
        public:
            GNSSENU( const Vector3d& lla );
            GNSSENU( double lat, double lon, double alt );
            GNSSENU( const GNSSENU& other );
            ~GNSSENU();

            const Matrix3d& rotation() const;
            const Vector3d& reference() const;

            void LLAtoENU( Vector3d& enu, const Vector3d& lla );
            void ECEFtoENU( Vector3d& enu, const Vector3d& ecef );

            void ENUtoLLA( Vector3d& lla, const Vector3d& enu );
            void ENUtoECEF( Vector3d& ecef, const Vector3d& enu );

            void setReferenceLLA( double lat, double lon, double alt );
            void setReferenceLLA( const Vector3d& lla );

        private:
            Matrix3d _rotation;
            Vector3d _reference;
    };

    inline GNSSENU::GNSSENU( const Vector3d& lla )
    {
        setReferenceLLA( lla );
    }

    inline GNSSENU::GNSSENU( double lat, double lon, double alt )
    {
        setReferenceLLA( Vector3d( lat, lon, alt ) );
    }

    inline GNSSENU::GNSSENU( const GNSSENU& other ) :
        _rotation( other._rotation ),
        _reference( other._reference )
    {
    }

    inline GNSSENU::~GNSSENU()
    {
    }

    inline void GNSSENU::setReferenceLLA( const Vector3d& lla )
    {
        GNSS::ENUTangentRotation( _rotation, lla);
        GNSS::LLAtoECEF( _reference, lla );
    }

    inline const Matrix3d& GNSSENU::rotation() const
    {
        return _rotation;
    }

    inline const Vector3d& GNSSENU::reference() const
    {
        return _reference;
    }

    inline void GNSSENU::LLAtoENU( Vector3d& enu, const Vector3d& lla )
    {
        Vector3d ecef;
        GNSS::LLAtoECEF( ecef, lla );
        ECEFtoENU( enu, ecef );
    }

    inline void GNSSENU::ECEFtoENU( Vector3d& enu, const Vector3d& ecef )
    {
        enu = _rotation * ( ecef - _reference );
    }

    inline void GNSSENU::ENUtoLLA( Vector3d& lla, const Vector3d& enu )
    {
        Vector3d ecef = _rotation.transpose() * enu + _reference;
        GNSS::ECEFtoLLA( lla, ecef );
    }

    inline void GNSSENU::ENUtoECEF( Vector3d& ecef, const Vector3d& enu )
    {
        ecef = _rotation.transpose() * enu + _reference;
    }

}
#endif
