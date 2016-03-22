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

#ifndef CVT_FERNS_H
#define CVT_FERNS_H

#include <cvt/gfx/Image.h>
#include <cvt/vision/PatchGenerator.h>
#include <cvt/vision/internal/Fern.h>
#include <cvt/vision/FAST.h>

#include <Eigen/Core>
#include <vector>
#include <string>
#include <utility>

namespace cvt 
{
	
	class Ferns 
	{
		
		public:
			Ferns( uint32_t patchSize = 21, uint32_t numOverallTests = 300, uint32_t numFerns = 30 );
			Ferns( const std::string & fileName );
			~Ferns();
			
			void train( const Image & img );
			
			double classify( Eigen::Vector2i & bestClass, const Image & img, const Eigen::Vector2i & p );
		
			void match( const::std::vector<Eigen::Vector2i> & features,
						const Image & img,
						std::vector<Eigen::Vector2d> & matchedModel,
					    std::vector<Eigen::Vector2d> & matchedFeatures );
			
			void save( const std::string & fileName );
			
		private:						
			uint32_t	_patchSize;
			uint32_t	_numFerns;
			uint32_t 	_nTests;
			uint32_t	_testsPerFern;
			uint32_t	_trainingSamples;
			
			std::vector<Fern>				_ferns;						
			FAST*                           _featureDetector;			
			std::vector<Feature2Df>	_modelFeatures;	

			void trainClass( size_t idx, PatchGenerator & patchGen, const Image & img );
	};
	
}
#endif