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

#ifndef CVT_CLCOMMANDQUEUE_H
#define CVT_CLCOMMANDQUEUE_H

#include <cvt/cl/CLObject.h>
#include <cvt/cl/CLUtil.h>
#include <cvt/cl/CLException.h>
#include <cvt/cl/CLBuffer.h>
#include <cvt/cl/CLEvent.h>
#include <cvt/cl/CLKernel.h>
#include <cvt/cl/CLNDRange.h>
#include <vector>

namespace cvt {
	class CLContext;
	class CLDevice;

	/**
	  \ingroup CL
	 */

	class CLCommandQueue : public CLObject<cl_command_queue>
	{
		public:
			CLCommandQueue( cl_command_queue q = NULL );
			CLCommandQueue( const CLCommandQueue& q );
			CLCommandQueue( const CLContext& context, const CLDevice& device, cl_command_queue_properties props = 0 );

			CLContext context() const;
			CLDevice device() const;

			/**
			  Returns the properties used when constructing the command-queue.
			 */
			CLUTIL_GETINFOTYPE( properties, CL_QUEUE_PROPERTIES, cl_command_queue_properties, _object, ::clGetCommandQueueInfo )

			void  enqueueReadBuffer( const CLBuffer& buf, void* dst, size_t size, size_t offset = 0, bool block = true,
								     const std::vector<CLEvent>* waitevents = NULL, CLEvent* event = NULL );
			void  enqueueWriteBuffer( const CLBuffer& buf, const void* src, size_t size, size_t offset = 0, bool block = true,
									  const std::vector<CLEvent>* waitevents = NULL, CLEvent* event = NULL );
			void  enqueueCopyBuffer( const CLBuffer& dst, const CLBuffer& src, size_t size, size_t dstoffset = 0, size_t srcoffset = 0,
								     const std::vector<CLEvent>* waitevents = NULL, CLEvent* event = NULL );
			void* enqueueMapBuffer( const CLBuffer& buf, cl_map_flags mapflags, size_t size, size_t offset = 0, bool block = true,
								    const std::vector<CLEvent>* waitevents = NULL, CLEvent* event = NULL );

			void  enqueueReadImage( const CLImage2D& img, void* dst, size_t x, size_t y, size_t width, size_t height, size_t stride = 0,
								    bool block = true, const std::vector<CLEvent>* waitevents = NULL, CLEvent* event = NULL );
		    void  enqueueWriteImage( const CLImage2D& img, const void* src, size_t x, size_t y, size_t width, size_t height, size_t stride = 0,
								    bool block = true, const std::vector<CLEvent>* waitevents = NULL, CLEvent* event = NULL );
			void  enqueueCopyImage( const CLImage2D& dst, const CLImage2D& src, size_t dstx, size_t dsty, size_t srcx, size_t srcy, size_t width, size_t height,
								    const std::vector<CLEvent>* waitevents = NULL, CLEvent* event = NULL );
			void* enqueueMapImage( const CLImage2D& img, cl_map_flags mapflags, size_t x, size_t y, size_t width, size_t height, size_t* stride,
								   bool block = true, const std::vector<CLEvent>* waitevents = NULL, CLEvent* event = NULL );

			void enqueueUnmap( const CLMemory& mem, const void* ptr,
							   const std::vector<CLEvent>* waitevents = NULL, CLEvent* event = NULL );

			void enqueueAcquireGLObject( const CLMemory& mem, const std::vector<CLEvent>* waitevents = NULL, CLEvent* event = NULL );

			void enqueueReleaseGLObject( const CLMemory& mem, const std::vector<CLEvent>* waitevents = NULL, CLEvent* event = NULL );


			void flush();
			void finish();

			void enqueueNDRangeKernel( const CLKernel& kernel, const CLNDRange& global, const CLNDRange& local = CLNDRange(),
									   const CLNDRange& offset = CLNDRange(), const std::vector<CLEvent>* waitevents = NULL, CLEvent* event = NULL );

		private:
			CLUTIL_GETINFOTYPE( _context, CL_QUEUE_CONTEXT, cl_context, _object, ::clGetCommandQueueInfo );
			CLUTIL_GETINFOTYPE( _device, CL_QUEUE_DEVICE, cl_device_id, _object, ::clGetCommandQueueInfo );
	};

	/**
		Transfer CLBuffer object data to host memory.
	  */
	inline void CLCommandQueue::enqueueReadBuffer( const CLBuffer& buf, void* dst, size_t size, size_t offset, bool block,
												   const std::vector<CLEvent>* waitevents, CLEvent* event )
	{
		cl_int err;
		err = ::clEnqueueReadBuffer( _object, buf, block, offset, size, dst,
									 waitevents?waitevents->size() : 0, waitevents?( const cl_event* ) &(*waitevents)[0]:NULL, ( cl_event* ) event );
		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
	}

	/**
		Transfer host memory data to CLBuffer object.
	  */
	inline void CLCommandQueue::enqueueWriteBuffer( const CLBuffer& buf, const void* src, size_t size, size_t offset, bool block,
												    const std::vector<CLEvent>* waitevents, CLEvent* event )
	{
		cl_int err;
		err = ::clEnqueueWriteBuffer( _object, buf, block, offset, size, src,
									  waitevents?waitevents->size() : 0, waitevents?( const cl_event* ) &(*waitevents)[0]:NULL, ( cl_event* ) event );
		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
	}

	/**
		Copy data between CLBuffer objects.
	  */
	inline void CLCommandQueue::enqueueCopyBuffer( const CLBuffer& dst, const CLBuffer& src, size_t size, size_t dstoffset, size_t srcoffset,
												   const std::vector<CLEvent>* waitevents, CLEvent* event )
	{
		cl_int err;
		err = ::clEnqueueCopyBuffer( _object, src, dst, srcoffset, dstoffset, size,
									 waitevents?waitevents->size() : 0, waitevents?( const cl_event* ) &(*waitevents)[0]:NULL, ( cl_event* ) event );
		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
	}

	/**
		Map CLBuffer object memory in host memory address-space.
	 */
	inline void* CLCommandQueue::enqueueMapBuffer( const CLBuffer& buf, cl_map_flags mapflags, size_t size, size_t offset, bool block,
												   const std::vector<CLEvent>* waitevents, CLEvent* event )
	{
		void* ptr;
		cl_int err;
		ptr = ::clEnqueueMapBuffer( _object, buf, block, mapflags, offset, size,
								    waitevents?waitevents->size() : 0, waitevents?( const cl_event* ) &(*waitevents)[0]:NULL, ( cl_event* ) event, &err );
		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
		return ptr;
	}

	/**
	  Copy data of CLImage2D object to host memory.
	  */
	inline void  CLCommandQueue::enqueueReadImage( const CLImage2D& img, void* dst, size_t x, size_t y, size_t width, size_t height, size_t stride,
												   bool block, const std::vector<CLEvent>* waitevents, CLEvent* event)
	{
		cl_int err;
		size_t origin[ 3 ] = { x, y, 0 };
		size_t region[ 3 ] = { width, height, 1 };

		err = ::clEnqueueReadImage( _object, img, block, origin, region, stride, 0, dst,
								   waitevents?waitevents->size() : 0, waitevents?( const cl_event* ) &(*waitevents)[0]:NULL, ( cl_event* ) event );
		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
	}

	/**
	  Copy data from host memory to CLImage2D object.
	 */
	inline void  CLCommandQueue::enqueueWriteImage( const CLImage2D& img, const void* src, size_t x, size_t y, size_t width, size_t height, size_t stride,
												    bool block, const std::vector<CLEvent>* waitevents, CLEvent* event)
	{
		cl_int err;
		size_t origin[ 3 ] = { x, y, 0 };
		size_t region[ 3 ] = { width, height, 1 };

		err = ::clEnqueueWriteImage( _object, img, block, origin, region, stride, 0, src,
									 waitevents?waitevents->size() : 0, waitevents?( const cl_event* ) &(*waitevents)[0]:NULL, ( cl_event* ) event );
		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
	}

	/**
		Copy data between two CLImage2D objects.
	  */
	inline void  CLCommandQueue::enqueueCopyImage( const CLImage2D& dst, const CLImage2D& src, size_t dstx, size_t dsty,
												   size_t srcx, size_t srcy, size_t width, size_t height,
												   const std::vector<CLEvent>* waitevents, CLEvent* event)
	{
		cl_int err;
		size_t srcorigin[ 3 ] = { srcx, srcy, 0 };
		size_t dstorigin[ 3 ] = { dstx, dsty, 0 };
		size_t region[ 3 ] = { width, height, 1 };

		err = ::clEnqueueCopyImage( _object, src, dst, srcorigin, dstorigin, region,
									waitevents?waitevents->size() : 0, waitevents?( const cl_event* ) &(*waitevents)[0]:NULL, ( cl_event* ) event );
		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
	}

	/**
		Map CLImage2D object memory in host memory address-space.
	*/
	inline void* CLCommandQueue::enqueueMapImage( const CLImage2D& img, cl_map_flags mapflags, size_t x, size_t y, size_t width, size_t height, size_t* stride,
												  bool block, const std::vector<CLEvent>* waitevents, CLEvent* event)
	{
		cl_int err;
		void* ptr;
		size_t origin[ 3 ] = { x, y, 0 };
		size_t region[ 3 ] = { width, height, 1 };

		ptr = ::clEnqueueMapImage( _object, img, block, mapflags, origin, region, stride, NULL,
								   waitevents?waitevents->size() : 0, waitevents?( const cl_event* ) &(*waitevents)[0]:NULL, ( cl_event* ) event, &err );
		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
		return ptr;
	}

	/**
	  Unmap previously mapped CL memory object.
	  */
	inline void CLCommandQueue::enqueueUnmap( const CLMemory& mem, const void* ptr,
											 const std::vector<CLEvent>* waitevents, CLEvent* event )
	{
		cl_int err;
		err = ::clEnqueueUnmapMemObject( _object, mem, ( void* ) ptr,
										waitevents?waitevents->size() : 0, waitevents?( const cl_event* ) &(*waitevents)[0]:NULL, ( cl_event* ) event );
		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
	}


	/**
	  Flush the CL command-queue.
	  */
	inline void CLCommandQueue::flush()
	{
		::clFlush( _object );
	}

	/**
		Wait until all previouly enqued commands have been completed.
	  */
	inline void CLCommandQueue::finish()
	{
		::clFinish( _object );
	}

	/**
		Enqueue kernel for execution.
	 */
	inline void CLCommandQueue::enqueueNDRangeKernel( const CLKernel& kernel, const CLNDRange& global, const CLNDRange& local,
													  const CLNDRange& offset, const std::vector<CLEvent>* waitevents , CLEvent* event )
	{
		cl_int err;
		err = ::clEnqueueNDRangeKernel( _object, kernel, global.dimension(), offset.dimension()?offset.range():NULL, global.range(),
									    local.dimension()?local.range():NULL,
										waitevents?waitevents->size() : 0, waitevents?( const cl_event* ) &(*waitevents)[0]:NULL, ( cl_event* ) event );
		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
	}

	inline void CLCommandQueue::enqueueAcquireGLObject( const CLMemory& mem, const std::vector<CLEvent>* waitevents , CLEvent* event )
	{
		cl_int err;

		err = ::clEnqueueAcquireGLObjects( _object, 1, ( const cl_mem* ) &mem, waitevents?waitevents->size() : 0, waitevents?( const cl_event* ) &(*waitevents)[0]:NULL, ( cl_event* ) event );

		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
	}

	inline void CLCommandQueue::enqueueReleaseGLObject( const CLMemory& mem, const std::vector<CLEvent>* waitevents , CLEvent* event )
	{
		cl_int err;

		err = ::clEnqueueReleaseGLObjects( _object, 1, ( const cl_mem* ) &mem, waitevents?waitevents->size() : 0, waitevents?( const cl_event* ) &(*waitevents)[0]:NULL, ( cl_event* ) event );

		if( err != CL_SUCCESS )
			throw CLException( __PRETTY_FUNCTION__, err );
	}

}

#endif
