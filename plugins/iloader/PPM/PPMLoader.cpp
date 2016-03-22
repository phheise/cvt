#include <cvt/util/PluginManager.h>
#include <cvt/gfx/Image.h>
#include <cvt/util/Exception.h>
#include <cvt/io/FileSystem.h>
#include <cvt/util/DataIterator.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

namespace cvt {
	class PPMLoader : public ILoader
	{
		public:
			PPMLoader() {}
			void load( Image& dst, const String& file );
			const String& extension( size_t n ) const { return _extension[ n ]; }
			size_t sizeExtensions() const { return 4; }
			const String& name() const { return _name; }

		private:
            enum PGMType
            {
                P2_GRAY_ASCII,
                P3_RGB_ASCII,
                P5_GRAY_BINARY,
                P6_RGB_BINARY
            };

            struct PGMHeader
            {
                PGMType type;
                int     width;
                int     height;
                int     colorDepth;
            };

			static String _name;
			static String _extension[ 4 ];

            void parseHeader( PGMHeader & header, DataIterator & dataIter );

            void nextDataLine( std::vector<String> & tokens, DataIterator & iter );

            void parseBinary( Image & img, DataIterator & data );
            void parseASCII( Image & img, DataIterator & data );

	};

	String PPMLoader::_name = "PPM";
	String PPMLoader::_extension[] = { ".ppm", ".PPM", ".pgm", ".PGM" };

     void PPMLoader::nextDataLine( std::vector<String> & tokens, DataIterator & iter )
    {
        while( iter.hasNext() ){
            iter.tokenizeNextLine( tokens, " \t" );
            if( tokens.size() && tokens[ 0 ][ 0 ] != '#' )
                return;
            tokens.clear();
        }
    }

    void PPMLoader::parseHeader( PGMHeader & header, DataIterator & dataIter )
    {
        std::vector<String> tokens;

        nextDataLine( tokens, dataIter );
        if( tokens.size() == 0 )
            throw CVTException( "PPM Header corrupt" );

        // first is the type:
        if ( tokens[ 0 ] == "P2" )
            header.type = P2_GRAY_ASCII;
        else if ( tokens[ 0 ] == "P3" )
            header.type = P3_RGB_ASCII;
        else if ( tokens[ 0 ] == "P5" )
            header.type = P5_GRAY_BINARY;
        else if ( tokens[ 0 ] == "P6" )
            header.type = P6_RGB_BINARY;
        else {
            String msg( "Unknown PGM Type: " );
            msg += tokens[ 0 ];
            throw CVTException( msg.c_str() );
        }

        tokens.clear();
        nextDataLine( tokens, dataIter );

        // width height
        if( tokens.size() < 2 )
            throw CVTException( "PPM Header corrupt: Expected  <width> <height>" );

        header.width = tokens[ 0 ].toInteger();
        header.height = tokens[ 1 ].toInteger();

        tokens.clear();
        nextDataLine( tokens, dataIter );

        // width height
        if( tokens.size() < 1 )
            throw CVTException( "PPM Header corrupt: Expected  <ColorDepth>" );
        header.colorDepth = tokens[ 0 ].toInteger();

    }

	void PPMLoader::load( Image& img, const String& path )
	{
        Data data;

        FileSystem::load( data, path );

        DataIterator dataIter( data );
        PGMHeader header;
        parseHeader( header, dataIter );

        switch ( header.type ) {
            case P2_GRAY_ASCII:
                if( header.colorDepth > 255 )
                    img.reallocate( header.width, header.height, IFormat::GRAY_UINT16 );
                else
                    img.reallocate( header.width, header.height, IFormat::GRAY_UINT8 );
                parseASCII( img, dataIter );
                break;
            case P3_RGB_ASCII:
                if( header.colorDepth > 255 )
                    img.reallocate( header.width, header.height, IFormat::RGBA_UINT16 );
                else
                    img.reallocate( header.width, header.height, IFormat::RGBA_UINT8 );
                parseASCII( img, dataIter );
                break;
            case P5_GRAY_BINARY:
                if( header.colorDepth > 255 )
                    img.reallocate( header.width, header.height, IFormat::GRAY_UINT16 );
                else
                    img.reallocate( header.width, header.height, IFormat::GRAY_UINT8 );
                parseBinary( img, dataIter );
                break;
            case P6_RGB_BINARY:
                if( header.colorDepth > 255 )
                    img.reallocate( header.width, header.height, IFormat::RGBA_UINT16 );
                else
                    img.reallocate( header.width, header.height, IFormat::RGBA_UINT8 );
                parseBinary( img, dataIter );
                break;
        }
	}

    void PPMLoader::parseBinary( Image & img, DataIterator & data )
    {
        size_t stride;
        uint8_t * iptr = img.map( &stride );
        uint8_t * ptr = iptr;

        size_t width = img.width();
        size_t height = img.height();

        size_t n;
        switch ( img.format().formatID ) {
            case IFORMAT_RGBA_UINT8:
            case IFORMAT_RGBA_UINT16:
            case IFORMAT_RGBA_FLOAT:
                n = img.width() * 3;
                break;
            case IFORMAT_GRAY_UINT8:
            case IFORMAT_GRAY_UINT16:
            case IFORMAT_GRAY_FLOAT:
                n = img.width();
                break;
            default:
                throw CVTException( "Unsupported image format for PPM" );
                break;
        }

        SIMD* simd = SIMD::instance();

        const uint8_t * src = data.pos();

        switch ( img.format().type ) {
            case IFORMAT_TYPE_FLOAT:
                if( data.remainingBytes() < ( n * height * sizeof( float ) ) )
                    throw CVTException( "NOT ENOUGH DATA REMAINING" );
                while ( height-- ) {
                    simd->Conv_XXXf_to_XXXAf( ( float* )ptr, ( const float* )src, width );
                    ptr += stride;
                    src += n * sizeof( float );
                }
                break;
            case IFORMAT_TYPE_UINT8:
                if( data.remainingBytes() < ( n * height ) )
                    throw CVTException( "NOT ENOUGH DATA REMAINING" );

                while ( height-- ) {
                    simd->Conv_XXXu8_to_XXXAu8( ptr, src, width );
                    ptr += stride;
                    src += n;
                }
                break;
            default:
                throw CVTException( "FORMAT NOT SUPPORTED" );
                break;
        }
        img.unmap( iptr );
    }

    void PPMLoader::parseASCII( Image & img, DataIterator & data )
    {
        size_t stride;
        uint8_t * iptr = img.map( &stride );
        uint8_t * ptr = iptr;

		size_t h = img.height();

        switch ( img.format().type ) {
            case IFORMAT_TYPE_UINT8:
                while ( h-- ){
					for( size_t w = 0; w < img.width(); w++ ){
						if( img.channels() == 4 ){
							for( int i = 0; i < 3; i++ ){
								ptr[ w * 4 + i ] = ( uint8_t )data.nextLong();
							}
							ptr[ w * 4 + 3 ] = 255;
						} else {
							ptr[ w ] = ( uint8_t )data.nextLong();
						}
					}
                    ptr += stride;
                }
                break;
            default:
                throw CVTException( "Format not supported in ASCII" );
                break;
        }

        img.unmap( iptr );
    }

}

static void _init( cvt::PluginManager* pm )
{
	cvt::ILoader* ppm = new cvt::PPMLoader();
	pm->registerPlugin( ppm );
}

CVT_PLUGIN( _init )
