#include <cvt/util/PluginManager.h>
#include <cvt/gfx/Image.h>
#include <cvt/util/Exception.h>
#include <cvt/util/ScopedBuffer.h>
#include <cvt/util/CPU.h>
#include <cvt/util/ScopedBuffer.h>
#include <cvt/io/FileSystem.h>
#include <cvt/util/DataIterator.h>
#include <cvt/gfx/IMapScoped.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

namespace cvt {
	class PFMLoader : public ILoader
	{
		public:
			PFMLoader() {}
			void load( Image& dst, const String& file );
			const String& extension( size_t n ) const { return _extension[ n ]; }
			size_t sizeExtensions() const { return 2; }
			const String& name() const { return _name; }

		private:
            enum PFMType
            {
                PFM_GRAY,
                PFM_RGB,
            };

            struct PFMHeader
            {
                PFMType type;
                int     width;
                int     height;
                float   scale;
            };

			static String _name;
			static String _extension[ 2 ];

            void parseHeader( PFMHeader & header, DataIterator & dataIter );

            void nextDataLine( std::vector<String> & tokens, DataIterator & iter );

            void parseBinary( Image & img, DataIterator & data );
            void parseBinarySwap( Image & img, DataIterator & data );

	};

	String PFMLoader::_name = "PFM";
	String PFMLoader::_extension[] = { ".pfm", ".PFM" };

     void PFMLoader::nextDataLine( std::vector<String> & tokens, DataIterator & iter )
    {
        while( iter.hasNext() ){
            iter.tokenizeNextLine( tokens, " \t" );
            if( tokens.size() && tokens[ 0 ][ 0 ] != '#' )
                return;
            tokens.clear();
        }
    }

    void PFMLoader::parseHeader( PFMHeader & header, DataIterator & dataIter )
    {
        std::vector<String> tokens;

        nextDataLine( tokens, dataIter );
        if( tokens.size() == 0 )
            throw CVTException( "PPM Header corrupt" );

        // first is the type:
        if ( tokens[ 0 ] == "Pf" )
            header.type = PFM_GRAY;
        else if ( tokens[ 0 ] == "PF" )
            header.type = PFM_RGB;
        else {
            String msg( "Unknown PFM Type: " );
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
        header.scale = tokens[ 0 ].toFloat();

    }

	void PFMLoader::load( Image& img, const String& path )
	{
        Data data;

        FileSystem::load( data, path );

        DataIterator dataIter( data );
        PFMHeader header;
        parseHeader( header, dataIter );

        if( header.type == PFM_GRAY ) {
            img.reallocate( header.width, header.height, IFormat::GRAY_FLOAT );
        } else if ( header.type == PFM_RGB ) {
            img.reallocate( header.width, header.height, IFormat::RGBA_FLOAT );
        }

        if( isLittleEndian() && header.scale < 0  )
            parseBinary( img, dataIter );
        else
            parseBinarySwap( img, dataIter );
	}

    void PFMLoader::parseBinary( Image & img, DataIterator & data )
    {
        IMapScoped<float> map( img );
        size_t width = img.width();
        size_t height = img.height();
        const uint8_t* src = data.pos();
        SIMD* simd = SIMD::instance();

        map.setLine( map.height() - 1 );

        switch ( img.channels() ) {
            case 1:
                if( data.remainingBytes() < ( width * height * sizeof( float ) ) )
                    throw CVTException( "NOT ENOUGH DATA REMAINING" );
                while ( height-- ) {
                    simd->Memcpy( ( uint8_t* ) map.ptr(), ( const uint8_t* )src, width * sizeof( float ) );
                    map--;
                    src += width * sizeof( float );
                }
                break;
            case 4:
                if( data.remainingBytes() < ( width * height * 3 * sizeof( float ) ) )
                    throw CVTException( "NOT ENOUGH DATA REMAINING" );

                while ( height-- ) {
                    simd->Conv_XXXf_to_XXXAf( map.ptr(), ( const float* )src, width );
                    map--;
                    src += width * 3 * sizeof( float );
                }
                break;
            default:
                throw CVTException( "FORMAT NOT SUPPORTED" );
                break;
        }
    }

    void PFMLoader::parseBinarySwap( Image & img, DataIterator & data )
    {
        IMapScoped<float> map( img );
        size_t width = img.width();
        size_t height = img.height();
        const uint8_t* src = data.pos();
        SIMD* simd = SIMD::instance();

        map.setLine( map.height() - 1 );

        switch ( img.channels() ) {
            case 1:
                if( data.remainingBytes() < ( width * height * sizeof( float ) ) )
                    throw CVTException( "NOT ENOUGH DATA REMAINING" );
                while ( height-- ) {
                    simd->BSwap32( ( uint32_t* ) map.ptr(), ( const uint32_t* ) src, width );
                    map--;
                    src += width * sizeof( float );
                }
                break;
            case 4:
                {
                    if( data.remainingBytes() < ( width * height * 3 * sizeof( float ) ) )
                        throw CVTException( "NOT ENOUGH DATA REMAINING" );

                    ScopedBuffer<float,true> buf( width * 3 );

                    while ( height-- ) {
                        simd->BSwap32( ( uint32_t* ) buf.ptr(), ( const uint32_t* ) src, width * 3 );
                        simd->Conv_XXXf_to_XXXAf( map.ptr(), ( const float* ) buf.ptr(), width );
                        map--;
                        src += width * 3 * sizeof( float );
                    }
                }
                break;
            default:
                throw CVTException( "FORMAT NOT SUPPORTED" );
                break;
        }
    }
}

static void _init( cvt::PluginManager* pm )
{
	cvt::ILoader* pfm = new cvt::PFMLoader();
	pm->registerPlugin( pfm );
}

CVT_PLUGIN( _init )
