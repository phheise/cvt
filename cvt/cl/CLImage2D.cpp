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

#include <cvt/cl/CLContext.h>
#include <cvt/cl/CLImage2D.h>

namespace cvt {
	/**
	  Create CLImage2D object with CLContext
	 */
	CLImage2D::CLImage2D( const CLContext& context, size_t width, size_t height, const CLImageFormat& format,
						  cl_mem_flags flags, size_t stride, void* host_ptr)
		: _width( width ), _height( height ), _format( format )
	{
		cl_int err;
		_object = ::clCreateImage2D( context, flags, ( cl_image_format* ) &format, width, height, stride, host_ptr, &err );
		if( err != CL_SUCCESS )
			throw CLException( err );
	}


	/**
	  Use default context to create CLImage2D object
	  \param width the width of the image
	  \param height the height of the image
	  \param format the CLImageFormat to be used for the image
	  \param format additional OpenCL memory flags to be used for creating the image
	 */
	CLImage2D::CLImage2D( size_t width, size_t height, const CLImageFormat& format, cl_mem_flags flags )
		: _width( width ), _height( height ), _format( format )
	{
		cl_int err;
		_object = ::clCreateImage2D( *CL::defaultContext(), flags, ( cl_image_format* ) &_format, _width, _height, 0, NULL, &err );
		if( err != CL_SUCCESS )
			throw CLException( err );
	}


	/**
		Use default context to create CLImage2D object with data copied from img.
		\param img the CLImage2D to be copied
		\param r   optional roi from img
	 */
	CLImage2D::CLImage2D( const CLImage2D& img, const Recti* r, cl_mem_flags flags )
		: CLMemory(), _width( img._width ), _height( img._height ), _format( img._format )
	{
		cl_int err;
		_object = ::clCreateImage2D( *CL::defaultContext(), flags, ( cl_image_format* ) &_format, _width, _height, 0, NULL, &err );
		if( err != CL_SUCCESS )
			throw CLException( err );

		Recti rect( 0, 0, ( int ) _width, ( int ) _height );
		if( r )
			rect.intersect( *r );

		CL::defaultQueue()->enqueueCopyImage( *this, img, 0, 0, rect.x, rect.y, rect.width, rect.height, NULL, NULL );
	}

	/**
	  Use default context to create CLImage2D from Image
	 */
/*	CLImage2D::CLImage2D( const Image& img )
		: _width( width ), _height( height ), _format( format )
	{
		//TODO
	}*/

	/**
	  Map the CLImage2D object memory in the host address-space for reading/writing using the default command-queue.
	  \param stride the stride of the mapped image.
	  \return pointer to the mapped memory.
	  */
	void* CLImage2D::map( size_t* stride )
	{
		//std::cout << "R/W map: " << mapCount() << std::endl;
		 return CL::defaultQueue()->enqueueMapImage( *this, CL_MAP_READ | CL_MAP_WRITE, 0, 0, _width, _height, stride );
	}

	/**
	  Map the CLImage2D object memory in the host address-space for reading using the default command-queue.
	  \param stride the stride of the mapped image.
	  \return pointer to the mapped memory.
	  */
	const void* CLImage2D::map( size_t* stride ) const
	{
		//std::cout << "R map: " << mapCount() << std::endl;
		 return ( const void* ) CL::defaultQueue()->enqueueMapImage( *this, CL_MAP_READ, 0, 0, _width, _height, stride );
	}

	/**
	  Unmap the previously mapped CLImage2D object memory using the default command-queue.
	  \param ptr the previously mapped data-pointer.
	  */
	void CLImage2D::unmap( const void* ptr ) const
	{
		CL::defaultQueue()->enqueueUnmap( *( ( CLMemory* )this ), ptr );
	}
}
