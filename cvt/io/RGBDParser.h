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

#ifndef CVT_RGBDPARSER_H
#define CVT_RGBDPARSER_H

#include <cvt/util/String.h>
#include <cvt/util/Data.h>
#include <cvt/util/DataIterator.h>
#include <cvt/io/FileSystem.h>
#include <cvt/io/RGBDInput.h>
#include <cvt/gfx/Image.h>
#include <cvt/math/Matrix.h>

namespace cvt
{
    class RGBDParser : public RGBDInput
    {
        public:
            struct RGBDSample
            {
                Image		rgb;
                Image		depth;

                /**
                 * @brief orientation from world: q_wc
                 */
                Quaterniond orientation;
                Vector3d    position;

                bool        poseValid;
                double		stamp;

                template<typename T>
                Matrix4<T> pose() const
                {
                    Matrix4d mat = orientation.toMatrix4();
                    mat[ 0 ][ 3 ] = position.x;
                    mat[ 1 ][ 3 ] = position.y;
                    mat[ 2 ][ 3 ] = position.z;
                    return ( Matrix4<T> )mat;
                }
            };

            RGBDParser( const String& folder, double maxStampDiff = 0.05 );

            void next();

            size_t				iter()    const { return _idx; }
            size_t				size()    const { return _stamps.size(); }            
            bool                hasNext() const { return _idx < _stamps.size(); }

            bool                hasGroundTruthPose() const { return _sample.poseValid; }
			Matrix4d            groundTruthPose()    const { return _sample.pose<double>(); }
            const Image&        depth() const { return _sample.depth; }
			const Image&        rgb()   const { return _sample.rgb; }
			double				stamp() const { return _sample.stamp; }

			const RGBDSample&	data()  const { return _sample; }
            void                setIdx( size_t idx ) { _idx = idx; }
            const String&       rgbFile( size_t idx ) const { return _rgbFiles[ idx ]; }
            const String&       depthFile( size_t idx ) const { return _depthFiles[ idx ]; }

        private:
            const double			 _maxStampDiff;
            String					 _folder;

            std::vector<Quaterniond> _orientations;
            std::vector<Vector3d>    _positions;
            std::vector<String>		 _rgbFiles;
            std::vector<String>		 _depthFiles;
            std::vector<double>		 _stamps;
            std::vector<bool>		 _poseValid;

            RGBDSample				_sample;
            size_t					_idx;


            void loadGroundTruth();
            void loadRGBFilenames( std::vector<double> & stamps );
            void loadDepthFilenames( std::vector<double> & stamps );
            void loadDepthAndRGB( std::vector<double>& rgbStamps, std::vector<double>& depthStamps );

            bool readNext( Quaterniond& orientation, Vector3d& position, double& stamp, DataIterator& iter );

            bool readNextFilename( String& filename, double& stamp, DataIterator& iter );

            void sortOutData( const std::vector<double>& rgbStamps,
                              const std::vector<double>& depthStamps,
                              bool rgbAndDepthAssocitated );

            size_t findClosestMatchInGTStamps( double val, size_t startIdx );

            size_t findClosestMatchInDepthStamps( double stamp,
                                                  size_t dIdx,
                                                  const std::vector<double>& depth );
    };


}

#endif
