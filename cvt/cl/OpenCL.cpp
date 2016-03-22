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

#include <cvt/cl/OpenCL.h>
#include <cvt/cl/CLDevice.h>
#include <cvt/cl/CLPlatform.h>
#include <cvt/cl/CLContext.h>

namespace cvt {
	bool			CL::_glsharing = false;
	CLContext*	    CL::_ctx = NULL;
	CLDevice*	    CL::_device = NULL;
	CLCommandQueue* CL::_queue = NULL;

	bool CL::setDefaultDevice( const CLDevice& dev )
	{
		if( !_ctx ) {
			_glsharing = false;
			_ctx = new CLContext( dev.platform(), dev );
			_device = new CLDevice( dev );
			_queue = new CLCommandQueue( *_ctx, *_device /*, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE*/ );
			return true;
		}
		return false;
	}

	bool CL::init( const GLContext* ctx )
	{
		std::vector<CLPlatform> clplatforms;
		CLPlatform::get( clplatforms );

		if( _ctx || !clplatforms.size() )
			return false;

		/* try to find platform/device with cl_APPLE_gl_sharing or cl_khr_gl_sharing */
		if( ctx ) {
			for( size_t i = 0; i < clplatforms.size(); i++ ) {
				//std::cout << clplatforms[ i ] << std::endl;
				std::vector<CLDevice> devs;
				clplatforms[ i ].devices( devs, CL_DEVICE_TYPE_GPU );
				for( size_t k = 0; k < devs.size(); k++ ) {
					std::vector<String> exts;
					devs[ i ].extensions( exts );
					for( size_t l = 0; l < exts.size(); l++) {
						if( exts[ l ] == "cl_APPLE_gl_sharing" || exts[ l ] == "cl_khr_gl_sharing" ) {
							try {
								_ctx = new CLContext( clplatforms[ i ], devs[ k ], ctx );
								_device = new CLDevice( devs[ k ] );
								_queue = new CLCommandQueue( *_ctx, *_device );
								_glsharing = true;
								return true;
							} catch( Exception& e ) {
								if( _ctx ) { delete _ctx; _ctx = NULL; }
								if( _device ) { delete _device; _device = NULL; }
								if( _queue ) { delete _queue; _queue = NULL; }
							}
						}
					}
				}
			}

		}

		// Search for GPU
		for( size_t i = 0; i < clplatforms.size(); i++ ) {
			std::vector<CLDevice> devs;
			clplatforms[ i ].devices( devs, CL_DEVICE_TYPE_GPU );
			if( devs.size() ) {
				return CL::setDefaultDevice( devs[ 0 ] );
			}
		}

		// Search for CPU
		for( size_t i = 0; i < clplatforms.size(); i++ ) {
			std::vector<CLDevice> devs;
			clplatforms[ i ].devices( devs, CL_DEVICE_TYPE_CPU );
			if( devs.size() ) {
				return CL::setDefaultDevice( devs[ 0 ] );
			}
		}

		return false;
	}

	void CL::cleanup()
	{
		if( CL::_ctx )
			delete CL::_ctx;
		if( CL::_device )
			delete CL::_device;
		if( CL::_queue )
			delete CL::_queue;
	}
}
