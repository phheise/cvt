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

#ifndef CVT_CHAMELEONSTEREO_H
#define CVT_CHAMELEONSTEREO_H

#include <cvt/gfx/Image.h>
#include <cvt/io/StereoInput.h>
#include <cvt/io/DC1394Camera.h>
#include <cvt/vision/StereoCameraCalibration.h>

namespace cvt {

    class ChameleonStereo : public StereoInput
    {
        public:

            struct Parameters {
                cvt::String leftId;
                cvt::String rightId;
                int leftStrobePin;
                int rightTriggerPin;
            };

            enum SubCamera {
                LEFT,
                RIGHT,
                BOTH
            };

            ChameleonStereo( const Parameters& params );

            ~ChameleonStereo();

            bool nextFrame( size_t timeout );

            /**
              left rectified image
             */
            const Image& left() const { return _leftRect; }

            /**
              left rectified image
             */
            const Image& right() const { return _rightRect; }

            /**
              calibration data (rectified rig)
             */
            const StereoCameraCalibration& stereoCalib() const { return _calib; }

            const String& leftId()  const { return _leftCam->identifier(); }
            const String& rightId() const { return _rightCam->identifier(); }

            void trigger() const { _leftCam->triggerFrame(); }

            /**
             *  @brief set shutter value
             *  @param val  - shutter value in seconds
             */
            void setShutter( float val );

            /**
             *  @brief set gain value
             *  @param val  - gain value in db
             */
            void setGain( float val );

            void setExposure( float val );
            void setWhiteBalance( uint32_t ubValue, uint32_t vrValue );

            void setFps( float fps );
            float fps() const;

            void setPacketSize( size_t n, SubCamera cam = BOTH );
            size_t packetSize( SubCamera cam = BOTH ) const;

            void setAreaOfInterest( const Recti& rect );
            Recti areaOfInterest() const;

            void setAutoShutter( bool val );
            void setAutoGain( bool val );
            void setAutoExposure( bool val );
            void setAutoWhiteBalance( bool val );

            /* enable / disable features */
            void enableAutoExposure( bool val );
            void enableAutoGain( bool val );
            void enableAutoShutter( bool val );
            void enableAutoWhiteBalance( bool val );

            bool gainMode() const;

            void loadPreset( DC1394Camera::CameraPreset preset );

        private:
            StereoCameraCalibration _calib;

            DC1394Camera*           _leftCam;
            DC1394Camera*           _rightCam;

            Image                   _leftRect;
            Image                   _rightRect;

            size_t idxForId( const cvt::String& id ) const;

            void configureLeft( int strobePin );
            void configureRight( int triggerPin );
            void configureStrobe( int pin );
    };

}

#endif // CVT_CHAMELEONSTEREO_H
