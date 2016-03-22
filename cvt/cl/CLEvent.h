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

#ifndef CVT_CLEVENT_H
#define CVT_CLEVENT_H

#include <cvt/cl/OpenCL.h>
#include <cvt/cl/CLObject.h>
#include <cvt/cl/CLUtil.h>

namespace cvt {
	class CLCommandQueue;
	class CLContext;

	/**
	  \ingroup CL
	 */
	class CLEvent : public CLObject<cl_event>
	{
		public:
			CLEvent( cl_event e = NULL ) : CLObject<cl_event>( e ) {};
			CLEvent( const CLEvent& e ) : CLObject<cl_event>( e._object ) {};

			CLUTIL_GETINFOTYPE( type, CL_EVENT_COMMAND_TYPE, cl_command_type, _object, ::clGetEventInfo )
			CLUTIL_GETINFOTYPE( status, CL_EVENT_COMMAND_EXECUTION_STATUS, cl_int, _object, ::clGetEventInfo )

			void wait() const;

			static void waitEvents( const std::vector<CLEvent>& waitevents );

			CLCommandQueue queue() const;
			CLContext context() const;

		private:
			CLUTIL_GETINFOTYPE( _queue, CL_EVENT_COMMAND_QUEUE, cl_command_queue, _object, ::clGetEventInfo )
			// OpenCL 1.1
			// CLUTIL_GETINFOTYPE( _context, CL_EVENT_CONTEXT, cl_context, _object, ::clGetEventInfo )
	};

	inline void CLEvent::wait() const
	{
		cl_int err;
		err = ::clWaitForEvents( 1, ( cl_event* ) this );
		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
	}

	inline void CLEvent::waitEvents( const std::vector<CLEvent>& waitevents )
	{
		cl_int err;
		err = ::clWaitForEvents( waitevents.size(), ( cl_event* )  ( const cl_event* ) &waitevents[0] );
		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
	}
}

#endif
