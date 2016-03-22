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

#include <cvt/vision/Patch.h>

namespace cvt {
    
    Patch::Patch( const Image & img, const Recti & rect, PatchFlags flags ) : 
        _computeFlags( flags ),
        _patchData( rect.width, rect.height, img.format() )
    {
        this->update( img, rect );
    }
    
    Patch::~Patch()
    {
    }
    
    void Patch::update( const Image & img, const Recti & rect )
    {
        _patchData.copyRect( 0, 0, img, rect );
        
        if( _computeFlags & NORMALIZE ){
            calcMeanAndVariance();
        }
    }
    
    float Patch::mean() const
    {
        return _mean;
    }
    
    float Patch::variance() const
    {
        return _var;
    }
    
    void Patch::calcMeanAndVariance()
    {
        size_t stride;
        switch ( _patchData.format().type ) {
            case IFORMAT_TYPE_FLOAT:
            {
                float * ptr = _patchData.map<float>( &stride );
                this->meanAndVariance<float>( ptr, stride );
                _patchData.unmap( ptr );
            }
                break;
            case IFORMAT_TYPE_UINT8:
            {
                uint8_t * ptr = _patchData.map<uint8_t>( &stride );
                this->meanAndVariance<uint8_t>( ptr, stride );
                _patchData.unmap( ptr );
            }
                break;
            case IFORMAT_TYPE_UINT16:
            {
                uint16_t * ptr = _patchData.map<uint16_t>( &stride );
                this->meanAndVariance( ptr, stride );
                _patchData.unmap( ptr );
            }
                break;    
            default:
                throw CVTException("MEAN AND STDDEV CALCULATION NOT IMPLEMENTED FOR GIVEN TYPE");
        }
    }
    
    void Patch::dump() const
    {
        switch ( _patchData.format().type ) {
            case IFORMAT_TYPE_UINT8:
            {
                size_t s;
                const uint8_t * ptr = _patchData.map( &s );
                for( size_t y = 0; y < height(); y++ ){
                    for ( size_t x = 0; x < width(); x++) {
                        std::cout <<  (int)ptr[ y * s + x ] << " ";
                    }
                    std::cout << std::endl;
                }
                std::cout << std::endl;
                _patchData.unmap( ptr );
            }
                break;
                
            default:
                throw CVTException( "dump not  implemented for format type" );
                break;
        }
    }
    
}