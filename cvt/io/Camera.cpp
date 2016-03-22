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

#include <cvt/io/Camera.h>

#ifdef APPLE
    #include <cvt/io/QTKitCamera.h>
#endif

#ifdef LINUX
	#include <cvt/io/V4L2Camera.h>
#endif
#ifdef UEYEUSB_FOUND
	#include <cvt/io/UEyeUsbCamera.h>
#endif

#ifdef DC1394_FOUND
#include <cvt/io/DC1394Camera.h>
#endif

#ifdef OpenNI_FOUND
#include <cvt/io/OpenNICamera.h>
#endif

#ifdef OpenNI2_FOUND
#include <cvt/io/OpenNI2Camera.h>
#endif

namespace cvt {

	std::vector<CameraInfo> Camera::_camInfos;

	Camera::Camera()
	{
	}

	Camera::~Camera()
	{
	}

	void Camera::updateInfo()
	{
		size_t count = 0;
#ifdef APPLE
		count = QTKitCamera::count();
		for( size_t i = 0; i < count; i++ ){
			Camera::_camInfos.push_back( CameraInfo() );
			QTKitCamera::cameraInfo( i, Camera::_camInfos.back() );
		}
#endif

#ifdef LINUX
		// all V4L2 devices
		for( size_t i = 0; i < V4L2Camera::count() ; i++ ){
			Camera::_camInfos.push_back( CameraInfo() );
			V4L2Camera::cameraInfo( i, Camera::_camInfos.back() );
		}
#endif

#ifdef DC1394_FOUND
		// dc1394 cameras
		for( size_t i = 0; i < DC1394Camera::count(); i++){
			Camera::_camInfos.push_back( CameraInfo() );
			DC1394Camera::cameraInfo( i, Camera::_camInfos.back() );
		}
#endif

		// ueye cameras
#ifdef UEYEUSB_FOUND
		count = UEyeUsbCamera::count();
		for( size_t i = 0; i < count; i++){
			Camera::_camInfos.push_back( CameraInfo() );
			UEyeUsbCamera::cameraInfo( i, Camera::_camInfos.back() );
		}
#endif

#ifdef OpenNI_FOUND
		count = OpenNICamera::count();
		for( size_t i = 0; i < count; i++ ){
			Camera::_camInfos.push_back( CameraInfo() );
			OpenNICamera::cameraInfo( i, Camera::_camInfos.back() );
		}
#endif
#ifdef OpenNI2_FOUND
		count = OpenNI2Camera::count();
		std::cout << "ONI2Devices " << count << std::endl;
		for( size_t i = 0; i < count; i++ ){
			Camera::_camInfos.push_back( CameraInfo() );
			OpenNI2Camera::cameraInfo( i, Camera::_camInfos.back() );
		}
#endif
	}

	Camera * Camera::get( size_t index, size_t width, size_t height,
						  size_t fps, const IFormat & format )
	{
		if( index >= Camera::_camInfos.size() ){
			throw CVTException( "Camera index out of bounds!" );
		}

		CameraInfo & camInfo = Camera::_camInfos[ index ];

		Camera * cam = NULL;

		const CameraMode & mode = camInfo.bestMatchingMode( format, width, height, fps );
		std::cout << "Selecting mode: " << mode << std::endl;

		switch( camInfo.type() ){
#ifdef UEYEUSB_FOUND
			case CAMERATYPE_UEYE:
				cam = new UEyeUsbCamera( camInfo.index(), mode );
				break;
#endif
#ifdef DC1394_FOUND
			case CAMERATYPE_DC1394:
				cam = new DC1394Camera( camInfo.index(), mode );                
				break;
#endif
#ifdef LINUX
			case CAMERATYPE_V4L2:
				cam = new V4L2Camera( camInfo.index(), mode );
				break;
#endif
#ifdef APPLE
			case CAMERATYPE_QTKIT:
				cam = new QTKitCamera( camInfo.index(), mode );
				break;
#endif
#ifdef OpenNI_FOUND
			case CAMERATYPE_OPENNI:
				cam = new OpenNICamera( camInfo.index(), mode );
				break;
#endif
#ifdef OpenNI2_FOUND
			case CAMERATYPE_OPENNI2:
				cam = new OpenNI2Camera( camInfo.index(), mode );
				break;
#endif
			default:
				throw CVTException( "Unkown camera type" );
		}

		return cam;
	}

	Camera* Camera::get( size_t index, const CameraMode& mode )
	{
		return Camera::get( index, mode.width, mode.height, mode.fps, mode.format );
	}
}
