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

#include <cvt/cl/CLBuffer.h>
#include <cvt/cl/CLContext.h>

namespace cvt
{
	CLBuffer::CLBuffer( const CLContext& context, size_t size, cl_mem_flags flags , void* host_ptr ) : _size( size )
	{
		cl_int err;
		_object = ::clCreateBuffer( context, flags, size, host_ptr, &err );
		if( err != CL_SUCCESS )
			throw CLException( err );
	}

	CLBuffer::CLBuffer( size_t size, cl_mem_flags flags ) : _size( size )
	{
		cl_int err;
		_object = ::clCreateBuffer( *CL::defaultContext(), flags, size, NULL, &err );
		if( err != CL_SUCCESS )
			throw CLException( err );
	}

	CLBuffer::CLBuffer( void* data, size_t size, cl_mem_flags flags ) : _size( size )
	{
		cl_int err;
		_object = ::clCreateBuffer( *CL::defaultContext(), flags | CL_MEM_COPY_HOST_PTR, size, data, &err );
		if( err != CL_SUCCESS )
			throw CLException( err );
	}

	CLBuffer::CLBuffer( const CLContext& context, const GLBuffer& buf, cl_mem_flags flags ) : _size( buf.size() )
	{
		cl_int err;
		_object = ::clCreateFromGLBuffer( context, flags, buf.glObject(), &err );
		if( err != CL_SUCCESS )
			throw CLException( err );
	}

	CLBuffer::CLBuffer( const GLBuffer& buf, cl_mem_flags flags ) : _size( buf.size() )
	{
		cl_int err;

		_object = ::clCreateFromGLBuffer( *CL::defaultContext(), flags, buf.glObject(), &err );
		if( err != CL_SUCCESS )
			throw CLException( err );
	}

	CLContext CLBuffer::context() const
	{
		return CLContext( _context() );
	}

	void CLBuffer::read( void* dst )
	{
		 CL::defaultQueue()->enqueueReadBuffer( *this, dst, _size );
	}

	void CLBuffer::write( void* src )
	{
		 CL::defaultQueue()->enqueueWriteBuffer( *this, src, _size );
	}

	/**
	  Map the CLBuffer object memory in the host address-space for reading/writing using the default command-queue.
	  \param stride the stride of the mapped image.
	  \return pointer to the mapped memory.
	  */
	void* CLBuffer::map()
	{
		 return CL::defaultQueue()->enqueueMapBuffer( *this, CL_MAP_READ | CL_MAP_WRITE, _size );
	}

	/**
	  Map the CLBuffer object memory in the host address-space for reading using the default command-queue.
	  \param stride the stride of the mapped image.
	  \return pointer to the mapped memory.
	  */
	const void* CLBuffer::map() const
	{
		 return ( const void* ) CL::defaultQueue()->enqueueMapBuffer( *this, CL_MAP_READ, _size );
	}

	/**
	  Unmap the previously mapped CLBuffer object memory using the default command-queue.
	  \param ptr the previously mapped data-pointer.
	  */
	void CLBuffer::unmap( const void* ptr ) const
	{
		CL::defaultQueue()->enqueueUnmap( *( ( CLMemory* )this ), ptr );
	}

	void CLBuffer::acquireGLObject() const
	{
		CL::defaultQueue()->enqueueAcquireGLObject( *( ( CLMemory* )this ) );
	}

	void CLBuffer::releaseGLObject() const
	{
		CL::defaultQueue()->enqueueReleaseGLObject( *( ( CLMemory* )this ) );
	}
}
