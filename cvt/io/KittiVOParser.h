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

#ifndef CVT_KITTIVOPARSER_H
#define CVT_KITTIVOPARSER_H

#include <cvt/util/String.h>
#include <cvt/math/Matrix.h>
#include <cvt/gfx/Image.h>
#include <cvt/io/StereoInput.h>

namespace cvt {

    class KittiVOParser : public StereoInput
    {
        public:
            KittiVOParser( const cvt::String& folder, bool useColorCams = false );
            ~KittiVOParser();

            const Image&    left()  const { return _left; }
            const Image&    right() const { return _right; }
            bool            nextFrame( size_t timeout );
            const StereoCameraCalibration& stereoCalib() const { return _calib; }

            const cvt::Matrix4d& projectionLeft()  const { return _calibLeft; }
            const cvt::Matrix4d& projectionRight() const { return _calibRight; }

            bool            hasNext() const;
            size_t          size()    const { return _sequence.size(); }
            bool            hasPose() const { return _curSample->hasPose; }
            const Matrix4d& pose()    const { return _curSample->pose; }
            double          stamp()   const { return _curSample->timestamp; }

        private:
            struct Sample {
                double          timestamp;
                cvt::String     leftFile;
                cvt::String     rightFile;
                bool            hasPose;
                cvt::Matrix4d   pose;
            };

            bool                    _useColor;
            size_t                  _iter;
            std::vector<Sample>     _sequence;
            cvt::Matrix4d           _calibLeft;
            cvt::Matrix4d           _calibRight;
            StereoCameraCalibration _calib;

            Image                   _left;
            Image                   _right;
            Sample*                 _curSample;

            void checkFileExistence( const cvt::String& file );
            void loadImageNames( std::vector<cvt::String>& names, const cvt::String& folder );
            void loadPoses( std::vector<cvt::Matrix4d>& poses, const cvt::String& file );
            void loadStamps( std::vector<double>& stamps, const cvt::String& file );
            void loadCalibration( const cvt::String& calibFile );
            void loadImages();

    };

}

#endif // KITTYVODATA_H
