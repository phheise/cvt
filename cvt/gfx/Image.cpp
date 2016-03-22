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

#include <cvt/gfx/Image.h>
#include <cvt/gfx/ImageAllocatorMem.h>
#include <cvt/gfx/ImageAllocatorCL.h>
#include <cvt/gfx/ImageAllocatorGL.h>
#include <cvt/gfx/IExpr.h>
#include <cvt/gfx/GFXEngineImage.h>
#include <cvt/gfx/IMapScoped.h>

#include <cvt/math/Math.h>
#include <cvt/util/SIMD.h>
#include <cvt/util/Exception.h>
#include <cvt/util/PluginManager.h>

#include <cvt/gfx/IConvert.h>
#include <cvt/gfx/IFill.h>
#include <cvt/gfx/IMorphological.h>
#include <cvt/gfx/IThreshold.h>
#include <cvt/gfx/IConvolve.h>
#include <cvt/gfx/IBoxFilter.h>
#include <cvt/gfx/ICanny.h>
#include <cvt/gfx/IColorCode.h>
#include <cvt/gfx/IDecompose.h>


#include <fstream>

namespace cvt {

	Image::Image( size_t w, size_t h, const IFormat & format, IAllocatorType memtype )
	{
		if( memtype == IALLOCATOR_CL )
			_mem = new ImageAllocatorCL();
		else if( memtype == IALLOCATOR_GL )
			_mem = new ImageAllocatorGL();
		else
			_mem = new ImageAllocatorMem();
	    _mem->alloc( w, h, format );
	}


	Image::Image( size_t w, size_t h, const IFormat & format, uint8_t* data, size_t stride )
	{
		_mem = new ImageAllocatorMem();
		ImageAllocatorMem * memAllocator = (ImageAllocatorMem *)_mem;
		memAllocator->alloc( w, h, format, data, stride );
	}


	Image::Image( const Image& img, IAllocatorType memtype )
	{
		if( memtype == IALLOCATOR_CL )
			_mem = new ImageAllocatorCL();
		else if( memtype == IALLOCATOR_GL )
			_mem = new ImageAllocatorGL();
		else
			_mem = new ImageAllocatorMem();
		_mem->copy( img._mem );
	}

	Image::Image( const String & fileName, IAllocatorType memtype )
	{
		if( memtype == IALLOCATOR_CL )
			_mem = new ImageAllocatorCL();
		else if( memtype == IALLOCATOR_GL )
			_mem = new ImageAllocatorGL();
		else
			_mem = new ImageAllocatorMem();
		this->load( fileName.c_str() );
	}

	Image::Image( const Image& source, const Recti* roi, bool ref, IAllocatorType memtype )
	{
		if( !ref ){
			if( memtype == IALLOCATOR_CL )
				_mem = new ImageAllocatorCL();
			else if( memtype == IALLOCATOR_GL )
				_mem = new ImageAllocatorGL();
			else
				_mem = new ImageAllocatorMem();
			_mem->copy( source._mem, roi );
		} else {
			throw CVTException("Shared image memory not implemented yet");
		}
	}

	void Image::reallocate( size_t w, size_t h, const IFormat & format, IAllocatorType memtype )
	{
		if( _mem->_width == w && _mem->_height == h && _mem->_format == format && _mem->type() == memtype )
			return;
		if( _mem->type() != memtype ) {
			delete _mem;
			if( memtype == IALLOCATOR_CL )
				_mem = new ImageAllocatorCL();
			else if( memtype == IALLOCATOR_GL )
				_mem = new ImageAllocatorGL();
			else
				_mem = new ImageAllocatorMem();
		}
		_mem->alloc( w, h, format );
	}

	void Image::copyRect( int x, int y, const Image& img, const Recti & rect )
	{
		checkFormat( img, __PRETTY_FUNCTION__, __LINE__, _mem->_format );
		int tx, ty;

		tx = -x + rect.x;
		ty = -y + rect.y;
		Recti rdst( 0, 0, ( int ) _mem->_width, ( int ) _mem->_height );
		rdst.translate( tx, ty );
		Recti rsrc( 0, 0, ( int ) img._mem->_width, ( int ) img._mem->_height );
		rsrc.intersect( rect );
		rsrc.intersect( rdst );
		if( rsrc.isEmpty() )
			return;
		rdst.copy( rsrc );
		rdst.translate( -tx, -ty );

		SIMD* simd = SIMD::instance();
		size_t dstride;
		uint8_t* dst = map( &dstride );
		uint8_t* dbase = dst;
		dst += rdst.y * dstride + bpp() * rdst.x;

		size_t sstride;
		const uint8_t* src = img.map( &sstride );
		const uint8_t* sbase = src;
		src += rsrc.y * sstride + rsrc.x * img.bpp();

		size_t n = rsrc.width * img.bpp();
		size_t i = rsrc.height;

		while( i-- ) {
			simd->Memcpy( dst, src, n );
			src += sstride;
			dst += dstride;
		}
		img.unmap( sbase );
		unmap( dbase );
	}


	Image::~Image()
	{
		delete _mem;
	}

	Image& Image::operator=( const Color& c )
	{
		fill( c );
		return *this;
	}

	GFXEngine* Image::gfxEngine()
    {
        return new GFXEngineImage( *this );
    }

	std::ostream& operator<<( std::ostream &out, const Image &f )
	{
		static const char* _mem_string[] = {
			"MEM",
			"CL",
			"GL"
		};

		out << "Size: " << f.width() << " x " << f.height() << " "
			<< f.format()
			<< " Memory:" << _mem_string[ f.memType() ] << std::endl;
		return out;
	}

	void Image::checkFormat( const Image & img, const char* func, size_t lineNum, const IFormat & format ) const
	{
		if( format != img.format() ){
			throw Exception("Image formats differ: channel order check failed", "Image", lineNum, func);
		}
	}

	void Image::checkSize( const Image & img, const char* func, size_t lineNum, size_t w, size_t h ) const
	{
		if( w != img.width() ){
			throw Exception("Image formats differ: width check failed", "Image", lineNum, func);
		}

		if( h != img.height() ){
			throw Exception("Image formats differ: height check failed", "Image", lineNum, func);
		}
	}

	void Image::checkFormatAndSize( const Image & img, const char* func, size_t lineNum ) const
	{
		checkFormat(img, func, lineNum, _mem->_format );
		checkSize(img, func, lineNum, _mem->_width, _mem->_height );
	}

	void Image::load( const String& path, ILoader* loader )
	{
		if( !loader ) {
			loader = PluginManager::instance().getILoaderForFilename( path );
			if( !loader ){
                String message( "No ILoader for file available: " );
                message += path;
				throw CVTException( message.c_str() );
            }
		}
		loader->load( *this, path );
	}

	void Image::save( const String& path, ISaver* saver ) const
	{
		if( !saver ) {
			saver = PluginManager::instance().getISaverForFilename( path );
			if( !saver )
				throw CVTException( "No ISaver for file available" );
		}
		saver->save( path, *this );
	}

	void Image::convert( Image& dst, IConvertFlags flags ) const
	{
		IConvert::convert( dst, *this, flags );
	}

	void Image::convert( Image & dst, const IFormat & dstFormat, IConvertFlags flags  ) const
	{
		dst.reallocate( _mem->_width, _mem->_height, dstFormat, dst.memType() );
		IConvert::convert( dst, *this, flags );
	}

	void Image::convert( Image& dst, const IFormat & dstformat, IAllocatorType memtype, IConvertFlags flags ) const
	{
		dst.reallocate( _mem->_width, _mem->_height, dstformat, memtype );
		IConvert::convert( dst, *this, flags );
	}

	void Image::fill( const Color& c )
	{
		IFill::fill( *this, c );
	}

    void Image::decompose( Image& chan1, Image& chan2, Image& chan3, Image& chan4 ) const
    {
        IDecompose::decompose( chan1, chan2, chan3, chan4, *this );
    }

    void Image::decompose( Image& chan1, Image& chan2, Image& chan3 ) const
    {
        IDecompose::decompose( chan1, chan2, chan3, *this );
    }

    void Image::decompose( Image& chan1, Image& chan2 ) const
    {
        IDecompose::decompose( chan1, chan2, *this );
    }

	void Image::dilate( Image& dst, size_t radius ) const
	{
		IMorphological::dilate( dst, *this, radius );
	}

	void Image::erode( Image& dst, size_t radius ) const
	{
		IMorphological::erode( dst, *this, radius );
	}


	void Image::threshold( Image& dst, float threshold ) const
	{
		IThreshold::threshold( dst, *this, threshold );
	}

	void Image::thresholdAdaptive( Image& dst, const Image& boxfiltered, float threshold ) const
	{
		IThreshold::thresholdAdaptive( dst, *this, boxfiltered, threshold );
	}

	void Image::convolve( Image& dst, const IKernel& kernel ) const
	{
		IConvolve::convolve( dst, *this, kernel );
	}

	void Image::convolve( Image& dst, const IKernel& hkernel, const IKernel& vkernel ) const
	{
		IConvolve::convolve( dst, *this, hkernel, vkernel );
	}

	void Image::boxfilter( Image& dst, size_t hradius, size_t vradius ) const
	{
		IBoxFilter::boxfilter( dst, *this, hradius, vradius );
	}

	void Image::colorCode( Image& dst, IColorCodeMap map, float min, float max, bool sRGB ) const
	{
		IColorCode::colorCode( dst, *this, map, min, max, sRGB );
	}


    void Image::canny( Image& dst, float low, float high ) const
    {
        ICanny::detectEdges( dst, *this, low, high );
    }

}
