/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2013, Philipp Heise and Sebastian Klose
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
#ifndef CVT_CLIAMGE2D_H
#define CVT_CLIAMGE2D_H

#include <cvt/cl/CLMemory.h>
#include <cvt/cl/CLUtil.h>
#include <cvt/cl/CLException.h>
#include <cvt/cl/CLImageFormat.h>

#include <cvt/gfx/Image.h>

namespace cvt {
    class CLContext;

    /**
      \ingroup CL
     */
    class CLImage2D : public CLMemory
    {
        public:
            CLImage2D( const CLContext& context, size_t width, size_t height, const CLImageFormat& format,
                      cl_mem_flags flags = CL_MEM_READ_WRITE, size_t stride = 0, void* host_ptr = NULL );
            CLImage2D( size_t width, size_t height, const CLImageFormat& format, cl_mem_flags flags = CL_MEM_READ_WRITE );

            CLImage2D( const CLImage2D& img, const Recti* r = NULL, cl_mem_flags flags = CL_MEM_READ_WRITE );

            //CLImage2D( const Image& img );

            void*       map( size_t* stride );
            const void* map( size_t* stride ) const;
            void        unmap( const void* ptr ) const;

            // Missing
            // - CL_IMAGE_ROW_PITCH  gives stride in bytes
            // - CL_IMAGE_WIDTH  width
            // - CL_IMAGE_HEIGHT height
            // - CL_IMAGE_FORMAT CLImageFormat
            // - CL_IMAGE_ELEMENT_SIZE bpp
        private:

            size_t _width;
            size_t _height;
            CLImageFormat _format;
    };

}
#endif
