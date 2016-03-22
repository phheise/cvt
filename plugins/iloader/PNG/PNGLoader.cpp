#include <cvt/util/PluginManager.h>
#include <cvt/gfx/Image.h>
#include <cvt/util/Exception.h>

#include <png.h>

namespace cvt {
	class PNGLoader : public ILoader
	{
		public:
			PNGLoader() {}
			void load( Image& dst, const String& file );
			const String& extension( size_t n ) const { return _extension[ n ]; }
			size_t sizeExtensions() const { return 2; }
			const String& name() const { return _name; }

		private:
			static String _name;
			static String _extension[ 2 ];
	};

	String PNGLoader::_name = "PNG";
	String PNGLoader::_extension[] = { ".png", ".PNG" };

	void PNGLoader::load( Image& img, const String& path )
	{
		size_t rdlen;
		FILE *fp = fopen( path.c_str(), "rb");
		if (!fp){
			String msg;
			msg.sprintf( "Cannot open PNG file: %s", path.c_str() );
			throw CVTException( msg.c_str() );
		}

		unsigned char header[8];
		png_uint_32 width, height;
		int bit_depth, color_type, interlace_type;

		rdlen = fread( header, 1, 8, fp);
		if( rdlen != 8 )
			throw CVTException( "Invalid PNG image header" );

		if( png_sig_cmp(header, 0, 8) != 0 )
			throw CVTException( "Invalid PNG image header" );

		png_structp png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, 0, 0, 0 );

		png_infop info_ptr = png_create_info_struct( png_ptr );

		if( setjmp( png_jmpbuf( png_ptr ) ) ) {
			fclose( fp );
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
			png_destroy_info_struct( png_ptr, (png_infopp) &info_ptr);
			throw CVTException( "IO error during PNG loading" );
		}
		png_init_io(png_ptr, fp);
		png_set_sig_bytes(png_ptr, 8);

		png_read_info(png_ptr, info_ptr);

		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
					 &interlace_type, NULL, NULL);

		/* tell libpng to strip 16 bit/color files down to 8 bits/color */
		//png_set_strip_16(png_ptr);

		/* expand paletted or RGB images with transparency to full alpha channels
		 * so the data will be available as RGBA quartets */
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
			png_set_expand(png_ptr);

		if (color_type == PNG_COLOR_TYPE_PALETTE)
			png_set_expand(png_ptr);

        if( bit_depth > 8 /* && LITTLE_ENDIAN */)
            png_set_swap( png_ptr );

		switch (color_type) {
			case PNG_COLOR_TYPE_GRAY:
				if( bit_depth <= 8 ) {
					if( bit_depth < 8 )
						png_set_expand( png_ptr );
					img.reallocate( width, height, IFormat::GRAY_UINT8 );
				} else if( bit_depth == 16 ) {
					img.reallocate( width, height, IFormat::GRAY_UINT16 );
				} else
					throw CVTException("Unsupported PNG format");
				break;
			case PNG_COLOR_TYPE_GRAY_ALPHA:
				if( bit_depth == 16 ) {
					img.reallocate( width, height, IFormat::GRAYALPHA_UINT16 );
				} else if( bit_depth == 8 )
					img.reallocate( width, height, IFormat::GRAYALPHA_UINT8 );
				else
					throw CVTException("Unsupported PNG format");
				break;
			case PNG_COLOR_TYPE_RGB:
			case PNG_COLOR_TYPE_PALETTE:
				/* expand paletted colors into true RGB triplets */
				png_set_add_alpha( png_ptr, 0xff, PNG_FILLER_AFTER );
			case PNG_COLOR_TYPE_RGBA:
				//png_set_bgr(png_ptr);
				if( bit_depth <= 8 ) {
                    if(bit_depth < 8)
						png_set_expand( png_ptr );
				    img.reallocate( width, height, IFormat::RGBA_UINT8 );
                } else if( bit_depth == 16 ) {
					img.reallocate( width, height, IFormat::RGBA_UINT16 );
				} else
					throw CVTException("Unsupported PNG format");
				break;
			default:
				std::cout << color_type << std::endl;
				throw CVTException("Unsupported PNG format");
				break;
		}


		png_bytepp row_pointers = new png_bytep[ height ];
		size_t stride;
		uint8_t* base = img.map( &stride );
		for (unsigned y = 0; y < height; y++)
			row_pointers[y] = base + y * stride;

		png_read_image( png_ptr, row_pointers );
		png_read_end( png_ptr, info_ptr );

		delete[] row_pointers;
		img.unmap( base );

		fclose( fp );
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
		png_destroy_info_struct( png_ptr, (png_infopp) &info_ptr);
	}
}

static void _init( cvt::PluginManager* pm )
{
	cvt::ILoader* png = new cvt::PNGLoader();
	pm->registerPlugin( png );
}

CVT_PLUGIN( _init )
