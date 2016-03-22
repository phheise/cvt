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

#include <cvt/cl/CLCommandQueue.h>
#include <cvt/cl/CLContext.h>
#include <cvt/cl/CLDevice.h>

namespace cvt {
	CLCommandQueue::CLCommandQueue( cl_command_queue q ) : CLObject<cl_command_queue>( q )
	{
	}

	CLCommandQueue::CLCommandQueue( const CLCommandQueue& q ) : CLObject<cl_command_queue>( q._object )
	{
	}

	CLCommandQueue::CLCommandQueue( const CLContext& context, const CLDevice& device, cl_command_queue_properties props )
	{
		cl_int err;
		_object = ::clCreateCommandQueue( context, device, props, &err );
		if( err != CL_SUCCESS )
			throw CLException( err );
	}

	/**
	  The context associated with the command-queue.
	 */
	CLContext CLCommandQueue::context() const
	{
		return CLContext( _context() );
	}

	/**
	  The device associated with the command-queue.
	 */
	CLDevice CLCommandQueue::device() const
	{
		return CLDevice( _device() );
	}

}
