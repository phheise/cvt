/*
   The MIT License (MIT)

   Copyright (c) 2015, Brian Jensen
   Copyright (c) 2016, BMW Car IT GmbH, Philipp Heise (philipp.heise@bmw.de)

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
#include <cvt/util/PluginManager.h>
#include <cvt/gfx/Image.h>
#include <cvt/util/Exception.h>
#include <cvt/gfx/IMapScoped.h>
#include <cvt/util/ScopedBuffer.h>

#include <jpeglib.h>

namespace cvt {
    class JPEGLoader : public ILoader
    {
        public:
            JPEGLoader() {}
            void load( Image& dst, const String& file );
            const String& extension( size_t n ) const { return _extension[ n ]; }
            size_t sizeExtensions() const { return 4; }
            const String& name() const { return _name; }

        private:
            static String _name;
            static String _extension[ 4 ];
    };

    String JPEGLoader::_name = "JPEG";
    String JPEGLoader::_extension[] = { ".jpeg", ".JPEG", ".jpg" , ".JPG" };

    void JPEGLoader::load( Image& img, const String& path )
    {

        FILE *fp = fopen( path.c_str(), "rb");
        if (!fp){
            String msg;
            msg.sprintf( "Cannot open JPEG file: %s", path.c_str() );
            throw CVTException( msg.c_str() );
        }

        struct jpeg_decompress_struct cinfo;
        struct jpeg_error_mgr jerr;

        jpeg_create_decompress( &cinfo );
        cinfo.err = jpeg_std_error( &jerr );

        jpeg_stdio_src( &cinfo, fp );
        jpeg_read_header( &cinfo, TRUE );

        if ( cinfo.num_components == 1 ) {
            img.reallocate( cinfo.image_width, cinfo.image_height, IFormat::GRAY_UINT8, img.memType() );
            cinfo.out_color_space = JCS_GRAYSCALE;
            jpeg_start_decompress( &cinfo );
            size_t yend = cinfo.image_height;
            IMapScoped<uint8_t> map( img );
            for( size_t y = 0; y < yend; y++ ) {
                uint8_t* ptr = map.ptr();
                jpeg_read_scanlines( &cinfo, ( JSAMPARRAY ) &ptr, 1 );
                map++;
            }
        } else if( cinfo.num_components == 3 ) {
            img.reallocate( cinfo.image_width, cinfo.image_height, IFormat::RGBA_UINT8, img.memType() );
            cinfo.out_color_space = JCS_RGB;
            jpeg_start_decompress( &cinfo );
            ScopedBuffer<uint8_t, true> buffer( sizeof( uint8_t ) * 3 * cinfo.image_width );
            size_t yend = cinfo.image_height;
            IMapScoped<uint8_t> map( img );
            SIMD* simd = SIMD::instance();
            for( size_t y = 0; y < yend; y++ ) {
                uint8_t* ptr = buffer.ptr();
                jpeg_read_scanlines( &cinfo, ( JSAMPARRAY ) &ptr, 1 );
                simd->Conv_XXXu8_to_XXXAu8( map.ptr(), buffer.ptr(), cinfo.image_width );
                map++;
            }
        } else {
            throw CVTException("Unsupported JPEG format");
        }

        jpeg_finish_decompress( &cinfo );
        jpeg_destroy_decompress( &cinfo );

        fclose( fp );
    }

}

static void _init( cvt::PluginManager* pm )
{
    cvt::ILoader* jpeg = new cvt::JPEGLoader();
    pm->registerPlugin( jpeg );
}

CVT_PLUGIN( _init )

