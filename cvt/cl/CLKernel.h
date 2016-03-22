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

#ifndef CVT_CLKERNEL_H
#define CVT_CLKERNEL_H

#include <cvt/cl/CLObject.h>
#include <cvt/cl/CLUtil.h>
#include <cvt/cl/CLException.h>
#include <cvt/cl/CLLocalSpace.h>
#include <cvt/cl/CLProgram.h>
#include <cvt/cl/CLBuffer.h>
#include <cvt/cl/CLImage2D.h>
#include <cvt/cl/CLNDRange.h>
#include <cvt/gfx/Image.h>
#include <cvt/gfx/ImageAllocatorCL.h>
#include <string>
#include <iostream>

namespace cvt {

	class CLKernel : public CLObject<cl_kernel>
	{
		public:
			CLKernel( cl_kernel k = NULL );
			CLKernel( const CLProgram& prog, const char* name );
			CLKernel( const CLProgram& prog, const String& name );
			CLKernel( const String& source, const String& name );

			void setArg( cl_uint index, CLBuffer& arg ) const;
			void setArg( cl_uint index, CLImage2D& arg ) const;
			void setArg( cl_uint index, Image& arg ) const;
			void setArg( cl_uint index, const CLBuffer& arg ) const;
			void setArg( cl_uint index, const CLImage2D& arg ) const;
			void setArg( cl_uint index, const Image& arg ) const;
			template<typename T>
			void setArg( cl_uint index, T arg ) const;
			void setArg( cl_uint index, size_t size, void* arg ) const;

			size_t maxWorkGroupSize() const;
			CLNDRange bestLocalRange1d( const CLNDRange& global ) const;
			CLNDRange bestLocalRange2d( const CLNDRange& global ) const;
			void run( const CLNDRange& global, const CLNDRange& local, const CLNDRange& offset = CLNDRange() ) const;
			void runWait( const CLNDRange& global, const CLNDRange& local, const CLNDRange& offset = CLNDRange() ) const;


			CLUTIL_GETINFOSTRING( functionName, CL_KERNEL_FUNCTION_NAME, _object, ::clGetKernelInfo  )

			CLUTIL_GETINFOTYPE( numArgs, CL_KERNEL_NUM_ARGS, cl_uint, _object, ::clGetKernelInfo );
	};

	inline CLKernel::CLKernel( cl_kernel k ) : CLObject<cl_kernel>( k )
	{
	}

	inline CLKernel::CLKernel( const CLProgram& prog, const char* name )
	{
		cl_int err;
		_object = ::clCreateKernel( prog, name, &err );
		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
	}

	inline CLKernel::CLKernel( const CLProgram& prog, const String& name )
	{
		cl_int err;
		_object = ::clCreateKernel( prog, name.c_str(), &err );
		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
	}


	inline CLKernel::CLKernel( const String& source, const String& name )
	{
		CLProgram prog( *CL::defaultContext(), source );
		if( ! prog.build( *CL::defaultDevice() ,"-cl-denorms-are-zero"/*, "-cl-single-precision-constant -cl-denorms-are-zero -cl-mad-enable -cl-fast-relaxed-math"*/ ) ) {
			String log;
			prog.buildLog( *CL::defaultDevice(), log );
			throw CVTException( log.c_str() );
		}

		cl_int err;
		_object = ::clCreateKernel( prog, name.c_str(), &err );
		if( err != CL_SUCCESS ) {
			throw CLException( __PRETTY_FUNCTION__, err );
		}
	}

	inline size_t CLKernel::maxWorkGroupSize() const
	{
		cl_int err;
		size_t kernmaxwg;

		err = ::clGetKernelWorkGroupInfo( _object, *CL::defaultDevice(), CL_KERNEL_WORK_GROUP_SIZE, sizeof( size_t ), &kernmaxwg, NULL );
		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
		return kernmaxwg;
	}

	inline CLNDRange CLKernel::bestLocalRange1d( const CLNDRange& global ) const
	{
		cl_int err;
		size_t devmaxwg = CL::defaultDevice()->maxWorkGroupSize();
		size_t kernmaxwg;
		CLNDRange ranges;

		ranges = CL::defaultDevice()->maxWorkItemSizes();

		err = ::clGetKernelWorkGroupInfo( _object, *CL::defaultDevice(), CL_KERNEL_WORK_GROUP_SIZE, sizeof( size_t ), &kernmaxwg, NULL );
		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );

		// FUCK - MacBook OpenCL is buggy - returns 1024 for max work item size, although it really is 1 ...
		//std::cout << "DEVICE MAX WG: " << devmaxwg << std::endl;
		//std::cout << "KERNEL MAX WG: " << kernmaxwg << std::endl;
		//std::cout << "MAX WG SIZES: " << ranges << std::endl;
		return Math::gcd<size_t>( *global.range(), Math::min( Math::min( devmaxwg, kernmaxwg ), *ranges.range() ) );
	}

	inline CLNDRange CLKernel::bestLocalRange2d( const CLNDRange& global ) const
	{
		cl_int err;
		size_t devmaxwg = CL::defaultDevice()->maxWorkGroupSize();
		size_t kernmaxwg;
		CLNDRange ranges;

		ranges = CL::defaultDevice()->maxWorkItemSizes();

		err = ::clGetKernelWorkGroupInfo( _object, *CL::defaultDevice(), CL_KERNEL_WORK_GROUP_SIZE, sizeof( size_t ), &kernmaxwg, NULL );
		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );

		size_t wx = Math::gcd<size_t>( global[ 0 ], Math::min( Math::min( devmaxwg, kernmaxwg ), ranges[ 0 ] ) );
		size_t wy = Math::gcd<size_t>( global[ 1 ], Math::min( Math::min( devmaxwg, kernmaxwg ), ranges[ 1 ] ) );

		while( wx * wy > Math::min( devmaxwg, kernmaxwg ) ) {
			if( wx > wy )
				wx = wx >> 1;
			else
				wy = wy >> 1;
		}
		return CLNDRange( wx, wy );
	}

	inline void CLKernel::setArg( cl_uint index, CLBuffer& buf ) const
	{
		cl_int err;
		cl_mem mem = ( cl_mem ) buf;
		err = ::clSetKernelArg( _object, index, sizeof( cl_mem ), &mem );
		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
	}

	inline void CLKernel::setArg( cl_uint index, const CLBuffer& buf ) const
	{
		cl_int err;
		cl_mem mem = ( cl_mem ) buf;
		err = ::clSetKernelArg( _object, index, sizeof( cl_mem ), &mem );
		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
	}

	inline void CLKernel::setArg( cl_uint index, CLImage2D& img ) const
	{
		cl_int err;
		cl_mem mem = ( cl_mem ) img;
		err = ::clSetKernelArg( _object, index, sizeof( cl_mem ), &mem );

		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
	}

	inline void CLKernel::setArg( cl_uint index, const CLImage2D& img ) const
	{
		cl_int err;
		cl_mem mem = ( cl_mem ) img;
		err = ::clSetKernelArg( _object, index, sizeof( cl_mem ), &mem );

		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
	}

	inline void CLKernel::setArg( cl_uint index, Image& img ) const
	{
		ImageAllocatorCL* memcl = dynamic_cast<ImageAllocatorCL*>( img._mem );
		if( !memcl )
			throw CVTException( "Image invalid for OpenCL" );
		setArg( index, *memcl->_climage );
	}

	inline void CLKernel::setArg( cl_uint index, const Image& img ) const
	{
		ImageAllocatorCL* memcl = dynamic_cast<ImageAllocatorCL*>( img._mem );
		if( !memcl )
			throw CVTException( "Image invalid for OpenCL" );
		setArg( index, *memcl->_climage );
	}


	template<typename T>
	inline void CLKernel::setArg( cl_uint index, T arg ) const
	{
		cl_int err;
		err = ::clSetKernelArg( _object, index, sizeof( arg ), &arg );
		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
	}

	template<>
	inline void CLKernel::setArg<CLLocalSpace>( cl_uint index, CLLocalSpace s ) const
	{
		cl_int err;
		err = ::clSetKernelArg( _object, index, s.size, NULL );
		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
	}


	inline void CLKernel::setArg( cl_uint index, size_t size, void* arg ) const
	{
		cl_int err;
		err = ::clSetKernelArg( _object, index, size, arg );
		if( err != CL_SUCCESS )
			throw CLException( err );
	}

	inline std::ostream& operator<<( std::ostream& out, const CLKernel& kern )
	{
		String name;
		kern.functionName( name );
		out << "Kernel:\n\tName: " << name;
		out << "\n\tArgs: " << kern.numArgs();
		out << "\n\tmax. WG-Size: " << kern.maxWorkGroupSize();
		return out;
	}
}

#endif
