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

#include <cvt/io/RawVideoWriter.h>
#include <cvt/util/Exception.h>
#include <cvt/gfx/IMapScoped.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

namespace cvt
{
	RawVideoWriter::RawVideoWriter( const String & filename ):
		_fd( -1 ),
		_currSize( 0 ),
		_maxSize( 0 ),
		_offsetInFile( 0 ),
		_map( 0 ),
		_mappedSize( 0 ),
		_pos( 0 ),
		_width( 0 ),
		_height( 0 ),
		_stride( 0 ),
		_formatID( 0 )
	{
		// create the file (open, truncate to header size)
		_fd = open( filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG );
		if( _fd < 0 ){
			char * err = strerror( errno );
			String msg( "Could not open file: " );
			msg += err;
			throw CVTException( msg.c_str() );
		}

		// get the system page size
		_pageSize = sysconf( _SC_PAGE_SIZE );
	}

	RawVideoWriter::~RawVideoWriter()
	{
		if( _fd != -1 ){
			if( _map != 0 ){
				if( munmap( _map, _mappedSize ) != 0 ){
					char * err = strerror( errno );
					String msg( "Could not unmap memory: " );
					msg += err;
					throw CVTException( msg.c_str() );
				}
			}

			// truncate to current size 
			if( ftruncate( _fd, _currSize ) < 0){
				char * err = strerror( errno );
				String msg( "Could not resize file: " );
				msg += err;
				throw CVTException( msg.c_str() );
			}

			if( close( _fd ) < 0 ){
				char * err = strerror( errno );
				String msg( "Could not close file: " );
				msg += err;
				throw CVTException( msg.c_str() );
			}
		}

	}

	void RawVideoWriter::write( const Image & img )
	{
		IMapScoped<const uint8_t> map( img );
		if( _width == 0 ){
			_width = img.width();
			_height = img.height();
			_stride = map.stride();
			_formatID = (size_t)img.format().formatID;
			_imgSize = _height * _stride;

			// need to write the header first
			remapFile( 4 * sizeof( uint32_t ) );
			writeHeader();
		} else {
			// check size and format
			if( _width != img.width() ||
				_height != img.height() ||
				_stride != map.stride() ||
				_formatID != ( size_t )img.format().formatID ){
				throw CVTException( "Trying to mix different image resolution or format in a single video!");
			}
		}

		// check if we need to remap
		if( ( _currSize + ( off_t )_imgSize ) > _maxSize ){
			remapFile();
		}

		SIMD * simd = SIMD::instance();
		simd->Memcpy( _pos, map.ptr(), _imgSize );

		// compute page aligned pointer position
		uint8_t* ppos = (uint8_t*)( ( size_t )_pos &( ~( _pageSize - 1 ) ) );
		size_t len = ( ( size_t )_pos + _imgSize - ( size_t )ppos );// & ( ~( _pageSize -1 ) );
		if( msync( ppos, len, MS_ASYNC ) == -1 ){
			char * err = strerror( errno );
			String msg( "Could not msync: " );
			msg += err;
			throw CVTException( msg.c_str() );
		}

		_pos += _imgSize;
		_offsetInFile += _imgSize;
		_currSize += _imgSize;
	}

	void RawVideoWriter::remapFile( size_t additionalBytes )
	{
		size_t resizeSize = 50 * _imgSize + additionalBytes;
		if( _map != 0 ){
			if( munmap( _map, _mappedSize ) != 0 ){
				char * err = strerror( errno );
				String msg( "Could not unmap memory: " );
				msg += err;
				_map = 0;
				_mappedSize = 0;
				throw CVTException( msg.c_str() );
			}
			_map = 0;
			_mappedSize = 0;
		}
		_maxSize += resizeSize;
		resizeFile();

		// file has maxSize 
		size_t mapSize = resizeSize;
		off_t alignedOffset = 0;
		size_t ptrOffset = 0;
		if( _offsetInFile != 0 ){
			// copmute page aligned offset: 
			alignedOffset = _offsetInFile & ( ~( _pageSize - 1 ) );
			ptrOffset = _offsetInFile - alignedOffset;
			mapSize += ptrOffset;
		}

		_map = mmap( 0, mapSize, PROT_WRITE, MAP_SHARED, _fd, alignedOffset );
		if( _map == MAP_FAILED ){
			std::cout << "MapSize: " << mapSize << " AlignedOffset: " << alignedOffset << ", _offsetInFile " << _offsetInFile << std::endl;
			char * err = strerror( errno );
			String msg( "Could not map file: " );
			msg += err;
			_map = 0;
			throw CVTException( msg.c_str() );
		} 
			
		// map successful
		_pos = ( uint8_t* )_map;
		_pos += ptrOffset;
		_mappedSize = mapSize;
		
		if( madvise( _map, _mappedSize, MADV_SEQUENTIAL ) != 0 ){
			char * err = strerror( errno );
			String msg( "Memadvise failed: " );
			msg += err;
			throw CVTException( msg.c_str() );
		}
	}

	void RawVideoWriter::writeHeader()
	{
		*( ( uint32_t* )_pos ) = _width; _pos += sizeof( uint32_t );
		*( ( uint32_t* )_pos ) = _height; _pos += sizeof( uint32_t );
		*( ( uint32_t* )_pos ) = _stride; _pos += sizeof( uint32_t );
		*( ( uint32_t* )_pos ) = _formatID; _pos += sizeof( uint32_t );
		_offsetInFile += 4 * sizeof( uint32_t );
	}

	void RawVideoWriter::resizeFile()
	{
		int res = lseek( _fd, _maxSize - 1, SEEK_SET );
		if( res == -1 ){
			char * err = strerror( errno );
			String msg( "File resize failed: " );
			msg += err;
			throw CVTException( msg.c_str() );
		}

		res = ::write( _fd, "", 1 );
		if( res == -1 ){
			char * err = strerror( errno );
			String msg( "File resize failed: " );
			msg += err;
			throw CVTException( msg.c_str() );
		}
	}
}

