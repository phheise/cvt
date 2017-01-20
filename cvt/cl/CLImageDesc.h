/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2014, Philipp Heise and Sebastian Klose
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
#ifndef CVT_CLIMAGEDESC_H
#define CVT_CLIMAGEDESC_H

#include <cvt/cl/OpenCL.h>
#include <iostream>

namespace cvt {
    /**
      \ingroup CL
     */
    class CLImageDesc
    {
        friend std::ostream& operator<<( std::ostream& out, const CLImageDesc& format );
        public:
            CLImageDesc( cl_image_desc desc );
            CLImageDesc( cl_mem_object_type type, size_t width, size_t height, size_t depth,
                         size_t arraySize, size_t rowPitch = 0, size_t slicePitch = 0,
                         cl_uint numMipLevels = 0, cl_uint numSamples = 0, cl_mem buffer = NULL );
            CLImageDesc( const CLImageDesc& desc );

            cl_mem_object_type type() const { return _desc.image_type; }
            size_t             width() const { return _desc.image_width; }
            size_t             height() const { return _desc.image_height; }
            size_t             depth() const { return _desc.image_depth; }
            size_t             arraySize() const { return _desc.image_array_size; }
            size_t             rowPitch() const { return _desc.image_row_pitch; }
            size_t             slicePitch() const { return _desc.image_slice_pitch; }
            cl_uint            numMipLevels() const { return _desc.num_mip_levels; }
            cl_uint            numSamples() const { return _desc.num_samples; }

            operator           cl_image_desc() const { return _desc; }
        private:
            cl_image_desc _desc;
    };

    inline CLImageDesc::CLImageDesc( cl_image_desc desc ) : _desc( desc )
    {
    }

    inline CLImageDesc::CLImageDesc( cl_mem_object_type type, size_t width, size_t height, size_t depth,
                                     size_t arraySize, size_t rowPitch = 0, size_t slicePitch = 0,
                                     cl_uint numMipLevels = 0, cl_uint numSamples = 0, cl_mem buffer = NULL ) :
        _desc.image_type( type),
        _desc.image_width( width ),
        _desc.image_height( height ),
        _desc.image_depth( depth ),
        _desc.image_array_size( arraySize ),
        _desc.image_row_pitch( rowPitch ),
        _desc.image_slice_pitch( slicePitch ),
        _desc.num_mip_levels( numMipLevels ),
        _desc.num_samples( numSamples ),
        _desc.buffer( buffer )
    {
    }

    inline CLImageDesc::CLImageDesc( const CLImageDesc& desc ) : _desc( desc._desc )
    {
    }

    inline std::ostream& operator<<( std::ostream& out, const CLImageDesc& format )
    {
        out << "CLImageDesc:" <<
            << "\n\t width:" << desc.width()
            << "\n\t height:" << desc.height()
            << "\n\t depth:" << desc.depth()
            << "\n\t array size:" << desc.arraySize()
            << "\n\t row pitch:" << desc.rowPitch()
            << "\n\t slice pitch:" << desc.slicePitch()
            << "\n\t num mip levels:" << desc.numMipLevels()
            << "\n\t num samples:" << desc.numSamples();
        return out;
    }
}

#endif
