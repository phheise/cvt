#include "CVTRawSaver.h"
#include <cvt/util/PluginManager.h>
#include <cvt/gfx/Image.h>
#include <iostream>
#include <fstream>

namespace cvt {
	String CVTRawSaver::_extension = ".cvtraw";
	String CVTRawSaver::_name = "CVTRaw";

	void CVTRawSaver::save( const String& path, const Image& img )
	{
		std::fstream file( path.c_str(),
						  std::fstream::out |
						  std::fstream::binary |
						  std::fstream::trunc );

		// header: width, height, stride, IFormatID
		size_t stride;
		const char *p;
		p = img.map<char>( &stride );

		uint32_t formatId = ( size_t )img.format().formatID;
		uint32_t width = img.width();
		uint32_t height = img.height();
		uint32_t sStride = stride;

		file.write( ( char* )&width, sizeof( uint32_t ) );
		file.write( ( char* )&height, sizeof( uint32_t ) );
		file.write( ( char* )&sStride, sizeof( uint32_t ) );
		file.write( ( char* )&formatId, sizeof( uint32_t ) );
		file.write( p, img.height() * stride );

		img.unmap( p );

		file.close();
	}
}

static void _init( cvt::PluginManager* pm )
{
	cvt::ISaver* cvtraw = new cvt::CVTRawSaver();
	pm->registerPlugin( cvtraw );
}

CVT_PLUGIN( _init )
