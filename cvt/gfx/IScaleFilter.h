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

#ifndef ISCALEFILTER_H
#define ISCALEFILTER_H

#include <string>
#include <cvt/math/Fixed.h>

namespace cvt {

	struct IConvolveAdaptiveSize {
		size_t numw;
		ssize_t incr;
	};

	struct IConvolveAdaptivef {
		IConvolveAdaptiveSize* size;
		float* weights;
	};

	struct IConvolveAdaptiveFixed {
		IConvolveAdaptiveSize* size;
		Fixed* weights;
	};

	class IScaleFilter {
		public:
			IScaleFilter( float support = 1.0f, float sharpsmooth = 0.0f ) : _support( support ), _sharpsmooth( sharpsmooth ) {}
			virtual ~IScaleFilter() {}
			virtual float eval( float x ) const = 0;
			virtual const std::string name() const = 0;
			size_t getAdaptiveConvolutionWeights( size_t dst, size_t src, IConvolveAdaptivef& conva, bool nonegincr = true ) const;
			size_t getAdaptiveConvolutionWeights( size_t dst, size_t src, IConvolveAdaptiveFixed& conva, bool nonegincr = true ) const;

		protected:
			float _support;
			float _sharpsmooth;
	};

	class IScaleFilterBilinear : public IScaleFilter
	{
		public:
			IScaleFilterBilinear( float sharpsmooth = 0.0f ) : IScaleFilter( 1.0f, sharpsmooth ) {}
			virtual float eval( float x ) const;
			virtual const std::string name() const { return "Bilinear"; }
	};

	class IScaleFilterCubic : public IScaleFilter
	{
		public:
			IScaleFilterCubic( float sharpsmooth = 0.0f ) : IScaleFilter( 2.0f, sharpsmooth ) {}
			virtual float eval( float x ) const;
			virtual const std::string name() const { return "Cubic"; }
	};

	class IScaleFilterCatmullRom : public IScaleFilter
	{
		public:
			IScaleFilterCatmullRom( float sharpsmooth = 0.0f ) : IScaleFilter( 2.0f, sharpsmooth ) {}
			virtual float eval( float x ) const;
			virtual const std::string name() const { return "CatmullRom"; }
	};

	class IScaleFilterMitchell : public IScaleFilter
	{
		public:
			IScaleFilterMitchell( float sharpsmooth = 0.0f ) : IScaleFilter( 2.0f, sharpsmooth ) {}
			virtual float eval( float x ) const;
			virtual const std::string name() const { return "Mitchell"; }
	};

	class IScaleFilterLanczos : public IScaleFilter
	{
		public:
			IScaleFilterLanczos( float support = 3.0f, float sharpsmooth = 0.0f ) : IScaleFilter( support, sharpsmooth ) {}
			virtual float eval( float x ) const;
			virtual const std::string name() const { return "Lanczos"; }
	};

	class IScaleFilterBlackman : public IScaleFilter
	{
		public:
			IScaleFilterBlackman( float support = 3.0f, float sharpsmooth = 0.0f ) : IScaleFilter( support, sharpsmooth ) {}
			virtual float eval( float x ) const;
			virtual const std::string name() const { return "Blackman"; }
	};

	class IScaleFilterBlackmanHarris : public IScaleFilter
	{
		public:
			IScaleFilterBlackmanHarris( float support = 3.0f, float sharpsmooth = 0.0f ) : IScaleFilter( support, sharpsmooth ) {}
			virtual float eval( float x ) const;
			virtual const std::string name() const { return "BlackmanHarris"; }
	};

	class IScaleFilterGauss : public IScaleFilter
	{
		public:
			IScaleFilterGauss( float support = 2.0f, float sharpsmooth = 0.0f ) : IScaleFilter( support, sharpsmooth ) {}
			virtual float eval( float x ) const ;
			virtual const std::string name() const { return "Gauss"; }
	};

};


#endif
