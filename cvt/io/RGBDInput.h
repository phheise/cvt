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

#ifndef CVT_RGBDINPUT_H
#define CVT_RGBDINPUT_H

#include <cvt/gfx/Image.h>
#include <cvt/math/Matrix.h>
#include <cvt/vision/CameraCalibration.h>

namespace cvt {

    class RGBDInput
    {
        public:
            virtual ~RGBDInput(){}

            virtual void            next() = 0;

            virtual bool            hasGroundTruthPose() const { return false; }

            virtual cvt::Matrix4d   groundTruthPose() const
            {
                cvt::Matrix4d m;
                m.setIdentity();
                return m;
            }

			virtual const Image&	depth() const = 0;
			virtual const Image&	rgb()   const = 0;
			virtual double			stamp() const { return 0.0; }

            const CameraCalibration& calibration() const { return _calib; }
            void setCalibration( const CameraCalibration& calib ) { _calib = calib; }

        protected:
            CameraCalibration   _calib;

    };

}

#endif // CVT_RGBDINPUT_H
