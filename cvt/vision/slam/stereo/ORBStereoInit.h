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

#ifndef CVT_ORB_STEREO_INIT_H
#define CVT_ORB_STEREO_INIT_H

#include <cvt/vision/slam/stereo/DepthInitializer.h>
#include <cvt/vision/slam/stereo/ORBStereoMatching.h>

namespace cvt
{
   class ORBStereoInit : public DepthInitializer
   {
      public:
          ORBStereoInit( const CameraCalibration& c0, const CameraCalibration& c1, size_t w, size_t h );

          void triangulateFeatures( std::vector<DepthInitResult> & triangulated,
                                    const std::vector<Vector2f> & avoidPositionsImg0,
                                    const Image& view0, const Image& view1 );

          ParamSet& parameters() { return _pset; }

          struct Parameters {
              float     maxEpilineDistance;
              float     maxDescriptorDistance;
              float     maxReprojectionError;
              float     minDepth;
              float     maxDepth;
              uint8_t   fastThreshold;
              uint32_t  orbMaxFeatures;
          };

      private:
          ORBStereoMatching     _matcher;
          ParamSet              _pset;
   };

}

#endif
