/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2013, Philipp Heise and Sebastian Klose
   Copyright (c) 2014, Brian Jensen <Jensen.J.Brian@gmail.com>

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

#include <cvt/util/CPU.h>
#include <cvt/util/Exception.h>
#include <cvt/util/ScopedBuffer.h>
#include <cvt/util/Util.h>
#include <cvt/gfx/Image.h>
#include <cvt/gfx/IMapScoped.h>
#include <cvt/gfx/ISaver.h>
#include <cvt/util/PluginManager.h>
#include <cvt/util/String.h>

#include <fstream>
#include <stdint.h>

namespace cvt
{
    struct AlphaRemover
    {
        typedef void ( SIMD::* ConversionFunction )( float*, const float*, size_t ) const;

        AlphaRemover( const Image& img )
        {
            int channels;
            if( img.format( ) == IFormat::BGRA_FLOAT || img.format( ) == IFormat::RGBA_FLOAT ) {
                _convFunc = &SIMD::Conv_XXXAf_to_XXXf;
                channels = 3;
            } else if( img.format( ) == IFormat::GRAYALPHA_FLOAT ) {
                _convFunc = &SIMD::Conv_GRAYALPHAf_to_GRAYf;
                channels = 1;
            } else
                throw CVTException( "Image has no alpha channel" );


            size_t len = img.width( ) * channels;
            _buf = new ScopedBuffer<float, true>( len );
            _elements = img.width( );
        }

        ~AlphaRemover( )
        {
            delete _buf;
        }

        float* stripLine( const float* src )
        {
            static SIMD* simd = SIMD::instance( );
            ( simd->*_convFunc )( _buf->ptr( ), src, _elements );
            return _buf->ptr( );
        }

        size_t bytes( ) const
        {
            //std::cout << "Buffer size " << _buf->size() << std::endl;
            //std::cout << _buf->size() * sizeof( float ) << std::endl;
            return _buf->size( ) * sizeof( float );
        }

        size_t elements( ) const
        {
            return _elements;
        }

        private:
            size_t _elements;
            ScopedBuffer<float, true>* _buf;
            ConversionFunction _convFunc;

    };

    /**
     * Writes an image in PFM format. See http://netpbm.sourceforge.net/doc/pfm.html for more info.
     *
     * @brief The PFMSaver image output class
     */
    class PFMSaver : public ISaver
    {
        public:
            PFMSaver( ) {}
            const String& extension( size_t ) const
            {
                static String extension = ".pfm";
                return extension;
            }

            size_t sizeExtensions( ) const { return 1; }

            const String& name( ) const
            {
                static String name = "PFMSaver";
                return name;
            }

            void save( const String& path, const Image& img )
            {
                std::ofstream file( path.c_str( ),
                    std::fstream::out |
                    std::fstream::binary |
                    std::fstream::trunc );

                if( !file.is_open( ) )
                    throw CVTException( "Could not open file for output" );

                // This is a floating point format, convert image if necessary appropriately
                const Image* floatImage = NULL;
                AlphaRemover* alphaRemover = NULL;

                if( img.format( ) != IFormat::GRAY_FLOAT ) {
                    Image* tmp = new Image( img.width( ), img.height( ), IFormat::floatEquivalent( img.format( ) ) );

                    //Check if converted image has an alpha channel
                    if( tmp->format( ) != IFormat::GRAY_FLOAT )
                        alphaRemover = new AlphaRemover( *tmp );

                    img.convert( *tmp );
                    floatImage = tmp;
                } else {
                    floatImage = &img;
                }

                // First write the 3 line header starting with the magic
                if( floatImage->channels( ) == 4 )
                    file << "PF" << std::endl;
                else
                    file << "Pf" << std::endl;

                //dimensions
                std::ostringstream oss;
                oss << img.width( ) << " " << img.height( );
                file << oss.str( ) << std::endl;

                //endianness and scale
                if( isLittleEndian( ) )
                    file << "-";
                file << "1.0" << std::endl;

                // Now write out the image, rasterized from left to right, bottom to top
                {
                    IMapScoped<const float> map( *floatImage );
                    for( int row = floatImage->height( ) - 1; row >= 0; row-- ) {
                        const float* rowPtr = map.line( row );
                        size_t count = 0;

                        if( alphaRemover ) {
                            rowPtr = alphaRemover->stripLine( rowPtr );
                            count = alphaRemover->bytes( );
                        } else {
                            count = floatImage->width( ) * floatImage->channels( ) * sizeof( float );
                        }

                        file.write( reinterpret_cast<const char*>( rowPtr ),  count );
                    }
                }

                // clean up
                if( floatImage != &img )
                    delete floatImage;
                if( alphaRemover )
                    delete alphaRemover;

                file.close( );
            }

    };
}

static void _init( cvt::PluginManager* pm )
{
    cvt::ISaver* pfmsaver = new cvt::PFMSaver;
    pm->registerPlugin( pfmsaver );
}

CVT_PLUGIN( _init )
