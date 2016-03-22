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

#include <cvt/gfx/ifilter/BrightnessContrast.h>
#include <cvt/cl/kernel/BC.h>

namespace cvt {

	static ParamInfo* _params[ 4 ] = {
		new ParamInfoTyped<Image*>( "Input", true /* inputParam */ ),
		new ParamInfoTyped<Image*>( "Output", false ),
		new ParamInfoTyped<float>( "Brightness", -1.0f /* min */, 1.0f /* max */, 0.0f /* default */, true ),
		new ParamInfoTyped<float>( "Contrast", -1.0f /* min */, 1.0f /* max */, 0.0f /* default */, true )
	};

	BrightnessContrast::BrightnessContrast() : IFilter( "BrightnessContrast", _params, 4, IFILTER_OPENCL ), _kernelBC( _BC_source, "BC" )
	{
	}

	BrightnessContrast::~BrightnessContrast()
	{
	}

	void BrightnessContrast::apply( const ParamSet* set, IFilterType t ) const
	{
		Image * in = set->arg<Image*>( 0 );
		Image * out = set->arg<Image*>( 1 );
		float b = set->arg<float>( 2 );
		float c = set->arg<float>( 3 );

		switch ( t ) {
			case IFILTER_OPENCL:
				this->applyOpenCL( *out, *in, b, c );
				break;
			default:
				throw CVTException( "Not implemented" );
		}
	}

	void BrightnessContrast::applyOpenCL( Image& dst, const Image& src, float brightness, float contrast ) const
	{
		size_t w, h;
		w = src.width();
		h = src.height();
		_kernelBC.setArg( 0, dst );
		_kernelBC.setArg( 1, src );
		_kernelBC.setArg( 2, brightness );
		_kernelBC.setArg( 3, contrast );
		_kernelBC.run( CLNDRange( w, h ), CLNDRange( 8, 8 ) );
	}

}
