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

#ifndef CVT_QTKITCAMERA_H
#define CVT_QTKITCAMERA_H

#include <cvt/io/Camera.h>
#include <cvt/gfx/Image.h>
#include <cvt/util/String.h>

namespace cvt {

    class QTKitCameraInterface;

    class QTKitCamera : public Camera
    {
        public:
            QTKitCamera( size_t camIndex, const CameraMode & mode );

            virtual ~QTKitCamera();

            size_t          width()     const;
            size_t          height()    const;
            bool            nextFrame( size_t timeout );
            const Image &   frame()     const;
            void            startCapture();
            void            stopCapture();

            const IFormat & format() const;
            const String &  identifier() const;

            static size_t   count();
            static void     cameraInfo( size_t index, CameraInfo & info );

        private:
            QTKitCameraInterface *  _device;
            String                  _identifier;
            Image                   _frame;
            bool                    _capturing;
    };

    inline size_t QTKitCamera::width() const
    {
        return _frame.width();
    }

    inline size_t QTKitCamera::height() const
    {
        return _frame.height();
    }

    inline const Image & QTKitCamera::frame() const
    {
        return _frame;
    }

    inline const IFormat & QTKitCamera::format() const
    {
        return _frame.format();
    }

    inline const String & QTKitCamera::identifier() const
    {
        return _identifier;
    }

}

#endif
