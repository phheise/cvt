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

#ifndef CVT_CLDEVICE_H
#define CVT_CLDEVICE_H

#include <cvt/cl/CLException.h>
#include <cvt/cl/CLUtil.h>
#include <cvt/cl/CLNDRange.h>
#include <cvt/util/String.h>
#include <vector>

namespace cvt {
	class CLPlatform;

	/**
	  \ingroup CL
	 */
	class CLDevice
	{
		friend std::ostream& operator<<( std::ostream& out, const CLDevice& cldev );
		public:
			CLDevice( cl_device_id id = NULL );
			~CLDevice();

			CLUTIL_GETINFOSTRING( name, CL_DEVICE_NAME, _id, ::clGetDeviceInfo  )
			CLUTIL_GETINFOSTRING( vendor, CL_DEVICE_VENDOR, _id, ::clGetDeviceInfo  )
			CLUTIL_GETINFOSTRING( driverVersion, CL_DRIVER_VERSION, _id, ::clGetDeviceInfo  )
			CLUTIL_GETINFOSTRING( profile, CL_DEVICE_PROFILE, _id, ::clGetDeviceInfo  )
			CLUTIL_GETINFOSTRING( version, CL_DEVICE_VERSION, _id, ::clGetDeviceInfo  )

			CLUTIL_GETINFOTYPE( deviceType, CL_DEVICE_TYPE, cl_device_type, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( vendorID, CL_DEVICE_VENDOR_ID, cl_uint, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( computeUnits, CL_DEVICE_MAX_COMPUTE_UNITS, cl_uint, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( maxWorkItemDimensions, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, cl_uint, _id, ::clGetDeviceInfo );
			//FIXME
			//CLUTIL_GETINFOTYPE( maxWorkItemSizes, CL_DEVICE_MAX_WORK_ITEM_SIZES, size_t[ 3 ], _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( maxWorkGroupSize, CL_DEVICE_MAX_WORK_GROUP_SIZE, size_t, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( preferredVectorWidthChar, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, cl_uint, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( preferredVectorWidthShort, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, cl_uint, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( preferredVectorWidthInt, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, cl_uint, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( preferredVectorWidthLong, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, cl_uint, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( preferredVectorWidthFloat, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, cl_uint, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( preferredVectorWidthDouble, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, cl_uint, _id, ::clGetDeviceInfo );

			CLUTIL_GETINFOTYPE( maxClockFrequency, CL_DEVICE_MAX_CLOCK_FREQUENCY, cl_uint, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( addressBits, CL_DEVICE_ADDRESS_BITS, cl_uint, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( maxMemAllocSize, CL_DEVICE_MAX_MEM_ALLOC_SIZE, cl_ulong, _id, ::clGetDeviceInfo );

			CLUTIL_GETINFOTYPE( imageSupport, CL_DEVICE_IMAGE_SUPPORT, cl_bool, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( maxReadImageArgs, CL_DEVICE_MAX_READ_IMAGE_ARGS, cl_uint, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( maxWriteImageArgs, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, cl_uint, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( image2dMaxWidth, CL_DEVICE_IMAGE2D_MAX_WIDTH, size_t, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( image2dMaxHeight, CL_DEVICE_IMAGE2D_MAX_HEIGHT, size_t, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( image3dMaxWidth, CL_DEVICE_IMAGE3D_MAX_WIDTH, size_t, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( image3dMaxHeight, CL_DEVICE_IMAGE3D_MAX_HEIGHT, size_t, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( image3dMaxDepth, CL_DEVICE_IMAGE3D_MAX_DEPTH, size_t, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( maxSamplers, CL_DEVICE_MAX_SAMPLERS, cl_uint, _id, ::clGetDeviceInfo );

			CLUTIL_GETINFOTYPE( maxParameterSize, CL_DEVICE_MAX_PARAMETER_SIZE, size_t, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( memBaseAddrAlign, CL_DEVICE_MEM_BASE_ADDR_ALIGN, cl_uint, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( minDataTypeAlignSize, CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, cl_uint, _id, ::clGetDeviceInfo );

			CLUTIL_GETINFOTYPE( singleFPConfig, CL_DEVICE_SINGLE_FP_CONFIG, cl_device_fp_config, _id, ::clGetDeviceInfo );

			CLUTIL_GETINFOTYPE( globalMemCacheType, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, cl_device_mem_cache_type, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( globalMemCachelineSize, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, cl_uint, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( globalMemCacheSize, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, cl_ulong, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( globalMemSize, CL_DEVICE_GLOBAL_MEM_SIZE, cl_ulong, _id, ::clGetDeviceInfo );

			CLUTIL_GETINFOTYPE( maxConstantBufferSize, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, cl_ulong, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( maxConstantArgs, CL_DEVICE_MAX_CONSTANT_ARGS, cl_uint, _id, ::clGetDeviceInfo );

			CLUTIL_GETINFOTYPE( localMemType, CL_DEVICE_LOCAL_MEM_TYPE, cl_device_local_mem_type, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( localMemSize, CL_DEVICE_LOCAL_MEM_SIZE, cl_ulong, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( errorCorrectionSupport, CL_DEVICE_ERROR_CORRECTION_SUPPORT, cl_bool, _id, ::clGetDeviceInfo );

			CLUTIL_GETINFOTYPE( profilingTimerResolution, CL_DEVICE_PROFILING_TIMER_RESOLUTION, size_t, _id, ::clGetDeviceInfo );

			CLUTIL_GETINFOTYPE( endianLittle, CL_DEVICE_ENDIAN_LITTLE, cl_bool, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOTYPE( deviceAvailable, CL_DEVICE_AVAILABLE, cl_bool, _id, ::clGetDeviceInfo );

			CLUTIL_GETINFOTYPE( compilerAvailable, CL_DEVICE_COMPILER_AVAILABLE, cl_bool, _id, ::clGetDeviceInfo );

			CLUTIL_GETINFOTYPE( executionCapabilities, CL_DEVICE_EXECUTION_CAPABILITIES, cl_device_exec_capabilities, _id, ::clGetDeviceInfo );

			CLUTIL_GETINFOTYPE( queueProperties, CL_DEVICE_QUEUE_PROPERTIES, cl_command_queue_properties, _id, ::clGetDeviceInfo );

			CLNDRange maxWorkItemSizes() const;

			CLPlatform platform() const;
			void extensions( std::vector<String>& extensions ) const;

			operator cl_device_id () const { return _id; }

		private:
			CLUTIL_GETINFOTYPE( _devicePlatform, CL_DEVICE_PLATFORM, cl_platform_id, _id, ::clGetDeviceInfo );
			CLUTIL_GETINFOSTRING( _extensions, CL_DEVICE_EXTENSIONS, _id, ::clGetDeviceInfo  )

			cl_device_id _id;
	};

	inline CLDevice::CLDevice( cl_device_id id ) : _id( id )
	{
	}

	inline CLDevice::~CLDevice()
	{
	}

	inline CLNDRange CLDevice::maxWorkItemSizes() const
	{
		cl_int err;
		size_t sizes[ 3 ];
		err = ::clGetDeviceInfo( _id, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof( size_t ) * 3, sizes, NULL );
		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
		return CLNDRange( sizes[ 0 ], sizes[ 1 ], sizes[ 2 ] );
	}

	inline std::ostream& operator<<( std::ostream& out, const CLDevice& cldev )
	{
		String str;
		cldev.vendor( str );
		out << "CLDevice\n\tVendor: " << str;
		cldev.name( str );
		out << "\n\tName: " << str;
		cldev.profile( str );
		out << "\n\tProfile: " << str;
		cldev.version( str );
		out << "\n\tDevice-Version: " << str;
		cldev.driverVersion( str );
		out << "\n\tDriver-Version: " << str;
		cldev._extensions( str );
		out << "\n\tExtensions: " << str;

		cl_bool imagesupport;
		imagesupport = cldev.imageSupport();
		out << "\n\tImage-support: " << ( imagesupport==CL_TRUE?"YES":"NO" );
		if( imagesupport ) {
			size_t maxw, maxh;
			maxw = cldev.image2dMaxWidth();
			out << "\n\tMax. image2d width: " << maxw;
			maxh = cldev.image2dMaxHeight();
			out << "\n\tMax. image2d height: " << maxh;
			cl_uint maxsamplers = cldev.maxSamplers();
			out << "\n\tMax. samplers: " << maxsamplers;
		}

		return out;
	}

}

#endif
