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
#ifndef CVT_CLMEMORY_H
#define CVT_CLMEMORY_H

#include <cvt/cl/OpenCL.h>
#include <cvt/cl/CLObject.h>
#include <cvt/cl/CLUtil.h>

namespace cvt {

    class CLMemory : public CLObject<cl_mem>
    {
        public:
            CLMemory();
            CLMemory( const cl_mem& other );
            CLMemory( const CLMemory& other );
            ~CLMemory();

            CLUTIL_GETINFOTYPE( memType, CL_MEM_TYPE, cl_mem_object_type, _object, ::clGetMemObjectInfo )
            CLUTIL_GETINFOTYPE( memFlags, CL_MEM_FLAGS, cl_mem_flags, _object, ::clGetMemObjectInfo )
            CLUTIL_GETINFOTYPE( memSize, CL_MEM_SIZE, size_t, _object, ::clGetMemObjectInfo )
            CLUTIL_GETINFOTYPE( hostPtr, CL_MEM_HOST_PTR, void*, _object, ::clGetMemObjectInfo )
            CLUTIL_GETINFOTYPE( mapCount, CL_MEM_MAP_COUNT, cl_uint, _object, ::clGetMemObjectInfo )
            CLUTIL_GETINFOTYPE( refCount, CL_MEM_REFERENCE_COUNT, cl_uint, _object, ::clGetMemObjectInfo )

            static const char* typeToString( cl_mem_object_type type  );
        private:
    };

    inline CLMemory::CLMemory()
    {
    }

    inline CLMemory::CLMemory( const cl_mem& other ) : CLObject<cl_mem>( other )
    {
    }

    inline CLMemory::CLMemory( const CLMemory& other ) : CLObject<cl_mem>( other )
    {
    }

    inline CLMemory::~CLMemory()
    {
    }

    inline const char* CLMemory::typeToString( cl_mem_object_type type  )
    {
        switch( type ) {
            case CL_MEM_OBJECT_BUFFER: return "CL_MEM_OBJECT_BUFFER";
            case CL_MEM_OBJECT_IMAGE2D: return "CL_MEM_OBJECT_IMAGE2D";
            case CL_MEM_OBJECT_IMAGE3D: return "CL_MEM_OBJECT_IMAGE3D";
#ifdef CL_VERSION_1_2
            case CL_MEM_OBJECT_IMAGE1D: return "CL_MEM_OBJECT_IMAGE1D";
            case CL_MEM_OBJECT_IMAGE1D_ARRAY: return "CL_MEM_OBJECT_IMAGE1D_ARRAY";
            case CL_MEM_OBJECT_IMAGE1D_BUFFER: return "CL_MEM_OBJECT_IMAGE1D_BUFFER";
            case CL_MEM_OBJECT_IMAGE2D_ARRAY: return "CL_MEM_OBJECT_IMAGE2D_ARRAY";
#endif
            default: return "Unknown";
        }
    }

}
#endif
