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
#include <cvt/math/Math.h>
#include <cvt/util/SIMD.h>
#include <cvt/util/Exception.h>
#include <cvt/util/ScopedBuffer.h>
#include <cvt/gfx/IMapScoped.h>

#include <iomanip>

namespace cvt {

	void Image::add( float alpha )
	{
		SIMD* simd = SIMD::instance();
		switch( _mem->_format.type ) {
			case IFORMAT_TYPE_FLOAT:
				{
					size_t stride;
					uint8_t* dst = map( &stride );
					uint8_t* dbase = dst;
					size_t h = _mem->_height;
					while( h-- ) {
						simd->AddValue1f( ( float* ) dst, ( float* ) dst, alpha, _mem->_width * _mem->_format.channels );
						dst += stride;
					}
					unmap( dbase );
				}
				break;
			default:
				throw CVTException("Unimplemented");

		}
	}


	void Image::add( const Color& c )
	{
		SIMD* simd = SIMD::instance();
		switch( _mem->_format.formatID ) {
			case IFORMAT_GRAY_FLOAT:
				{
					size_t stride;
					uint8_t* dst = map( & stride );
					uint8_t* dbase = dst;
					size_t h = _mem->_height;

					while( h-- ) {
						simd->AddValue1f( ( float* ) dst, ( float* ) dst, c.gray(), _mem->_width );
						dst += stride;
					}
					unmap( dbase );
				}
				break;
			case IFORMAT_RGBA_FLOAT:
				{
					float v[ 4 ] = { c.red(), c.green(), c.blue(), c.alpha() };
					size_t stride;
					uint8_t* dst = map( &stride );
					uint8_t* dbase = dst;
					size_t h = _mem->_height;

					while( h-- ) {
						simd->AddValue4f( ( float* ) dst, ( float* ) dst, v, _mem->_width * _mem->_format.channels );
						dst += stride;
					}
					unmap( dbase );
				}
				break;
			case IFORMAT_BGRA_FLOAT:
				{
					float v[ 4 ] = { c.blue(), c.green(), c.red(), c.alpha() };
					size_t stride;
					uint8_t* dst = map( &stride );
					uint8_t* dbase = dst;
					size_t h = _mem->_height;

					while( h-- ) {
						simd->AddValue4f( ( float* ) dst, ( float* ) dst, v, _mem->_width * _mem->_format.channels );
						dst += stride;
					}
					unmap( dbase );
				}
				break;
			default:
				throw CVTException("Unimplemented");

		}
	}

	void Image::sub( float alpha )
	{
		SIMD* simd = SIMD::instance();
		switch( _mem->_format.type ) {
			case IFORMAT_TYPE_FLOAT:
				{
					size_t stride;
					uint8_t* dst = map( &stride );
					uint8_t* dbase = dst;
					size_t h = _mem->_height;
					while( h-- ) {
						simd->SubValue1f( ( float* ) dst, ( float* ) dst, alpha, _mem->_width * _mem->_format.channels );
						dst += stride;
					}
					unmap( dbase );
				}
				break;
			default:
				throw CVTException("Unimplemented");

		}
	}

	void Image::sub( const Color& c )
	{
		SIMD* simd = SIMD::instance();
		switch( _mem->_format.formatID ) {
			case IFORMAT_GRAY_FLOAT:
				{
					size_t stride;
					uint8_t* dst = map( &stride );
					uint8_t* dbase = dst;
					size_t h = _mem->_height;

					while( h-- ) {
						simd->SubValue1f( ( float* ) dst, ( float* ) dst, c.gray(), _mem->_width );
						dst += stride;
					}
					unmap( dbase );
				}
				break;
			case IFORMAT_RGBA_FLOAT:
				{
					float v[ 4 ] = { c.red(), c.green(), c.blue(), c.alpha() };
					size_t stride;
					uint8_t* dst = map( &stride );
					uint8_t* dbase = dst;
					size_t h = _mem->_height;

					while( h-- ) {
						simd->SubValue4f( ( float* ) dst, ( float* ) dst, v, _mem->_width * _mem->_format.channels );
						dst += stride;
					}
					unmap( dbase );
				}
				break;
			case IFORMAT_BGRA_FLOAT:
				{
					float v[ 4 ] = { c.blue(), c.green(), c.red(), c.alpha() };
					size_t stride;
					uint8_t* dst = map( &stride );
					uint8_t* dbase = dst;
					size_t h = _mem->_height;

					while( h-- ) {
						simd->SubValue4f( ( float* ) dst, ( float* ) dst, v, _mem->_width * _mem->_format.channels );
						dst += stride;
					}
					unmap( dbase );
				}
				break;


			default:
				throw CVTException("Unimplemented");

		}
	}

	void Image::mul( float alpha )
	{
		SIMD* simd = SIMD::instance();

		switch( _mem->_format.type ) {
			case IFORMAT_TYPE_FLOAT:
				{
					size_t stride;
					uint8_t* dst = map( &stride );
					uint8_t* dbase = dst;
					size_t h = _mem->_height;

					while( h-- ) {
						simd->MulValue1f( ( float* ) dst, ( float* ) dst, alpha, _mem->_width * _mem->_format.channels );
						dst += stride;
					}
					unmap( dbase );
				}
				break;
			case IFORMAT_TYPE_UINT16:
				{
					size_t stride;
					uint16_t* dst = map<uint16_t>( &stride );
					uint16_t* dbase = dst;
					size_t h = _mem->_height;

					while( h-- ) {
						simd->MulValue1ui16( dst, dst, alpha, _mem->_width * _mem->_format.channels );
						dst += stride;
					}
					unmap( dbase );
				}
				break;
			default:
				throw CVTException("Unimplemented");

		}
	}

	void Image::mul( const Color& c )
	{
		SIMD* simd = SIMD::instance();
		switch( _mem->_format.formatID ) {
			case IFORMAT_GRAY_FLOAT:
				{
					size_t stride;
					uint8_t* dst = map( &stride );
					uint8_t* dbase = dst;
					size_t h = _mem->_height;

					while( h-- ) {
						simd->MulValue1f( ( float* ) dst, ( float* ) dst, c.gray(), _mem->_width );
						dst += stride;
					}
					unmap( dbase );
				}
				break;
			case IFORMAT_RGBA_FLOAT:
				{
					float v[ 4 ] = { c.red(), c.green(), c.blue(), c.alpha() };
					size_t stride;
					uint8_t* dst = map( &stride );
					uint8_t* dbase = dst;
					size_t h = _mem->_height;

					while( h-- ) {
						simd->MulValue4f( ( float* ) dst, ( float* ) dst, v, _mem->_width * _mem->_format.channels );
						dst += stride;
					}
					unmap( dbase );
				}
				break;
			case IFORMAT_BGRA_FLOAT:
				{
					float v[ 4 ] = { c.blue(), c.green(), c.red(), c.alpha() };
					size_t stride;
					uint8_t* dst = map( &stride );
					uint8_t* dbase = dst;
					size_t h = _mem->_height;

					while( h-- ) {
						simd->MulValue4f( ( float* ) dst, ( float* ) dst, v, _mem->_width * _mem->_format.channels );
						dst += stride;
					}
					unmap( dbase );
				}
				break;
			default:
				throw CVTException("Unimplemented");

		}
	}

	void Image::add( const Image& i )
	{
		if( _mem->_width != i._mem->_width ||
			_mem->_height != i._mem->_height ||
			_mem->_format != i._mem->_format )
			throw CVTException("Image mismatch");

		SIMD* simd = SIMD::instance();
		switch( _mem->_format.type ) {
			case IFORMAT_TYPE_FLOAT:
				{
					size_t sstride, dstride;
					const uint8_t* src = i.map( &sstride );
					const uint8_t* sbase = src;
					uint8_t* dst = map( &dstride );
					uint8_t* dbase = dst;

					size_t h = _mem->_height;
					while( h-- ) {
						simd->Add( ( float* ) dst, ( float* ) dst, ( float* ) src, _mem->_width * _mem->_format.channels );
						src += sstride;
						dst += dstride;
					}
					unmap( dbase );
					i.unmap( sbase );
				}
				break;
			default:
				throw CVTException("Unimplemented");

		}
	}

	void Image::sub( const Image& i )
	{
		if( _mem->_width != i._mem->_width ||
			_mem->_height != i._mem->_height ||
			_mem->_format != i._mem->_format )
			throw CVTException("Image mismatch");

		SIMD* simd = SIMD::instance();
		switch( _mem->_format.type ) {
			case IFORMAT_TYPE_FLOAT:
				{
					size_t sstride, dstride;
					const uint8_t* src = i.map( &sstride );
					const uint8_t* sbase = src;
					uint8_t* dst = map( &dstride );
					uint8_t* dbase = dst;

					size_t h = _mem->_height;
					while( h-- ) {
						simd->Sub( ( float* ) dst, ( float* ) dst, ( float* ) src, _mem->_width * _mem->_format.channels );
						src += sstride;
						dst += dstride;
					}
					unmap( dbase );
					i.unmap( sbase );
				}
				break;
			default:
				throw CVTException("Unimplemented");

		}
	}

	void Image::mul( const Image& i )
	{
		if( _mem->_width != i._mem->_width ||
			_mem->_height != i._mem->_height ||
			_mem->_format != i._mem->_format )
			throw CVTException("Image mismatch");

		SIMD* simd = SIMD::instance();
		switch( _mem->_format.type ) {
			case IFORMAT_TYPE_FLOAT:
				{
					size_t sstride, dstride;
					const uint8_t* src = i.map( &sstride );
					const uint8_t* sbase = src;
					uint8_t* dst = map( &dstride );
					uint8_t* dbase = dst;

					size_t h = _mem->_height;
					while( h-- ) {
						simd->Mul(( float* ) dst, ( float* ) dst, ( float* ) src, _mem->_width * _mem->_format.channels );
						src += sstride;
						dst += dstride;
					}
					unmap( dbase);
					i.unmap( sbase );
				}
				break;
			default:
				throw CVTException("Unimplemented");

		}
	}

	void Image::mad( const Image& i, float alpha )
	{
		if( _mem->_width != i._mem->_width ||
			_mem->_height != i._mem->_height ||
			_mem->_format != i._mem->_format )
			throw CVTException("Image mismatch");

		SIMD* simd = SIMD::instance();
		switch( _mem->_format.type ) {
			case IFORMAT_TYPE_FLOAT:
				{
					IMapScoped<const float> srcmap( i );
					IMapScoped<float> dstmap( *this );

					size_t h = height();
					while( h-- ) {
						simd->MulAddValue1f( dstmap.ptr(), srcmap.ptr(), alpha, width() * _mem->_format.channels );
						srcmap++;
						dstmap++;
					}
				}
				break;
			default:
				throw CVTException("Unimplemented");

		}
	}

	float Image::ssd( const Image& i ) const
	{
		if( _mem->_width != i._mem->_width ||
			_mem->_height != i._mem->_height ||
			_mem->_format != i._mem->_format )
			throw CVTException("Image mismatch");

		SIMD* simd = SIMD::instance();

		float ssd = 0.0f;

		switch( _mem->_format.type ) {
			case IFORMAT_TYPE_FLOAT:
			{
				size_t astride, bstride;
				const uint8_t* srcA = i.map( &astride );
				const uint8_t* baseA = srcA;
				const uint8_t* srcB = map( &bstride );
				const uint8_t* baseB = srcB;

				size_t h = _mem->_height;
				while( h-- ) {
					ssd += simd->SSD( ( float* ) srcA, ( float* )srcB, _mem->_width * _mem->_format.channels );
					srcA += astride;
					srcB += bstride;
				}
				unmap( baseB );
				i.unmap( baseA );
			}
				break;
			case IFORMAT_TYPE_UINT8:
			{
				size_t astride, bstride;
				const uint8_t* srcA = i.map( &astride );
				const uint8_t* baseA = srcA;
				const uint8_t* srcB = map( &bstride );
				const uint8_t* baseB = srcB;

				size_t h = _mem->_height;
				while( h-- ) {
					ssd += simd->SSD( srcA, srcB, _mem->_width * _mem->_format.channels );
					srcA += astride;
					srcB += bstride;
				}
				unmap( baseB );
				i.unmap( baseA );
			}
				break;
			default:
				throw CVTException("Unimplemented");

		}
		return ssd;
	}

	float Image::sad( const Image& i ) const
	{
		if( _mem->_width != i._mem->_width ||
			_mem->_height != i._mem->_height ||
			_mem->_format != i._mem->_format )
			throw CVTException("Image mismatch");

		SIMD* simd = SIMD::instance();

		float sad = 0.0f;

		switch( _mem->_format.type ) {
			case IFORMAT_TYPE_FLOAT:
				{
					size_t astride, bstride;
					const uint8_t* srcA = i.map( &astride );
					const uint8_t* baseA = srcA;
					const uint8_t* srcB = map( &bstride );
					const uint8_t* baseB = srcB;

					size_t h = _mem->_height;
					while( h-- ) {
						sad += simd->SAD( ( float* ) srcA, ( float* )srcB, _mem->_width * _mem->_format.channels );
						srcA += astride;
						srcB += bstride;
					}
					unmap( baseB );
					i.unmap( baseA );
				}
				break;
			case IFORMAT_TYPE_UINT8:
			{
				size_t astride, bstride;
				const uint8_t* srcA = i.map( &astride );
				const uint8_t* baseA = srcA;
				const uint8_t* srcB = map( &bstride );
				const uint8_t* baseB = srcB;

				size_t h = _mem->_height;
				while( h-- ) {
					sad += simd->SAD( srcA, srcB, _mem->_width * _mem->_format.channels );
					srcA += astride;
					srcB += bstride;
				}
				unmap( baseB );
				i.unmap( baseA );
			}
				break;
			default:
				throw CVTException("Unimplemented");

		}

		return sad ;
	}

	void Image::scale( Image& idst, size_t width, size_t height, const IScaleFilter& filter ) const
	{
		switch ( _mem->_format.type ) {
			case IFORMAT_TYPE_FLOAT:
				scaleFloat( idst, width, height, filter );
				break;
			case IFORMAT_TYPE_UINT8:
				scaleU8( idst, width, height, filter );
				break;
			default:
				throw CVTException("Unimplemented");
				break;
		}
	}

	void Image::scaleFloat( Image& idst, size_t width, size_t height, const IScaleFilter& filter ) const
	{
		IConvolveAdaptivef scalerx;
		IConvolveAdaptivef scalery;
		IConvolveAdaptiveSize* pysw;
		float* pyw;
		const uint8_t* src;
		const uint8_t* osrc;
		const uint8_t* send;
		uint8_t* dst;
		uint8_t* odst;
		size_t sstride, dstride;
		size_t i, l;
		int32_t k;
		float** buf;
		size_t bufsize;
		size_t curbuf;
		void (SIMD::*scalex_func)( float* _dst, float const* _src, const size_t width, IConvolveAdaptivef* conva ) const;
		SIMD* simd = SIMD::instance();


		if( _mem->_format.channels == 1 ) {
			scalex_func = &SIMD::ConvolveAdaptiveClamp1f;
		} else if( _mem->_format.channels == 2 ) {
			scalex_func = &SIMD::ConvolveAdaptiveClamp2f;
		} else {
			scalex_func = &SIMD::ConvolveAdaptiveClamp4f;
		}

		//checkFormat( idst, __PRETTY_FUNCTION__, __LINE__, _mem->_format );
		//checkSize( idst, __PRETTY_FUNCTION__, __LINE__, width, height );
		idst.reallocate( width, height, this->format() );

		osrc = src = map( &sstride );
		odst = dst = idst.map( &dstride );
		send = src + sstride * _mem->_height;

		bufsize = filter.getAdaptiveConvolutionWeights( height, _mem->_height, scalery, true );
		filter.getAdaptiveConvolutionWeights( width, _mem->_width, scalerx, false );

		buf = new float*[ bufsize ];
		/* allocate and fill buffer */
		for( i = 0; i < bufsize; i++ ) {
			if( posix_memalign( ( void** ) &buf[ i ], 16, sizeof( float ) * width * _mem->_format.channels) )
				throw CVTException("Out of memory");
			( simd->*scalex_func )( ( float* ) buf[ i ], ( float* ) src, width, &scalerx );
			src += sstride;
		}
		curbuf = 0;

		pysw = scalery.size;
		pyw = scalery.weights;

		while( height-- ) {
			if( pysw->incr ) {
				for( k = 0; k < pysw->incr && src < send ; k++ ) {
					( simd->*scalex_func )( ( float* ) buf[ ( curbuf + k ) % bufsize ], ( float* ) src, width, &scalerx );
					src += sstride;
				}
				curbuf = ( curbuf + pysw->incr ) % bufsize;
			}

			l = 0;
			while( Math::abs( *pyw ) < Math::EPSILONF ) {
				l++;
				pyw++;
			}
			simd->MulValue1f( ( float* ) dst, buf[ ( curbuf + l ) % bufsize ], *pyw++, width * _mem->_format.channels );
			l++;
			for( ; l < pysw->numw; l++ ) {
				if( Math::abs( *pyw ) > Math::EPSILONF )
					simd->MulAddValue1f( ( float* ) dst, buf[ ( curbuf + l ) % bufsize ], *pyw, width * _mem->_format.channels );
				pyw++;
			}
			pysw++;
			dst += dstride;
		}

		idst.unmap( odst );
		unmap( osrc );

		for( i = 0; i < bufsize; i++ )
			free( buf[ i ] );
		delete[] buf;
		delete[] scalerx.size;
		delete[] scalerx.weights;
		delete[] scalery.size;
		delete[] scalery.weights;
	}

	void Image::scaleU8( Image& idst, size_t width, size_t height, const IScaleFilter& filter ) const
	{
		IConvolveAdaptiveFixed scalerx;
		IConvolveAdaptiveFixed scalery;

		IConvolveAdaptiveSize* pysw;
		Fixed* pyw;

		const uint8_t* src;
		const uint8_t* osrc;
		const uint8_t* send;
		uint8_t* dst;
		uint8_t* odst;
		size_t sstride, dstride;
		size_t i, l;
		int32_t k;
		Fixed** buf;
		Fixed* accumBuf;
		size_t bufsize;
		size_t curbuf;
		void (SIMD::*scalex_func)( Fixed* _dst, uint8_t const* _src, const size_t width, IConvolveAdaptiveFixed* conva ) const;
		SIMD* simd = SIMD::instance();

		if( _mem->_format.channels == 1 ) {
			scalex_func = &SIMD::ConvolveAdaptive1Fixed;
		} else if( _mem->_format.channels == 2 ) {
			scalex_func = &SIMD::ConvolveAdaptive2Fixed;
		} else {
			scalex_func = &SIMD::ConvolveAdaptive4Fixed;
		}

		idst.reallocate( width, height, this->format() );

		osrc = src = map( &sstride );
		odst = dst = idst.map( &dstride );
		send = src + sstride * _mem->_height;

		bufsize = filter.getAdaptiveConvolutionWeights( height, _mem->_height, scalery, true );
		filter.getAdaptiveConvolutionWeights( width, _mem->_width, scalerx, false );

		buf = new Fixed*[ bufsize ];
		// allocate and fill buffer
		for( i = 0; i < bufsize; i++ ) {
			if( posix_memalign( ( void** ) &buf[ i ], 16, sizeof( Fixed ) * width * _mem->_format.channels) )
				throw CVTException("Out of memory");
			( simd->*scalex_func )( buf[ i ], src, width, &scalerx );
			src += sstride;
		}
		curbuf = 0;

		pysw = scalery.size;
		pyw = scalery.weights;

		if( posix_memalign( ( void** ) &accumBuf, 16, sizeof( Fixed ) * width * _mem->_format.channels ) )
			throw CVTException("Out of memory");

		while( height-- ) {
			if( pysw->incr ) {
				for( k = 0; k < pysw->incr && src < send ; k++ ) {
					( simd->*scalex_func )( buf[ ( curbuf + k ) % bufsize ], src, width, &scalerx );
					src += sstride;
				}
				curbuf = ( curbuf + pysw->incr ) % bufsize;
			}

			l = 0;
			while( *pyw == ( Fixed )0.0f ) {
				l++;
				pyw++;
			}
			simd->MulValue1fx( accumBuf, buf[ ( curbuf + l ) % bufsize ], *pyw++, width * _mem->_format.channels );
			l++;
			for( ; l < pysw->numw; l++ ) {
				if( *pyw != ( Fixed )0.0f )
					simd->MulAddValue1fx( accumBuf, buf[ ( curbuf + l ) % bufsize ], *pyw, width * _mem->_format.channels );
				pyw++;
			}

			for( size_t w = 0;  w < width * _mem->_format.channels; w++ ){
				dst[ w ] = Math::clamp( accumBuf[ w ].round(), 0, 255 );
			}

			pysw++;
			dst += dstride;
		}

		idst.unmap( odst );
		unmap( osrc );

		free( accumBuf );
		for( i = 0; i < bufsize; i++ )
			free( buf[ i ] );
		delete[] buf;
		delete[] scalerx.size;
		delete[] scalerx.weights;
		delete[] scalery.size;
		delete[] scalery.weights;
	}

	void Image::warpBilinear( Image& idst, const Image& warp ) const
	{
		size_t m, n, k, K;
		size_t sstride, dstride, wstride;

		if( _mem->_format.type == IFORMAT_TYPE_FLOAT &&
			warp._mem->_format == IFormat::GRAYALPHA_FLOAT ) {
			const uint8_t* src;
			const uint8_t* osrc;
			uint8_t* dst;
			uint8_t* odst;
			const uint8_t* wrp;
			const uint8_t* owrp;
			float* pdst;
			const float* pwrp;
			float data[ 4 ];

			checkFormatAndSize( idst, __PRETTY_FUNCTION__, __LINE__ );

			osrc = src = map( &sstride );
			odst = dst = idst.map( &dstride );
			owrp = wrp = warp.map( &wstride );
			K = channels();

			for( n = 0; n < warp._mem->_height; n++ ) {
				pdst = ( float* ) dst;
				pwrp = ( float* ) wrp;
				for( m = 0; m < warp._mem->_width; m++ ) {
					float x, y, alpha, beta;
					size_t ix[ 2 ], iy[ 2 ];
					x = *pwrp++;
					y = *pwrp++;
					alpha = x - Math::floor( x );
					beta  = y - Math::floor( y );
					ix[ 0 ] = ( size_t ) Math::clamp( ( float ) m + x, 0.0f, ( float ) ( _mem->_width - 1 ) );
					iy[ 0 ] = ( size_t ) Math::clamp( ( float ) n + y, 0.0f, ( float ) ( _mem->_height - 1 ) );
					ix[ 1 ] = Math::min( ix[ 0 ] + 1, _mem->_width - 1 );
					iy[ 1 ] = Math::min( iy[ 0 ] + 1, _mem->_height - 1 );
					for( k = 0; k < K; k++ ) {
						data[ 0 ] = *( ( float* ) ( src + sstride * iy[ 0 ] + ( ix[ 0 ] * K + k ) * sizeof( float ) ) );
						data[ 1 ] = *( ( float* ) ( src + sstride * iy[ 0 ] + ( ix[ 1 ] * K + k ) * sizeof( float ) ) );
						data[ 2 ] = *( ( float* ) ( src + sstride * iy[ 1 ] + ( ix[ 0 ] * K + k ) * sizeof( float ) ) );
						data[ 3 ] = *( ( float* ) ( src + sstride * iy[ 1 ] + ( ix[ 1 ] * K + k ) * sizeof( float ) ) );
						data[ 0 ] = Math::mix( data[ 0 ], data[ 1 ], alpha );
						data[ 2 ] = Math::mix( data[ 2 ], data[ 3 ], alpha );
						*pdst++ = Math::mix( data[ 0 ], data[ 2 ], beta );
					}
				}
				dst += dstride;
				wrp += wstride;
			}
			warp.unmap( owrp );
			idst.unmap( odst );
			unmap( osrc );
		} else
			throw CVTException("Unimplemented");
	}

    void Image::integralImage( Image & dst ) const
    {
        dst.reallocate( this->width(), this->height(), IFormat::floatEquivalent( this->format() ), _mem->type() );

        size_t inStride;
        size_t dstStride;

        float* out = dst.map<float>( &dstStride );
        SIMD* simd = SIMD::instance();

        IFormatID fId = this->format().formatID;

        switch ( fId ) {
            case IFORMAT_GRAY_UINT8:
            {
                const uint8_t* in = this->map<uint8_t>( &inStride );
                simd->prefixSum1_u8_to_f( out, dstStride, in, inStride, width(), height() );
                this->unmap( in );
            }
            break;
            case IFORMAT_GRAY_FLOAT:
            {
                const float* in = this->map<float>( &inStride );
                simd->prefixSum1_f_to_f( out, dstStride, in, inStride, width(), height() );
                this->unmap( in );
            }
            break;
            case IFORMAT_BGRA_UINT8:
            case IFORMAT_RGBA_UINT8:
            {
                const uint8_t* in = this->map<uint8_t > ( &inStride );
                simd->prefixSum1_xxxxu8_to_f( out, dstStride, in, inStride, width( ), height( ) );
                this->unmap( in );
            }
                break;
            default:
                this->unmap( out );
                String msg;
                msg.sprintf( "IntegralImage not implemented for type: %s", fId );
                throw CVTException( msg.c_str() );
        }
        dst.unmap( out );
    }

    void Image::squaredIntegralImage( Image & dst ) const
    {
        dst.reallocate( this->width(), this->height(), IFormat::floatEquivalent( this->format() ), _mem->type() );

        size_t inStride;
        size_t dstStride;

        float * out = dst.map<float>( &dstStride );
        SIMD * simd = SIMD::instance();

        IFormatID fId = this->format().formatID;

        switch ( fId ) {
            case IFORMAT_GRAY_UINT8:
            {
                const uint8_t * in = this->map<uint8_t>( &inStride );
                simd->prefixSumSqr1_u8_to_f( out, dstStride, in, inStride, width(), height() );
                this->unmap( in );
            }
            break;
            case IFORMAT_GRAY_FLOAT:
            {
                const float * in = this->map<float>( &inStride );
                simd->prefixSumSqr1_f_to_f( out, dstStride, in, inStride, width(), height() );
                this->unmap( in );
            }
            break;

            default:
                this->unmap( out );
                String msg;
                msg.sprintf( "Squared integralImage not implemented for type: %s", fId );
                throw CVTException( msg.c_str() );
        }
        dst.unmap( out );
    }


	void Image::pyrdown( Image& dst ) const
	{
		dst.reallocate( width() / 2, height() / 2, format(), _mem->type() );

		IFormatID fId = this->format().formatID;
		switch( fId ) {
			case IFORMAT_GRAY_UINT8: return pyrdown1U8( dst );
			default:
				String msg;
				msg.sprintf( "Pyrdown not implemented for type: %s", fId );
				throw CVTException( msg.c_str() );
		}
	}

	void Image::pyrdown1U8( Image& out ) const
	{
		size_t bstride = Math::pad16( out.width() );
		uint16_t* buf;
		size_t sstride, dstride;
		const uint8_t* src = map( &sstride );
		uint8_t* dst = out.map( &dstride );
		const uint8_t* psrc = src;
		uint8_t* pdst = dst;
		uint16_t* rows[ 5 ];

		SIMD* simd = SIMD::instance();

		ScopedBuffer<uint16_t, true> scopebuf( bstride * 5 );
		buf = scopebuf.ptr();
//		if( posix_memalign( ( void** ) &buf, 16, sizeof( uint16_t ) * bstride * 5 ) )
//			throw CVTException("Out of memory");

		simd->pyrdownHalfHorizontal_1u8_to_1u16( buf, psrc, width() );
		psrc += sstride;
		simd->pyrdownHalfHorizontal_1u8_to_1u16( buf + bstride, psrc, width() );
		psrc += sstride;
		simd->pyrdownHalfHorizontal_1u8_to_1u16( buf + 2 * bstride, psrc, width() );
		psrc += sstride;
		simd->pyrdownHalfHorizontal_1u8_to_1u16( buf + 3 * bstride, psrc, width() );
		psrc += sstride;

		rows[ 0 ] = buf;
		rows[ 1 ] = buf;
		rows[ 2 ] = buf + bstride;
		rows[ 3 ] = buf + 2 * bstride;
		rows[ 4 ] = buf + 3 * bstride;

		simd->pyrdownHalfVertical_1u16_to_1u8( pdst, rows, out.width() );
		pdst += dstride;

		rows[ 0 ] = buf + 4 * bstride;
		size_t h = out.height() - 2;
		while( h-- ) {
			uint16_t* tmp1 = rows[ 0 ];
			simd->pyrdownHalfHorizontal_1u8_to_1u16( tmp1, psrc, width() );
			psrc += sstride;
			uint16_t* tmp2 = rows[ 1 ];
			simd->pyrdownHalfHorizontal_1u8_to_1u16( tmp2, psrc, width() );
			psrc += sstride;

			rows[ 0 ] = rows[ 2 ];
			rows[ 1 ] = rows[ 3 ];
			rows[ 2 ] = rows[ 4 ];
			rows[ 3 ] = tmp1;
			rows[ 4 ] = tmp2;

			simd->pyrdownHalfVertical_1u16_to_1u8( pdst, rows, out.width() );
			pdst += dstride;
		}

		uint16_t* tmp1 = rows[ 0 ];
		if( height() & 1 )
			simd->pyrdownHalfHorizontal_1u8_to_1u16( tmp1, psrc, width() );
		else
			tmp1 = rows[ 4 ];

		rows[ 0 ] = rows[ 2 ];
		rows[ 1 ] = rows[ 3 ];
		rows[ 2 ] = rows[ 4 ];
		rows[ 3 ] = tmp1;
		rows[ 4 ] = tmp1;
		simd->pyrdownHalfVertical_1u16_to_1u8( pdst, rows, out.width() );

		unmap( src );
		out.unmap( dst );
	}


	void Image::printValues( std::ostream& o, const Recti& _rect ) const
	{
		size_t w, h, stride;

		Recti rect( _rect );
		rect.intersect( this->rect() );


		if( rect.isEmpty() )
			return;

		switch ( _mem->_format.formatID ) {
			case IFORMAT_GRAY_UINT8:
			case IFORMAT_BAYER_RGGB_UINT8:
				{
					const uint8_t* ptr, *base;
					base = ptr = map( &stride );
					ptr += rect.y * stride + rect.x;
					h = rect.height;
					while( h-- ) {
						w = rect.width;
						for( size_t x = 0; x < w; x++ )
							o << "0x" << std::setw( 2 ) << std::hex << *( ptr + x ) << " ";
						o << "\n";
						ptr += stride;
					}
					unmap( base );
				}
				break;
			case IFORMAT_GRAY_FLOAT:
				{
					const uint8_t* ptr, *base;
					base = ptr = map( &stride );
					ptr += rect.y * stride + rect.x * sizeof( float );
					h = rect.height;
					while( h-- ) {
						w = rect.width;
						for( size_t x = 0; x < w; x++ )
							o << std::setiosflags( std::ios::fixed ) << std::setprecision( 2 ) << *( ( ( float* ) ptr ) + x ) << " ";
						o << "\n";
						ptr += stride;
					}
					unmap( base );
				}
				break;
			case IFORMAT_RGBA_UINT8:
			case IFORMAT_BGRA_UINT8:
				{
					const uint8_t* ptr, *base;
					base = ptr = map( &stride );
					ptr += rect.y * stride + rect.x * 4;
					h = rect.height;
					while( h-- ) {
						w = rect.width * 4;
						for( size_t x = 0; x < w; x++ )
							o << "0x" << std::setw( 2 ) << std::hex << *( ptr + x ) << " ";
						o << "\n";
						ptr += stride;
					}
					unmap( base );
				}
				break;
			case IFORMAT_RGBA_FLOAT:
			case IFORMAT_BGRA_FLOAT:
				{
					const uint8_t* ptr, *base;
					base = ptr = map( &stride );
					ptr += rect.y * stride + rect.x * sizeof( float ) * 4;
					h = rect.height;
					while( h-- ) {
						w = rect.width * 4;
						for( size_t x = 0; x < w; x++ )
							o << std::setiosflags( std::ios::fixed ) << std::setprecision( 2 ) << *( ( ( float* ) ptr ) + x ) << " ";
						o << "\n";
						ptr += stride;
					}
					unmap( base );
				}
				break;
			default:
				throw CVTException("Unimplemented");
				break;
		}
	}
}
