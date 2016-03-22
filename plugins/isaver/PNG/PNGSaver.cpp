#include <cvt/util/PluginManager.h>
#include <cvt/gfx/Image.h>
#include <cvt/util/Exception.h>

#include <png.h>


namespace cvt {
	class PNGSaver : public ISaver
	{
		public:
			PNGSaver() {}
			void save( const String& file, const Image& img );
			const String& extension( size_t n ) const { return _extension[ n ]; }
			size_t sizeExtensions() const { return 2; }
			const String& name() const { return _name; }

		private:
			static String _name;
			static String _extension[];
	};

	String PNGSaver::_name = "PNG";
	String PNGSaver::_extension[] = { ".png", ".PNG" };

	void PNGSaver::save( const String& path, const Image& img )
    {
        FILE *fp;
        fp = fopen(path.c_str(), "wb");
        if (fp == NULL){
            throw CVTException("Could not create file ...");
        }

        png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
        if (png_ptr == NULL) {
            fclose(fp);
            throw CVTException("Could not create png_structp");
        }

        /* Allocate/initialize the image information data.  REQUIRED */
        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr == NULL){
            fclose(fp);
            png_destroy_write_struct(&png_ptr,  ( png_infopp )NULL);
            throw CVTException("Could not create png info struct");
        }

        // convert image to UINT8 equivalent!
        const Image* tmpImage = NULL;
        if( img.format().type != IFORMAT_TYPE_UINT8 && img.format().type != IFORMAT_TYPE_UINT16  ) {
            Image* itmp = new Image( img.width(), img.height(), IFormat::uint8Equivalent( img.format() ) );
            img.convert( *itmp );
            tmpImage = itmp;
        } else {
            tmpImage = &img;
        }

        /* Set error handling.  REQUIRED if you aren't supplying your own
         * error handling functions in the png_create_write_struct() call.
         */
        if (setjmp(png_jmpbuf(png_ptr))){
            fclose(fp);
            png_destroy_write_struct(&png_ptr, &info_ptr);
            throw CVTException("Error while writing png");
        }

        /* Set up the output control if you are using standard C streams */
        png_init_io( png_ptr, fp );



        int channels;
        switch( tmpImage->format().formatID ) {
            case IFORMAT_GRAY_UINT8:
            case IFORMAT_GRAY_UINT16:
                channels = PNG_COLOR_TYPE_GRAY;
                break;
            case IFORMAT_GRAYALPHA_UINT8:
            case IFORMAT_GRAYALPHA_UINT16:
                channels = PNG_COLOR_TYPE_GA;
                break;
            case IFORMAT_RGBA_UINT8:
            case IFORMAT_RGBA_UINT16:
                channels = PNG_COLOR_TYPE_RGBA;
                break;
            case IFORMAT_BGRA_UINT8:
            case IFORMAT_BGRA_UINT16:
                channels = PNG_COLOR_TYPE_RGBA;
                png_set_bgr( png_ptr );
                break;
            default:
                throw CVTException("Input channel format not supported for writing");
                break;
        }

        png_set_IHDR(png_ptr, info_ptr,
                     tmpImage->width(),
                     tmpImage->height(),
                     tmpImage->bpc() * 8,
                     channels,
                     PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE,
                     PNG_FILTER_TYPE_BASE );

        /*
         *	Optional gamma chunk is strongly suggested if you have any guess
         *	as to the correct gamma of the image.
         */
        //double gamma = 2.2;
        //png_set_gAMA(png_ptr, info_ptr, gamma);   

        /* Write the file header information.  REQUIRED */
        png_write_info( png_ptr, info_ptr );

        /* WARNING: this must be after png_write_info - otherwise it is somehow ignored ?!*/
        if( tmpImage->format().type == IFORMAT_TYPE_UINT16 ) {
            png_set_swap( png_ptr );
        }

        png_bytep row_pointers[ tmpImage->height() ];

        if (tmpImage->height() > PNG_UINT_32_MAX/sizeof(png_bytep)){
            throw CVTException("Image is too tall to process in memory");
        }

        size_t stride;
        const uint8_t* base = tmpImage->map( &stride );
        for(size_t k = 0; k < tmpImage->height(); k++)
            row_pointers[ k ] = ( png_bytep ) ( base + stride * k );


        png_write_image( png_ptr, row_pointers );

        /* It is REQUIRED to call this to finish writing the rest of the file */
        png_write_end( png_ptr, info_ptr );

        tmpImage->unmap( base );

        /* Clean up after the write, and free any memory allocated */
        png_destroy_write_struct( &png_ptr, &info_ptr );

        /* Close the file */
        fclose( fp );

        if( tmpImage != &img )
            delete tmpImage;
    }
}

static void _init( cvt::PluginManager* pm )
{
	cvt::ISaver* png = new cvt::PNGSaver();
	pm->registerPlugin( png );
}

CVT_PLUGIN( _init )
