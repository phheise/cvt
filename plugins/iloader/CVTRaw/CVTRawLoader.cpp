#include "CVTRawLoader.h"
#include <cvt/util/PluginManager.h>
#include <cvt/gfx/Image.h>
#include <iostream>
#include <fstream>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>

namespace cvt {
	String CVTRawLoader::_extension = ".cvtraw";
	String CVTRawLoader::_name = "CVTRaw";

	void CVTRawLoader::load( Image& img, const String& path )
	{
		int fd = open( path.c_str(), O_RDONLY );

		if( fd == -1 ){
			String error( "Could not open file: " );
			error += path;
			throw CVTException( error.c_str() );
		}

		// get the size of the file
		struct stat fileInfo;
		if( fstat( fd, &fileInfo ) < 0 ){
			close( fd );
			throw CVTException( "Could not get file information" );
		}

		size_t fileSize = fileInfo.st_size;	

		// file is opened -> map it
		void * origPtr = mmap( 0, fileSize, PROT_READ, MAP_FILE | MAP_SHARED, fd, 0 );
		uint8_t* ptr = ( uint8_t* )origPtr;
		

		if( origPtr == MAP_FAILED ){
			char * errorMsg = strerror( errno );
			String error( "mmap failed: " );
			error += errorMsg;
			close( fd );
			throw CVTException( error.c_str() );
		}
		close( fd );

		// header: width, height, stride, IFormat
		uint32_t savedStride, width, height, formatId;
		
		width		= *( ( uint32_t* )ptr ); ptr+= ( sizeof( uint32_t ) );
		height		= *( ( uint32_t* )ptr ); ptr+= ( sizeof( uint32_t ) );
		savedStride = *( ( uint32_t* )ptr ); ptr+= ( sizeof( uint32_t ) );
		formatId	= *( ( uint32_t* )ptr ); ptr+= ( sizeof( uint32_t ) );

		img.reallocate( width, height, IFormat::formatForId( ( IFormatID ) formatId ) );

		uint8_t *p, *punmap;
		size_t stride;
		p = punmap = img.map<uint8_t>( &stride );

		SIMD* simd = SIMD::instance();
		if( savedStride == stride ){
			simd->Memcpy( p, ptr, height * stride );
		} else {
			while( height-- ){
				simd->Memcpy( p, ptr, width * img.bpp() );
				p += stride;
				ptr += savedStride;
			}
		}

		if( munmap( origPtr, fileSize ) < 0 ){
			throw CVTException( "Could not unmap memory!");
		}

		img.unmap( punmap );
	}
}

static void _init( cvt::PluginManager* pm )
{
	cvt::ILoader* cvtraw = new cvt::CVTRawLoader();
	pm->registerPlugin( cvtraw );
}

CVT_PLUGIN( _init )
