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

#include <cvt/io/V4L2Camera.h>
#include <cvt/io/FileSystem.h>

#include <iostream>

#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <asm/types.h>
#include <errno.h>
#include <linux/videodev2.h>
#include <stdio.h>

#include "util/Exception.h"
#include "math/Math.h"

namespace cvt
{

	const int V4L2Camera::supportedPixFormats[] = { V4L2_PIX_FMT_RGB32, V4L2_PIX_FMT_BGR32, V4L2_PIX_FMT_YUYV,
													V4L2_PIX_FMT_UYVY, V4L2_PIX_FMT_GREY, V4L2_PIX_FMT_Y16};

	const int V4L2Camera::standardWidths[] = {1024, 640, 320, 704, 352};
	const int V4L2Camera::standardHeights[] = {768, 480, 240, 576, 288};

	V4L2Camera::V4L2Camera( size_t camIndex, const CameraMode & mode ) :
		_width( mode.width ),
		_height( mode.height ),
		_fps( mode.fps ),
		_numBuffers( 4 ),
		_camIndex( camIndex ),
		_opened( false ),
		_capturing( false ),
		_nextBuf( -1 ),
		_fd( -1 ),
		_buffers( NULL ),
		_frame( NULL ),
		_format( mode.format ),
		_stamp( 0.0 ),
		_frameIdx( 0 ),
		_extControlsToSet( 0 ),
		_autoExposure( false ),
		_autoIris( false ),
		_autoFocus( true ),
		_autoWhiteBalance( true ),
		_backLightCompensation( false ),
		_absExposureVal( 250 ),
		_identifier( "v4l2_" )
	{
		open( );
		init( );

		_identifier += camIndex;
	}

	V4L2Camera::~V4L2Camera( )
	{
		if( _opened )
			close( );
	}

	void V4L2Camera::open( )
	{
		std::vector<String> devices;
		V4L2Camera::listDevices( devices );

		if( _camIndex >= devices.size( ) ) {
			throw CVTException( "device index out of bounds!" );
		}

		_fd = ::open( devices[ _camIndex ].c_str( ), O_RDWR | O_NONBLOCK );

		if( _fd == -1 ) {
			String str( "Could not open device named \"" );
			str +=  devices[ _camIndex ];
			str + "\"";
			throw CVTException( str.c_str( ) );
		}

		_opened = true;
	}

	void V4L2Camera::close( )
	{
		for( size_t i = 0; i < _numBuffers; i++ ) {
			if( ( _buffers[ i ].start != MAP_FAILED ) && _buffers[ i ].length ) {
				if( munmap( _buffers[ i ].start, _buffers[ i ].length ) < 0 ) {
					throw CVTException( "Could not unmap buffer " + i );
				}
			}
		}

		if( _extControlsToSet != 0 )
			delete[] _extControlsToSet;

		if( _buffers != 0 )
			delete[] _buffers;

		if( _fd != -1 )
			::close( _fd );

		_opened = false;
	}

	void V4L2Camera::init( )
	{
		// set the time code
		_timeCode.type = V4L2_TC_TYPE_30FPS;
		_timeCode.flags = V4L2_TC_FLAG_DROPFRAME;

		// initialize V4L2 stuff
		struct v4l2_format fmt;
		memset(&fmt, 0, sizeof fmt);

		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fmt.fmt.pix.width = _width;
		fmt.fmt.pix.height = _height;
		fmt.fmt.pix.field = V4L2_FIELD_ANY;

		switch( _format.formatID ) {
			case IFORMAT_YUYV_UINT8:
				fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
				break;

			case IFORMAT_UYVY_UINT8:
				fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;
				break;

			case IFORMAT_BGRA_UINT8:
				fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR32;
				break;

			case IFORMAT_RGBA_UINT8:
				fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB32;
				break;

			case IFORMAT_GRAY_UINT8:
				fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
				break;

			default:
				throw CVTException( "Format not supported!" );
				break;
		}


		if( ioctl( _fd, VIDIOC_S_FMT, &fmt ) != 0 ) {
			//TODO Read errno
			throw CVTException( "Unable to set requested format!" );
		}

		_width = fmt.fmt.pix.width;
		_height = fmt.fmt.pix.height;

		if( _frame )
			delete _frame;

		_frame = new Image( _width, _height, _format );

		// set stream parameter (fps):
		v4l2_streamparm streamParameter;
		memset(&streamParameter, 0, sizeof streamParameter);

		streamParameter.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		streamParameter.parm.capture.timeperframe.numerator = 1000;
		streamParameter.parm.capture.timeperframe.denominator = _fps * 1000;

		if( ioctl( _fd, VIDIOC_S_PARM, &streamParameter ) ) {
			throw CVTException( "Could not set stream parameters!" );
		}

		//We need to round because NTSC recording stuff uses non integer frame rates
		if( Math::round( static_cast<float>( streamParameter.parm.capture.timeperframe.denominator ) /
				static_cast<float>( streamParameter.parm.capture.timeperframe.numerator ) ) != _fps ) {
			std::cerr << "Requested framerate (" << _fps << ") not supported by device => using " <<
				streamParameter.parm.capture.timeperframe.denominator /
				streamParameter.parm.capture.timeperframe.numerator << " fps" << std::endl;
		}

		// request the buffers
		v4l2_requestbuffers requestBuffers = {0};
		requestBuffers.count = _numBuffers;
		requestBuffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		requestBuffers.memory = V4L2_MEMORY_MMAP;

		if( ioctl( _fd, VIDIOC_REQBUFS, &requestBuffers ) != 0 ) {
			throw CVTException( "VIDIOC_REQBUFS - Unable to allocate buffers" );
		}

		if( requestBuffers.count < _numBuffers ) {
			throw CVTException( "VIDIOC_REQBUFS: Unable to allocate the request number of buffers" );
		}

		_buffers = new buffer_t[ _numBuffers ];

		queryBuffers( false );
		enqueueBuffers( );
	}

	void V4L2Camera::startCapture( )
	{
		if( !_capturing ) {
			int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

			if( ioctl( _fd, VIDIOC_STREAMON, &type ) != 0 ) {
				throw CVTException( "Could not start streaming!" );
			}
			_capturing = true;
		}
	}

	void V4L2Camera::stopCapture( )
	{
		if( _capturing ) {
			int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

			if( ioctl( _fd, VIDIOC_STREAMOFF, &type ) ) {
				throw CVTException( "Could not stop streaming!" );
			}
			_capturing = false;
		}
	}

	bool V4L2Camera::nextFrame( size_t tout )
	{
		fd_set rdset;
		struct timeval timeout = {0};

		v4l2_buffer buffer = {0};
		buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buffer.memory = V4L2_MEMORY_MMAP;

		if( !_capturing )
			startCapture( );

		FD_ZERO( &rdset );
		FD_SET( _fd, &rdset );

		timeout.tv_usec = tout * 1000; // ms

		// select - wait for data or timeout
		int ret = select( _fd + 1, &rdset, NULL, NULL, &timeout );
		if( ret < 0 ) {
			throw CVTException( "Could not grab image (select error)" );
		} else if( ret == 0 ) {
			return false;
		} else if( ( ret > 0 ) && ( FD_ISSET( _fd, &rdset ) ) ) {
			if( ioctl( _fd, VIDIOC_DQBUF, &buffer ) != 0 ) {
				throw CVTException( "Unable to dequeue buffer!" );
			}
		}

		// get frame from buffer
		size_t stride;
		uint8_t* ptrM;
		uint8_t* ptr;

		ptrM = ptr = _frame->map( &stride );
		size_t h = _frame->height( );
		uint8_t* bufPtr = static_cast<uint8_t*>( _buffers[ buffer.index ].start );
		size_t bufStride = _frame->width( ) * _format.bpp;
		SIMD* simd = SIMD::instance( );
		while( h-- ) {
			simd->Memcpy( ptr, bufPtr, bufStride );
			ptr += stride;
			bufPtr += bufStride;
		}
		_frame->unmap( ptrM );


		_frameIdx = buffer.sequence;
		_stamp    = static_cast<double>( buffer.timestamp.tv_sec ) +
					static_cast<double>( buffer.timestamp.tv_usec ) / 1000000.0;

		if( ioctl( _fd, VIDIOC_QBUF, &buffer ) != 0 ) {
			throw CVTException( "Unable to requeue buffer" );
		}

		return true;
	}

	const Image & V4L2Camera::frame( ) const
	{
		assert( _frame != NULL );
		return *_frame;
	}

	void V4L2Camera::updateAutoIrisExp( )
	{
		if( _autoExposure ) {
			if( _autoIris ) {
				V4L2Camera::control( _fd, V4L2_CID_EXPOSURE_AUTO, V4L2_EXPOSURE_AUTO );
			} else {
				V4L2Camera::control( _fd, V4L2_CID_EXPOSURE_AUTO, V4L2_EXPOSURE_APERTURE_PRIORITY );
			}
		} else {
			if( _autoIris ) {
				V4L2Camera::control( _fd, V4L2_CID_EXPOSURE_AUTO, V4L2_EXPOSURE_SHUTTER_PRIORITY );
			} else {
				V4L2Camera::control( _fd, V4L2_CID_EXPOSURE_AUTO, V4L2_EXPOSURE_MANUAL );
			}

			V4L2Camera::control( _fd, V4L2_CID_EXPOSURE_ABSOLUTE, ( int ) _absExposureVal );
		}
	}

	void V4L2Camera::setAutoIris( bool b )
	{
		_autoIris = b;
		updateAutoIrisExp( );
	}

	void V4L2Camera::setAutoExposure( bool b )
	{
		_autoExposure = b;
		updateAutoIrisExp( );
	}

	void V4L2Camera::setExposureValue( unsigned int val )
	{
		_absExposureVal = val;
		V4L2Camera::control( _fd, V4L2_CID_EXPOSURE_ABSOLUTE, ( int ) _absExposureVal );
	}

	void V4L2Camera::setAutoFocus( bool b )
	{
		_autoFocus = b;
		V4L2Camera::control( _fd, V4L2_CID_FOCUS_AUTO, _autoFocus );
	}

	void V4L2Camera::setAutoWhiteBalance( bool b )
	{
		_autoWhiteBalance = b;
		V4L2Camera::control( _fd, V4L2_CID_AUTO_WHITE_BALANCE, _autoWhiteBalance );
	}

	void V4L2Camera::setBacklightCompensation( bool b )
	{
		_backLightCompensation = b;
		V4L2Camera::control( _fd, V4L2_CID_BACKLIGHT_COMPENSATION, _backLightCompensation );
	}

	void V4L2Camera::control( int fd, int field, int value )
	{
		struct v4l2_queryctrl queryctrl;

		memset( &queryctrl, 0, sizeof( queryctrl ) );
		// set the field
		queryctrl.id = field;

		if( ioctl( fd, VIDIOC_QUERYCTRL, &queryctrl ) == -1 ) {
			std::cout << "Error: This ioctl is not supported by the device" << std::endl;
		} else if( queryctrl.flags & V4L2_CTRL_FLAG_DISABLED ) {
			std::cout << "Field " << field << " is not supported" << std::endl;
		} else {
			struct v4l2_control control;
			memset( &control, 0, sizeof( control ) );
			control.id = field;
			control.value = value;

			if( ioctl( fd, VIDIOC_S_CTRL, &control ) == -1 ) {
				std::cout << "VIDIOC_S_CTRL ERROR: while setting value for field" << std::endl;
			}
		}
	}

	void V4L2Camera::extendedControl( )
	{
		// set the vield in _extendedControls:
		memset( &_extendedControls, 0, sizeof( v4l2_ext_controls ) );
		_extendedControls.ctrl_class = V4L2_CTRL_CLASS_CAMERA;
		_extendedControls.count = 2;
		_extendedControls.controls = _extControlsToSet;

		if( _extControlsToSet != 0 )
			delete[] _extControlsToSet;

		_extControlsToSet = new v4l2_ext_control[ 2 ];
		memset( &_extControlsToSet[ 0 ], 0, sizeof( v4l2_ext_control ) );
		memset( &_extControlsToSet[ 1 ], 0, sizeof( v4l2_ext_control ) );

		// set pan:
		_extControlsToSet[ 0 ].id = V4L2_CID_PAN_ABSOLUTE;
		//_extControlsToSet[0].value64 = 10*3600;

		_extControlsToSet[ 1 ].id = V4L2_CID_TILT_ABSOLUTE;
		//_extControlsToSet[1].value64 = -10*3600;

		if( ioctl( _fd, VIDIOC_G_EXT_CTRLS, &_extendedControls ) == -1 ) {
			std::cout << "Error setting extended controls ..." << std::endl;

			std::cout << "Error at index " << _extendedControls.error_idx << std::endl;
			perror( "Problem:" );
		} else {
			std::cout << "CURRENT VALUES:" << std::endl;
			std::cout << "PAN: " << _extControlsToSet[ 0 ].value << std::endl;
			std::cout << "TILT: " << _extControlsToSet[ 1 ].value << std::endl;
		}

	}

	void V4L2Camera::queryBuffers( bool unmap )
	{
		int ret = 0;

		for( unsigned int i = 0; i < _numBuffers ; i++ )
		{
			// unmap old buffer
			if( unmap ) {
				if( -1 == munmap( _buffers[ i ].start, _buffers[ i ].length ) ) {
					std::cerr << "Unable to unmap V4L2 buffer " << i + 1 << " of " << _numBuffers << ": "
							  << strerror( errno ) << std::endl;
					throw CVTException( "V4L2 munmap failed: " );
				}
			}

			v4l2_buffer buffer = {0};
			buffer.index = i;
			buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buffer.timestamp.tv_sec = 0;                //get frame as soon as possible
			buffer.timestamp.tv_usec = 0;
			buffer.memory = V4L2_MEMORY_MMAP;

			if( ioctl( _fd, VIDIOC_QUERYBUF, &buffer ) != 0 ) {
				throw CVTException( "VIDIOC_QUERYBUF failed" );
			}

			if( 0 == buffer.length ) {
				throw CVTException( "V4L2 querried buffer length is 0" );
			}

			// map new buffer
			_buffers[ i ].start = mmap( NULL, buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, buffer.m.offset );
			_buffers[ i ].length = buffer.length;

			if( _buffers[ i ].start == MAP_FAILED ) {
				throw CVTException( "V4L2 buffer mmap failed" );
			}
		}
	}

	void V4L2Camera::enqueueBuffers( )
	{
		for( unsigned int i = 0; i < _numBuffers; i++ ) {
			v4l2_buffer buffer = {0};
			buffer.index = i;
			buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buffer.timestamp.tv_sec = 0; //get frame as soon as possible
			buffer.timestamp.tv_usec = 0;
			buffer.memory = V4L2_MEMORY_MMAP;

			if( ioctl( _fd, VIDIOC_QBUF, &buffer ) ) {
				throw CVTException( "VIDIOC_QBUF - Unable to queue buffer" );
			}
		}
	}

	size_t V4L2Camera::count( )
	{
		std::vector<String> devs;
		listDevices( devs );
		return devs.size( );
	}

	const IFormat& V4L2Camera::formatForV4L2PixFormat( uint32_t pixelformat )
	{
		switch( pixelformat ) {
			case V4L2_PIX_FMT_YUYV:
				return IFormat::YUYV_UINT8;
				break;

			case V4L2_PIX_FMT_UYVY:
				return IFormat::UYVY_UINT8;
				break;

			case V4L2_PIX_FMT_BGR32:
				return IFormat::BGRA_UINT8;
				break;

			case V4L2_PIX_FMT_RGB32:
				return IFormat::RGBA_UINT8;
				break;

			case V4L2_PIX_FMT_GREY:
				return IFormat::GRAY_UINT8;
				break;

			case V4L2_PIX_FMT_Y16:
				return IFormat::GRAY_UINT16;
		}

		std::stringstream errorMsg;
		errorMsg << "Unsupported V4L2 pixel format: "
				 << char( pixelformat & 0xFF )
				 << char( ( pixelformat >> 8 ) & 0xFF )
				 << char( ( pixelformat >> 16 ) & 0xFF )
				 << char( ( pixelformat >> 24 ) & 0xFF );

		throw CVTException( errorMsg.str( ).c_str( ) );
	}

	void V4L2Camera::cameraInfo( size_t index, CameraInfo & info )
	{
		std::vector<String> deviceNames;
		listDevices( deviceNames );
		int ret;

		if( index >= deviceNames.size( ) )
			throw CVTException( "No device with such index!" );

		int fd = ::open( deviceNames[ index ].c_str( ), O_RDWR | O_NONBLOCK );
		if( fd < 0 ) {
			String str( "Could not open device to get information: " );
			str += deviceNames[ index ];
			throw CVTException( str.c_str( ) );
		}

		struct v4l2_capability caps;
		if( ioctl( fd, VIDIOC_QUERYCAP, &caps ) != 0 )
			throw CVTException( "VIDIOC_QUERYCAP ioctl failed!" );

		String name;
		name.sprintf( "%s", ( char* ) caps.card );
		info.setName( name );
		info.setIndex( index );
		info.setType( CAMERATYPE_V4L2 );

		struct v4l2_fmtdesc formatDescription;
		memset( &formatDescription, 0, sizeof( v4l2_fmtdesc ) );
		formatDescription.index = 0;
		formatDescription.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		CameraMode currentMode;

		while( ioctl( fd, VIDIOC_ENUM_FMT, &formatDescription ) == 0 ) {
			formatDescription.index++;
			try {
				currentMode.format = formatForV4L2PixFormat( formatDescription.pixelformat );
			} catch( Exception& e ) {
				continue;   //the driver's pixformat is not supported, try the next one
			}

			struct v4l2_frmsizeenum frameSize;
			memset( &frameSize, 0, sizeof( v4l2_frmsizeenum ) );
			frameSize.index = 0;
			frameSize.pixel_format = formatDescription.pixelformat;

			while( ioctl( fd, VIDIOC_ENUM_FRAMESIZES, &frameSize ) == 0 ) {
				frameSize.index++;
				if( frameSize.type == V4L2_FRMSIZE_TYPE_DISCRETE ) {
					currentMode.width = frameSize.discrete.width;
					currentMode.height = frameSize.discrete.height;

					// enumerate frame intervals:
					struct v4l2_frmivalenum fps;
					memset( &fps, 0, sizeof( v4l2_frmivalenum ) );
					fps.index = 0;
					fps.pixel_format = formatDescription.pixelformat;
					fps.width = frameSize.discrete.width;
					fps.height = frameSize.discrete.height;
					while( ioctl( fd, VIDIOC_ENUM_FRAMEINTERVALS, &fps ) == 0 ) {
						fps.index++;
						if( fps.type == V4L2_FRMIVAL_TYPE_DISCRETE ) {
							//some drivers offer NTSC frame rates (like 30000 / 1001 or 29.97 FPS)
							//so make sure we take the correct rounded value
							currentMode.fps = Math::round( static_cast<float>( fps.discrete.denominator )
								/ static_cast<float>( fps.discrete.numerator ) );
							currentMode.description = "enumerated";
							info.addMode( currentMode );
						}
					}
				} else        //V4L2 docs indicate that only in the discrete case will increasing the index make sense
					break;
			}
		}

		//because some of drivers do not have a complete implementation, we have to negotiate the available modes
		//instead of enumerating them
		if( info.numModes( ) == 0 ) {
			//For the time being we fall back on the default setting currently set
			//by the userspace tools
			//ideally we would negotiate settings with VIDIOC_TRY_FMT
			struct v4l2_format fmt;
			memset( &fmt, 0, sizeof( fmt ) );
			fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			if( ioctl( fd, VIDIOC_G_FMT, &fmt ) == 0 && ( fmt.type & V4L2_BUF_TYPE_VIDEO_CAPTURE ) ) {
				for( size_t i = 0; i < sizeof( supportedPixFormats ); i++ ) {
					memset( &fmt, 0, sizeof( fmt ) );
					fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
					fmt.fmt.pix.pixelformat = supportedPixFormats[ i ];

					for( size_t j = 0; j < sizeof( standardWidths ); j++ ) {
						fmt.fmt.pix.height = standardHeights[ j ];
						fmt.fmt.pix.width = standardWidths[ j ];
						fmt.fmt.pix.field = V4L2_FIELD_ANY;

						//check if the format combination is supported
						if( ioctl( fd, VIDIOC_TRY_FMT, &fmt ) != 0 )
							continue;

						//V4L2 API specifies that the driver can change the request parameters
						//for the time being, if the params don't match up with our requrests,
						//simply ignore them
						if( fmt.fmt.pix.height != standardHeights[ j ] ||
							fmt.fmt.pix.width != standardWidths[ j ] ||
							fmt.fmt.pix.pixelformat != supportedPixFormats[ i ] )
							continue;

						v4l2_streamparm sparm;
						memset( &sparm, 0, sizeof( sparm ) );
						sparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
						if( ioctl( fd, VIDIOC_G_PARM, &sparm ) == 0 ) {
							currentMode.format = formatForV4L2PixFormat( fmt.fmt.pix.pixelformat );
							currentMode.height = fmt.fmt.pix.height;
							currentMode.width = fmt.fmt.pix.width;
							currentMode.description = "negotiated";

							//some drivers offer NTSC frame rates (like 30000 / 1001 or 29.97 FPS)
							//so make sure we take the correct rounded value
							currentMode.fps = Math::round( static_cast<float>( sparm.parm.capture.timeperframe.denominator ) /
								static_cast<float>( sparm.parm.capture.timeperframe.numerator ) );
							info.addMode( currentMode );
						}
					}
				}
			}
		}

		::close( fd );
	}

	/* check for openable v4l devices in /sys/class/video4linux */
	void V4L2Camera::listDevices( std::vector<String> & devices, bool verbose )
	{
		std::vector<String> possibleDevs;
		FileSystem::ls( "/sys/class/video4linux", possibleDevs );
		struct v4l2_capability caps;
		String ss;
		for( size_t i = 0; i < possibleDevs.size( ); i++ ) {
			if( verbose )
				std::cout << "trying v4l2 device: " << possibleDevs[ i ] << ". ";

			ss = "/dev/";
			ss += possibleDevs[ i ];

			//quick exclusion checks here
			struct stat st;
			if( -1 == stat( ss.c_str( ), &st ) ) {
				if( verbose )
					std::cout << "Failure! Unable to stat device." << std::endl;

				continue;
			}

			if( !S_ISCHR( st.st_mode ) ) {
				if( verbose )
					std::cout << "Failure! Not a character device." << std::endl;

				continue;
			}

			int fd = ::open( ss.c_str( ), O_RDWR | O_NONBLOCK );
			if( fd < 0 ) {
				if( verbose )
					std::cout << "Failure! Unable to open device." << std::endl;

				continue;
			}

			if( ioctl( fd, VIDIOC_QUERYCAP, &caps ) ) {
				if( verbose )
					std::cout << "Failure! Driver returned a negative v4l2 response." << std::endl;

				continue;
			}

			if( caps.capabilities & ( V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING ) ) {
				devices.push_back( ss );
				if( verbose )
					std::cout << "Success!" << std::endl;
			}

			else if( verbose )
				std::cout << "Failure! Device does not support capture or streaming." << std::endl;

			::close( fd );
		}
	}
}
