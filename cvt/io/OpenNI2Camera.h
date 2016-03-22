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

#ifndef CVT_OPENNI2_CAMERA_H
#define CVT_OPENNI2_CAMERA_H

#include <cvt/io/Camera.h>
#include <cvt/io/RGBDInput.h>
#include <openni2/OpenNI.h>

namespace cvt
{
    class OpenNI2Camera : public Camera, public RGBDInput
    {
        public:
            OpenNI2Camera( size_t idx, const CameraMode& mode );
            ~OpenNI2Camera();

            void			startCapture();
            void			stopCapture();
            bool			nextFrame( size_t timeOut = 30 );
            const Image&	frame() const { return _rgb; }
            const Image&	depth() const { return _depth; }

            size_t			width()  const { return _rgb.width();  }
            size_t			height() const { return _rgb.height(); }
            const IFormat&	format() const { return _rgb.format(); }
            const String&	identifier() const { return _identifier;}

            void            setSyncRGBDepth( bool val );

            // enable / disable registration of RGB to Depth frame
            void            setRegisterDepthToRGB( bool val );

            void            setAutoExposure( bool val );
            void            setAutoWhiteBalance( bool val );
            bool            autoExposure();
            bool            autoWhiteBalance();

            int             exposure();
            void            setExposure( int val );
            int             gain();
            void            setGain( int val );
            bool            isRGBMirroring() const;
            bool            isDepthMirroring() const;
            void            setRGBMirroring( bool val );
            void            setDepthMirroring( bool val );

            static size_t	count();
            static void		cameraInfo( size_t index, CameraInfo & info );

            /* RGBDInput interface */
            const Image&    rgb() const { return frame(); }
            double          stamp() const { return _rgbStamp; }
            double          depthStamp() const { return _depthStamp; }
            void            next() { nextFrame( 30 ); }

        private:
            Image				_rgb;
            Image				_depth;
            String				_identifier;

            openni::Device      _device;
            openni::VideoStream _rgbStream;
            openni::VideoStream _depthStream;

            uint64_t            _rgbStamp;
            uint64_t            _depthStamp;


    };
}

#endif
