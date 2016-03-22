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

#ifndef CVTIMAGE_H
#define CVTIMAGE_H
#include <iostream>

#include <stdlib.h>
#include <stdint.h>

#include <cvt/gfx/Color.h>
#include <cvt/gfx/IFormat.h>
#include <cvt/gfx/IScaleFilter.h>
#include <cvt/gfx/IConvert.h>
#include <cvt/geom/Rect.h>
#include <cvt/gfx/ImageAllocator.h>
#include <cvt/gfx/IKernel.h>
#include <cvt/util/String.h>
#include <cvt/gfx/IExprType.h>
#include <cvt/gfx/IColorCodeMap.h>
#include <cvt/gfx/Drawable.h>

namespace cvt {
	class ISaver;
	class ILoader;

	template<typename T1, typename T2, IExprType op> class IExprBinary;

	class Image : public Drawable
	{
		friend std::ostream& operator<<(std::ostream &os, const Image &f);
		friend class CLKernel;
		// FIXME: provide method to access allocator
		friend class GLDrawImageProg;
		friend class GLTexMLSProg;

		public:
			Image( size_t w = 1, size_t h = 1, const IFormat & format = IFormat::RGBA_UINT8, IAllocatorType memtype = IALLOCATOR_MEM );
			Image( size_t w, size_t h, const IFormat & format, uint8_t* data, size_t stride = 0 );
			Image( const Image& img, IAllocatorType memtype = IALLOCATOR_MEM );
			Image( const String & fileName, IAllocatorType memtype = IALLOCATOR_MEM );
			Image( const Image& source, const Recti* roi, bool ref = false, IAllocatorType memtype = IALLOCATOR_MEM );
			~Image();
			size_t width() const;
			size_t height() const;
			Recti rect() const;

			size_t channels() const;

			/**
			 * @return bits per channel
			 */
			size_t bpc() const;

			/**
			 * @return bytes per pixel!
			 */
			size_t bpp() const;

			const IFormat & format() const;
			IAllocatorType memType() const { return _mem->type(); }
			uint8_t* map( size_t* stride ) { return _mem->map( stride ); }
			const uint8_t * map( size_t* stride ) const { return ( const uint8_t* ) _mem->map( stride ); }
			template<typename _T> _T* map( size_t* stride );
			template<typename _T> const _T* map( size_t* stride ) const;
			void unmap( const uint8_t* ptr ) const { _mem->unmap( ptr ); }
			template<typename _T> void unmap( const _T* ptr ) const;

			void reallocate( size_t w, size_t h, const IFormat & format = IFormat::RGBA_UINT8, IAllocatorType memtype = IALLOCATOR_MEM );
			void reallocate( const Image& i, IAllocatorType memtype = IALLOCATOR_MEM );

			void copyRect( int x, int y, const Image& i, const Recti & roi );

			Image* clone() const;
			void convert( Image& dst, const IFormat & format, IAllocatorType memtype, IConvertFlags flags = ICONVERT_DEBAYER_LINEAR  ) const;
			void convert( Image& dst, const IFormat & format, IConvertFlags flags = ICONVERT_DEBAYER_LINEAR  ) const;
			void convert( Image& dst, IConvertFlags flags = ICONVERT_DEBAYER_LINEAR  ) const;
			void scale( Image& dst, size_t width, size_t height, const IScaleFilter& filter ) const;

			void load( const String& path, ILoader* loader = NULL );
			void save( const String& path, ISaver* loader = NULL ) const;

			void fill( const Color& c );

			void add( float alpha = 0.0f );
			void sub( float alpha = 0.0f );
			void mul( float alpha = 1.0f );
			void add( const Image& i );
			void sub( const Image& i );
			void mul( const Image& i );
			void mad( const Image& i, float alpha = 1.0f );

			float ssd( const Image& i ) const;
			float sad( const Image& i ) const;

			void add( const Color& c );
			void sub( const Color& c );
			void mul( const Color& c );

			void convolve( Image& dst, const IKernel& kernel ) const;
			void convolve( Image& dst, const IKernel& hkernel, const IKernel& vkernel ) const;

            /**
              @brief Decompose RGBA/BGRA image to seperate images containing the single channels
             */
            void decompose( Image& chan1, Image& chan2, Image& chan3, Image& chan4 ) const;

            /**
              @brief Decompose RGBA/BGRA image to seperate images containing the single channels except
                     for the alpha channel which is skipped.
             */
            void decompose( Image& chan1, Image& chan2, Image& chan3 ) const;

            /**
              @brief Decompose GRAY-ALPHA image to sperate image conating the GRAY and the ALPHA channel only
             */
            void decompose( Image& chan1, Image& chan2 ) const;

			void dilate( Image& dst, size_t radius ) const;
			void erode( Image& dst, size_t radius ) const;

			void threshold( Image& dst, float threshold ) const;
			void thresholdAdaptive( Image& dst, const Image& boxfiltered, float threshold ) const;

			void boxfilter( Image& dst, size_t hradius, size_t vradius = 0 ) const;

			void colorCode( Image& dst, IColorCodeMap map, float min = 0.0f, float max = 1.0f, bool sRGB = true ) const;

            void canny( Image& dst, float low, float hight ) const;

            void minmax( float& min, float& max, float mininit = INFINITY, float maxinit = -INFINITY ) const;

			Image& assign( const Image& c, IAllocatorType memtype = IALLOCATOR_MEM );

			Image& operator=( const Color& c );
			Image& operator=( const Image& c );


			template<typename T1, typename T2, IExprType op>
			Image& operator=( const IExprBinary<T1,T2,op>& expr );

			void warpBilinear( Image& idst, const Image& warp ) const;

            void integralImage( Image & dst ) const;
            void squaredIntegralImage( Image & dst ) const;

            void pyrdown( Image& dst ) const;

            void printValues( std::ostream& o, const Recti& rect ) const;

            GFXEngine* gfxEngine();

		private:
			void scaleFloat( Image& idst, size_t width, size_t height, const IScaleFilter& filter ) const;
			void scaleU8( Image& idst, size_t width, size_t height, const IScaleFilter& filter ) const;

			void checkFormat( const Image & img, const char* func, size_t lineNum, const IFormat & format ) const;
			void checkSize( const Image & img, const char* func, size_t lineNum, size_t w, size_t h ) const;
			void checkFormatAndSize( const Image & img, const char* func, size_t lineNum ) const;

			void pyrdown1U8( Image& dst ) const;

			ImageAllocator* _mem;
	};

	std::ostream& operator<<(std::ostream &out, const Image &f);

	inline Image* Image::clone() const
	{
		return new Image( *this );
	}

	const inline IFormat & Image::format() const
	{
		return _mem->_format;
	}

	inline size_t Image::width() const
	{
		return _mem->_width;
	}

	inline size_t Image::height() const
	{
		return _mem->_height;
	}

    inline Recti Image::rect() const
    {
        return Recti( 0, 0, this->width(), this->height() );
    }

	inline void Image::reallocate( const Image& i, IAllocatorType memtype )
	{
		reallocate( i._mem->_width, i._mem->_height, i._mem->_format, memtype );
	}

	template<typename _T>
	inline _T* Image::map( size_t* stride )
	{
		uint8_t* ret = _mem->map( stride );
		*stride /= sizeof( _T );
		return ( _T * ) ret;
	}

	template<typename _T>
	inline const _T* Image::map( size_t* stride ) const
	{
		const uint8_t* ret = _mem->map( stride );
		*stride /= sizeof( _T );
		return ( const _T * ) ret;
	}

	template<typename _T>
	inline void Image::unmap( const _T* ptr ) const
	{
		 _mem->unmap( ( uint8_t* ) ptr );
	}

	inline Image& Image::operator=( const Image& img )
	{
		if( this != &img )
			_mem->copy( img._mem );
		return *this;
	}

	inline Image& Image::assign( const Image& img, IAllocatorType memtype )
    {
        if( this == &img )
			throw CVTException("Can't self assign image!");

        reallocate( img, memtype );
        _mem->copy( img._mem );
		return *this;
    }

	inline size_t Image::channels() const
	{
		return _mem->_format.channels;
	}

	inline size_t Image::bpc() const
	{
		return _mem->_format.bpc;
	}

	inline size_t Image::bpp() const
	{
		return _mem->_format.bpp;
	}
}


#endif
