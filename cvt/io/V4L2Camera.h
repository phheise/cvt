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

#ifndef CVT_V4L2CAMERA_H
#define CVT_V4L2CAMERA_H

#include <cvt/io/Camera.h>

#include <linux/videodev2.h>
#include <cvt/gfx/Image.h>

namespace cvt
{

	class V4L2Camera : public Camera
	{
		public:
			V4L2Camera( size_t camIndex,
			const CameraMode & mode );

			virtual ~V4L2Camera( );

			size_t width( ) const;
			size_t height( ) const;
			const IFormat & format( ) const;
			const Image & frame( ) const;
			bool  nextFrame( size_t timeout = 30 );
			void  startCapture( );
			void  stopCapture( );

			/* unique identifier */
			const String&   identifier( ) const { return _identifier; }
			size_t          frameIndex( ) const { return _frameIdx; }
			double          stamp( ) const { return _stamp; }

			void updateAutoIrisExp( );
			void setAutoIris( bool b );
			void setAutoExposure( bool b );
			void setExposureValue( unsigned int val );
			void setAutoFocus( bool b );
			void setAutoWhiteBalance( bool b );
			void setBacklightCompensation( bool b );

			static size_t count( );
			static void   cameraInfo( size_t index, CameraInfo & info );

			/**
			 * \brief Function for determining capable devices on the system, can be used for debugging
			 */
			static void listDevices( std::vector<String> & devices, bool verbose = false );

			typedef struct buffer {
				void* start;
				size_t length;
			} buffer_t;

		private:
			size_t _width;
			size_t _height;
			size_t _fps;
			size_t _numBuffers;
			size_t _camIndex;
			bool   _opened;
			bool   _capturing;
			int    _nextBuf;

			// the device file descriptor
			int _fd;

			// memory buffers for mmap frames
			buffer_t* _buffers;

			Image*  _frame;
			IFormat _format;

			// frame acquisition time
			double _stamp;
			// sequence number of last acquired frame
			size_t _frameIdx;

			//TODO remove the rest of unnecessary state
			/** V4L2 specific **/
			v4l2_ext_control*  _extControlsToSet;
			v4l2_timecode      _timeCode;
			v4l2_queryctrl     _queryCtrl;
			v4l2_querymenu     _queryMenu;
			v4l2_input         _input;
			v4l2_control       _control;
			v4l2_ext_controls  _extendedControls;

			bool   _autoExposure;
			bool   _autoIris;
			bool   _autoFocus;
			bool   _autoWhiteBalance;
			bool   _backLightCompensation;
			size_t _absExposureVal;

			String _identifier;

			//formats
			static const int supportedPixFormats[];
			static const int standardWidths[];
			static const int standardHeights[];

			// private helper funcs ...
			void                  open( );
			void                  close( );
			void                  init( );
			void                  queryBuffers( bool unmap = false );
			void                  enqueueBuffers( );
			void                  extendedControl( );
			static void           control( int fd, int field, int value );
			static const IFormat& formatForV4L2PixFormat( uint32_t pixelformat );

	};

	inline size_t V4L2Camera::width( ) const
	{
		return _width;
	}

	inline size_t V4L2Camera::height( ) const
	{
		return _height;
	}

	inline const IFormat & V4L2Camera::format( ) const
	{
		return _format;
	}

}

#endif
