#include <cvt/util/PluginManager.h>
#include <cvt/gfx/Image.h>
#include <cvt/util/Exception.h>
#include <cvt/util/SIMD.h>
#include <cvt/util/ScopedBuffer.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

namespace cvt {

#define TGA_TYPE_NO_DATA 0
#define TGA_TYPE_CMAP    1
#define TGA_TYPE_TRUECOLOR 2
#define TGA_TYPE_MONO      3
#define TGA_TYPE_RLE_CMAP    9
#define TGA_TYPE_RLE_TRUECOLOR 10
#define TGA_TYPE_RLE_MONO      11

#define TGA_ORIGIN_MASK 0x30
#define TGA_ORIGIN_TOPLEFT 0x20
#define TGA_ORIGIN_TOPRIGHT 0x30
#define TGA_ORIGIN_BOTTOMLEFT 0x0
#define TGA_ORIGIN_BOTTOMRIGHT 0x10

#define TGA_ALPHABIT_MASK 0xf
#define TGA_RLE_PACKET 0x80

#define TGA_HEADER_SIZE 18

#define TGA_RLE_BUFFER_SIZE 4096

	static const char* _tga_signature = "TRUEVISION-XFILE.";

	struct TGAHeader {
		uint8_t idlength;
		uint8_t cmaptype;
		uint8_t imagetype;
		uint16_t cmapindex;
		uint16_t cmaplen;
		uint8_t cmapdepth;
		uint16_t xorigin;
		uint16_t yorigin;
		uint16_t width;
		uint16_t height;
		uint8_t depth;
		uint8_t desc;
	} __attribute__ ((__packed__));

	struct TGAExtension {
		uint16_t size;
		uint8_t authorname[41];
		uint8_t authorcomment[4][81];
		uint16_t datetime[6];
		uint8_t jobnameid[41];
		uint16_t jobtime[3];
		uint8_t softwareid[41];
		uint16_t softwareversion;
		uint8_t softwareletter;
		uint32_t keycolor;
		uint16_t ascpectratio[2];
		uint16_t gamma[2];
		uint32_t colcorroffset;
		uint32_t pstampoffset;
		uint32_t scanlineoffset;
		uint8_t attributes;
		/*
		   followed by:
		   -scanline table
		   -postage stamp image
		   -color correction table
		 */
	} __attribute__ ((__packed__));

	struct TGAFooter {
		uint32_t extoffset;
		uint32_t devoffset;
		char signature[18]; /*TRUEVISION-XFILE.\0*/
	} __attribute__ ((__packed__));


	static inline ssize_t tga_rle_fillbuffer( FILE* file, uint8_t* buffer, uint8_t* curptr, size_t keep )
	{
		uint8_t* dst;
		size_t n = keep;
		ssize_t ret;

		dst = buffer;

		while( n-- )
			*dst++ = *curptr++;

		if( ( ret = fread( dst, TGA_RLE_BUFFER_SIZE - keep, 1, file ) ) <= 0 )
			throw CVTException( "Corrupted TGA file!" );
		return ret;
	}

	static inline void* tga_origin( TGAHeader* header, void* dst, ssize_t* stride )
	{
		void* ret;
		switch( header->desc & TGA_ORIGIN_MASK ) {
			case TGA_ORIGIN_BOTTOMLEFT:
			case TGA_ORIGIN_BOTTOMRIGHT:
				ret = ( ( uint8_t* ) dst ) + ( header->height - 1 ) *  *stride ;
				*stride = -*stride;
				return ret;
			case TGA_ORIGIN_TOPLEFT:
			case TGA_ORIGIN_TOPRIGHT:
			default:
				return dst;
		}
	}

	static inline bool tga_apply_cmap8( void* dest, void* source, size_t pixels, uint32_t offset, uint8_t* cmap, uint32_t cmaplen )
	{
		uint8_t* dst = ( uint8_t* ) dest;
		uint8_t* src = ( uint8_t* ) source;
		uint32_t index;

		while( pixels-- ) {
			index = offset + *src++;
			if( index > cmaplen )
				return false;
			*dst++ = cmap[ index ];
		}
		return true;
	}

	static inline bool tga_apply_cmap32( void* dest, void* source, size_t pixels, uint32_t offset, uint32_t* cmap, uint32_t cmaplen )
	{
		uint32_t* dst = ( uint32_t* ) dest;
		uint8_t* src = ( uint8_t* ) source;
		uint32_t index;

		while( pixels-- ) {
			index = offset + *src++;
			if( index > cmaplen )
				return false;
			*dst++ = cmap[ index ];
		}
		return true;
	}

/*
   Supported combinations of depth and attribute-bits:

   Depth   Attribute-Bits
   ----------------------
   48	    0	TODO
   32	    8
   32	    0
   24	    0
   16	    1	TODO
   16	    0	TODO
   15	    0	TODO
 */
static void tga_decode_color( Image& img, FILE* file, TGAHeader* header, TGAExtension* ext )
{
	uint8_t* dst;
	uint8_t* pdst;
	size_t stride;
	size_t height;
	ssize_t sstride;
	size_t read;
	size_t dataoffset;
	IFormat format( IFormat::BGRA_UINT8 ); // Assume BGRA_UINT8
	uint8_t alphabits = header->desc & TGA_ALPHABIT_MASK;

	/*seek to data*/
	dataoffset = TGA_HEADER_SIZE + header->idlength;
	if( fseek( file, dataoffset, SEEK_SET ) < 0 )
		throw CVTException( "Corrupted TGA file! " );

	if( header->depth == 32 &&  ( alphabits == 8 || alphabits == 0 ) ) {
		/* Extension area available */
		if( ext ) {
			if( ext->attributes == 3 || ext->attributes == 4 /* FIXME: premultiplied*/ )
				format = IFormat::BGRA_UINT8;
			else
				throw CVTException( "Unsupported TGA file!" );
		}

		img.reallocate( header->width, header->height, format );
		dst = img.map( &stride );
		height = header->height;
		sstride = stride;

		pdst = ( uint8_t* ) tga_origin( header, dst, &sstride );

		while( height-- ) {
			if( ( read = fread( pdst, sizeof( uint32_t ), header->width, file ) ) != header->width )
				throw CVTException( "Corrupted TGA file!" );
			// FIXME: set arbitrary alpha to 1
/*			if( !alphabits )
				ziutil_xxxa_to_xxx1_ub( dst, dst, header->width );*/
			pdst += sstride;
		}
		img.unmap( dst );
	} else if( header->depth == 24 && alphabits == 0 ) {
		img.reallocate( header->width, header->height, IFormat::BGRA_UINT8 );
		dst = img.map( &stride );
		height = header->height;
		sstride = stride;
		pdst = ( uint8_t* ) tga_origin( header, dst, &sstride );
		//uint8_t* buffer = new uint8_t[ header->width * 3 ];
		ScopedBuffer<uint8_t,true> buffer( header->width * 3 );
		SIMD* simd = SIMD::instance();

		while( height-- ) {
			if( ( read = fread( buffer.ptr(), sizeof( uint8_t ), header->width * 3, file ) ) != ( size_t ) header->width * 3 )
				throw CVTException( "Corrupted TGA file!" );
			simd->Conv_XXXu8_to_XXXAu8( pdst, buffer.ptr(), header->width );
			pdst += sstride;
		}
		img.unmap( dst );
	} else {
		throw CVTException( "Unsupported TGA file!" );
	}
}


/*
   Supported combinations of depth and attribute-bits:

   Depth   Attribute-Bits
   ----------------------
   16	    0	TODO
   16	    8	TODO
   8	    0
 */
static void tga_decode_gray( Image& img, FILE* file, TGAHeader* header, TGAExtension* )
{
	uint8_t* dst;
	uint8_t* pdst;
	size_t stride;
	size_t height;
	ssize_t read, len, sstride;
	size_t dataoffset;
	uint8_t alphabits = header->desc & TGA_ALPHABIT_MASK;

	/*seek to data*/
	dataoffset = TGA_HEADER_SIZE + header->idlength;
	if( fseek( file, dataoffset, SEEK_SET ) < 0 )
		throw CVTException( "Corrupted TGA file! " );

	if( header->depth == 8 && alphabits == 0 ) {
		img.reallocate( header->width, header->height, IFormat::GRAY_UINT8 );
		dst = img.map( &stride );
		height = header->height;
		len = header->width * sizeof( uint8_t );
		sstride = stride;
		pdst = ( uint8_t* ) tga_origin( header, dst, &sstride );

		while( height-- ) {
			if( ( read = fread( pdst, sizeof( uint8_t ), len, file ) ) != len )
				throw CVTException( "Corrupted TGA file! " );
			pdst += sstride;
		}
		img.unmap( dst );
	} else if( header->depth == 16 && alphabits == 8 ) {
		img.reallocate( header->width, header->height, IFormat::GRAYALPHA_UINT8 );
		dst = img.map( &stride );
		height = header->height;
		len = header->width * 2 * sizeof( uint8_t );
		sstride = stride;
		pdst = ( uint8_t* ) tga_origin( header, dst, &sstride );

		while( height-- ) {
			if( ( read = fread( pdst, sizeof( uint8_t ), len, file ) ) != len )
				throw CVTException( "Corrupted TGA file! " );
			pdst += sstride;
		}
		img.unmap( dst );
	} else {
		throw CVTException( "Unsupported TGA file!" );
	}
}

#if 0
/*
   Supported combinations of cmap-depth, depth and attribute-bits:

   Depth   Cmap-Depth  Attribute-Bits
   ----------------------------------
   8	    32		8
   8	    32		0
   8	    24		0
   8	    8		0
 */
static ZImage* _ztga_decode_cmap( ZStream* strm, TGAHeader* header, TGAExtension* ext ZATTRIBUTE_UNUSED,
								 ZProgressDataErrorFunc func, void* data, ZError* error )
{
	ZImage* ret = NULL;
	uint8_t* dst;
	ssize_t stride;
	ssize_t height, len;
	ssize_t read;
	size_t dataoffset;
	ZError err;
	uint8_t* buffer;
	float progress = 0.0f;
	float lprogress = 0.0f;
	float iprogress;
	uint8_t alphabits = header->desc & TGA_ALPHABIT_MASK;

	iprogress = 1.0f / ( ( float ) header->height - 1.0f );

	/*seek to cmap*/
	dataoffset = TGA_HEADER_SIZE + header->idlength;
	if( zstream_seek( strm, dataoffset, Z_STREAMORIGIN_SET ) < 0 ) {
		err = zstream_get_status( strm );
		_ztga_error( err, func, data, error);
		return NULL;
	}

	if( header->depth == 8 &&
	   ( ( header->cmapdepth == 32 && ( alphabits == 8 || alphabits == 0 ) ) ||
		( header->cmapdepth == 24 && alphabits == 0 ) ) ) {
		uint32_t* cmap;

		if( header->cmapdepth == 32 ) {
			cmap = zmalloc( sizeof( uint32_t ) * header->cmaplen );
			len = sizeof( uint32_t ) * header->cmaplen;
		} else {
			cmap = zmalloc( sizeof( uint8_t ) * 3 * header->cmaplen );
			len = sizeof( uint8_t ) * 3 * header->cmaplen;
		}

		if( ( read = zstream_read( strm, cmap, len ) ) != len ) {
			if( read < 0 )
				err = zstream_get_status( strm );
			else
				err = Z_ERROR_CORRUPTED;
			_ztga_error( err, func, data, error );
			zfree( cmap );
			return NULL;
		}

		/* hrm - very often ... */
		/* if( header->cmapdepth == 32 && alphabits == 0 ) {
		   ziutil_xxxa_to_xxx1_ub( cmap, cmap, header->cmaplen );
		   } else */ if( header->cmapdepth == 24 ) {
			   /* convert RGB cmap to RGBA */
			   uint32_t* newcmap;
			   newcmap = zmalloc( sizeof( uint32_t ) * header->cmaplen );
			   ziutil_rgb_to_rgba_ub( newcmap, cmap, header->cmaplen );
			   zfree( cmap );
			   cmap = newcmap;
		   }

	/* seek to data */
	dataoffset = TGA_HEADER_SIZE + header->idlength + len;
	if( zstream_seek( strm, dataoffset, Z_STREAMORIGIN_SET ) < 0 ) {
		err = zstream_get_status( strm );
		_ztga_error( err, func, data, error);
		return NULL;
	}

	buffer = zmalloc( sizeof( uint8_t ) * header->width );
	ret = zimage_new( Z_IMAGEFORMAT_BGRA_UB, header->width, header->height );
	dst = zimage_get_data( ret );
	stride = zimage_get_stride( ret );
	height = header->height;
	len = header->width * sizeof( uint8_t );

	dst = _ztga_origin( header, dst, &stride );

	while( height-- ) {
		if( ( read = zstream_read( strm, buffer, len ) ) != len ) {
			if( read < 0 )
				err = zstream_get_status( strm );
			else
				err = Z_ERROR_CORRUPTED;
			_ztga_error( err, func, data, error );
			zfree( buffer );
			zfree( cmap );
			zimage_destroy( ret );
			return NULL;
		}

		if( !_ztga_apply_cmap32( dst, buffer, header->width, header->cmapindex, cmap, header->cmaplen ) ) {
			_ztga_error( Z_ERROR_RANGE, func, data, error );
			zfree( buffer );
			zfree( cmap );
			zimage_destroy( ret );
			return NULL;
		}

		/* progress */
		if( func ) {
			progress += iprogress;
			if( progress >= lprogress ) {
				if( !func( data, MIN( progress, 1.0f ), Z_ERROR_NONE ) ) {
					zfree( buffer );
					zfree( cmap );
					zimage_destroy( ret );
					return NULL;
				}
				lprogress = MIN( progress + 0.01f, 1.0f);
			}
		}
		dst += stride;
	}
	zfree( buffer );
	zfree( cmap );
	} else if( header->depth == 8 && header->cmapdepth == 8 && alphabits == 0 ) {
		uint8_t* cmap;

		cmap = zmalloc( sizeof( uint8_t ) * header->cmaplen );
		len = sizeof( uint8_t ) * header->cmaplen;

		if( ( read = zstream_read( strm, cmap, len ) ) != len ) {
			if( read < 0 )
				err = zstream_get_status( strm );
			else
				err = Z_ERROR_CORRUPTED;
			_ztga_error( err, func, data, error );
			zfree( cmap );
			return NULL;
		}

		/* seek to data */
		dataoffset = TGA_HEADER_SIZE + header->idlength + len;
		if( zstream_seek( strm, dataoffset, Z_STREAMORIGIN_SET ) < 0 ) {
			err = zstream_get_status( strm );
			_ztga_error( err, func, data, error);
			return NULL;
		}

		buffer = zmalloc( sizeof( uint8_t ) * header->width );
		ret = zimage_new( Z_IMAGEFORMAT_GRAY_UB, header->width, header->height );
		dst = zimage_get_data( ret );
		stride = zimage_get_stride( ret );
		height = header->height;
		len = header->width * sizeof( uint8_t );

		dst = _ztga_origin( header, dst, &stride );

		while( height-- ) {
			if( ( read = zstream_read( strm, buffer, len ) ) != len ) {
				if( read < 0 )
					err = zstream_get_status( strm );
				else
					err = Z_ERROR_CORRUPTED;
				_ztga_error( err, func, data, error );
				zfree( buffer );
				zfree( cmap );
				zimage_destroy( ret );
				return NULL;
			}

			if( !_ztga_apply_cmap8( dst, buffer, header->width, header->cmapindex, cmap, header->cmaplen ) ) {
				_ztga_error( Z_ERROR_RANGE, func, data, error );
				zfree( buffer );
				zfree( cmap );
				zimage_destroy( ret );
				return NULL;
			}

			/* progress */
			if( func ) {
				progress += iprogress;
				if( progress >= lprogress ) {
					if( !func( data, MIN( progress, 1.0f ), Z_ERROR_NONE ) ) {
						zfree( buffer );
						zfree( cmap );
						zimage_destroy( ret );
						return NULL;
					}
					lprogress = MIN( progress + 0.01f, 1.0f);
				}
			}
			dst += stride;
		}
		zfree( buffer );
		zfree( cmap );
	} else {
		_ztga_error( Z_ERROR_UNSUPPORTED, func, data, error );
	}

	return ret;
}

/*
   Supported combinations of depth and attribute-bits:

   Depth   Attribute-Bits
   ----------------------
   48	    0	TODO
   32	    8
   32	    0
   24	    0
   16	    1	TODO
   16	    0	TODO
   15	    0	TODO
 */
static ZImage* _ztga_decode_color_rle( ZStream* strm, TGAHeader* header, TGAExtension* ext ZATTRIBUTE_UNUSED,
									  ZProgressDataErrorFunc func, void* data, ZError* error )
{
	ZImage* ret = NULL;
	uint8_t* dst;
	uint8_t* buffer;
	uint8_t* bufptr;
	uint8_t rleval, rlelen;
	ssize_t stride;
	size_t width;
	size_t height;
	ssize_t read;
	ssize_t keep;
	size_t dataoffset;
	ZError err;
	float progress = 0.0f;
	float lprogress = 0.0f;
	float iprogress;
	uint8_t alphabits = header->desc & TGA_ALPHABIT_MASK;

	iprogress = 1.0f / ( ( float ) header->height );

	/*seek to data*/
	dataoffset = TGA_HEADER_SIZE + header->idlength;
	if( zstream_seek( strm, dataoffset, Z_STREAMORIGIN_SET ) < 0 ) {
		err = zstream_get_status( strm );
		_ztga_error( err, func, data, error);
		return NULL;
	}

	if( header->depth == 32 && ( alphabits == 0 || alphabits == 8 ) ) {
		uint32_t* dptr;

		ret = zimage_new( Z_IMAGEFORMAT_BGRA_UB, header->width, header->height );
		dst = zimage_get_data( ret );
		stride = zimage_get_stride( ret );
		height = header->height;
		read = 0;

		dst = _ztga_origin( header, dst, &stride );

		buffer = zmalloc( TGA_RLE_BUFFER_SIZE * sizeof( uint8_t ) );
		bufptr = buffer;

		while( height-- ) {
			dptr = ( uint32_t* ) dst;
			width = header->width;
			/* buffer filled and still pixels to do */
			while( width ) {
				/* fill buffer */
				if( !read ) {
					if( ( read = _ztga_rle_fillbuffer( strm, buffer, NULL, 0, &err ) ) <= 0 ) {
						_ztga_error( err, func, data, error );
						zfree( buffer );
						zimage_destroy( ret );
						return NULL;
					}
					bufptr = buffer;
				}
				rleval = *bufptr++;
				rlelen = ( rleval & ( ~TGA_RLE_PACKET ) ) + 1U;
				read--;

				/* more data than we need */
				if( rlelen > width ) {
					_ztga_error( Z_ERROR_CORRUPTED, func, data, error );
					zfree( buffer );
					zimage_destroy( ret );
					return NULL;
				}

				width -= rlelen;

				/* not enoguh data left in buffer */
				if( read < 4 ) {
					keep = read;
					if( ( read = _ztga_rle_fillbuffer( strm, buffer, bufptr, keep, &err ) ) <= 0 ) {
						_ztga_error( err, func, data, error );
						zfree( buffer );
						zimage_destroy( ret );
						return NULL;
					}
					read += keep;
					bufptr = buffer;
				}

				/* rle packet */
				if( rleval & TGA_RLE_PACKET ) {
					while( rlelen-- ) {
						*dptr++ = *( ( uint32_t* ) bufptr );
					}
					bufptr += 4;
					read -= 4;
				} else { /* raw packet */
					/* copy rleval pixels to dptr */
					while( rlelen-- ) {
						*dptr++ = *( ( uint32_t* )bufptr );
						bufptr += 4;
						read -= 4;
						/* again no data anymore */
						if( read < 4 ) {
							keep = read;
							if( ( read = _ztga_rle_fillbuffer( strm, buffer, bufptr, keep, &err ) ) <= 0 ) {
								_ztga_error( err, func, data, error );
								zfree( buffer );
								zimage_destroy( ret );
								return NULL;
							}
							read += keep;
							bufptr = buffer;
						}
					}
				}
			}
			if( !alphabits )
				ziutil_xxxa_to_xxx1_ub( dst, dst, header->width );
			/* progress */
			if( func ) {
				progress += iprogress;
				if( progress >= lprogress ) {
					if( !func( data, MIN( progress, 1.0f ), Z_ERROR_NONE ) ) {
						zfree( buffer );
						zimage_destroy( ret );
						return NULL;
					}
					lprogress = MIN( progress + 0.01f, 1.0f);
				}
			}
			dst += stride;
		}
		zfree( buffer );
	} else if( header->depth == 24 && alphabits == 0 ) {
		uint8_t* dptr;

		ret = zimage_new( Z_IMAGEFORMAT_BGRA_UB, header->width, header->height );
		dst = zimage_get_data( ret );
		stride = zimage_get_stride( ret );
		height = header->height;
		read = 0;

		dst = _ztga_origin( header, dst, &stride );

		buffer = zmalloc( TGA_RLE_BUFFER_SIZE * sizeof( uint8_t ) );
		bufptr = buffer;

		while( height-- ) {
			dptr = dst;
			width = header->width;
			/* still pixels to do */
			while( width ) {
				/* fill buffer */
				if( !read ) {
					if( ( read = _ztga_rle_fillbuffer( strm, buffer, NULL, 0, &err ) ) <= 0 ) {
						_ztga_error( err, func, data, error );
						zfree( buffer );
						zimage_destroy( ret );
						return NULL;
					}
					bufptr = buffer;
				}
				rleval = *bufptr++;
				rlelen = ( rleval & ( ~TGA_RLE_PACKET ) ) + 1U;
				read--;

				/* more data than we need */
				if( rlelen > width ) {
					_ztga_error( Z_ERROR_CORRUPTED, func, data, error );
					zfree( buffer );
					zimage_destroy( ret );
					return NULL;
				}

				width -= rlelen;

				/* not enoguh data left in buffer */
				if( read < 3 ) {
					keep = read;
					if( ( read = _ztga_rle_fillbuffer( strm, buffer, bufptr, keep, &err ) ) <= 0 ) {
						_ztga_error( err, func, data, error );
						zfree( buffer );
						zimage_destroy( ret );
						return NULL;
					}
					read += keep;
					bufptr = buffer;
				}

				/* rle packet */
				if( rleval & TGA_RLE_PACKET ) {
					while( rlelen-- ) {
						*dptr++ = *( bufptr );
						*dptr++ = *( bufptr + 1 );
						*dptr++ = *( bufptr + 2 );
						*dptr++ = 0xff;
					}
					bufptr += 3;
					read -= 3;
					/* */
				} else { /* raw packet */
					/* copy rleval pixels to dptr */
					while( rlelen-- ) {
						*dptr++ = *bufptr++;
						*dptr++ = *bufptr++;
						*dptr++ = *bufptr++;
						*dptr++ = 0xff;
						read -= 3;
						/* again no data anymore */
						if( read < 3 ) {
							keep = read;
							if( ( read = _ztga_rle_fillbuffer( strm, buffer, bufptr, keep, &err ) ) <= 0 ) {
								_ztga_error( err, func, data, error );
								zfree( buffer );
								zimage_destroy( ret );
								return NULL;
							}
							read += keep;
							bufptr = buffer;
						}
					}
				}
			}
			/* progress */
			if( func ) {
				progress += iprogress;
				if( progress >= lprogress ) {
					if( !func( data, MIN( progress, 1.0f ), Z_ERROR_NONE ) ) {
						zfree( buffer );
						zimage_destroy( ret );
						return NULL;
					}
					lprogress = MIN( progress + 0.01f, 1.0f);
				}
			}
			dst += stride;
		}
		zfree( buffer );
	} else {
		err = Z_ERROR_UNSUPPORTED;
		_ztga_error( err, func, data, error );
	}

	return ret;
}

/*
   Supported combinations of depth and attribute-bits:

   Depth   Attribute-Bits
   ----------------------
   16	    0	TODO
   16	    8	TODO
   8	    0
 */
static ZImage* _ztga_decode_gray_rle( ZStream* strm, TGAHeader* header, TGAExtension* ext ZATTRIBUTE_UNUSED,
									 ZProgressDataErrorFunc func, void* data, ZError* error )
{
	ZImage* ret = NULL;
	uint8_t* dst;
	uint8_t* dptr;
	uint8_t* buffer;
	uint8_t* bufptr;
	uint8_t rleval, rlelen;
	ssize_t stride;
	size_t width;
	size_t height;
	ssize_t read;
	size_t dataoffset;
	ZError err;
	float progress = 0.0f;
	float lprogress = 0.0f;
	float iprogress;
	uint8_t alphabits = header->desc & TGA_ALPHABIT_MASK;

	iprogress = 1.0f / ( ( float ) header->height );

	/*seek to data*/
	dataoffset = TGA_HEADER_SIZE + header->idlength;
	if( zstream_seek( strm, dataoffset, Z_STREAMORIGIN_SET ) < 0 ) {
		err = zstream_get_status( strm );
		_ztga_error( err, func, data, error);
		return NULL;
	}

	if( header->depth == 8 && alphabits == 0 ) {
		ret = zimage_new( Z_IMAGEFORMAT_GRAY_UB, header->width, header->height );
		dst = zimage_get_data( ret );
		stride = zimage_get_stride( ret );
		height = header->height;
		read = 0;

		dst = _ztga_origin( header, dst, &stride );

		buffer = zmalloc( TGA_RLE_BUFFER_SIZE * sizeof( uint8_t ) );
		bufptr = buffer;

		while( height-- ) {
			dptr = dst;
			width = header->width;
			/* buffer filled and still pixels to do */
			while( width ) {
				/* fill buffer */
				if( !read ) {
					if( ( read = _ztga_rle_fillbuffer( strm, buffer, NULL, 0, &err ) ) <= 0 ) {
						_ztga_error( err, func, data, error );
						zfree( buffer );
						zimage_destroy( ret );
						return NULL;
					}
					bufptr = buffer;
				}
				rleval = *bufptr++;
				rlelen = ( rleval & ( ~TGA_RLE_PACKET ) ) + 1U;
				read--;

				/* more data than we need */
				if( rlelen > width ) {
					_ztga_error( Z_ERROR_CORRUPTED, func, data, error );
					zfree( buffer );
					zimage_destroy( ret );
					return NULL;
				}

				width -= rlelen;

				/* no data left in buffer */
				if( !read ) {
					if( ( read = _ztga_rle_fillbuffer( strm, buffer, NULL, 0, &err ) ) <= 0 ) {
						_ztga_error( err, func, data, error );
						zfree( buffer );
						zimage_destroy( ret );
						return NULL;
					}
					bufptr = buffer;
				}

				/* rle packet */
				if( rleval & TGA_RLE_PACKET ) {
					while( rlelen-- )
						*dptr++ = *bufptr;
					bufptr++;
					read--;
					/* */
				} else { /* raw packet */
					/* copy rleval pixels to dptr */
					while( rlelen-- ) {
						*dptr++ = *bufptr++;
						read--;
						/* again no data anymore */
						if( !read ) {
							if( ( read = _ztga_rle_fillbuffer( strm, buffer, NULL, 0, &err ) ) <= 0 ) {
								_ztga_error( err, func, data, error );
								zfree( buffer );
								zimage_destroy( ret );
								return NULL;
							}
							bufptr = buffer;
						}
					}
				}
			}
			/* progress */
			if( func ) {
				progress += iprogress;
				if( progress >= lprogress ) {
					if( !func( data, MIN( progress, 1.0f ), Z_ERROR_NONE ) ) {
						zfree( buffer );
						zimage_destroy( ret );
						return NULL;
					}
					lprogress = MIN( progress + 0.01f, 1.0f);
				}
			}
			dst += stride;
		}
		zfree( buffer );
	} else {
		err = Z_ERROR_UNSUPPORTED;
		_ztga_error( err, func, data, error );
	}

	return ret;
}

/*
   Supported combinations of cmap-depth, depth and attribute-bits:

   Depth   Cmap-Depth  Attribute-Bits
   ----------------------------------
   8	    32		8
   8	    32		0
   8	    24		0
   8	    8		0
 */
static ZImage* _ztga_decode_cmap_rle( ZStream* strm, TGAHeader* header, TGAExtension* ext ZATTRIBUTE_UNUSED,
									 ZProgressDataErrorFunc func, void* data, ZError* error )
{
	ZImage* ret = NULL;
	uint8_t* dst;
	uint8_t* dptr;
	uint8_t* buffer;
	uint8_t* bufptr;
	uint8_t rleval, rlelen;
	ssize_t stride;
	ssize_t len;
	size_t width;
	size_t height;
	ssize_t read;
	size_t dataoffset;
	ZError err;
	uint8_t* mapbuffer;
	float progress = 0.0f;
	float lprogress = 0.0f;
	float iprogress;
	uint8_t alphabits = header->desc & TGA_ALPHABIT_MASK;

	iprogress = 1.0f / ( ( float ) header->height - 1.0f );

	/*seek to cmap*/
	dataoffset = TGA_HEADER_SIZE + header->idlength;
	if( zstream_seek( strm, dataoffset, Z_STREAMORIGIN_SET ) < 0 ) {
		err = zstream_get_status( strm );
		_ztga_error( err, func, data, error);
		return NULL;
	}

	if( header->depth == 8 &&
	   ( ( header->cmapdepth == 32 && ( alphabits == 8 || alphabits == 0 ) ) ||
		( header->cmapdepth == 24 && alphabits == 0 ) ) ) {
		uint32_t* cmap;

		if( header->cmapdepth == 32 ) {
			cmap = zmalloc( sizeof( uint32_t ) * header->cmaplen );
			len = sizeof( uint32_t ) * header->cmaplen;
		} else {
			cmap = zmalloc( sizeof( uint8_t ) * 3 * header->cmaplen );
			len = sizeof( uint8_t ) * 3 * header->cmaplen;
		}

		if( ( read = zstream_read( strm, cmap, len ) ) != len ) {
			if( read < 0 )
				err = zstream_get_status( strm );
			else
				err = Z_ERROR_CORRUPTED;
			_ztga_error( err, func, data, error );
			zfree( cmap );
			return NULL;
		}

		if( header->cmapdepth == 32 && alphabits == 0 ) {
			ziutil_xxxa_to_xxx1_ub( cmap, cmap, header->cmaplen );
		} else if( header->cmapdepth == 24 ) {
			/* conver RGB cmap to RGBA */
			uint32_t* newcmap;
			newcmap = zmalloc( sizeof( uint32_t ) * header->cmaplen );
			ziutil_rgb_to_rgba_ub( newcmap, cmap, header->cmaplen );
			zfree( cmap );
			cmap = newcmap;
		}

		/* seek to data */
		dataoffset = TGA_HEADER_SIZE + header->idlength + len;
		if( zstream_seek( strm, dataoffset, Z_STREAMORIGIN_SET ) < 0 ) {
			err = zstream_get_status( strm );
			_ztga_error( err, func, data, error);
			return NULL;
		}

		ret = zimage_new( Z_IMAGEFORMAT_BGRA_UB, header->width, header->height );
		dst = zimage_get_data( ret );
		stride = zimage_get_stride( ret );
		height = header->height;

		mapbuffer = zmalloc( sizeof( uint8_t ) * header->width );
		buffer = zmalloc( TGA_RLE_BUFFER_SIZE * sizeof( uint8_t ) );
		bufptr = buffer;

		read = 0;

		dst = _ztga_origin( header, dst, &stride );

		while( height-- ) {
			dptr = mapbuffer;
			width = header->width;
			/* buffer filled and still pixels to do */
			while( width ) {
				/* fill buffer */
				if( !read ) {
					if( ( read = _ztga_rle_fillbuffer( strm, buffer, NULL, 0, &err ) ) <= 0 ) {
						_ztga_error( err, func, data, error );
						zfree( cmap );
						zfree( mapbuffer );
						zfree( buffer );
						zimage_destroy( ret );
						return NULL;
					}
					bufptr = buffer;
				}
				rleval = *bufptr++;
				rlelen = ( rleval & ( ~TGA_RLE_PACKET ) ) + 1U;
				read--;

				/* more data than we need */
				if( rlelen > width ) {
					_ztga_error( Z_ERROR_CORRUPTED, func, data, error );
					zfree( cmap );
					zfree( mapbuffer );
					zfree( buffer );
					zimage_destroy( ret );
					return NULL;
				}

				width -= rlelen;

				/* no data left in buffer */
				if( !read ) {
					if( ( read = _ztga_rle_fillbuffer( strm, buffer, NULL, 0, &err ) ) <= 0 ) {
						_ztga_error( err, func, data, error );
						zfree( cmap );
						zfree( mapbuffer );
						zfree( buffer );
						zimage_destroy( ret );
						return NULL;
					}
					bufptr = buffer;
				}

				/* rle packet */
				if( rleval & TGA_RLE_PACKET ) {
					while( rlelen-- )
						*dptr++ = *bufptr;
					bufptr++;
					read--;
					/* */
				} else { /* raw packet */
					/* copy rleval pixels to dptr */
					while( rlelen-- ) {
						*dptr++ = *bufptr++;
						read--;
						/* again no data anymore */
						if( !read ) {
							if( ( read = _ztga_rle_fillbuffer( strm, buffer, NULL, 0, &err ) ) <= 0 ) {
								_ztga_error( err, func, data, error );
								zfree( cmap );
								zfree( mapbuffer );
								zfree( buffer );
								zimage_destroy( ret );
								return NULL;
							}
							bufptr = buffer;
						}
					}
				}
			}

			if( !_ztga_apply_cmap32( dst, mapbuffer, header->width, header->cmapindex, cmap, header->cmaplen ) ) {
				_ztga_error( Z_ERROR_RANGE, func, data, error );
				zfree( cmap );
				zfree( mapbuffer );
				zfree( buffer );
				zimage_destroy( ret );
				return NULL;
			}

			/* progress */
			if( func ) {
				progress += iprogress;
				if( progress >= lprogress ) {
					if( !func( data, MIN( progress, 1.0f ), Z_ERROR_NONE ) ) {
						zfree( cmap );
						zfree( mapbuffer );
						zfree( buffer );
						zimage_destroy( ret );
						return NULL;
					}
					lprogress = MIN( progress + 0.01f, 1.0f);
				}
			}
			dst += stride;
		}
		zfree( mapbuffer );
		zfree( buffer );
		zfree( cmap );
	} else if( header->depth == 8 && header->cmapdepth == 8 && alphabits == 0 ) {
		uint8_t* cmap;

		cmap = zmalloc( sizeof( uint8_t ) * header->cmaplen );
		len = sizeof( uint8_t ) * header->cmaplen;

		if( ( read = zstream_read( strm, cmap, len ) ) != len ) {
			if( read < 0 )
				err = zstream_get_status( strm );
			else
				err = Z_ERROR_CORRUPTED;
			_ztga_error( err, func, data, error );
			zfree( cmap );
			return NULL;
		}

		/* seek to data */
		dataoffset = TGA_HEADER_SIZE + header->idlength + len;
		if( zstream_seek( strm, dataoffset, Z_STREAMORIGIN_SET ) < 0 ) {
			err = zstream_get_status( strm );
			_ztga_error( err, func, data, error);
			return NULL;
		}

		ret = zimage_new( Z_IMAGEFORMAT_GRAY_UB, header->width, header->height );
		dst = zimage_get_data( ret );
		stride = zimage_get_stride( ret );
		height = header->height;

		mapbuffer = zmalloc( sizeof( uint8_t ) * header->width );
		buffer = zmalloc( TGA_RLE_BUFFER_SIZE * sizeof( uint8_t ) );
		bufptr = buffer;

		read = 0;

		dst = _ztga_origin( header, dst, &stride );

		while( height-- ) {
			dptr = mapbuffer;
			width = header->width;
			/* buffer filled and still pixels to do */
			while( width ) {
				/* fill buffer */
				if( !read ) {
					if( ( read = _ztga_rle_fillbuffer( strm, buffer, NULL, 0, &err ) ) <= 0 ) {
						_ztga_error( err, func, data, error );
						zfree( cmap );
						zfree( mapbuffer );
						zfree( buffer );
						zimage_destroy( ret );
						return NULL;
					}
					bufptr = buffer;
				}
				rleval = *bufptr++;
				rlelen = ( rleval & ( ~TGA_RLE_PACKET ) ) + 1U;
				read--;

				/* more data than we need */
				if( rlelen > width ) {
					_ztga_error( Z_ERROR_CORRUPTED, func, data, error );
					zfree( cmap );
					zfree( mapbuffer );
					zfree( buffer );
					zimage_destroy( ret );
					return NULL;
				}

				width -= rlelen;

				/* no data left in buffer */
				if( !read ) {
					if( ( read = _ztga_rle_fillbuffer( strm, buffer, NULL, 0, &err ) ) <= 0 ) {
						_ztga_error( err, func, data, error );
						zfree( cmap );
						zfree( mapbuffer );
						zfree( buffer );
						zimage_destroy( ret );
						return NULL;
					}
					bufptr = buffer;
				}

				/* rle packet */
				if( rleval & TGA_RLE_PACKET ) {
					while( rlelen-- )
						*dptr++ = *bufptr;
					bufptr++;
					read--;
					/* */
				} else { /* raw packet */
					/* copy rleval pixels to dptr */
					while( rlelen-- ) {
						*dptr++ = *bufptr++;
						read--;
						/* again no data anymore */
						if( !read ) {
							if( ( read = _ztga_rle_fillbuffer( strm, buffer, NULL, 0, &err ) ) <= 0 ) {
								_ztga_error( err, func, data, error );
								zfree( cmap );
								zfree( mapbuffer );
								zfree( buffer );
								zimage_destroy( ret );
								return NULL;
							}
							bufptr = buffer;
						}
					}
				}
			}

			if( !_ztga_apply_cmap8( dst, mapbuffer, header->width, header->cmapindex, cmap, header->cmaplen ) ) {
				_ztga_error( Z_ERROR_RANGE, func, data, error );
				zfree( cmap );
				zfree( mapbuffer );
				zfree( buffer );
				zimage_destroy( ret );
				return NULL;
			}

			/* progress */
			if( func ) {
				progress += iprogress;
				if( progress >= lprogress ) {
					if( !func( data, MIN( progress, 1.0f ), Z_ERROR_NONE ) ) {
						zfree( cmap );
						zfree( mapbuffer );
						zfree( buffer );
						zimage_destroy( ret );
						return NULL;
					}
					lprogress = MIN( progress + 0.01f, 1.0f);
				}
			}
			dst += stride;
		}
		zfree( mapbuffer );
		zfree( buffer );
		zfree( cmap );
	} else {
		_ztga_error( Z_ERROR_UNSUPPORTED, func, data, error );
	}

	return ret;
}
#endif


	class TGALoader : public ILoader
	{
		public:
			TGALoader() {}

			void load( Image& dst, const String& file );

			const String& extension( size_t n ) const { return _extension[ n ]; }
			size_t sizeExtensions() const { return 2; }
			const String& name() const { return _name; }

		private:

			static String _name;
			static String _extension[ 2 ];
	};

	String TGALoader::_name = "TGA";
	String TGALoader::_extension[] = { ".tga", ".TGA" };


	void TGALoader::load( Image& img, const String& path )
	{
		TGAHeader header;
		TGAFooter footer;
		TGAExtension extbuf;
		TGAExtension* ext = NULL;
		size_t read;
		bool isversion2 = false;
		FILE* file;

		if( ( file = fopen( path.c_str(), "r" ) ) == NULL )
			throw CVTException( ( String( "Unable to open TGA file:" ) + path ).c_str() );


		/*check for TARGA file*/
		if( fseek( file, -26L, SEEK_END ) < 0 )
			throw CVTException( "Corrupted TGA file" );

		if( ( read = fread( &footer, sizeof( uint8_t ), 26, file ) ) != 26 )
			throw CVTException( "Corrupted TGA file" );

		/* FIXME */
		if( !strcmp( _tga_signature, footer.signature ) ) {
			isversion2 = true;
		}

		/*read header*/
		if( fseek( file, 0, SEEK_SET ) < 0 )
			throw CVTException( "Corrupted TGA file" );

		if( ( read = fread( &header, sizeof( uint8_t ), TGA_HEADER_SIZE, file ) ) != TGA_HEADER_SIZE ) {
			throw CVTException( "Corrupted TGA file" );
		}

		// FIXME: convert le16 to native host format
/*	header.cmapindex = zswab_le16_to_cpu( header.cmapindex );
	header.cmaplen = zswab_le16_to_cpu( header.cmaplen );
	header.xorigin = zswab_le16_to_cpu( header.xorigin );
	header.yorigin = zswab_le16_to_cpu( header.yorigin );
	header.width = zswab_le16_to_cpu( header.width );
	header.height = zswab_le16_to_cpu( header.height );*/

		/*
		   printf( "\nTGA Version %d\n", isversion2?2:1 );
		   printf( "IDLENGTH: 0x%0x\n", header.idlength );
		   printf( "COLORMAPTYPE: 0x%0x\n", header.cmaptype );
		   printf( "IMAGETYPE: 0x%0x\n", header.imagetype );
		   printf( "COLORMAP-INDEX: 0x%0x\n", header.cmapindex );
		   printf( "COLORMAP-LENGTH: 0x%0x\n", header.cmaplen );
		   printf( "COLORMAP-DEPTH: 0x%0x\n", header.cmapdepth );
		   printf( "x-ORIGIN: %d\n", header.xorigin );
		   printf( "y-ORIGIN: %d\n", header.yorigin );
		   printf( "WIDTH: %d\n", header.width );
		   printf( "HEIGHT: %d\n", header.height );
		   printf( "DEPTH: %d\n", header.depth );
		   printf( "DESC: 0x%0x\n", header.desc );
		 */


		if( isversion2 ) {

			// FIXME: le32 to native host
/*			footer.extoffset = zswab_le32_to_cpu( footer.extoffset );
			footer.devoffset = zswab_le32_to_cpu( footer.devoffset );*/


			if( footer.extoffset ) {
				if( fseek( file, footer.extoffset, SEEK_SET ) < 0 )
					throw CVTException( "Corrupted TGA file" );

				if( ( read = fread( &extbuf, sizeof( TGAExtension ), 1, file ) ) != sizeof( TGAExtension ) )
					throw CVTException( "Corrupted TGA file" );
				ext = &extbuf;
				/*
				   printf( "Extension:\n" );
				   printf( "\tAuthorname: %s\n", ext.authorname );
				   printf( "\tAuthor-Comment: %s\n", (char*) ext.authorcomment );
				   printf( "\tJonname/ID: %s\n", ext.jobnameid );
				   printf( "\tSoftware/ID: %s\n", ext.softwareid );
				 */
			}
		}

		switch( header.imagetype ) {
			/*case TGA_TYPE_CMAP:
				if( header.cmaptype != 1 )
					throw CVTException("Unsupported TGA file!");
				_ztga_decode_cmap( img, file, &header, ext );
				break;*/
			case TGA_TYPE_TRUECOLOR:
				if( header.cmaptype != 0 )
					throw CVTException("Unsupported TGA file!");
				tga_decode_color( img, file, &header, ext );
				break;
			case TGA_TYPE_MONO:
				if( header.cmaptype != 0 )
					throw CVTException("Unsupported TGA file!");
				tga_decode_gray( img, file, &header, ext );
				break;
/*			case TGA_TYPE_RLE_CMAP:
				if( header.cmaptype != 1 )
					throw CVTException("Unsupported TGA file!");
				tga_decode_cmap_rle( img, file, &header, ext );
				break;
			case TGA_TYPE_RLE_TRUECOLOR:
				if( header.cmaptype != 0 )
					throw CVTException("Unsupported TGA file!");
				_ztga_decode_color_rle( img, file, &header, ext );
				break;

			case TGA_TYPE_RLE_MONO:
				if( header.cmaptype != 0 )
					throw CVTException("Unsupported TGA file!");
				tga_decode_gray_rle( img, file, &header, ext );
				break;*/
			default:
					throw CVTException("Unsupported TGA file!");
				break;
		}

		fclose( file );
	}

}

static void _init( cvt::PluginManager* pm )
{
	cvt::ILoader* tga = new cvt::TGALoader();
	pm->registerPlugin( tga );
}

CVT_PLUGIN( _init )
