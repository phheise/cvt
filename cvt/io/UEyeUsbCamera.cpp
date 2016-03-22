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

#include "UEyeUsbCamera.h"

#include <cvt/gfx/IMapScoped.h>

#include <iostream>
#include <sstream>

namespace cvt
{
	UEyeUsbCamera::UEyeUsbCamera(  size_t camIndex, const CameraMode & mode ) :
		_camIndex( camIndex ), _camHandle( 0 ),
		_width( mode.width ), _height( mode.height ), _stride( _width ),
		_frame( _width, _height, mode.format ),
		_runMode( UEYE_MODE_FREERUN )
	{
		this->open( mode );
		this->setIdentifier();

		// load internally stored parameters as default starting point
		// loadParameters( "//cam//set1" );
	}

	UEyeUsbCamera::~UEyeUsbCamera()
	{
		if ( _camHandle != 0 ){
			this->stopCapture();
			this->freeMemories();
			is_ExitImageQueue( _camHandle );
			is_ExitCamera( _camHandle );
		}
	}

	const String & UEyeUsbCamera::identifier() const
	{
		return _identifier;
	}

	void UEyeUsbCamera::setIdentifier() 
	{
		CAMINFO camInfo;
		is_GetCameraInfo( _camHandle, &camInfo );
		_identifier = String( camInfo.SerNo );
	}

	void UEyeUsbCamera::setAutoGain( bool value )
	{
		double pVal1 = value?1:0;
		double pVal2 = 0;
		is_SetAutoParameter( _camHandle, IS_SET_ENABLE_AUTO_GAIN, &pVal1, &pVal2 );
	}

	void UEyeUsbCamera::setGainBoost( bool value )
	{
		INT mode = IS_SET_GAINBOOST_ON;
		if( !value )
			mode = IS_SET_GAINBOOST_OFF;
		is_SetGainBoost( _camHandle, mode );
	}

	void UEyeUsbCamera::setAutoShutter( bool value )
	{
		double pVal1 = value?1:0;
		double pVal2 = 0;
		is_SetAutoParameter( _camHandle, IS_SET_ENABLE_AUTO_SHUTTER, &pVal1, &pVal2 );
	}

	void UEyeUsbCamera::setAutoSensorShutter( bool value )
	{
		double pVal1 = value?1:0;
		double pVal2 = 0;
		int ret = is_SetAutoParameter( _camHandle, IS_SET_ENABLE_AUTO_SENSOR_SHUTTER, &pVal1, &pVal2 );
		if( ret != IS_SUCCESS ){
			std::cout << "Error when setting auto sensor shutter" << std::endl;
		}
	}

	void UEyeUsbCamera::setAutoWhiteBalance( bool value )
	{
		double val = value?1.0:0.0;
		int ret = is_SetAutoParameter( _camHandle, IS_SET_ENABLE_AUTO_WHITEBALANCE, &val, NULL );
		if( ret != IS_SUCCESS ){
			std::cout << "Error when setting auto white balance" << std::endl;
		}
	}	

	void UEyeUsbCamera::setWhiteBalanceOnce()
	{
		double val = 1.0;
		int ret = is_SetAutoParameter( _camHandle, IS_SET_AUTO_WB_ONCE, &val, NULL );
		if( ret != IS_SUCCESS ){
			std::cout << "Error when setting auto white balance once" << std::endl;
		}
		setAutoWhiteBalance( true );
	}

	void UEyeUsbCamera::setMaxAutoShutter( double value )
	{
		double pVal1 = value;
		double pVal2 = 0;
		is_SetAutoParameter( _camHandle, IS_SET_AUTO_SHUTTER_MAX, &pVal1, &pVal2 );
	}

	void UEyeUsbCamera::setExposureTime( double value )
	{
		double val;
		double min, max;
		exposureRange( min, max );
		
		if( value < min )
			val = min;
		else if ( value > max )
			val = max;
		val = value;
			
		is_Exposure( _camHandle, IS_EXPOSURE_CMD_SET_EXPOSURE, &val, sizeof( double ) );
	}

	void UEyeUsbCamera::exposureRange( double & min, double & max )
	{
		min = max = 0.0;
		is_Exposure( _camHandle, IS_EXPOSURE_CMD_GET_EXPOSURE_RANGE_MIN, &min, sizeof( double ) );
		is_Exposure( _camHandle, IS_EXPOSURE_CMD_GET_EXPOSURE_RANGE_MAX, &max, sizeof( double ) );
	}

	void UEyeUsbCamera::setPixelClock( unsigned int value )
	{
		//is_SetPixelClock( _camHandle, value );
		is_PixelClock( _camHandle, IS_PIXELCLOCK_CMD_SET, (void*)&value, sizeof( unsigned int ) );
	}

	void UEyeUsbCamera::pixelClockRange( unsigned int& min, unsigned int& max, unsigned int& step ) const
	{
		UINT range[ 3 ];
		is_PixelClock( _camHandle, IS_PIXELCLOCK_CMD_GET_RANGE, ( void* )range, sizeof( range ) );
		min = range[ 0 ];
		max = range[ 1 ];
		step = range[ 2 ];
	}

	void UEyeUsbCamera::setHorizontalMirror( bool value )
	{
		is_SetRopEffect( _camHandle, IS_SET_ROP_MIRROR_UPDOWN, value?1:0, 0 );
	}

	void UEyeUsbCamera::setVerticalMirror( bool value )
	{
		is_SetRopEffect( _camHandle, IS_SET_ROP_MIRROR_LEFTRIGHT, value?1:0, 0 );
	}

	void UEyeUsbCamera::getHardwareGains( int& master, int& red, int& green, int& blue )
	{
		master	= is_SetHardwareGain( _camHandle, IS_GET_MASTER_GAIN, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER );
		red		= is_SetHardwareGain( _camHandle, IS_GET_RED_GAIN, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER );
		green	= is_SetHardwareGain( _camHandle, IS_GET_GREEN_GAIN, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER );
		blue	= is_SetHardwareGain( _camHandle, IS_GET_BLUE_GAIN, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER );

		if( master	== IS_NO_SUCCESS ||
		    red		== IS_NO_SUCCESS ||
		    green   == IS_NO_SUCCESS ||
			blue	== IS_NO_SUCCESS )
			throw CVTException( "Error when getting hardware gains" );
	}

	void UEyeUsbCamera::setHardwareGains( int master, int red, int green, int blue )
	{
		int ret = is_SetHardwareGain( _camHandle, master, red, green, blue );		
		if( ret != IS_SUCCESS )
			throw CVTException( "Error when setting hardware gains" );
	}

	void UEyeUsbCamera::setFramerate( double fps )
	{
		double newFps;
		if( is_SetFrameRate( _camHandle, fps, &newFps ) == IS_NO_SUCCESS ) {
			std::cout << "New framerate: " << newFps << std::endl;
		} else {
			setExposureTime( 15.0 );
		}
	}

	bool UEyeUsbCamera::initCam()
	{
		INT nRet = is_InitCamera( &_camHandle, NULL );
		if ( nRet == IS_STARTER_FW_UPLOAD_NEEDED ){
			// Time for the firmware upload = 25 seconds by default
			INT nUploadTime = 25000;
			is_GetDuration( _camHandle, IS_SE_STARTER_FW_UPLOAD, &nUploadTime );

			std::cout << "This camera requires a new firmware. The upload will take around" << nUploadTime / 1000 << " seconds. Please wait ..." <<std::endl;

			// specifying "IS_ALLOW_STARTER_FIRMWARE_UPLOAD"
			_camHandle = (HIDS)( ( (INT)_camHandle ) | IS_ALLOW_STARTER_FW_UPLOAD );
			nRet = is_InitCamera( &_camHandle, NULL );
		}

		if( nRet == IS_SUCCESS )
			return true;
		return false;
	}

	void UEyeUsbCamera::open( const CameraMode & mode )
	{
		if ( _camHandle != 0 ) {
			//free old image mem.
			this->freeMemories();
			is_ExitCamera( _camHandle );
		}

		_camHandle = _camIndex | IS_USE_DEVICE_ID;

		if( !initCam() )
			throw CVTException( "Could not initialize camera" );

		setAreaOfInterest( Recti( 0, 0, _width, _height ) );
		
		if( mode.format == IFormat::BAYER_RGGB_UINT8 ){
// this is gone in 4.20!
#ifdef IS_CM_BAYER_RG8
			is_SetColorMode( _camHandle, IS_CM_BAYER_RG8 );
#else
            is_SetColorMode( _camHandle, IS_CM_SENSOR_RAW8 );
#endif

		} else {
			throw CVTException( "Color mode not supported by UEyeUsbCamera class" );
		}

		is_GetImageMemPitch( _camHandle, &_stride );		
		_frame.reallocate( _width, _height, mode.format );

		this->initMemories( mode );
		if( is_InitImageQueue( _camHandle, 0 ) != IS_SUCCESS ){
			std::cout << "COULD NOT INIT IMAGE QUEUE" << std::endl;
		}

		double newFPS;
		if( is_SetFrameRate( _camHandle, mode.fps, &newFPS ) == IS_NO_SUCCESS ){
			std::cout << "Could not set FrameRate" << std::endl;
		}
	}

	void UEyeUsbCamera::setAreaOfInterest( const Recti& roi )
	{
		IS_RECT aoiRect;
		aoiRect.s32X = roi.x | IS_AOI_IMAGE_POS_ABSOLUTE;
		aoiRect.s32Y = roi.y | IS_AOI_IMAGE_POS_ABSOLUTE;
		aoiRect.s32Width = roi.width;
		aoiRect.s32Height = roi.height;

		if( is_AOI( _camHandle, IS_AOI_IMAGE_SET_AOI, &aoiRect, sizeof( aoiRect ) ) == IS_NO_SUCCESS ){
            throw CVTException( "error when setting AOI" );
		}
	}

	void UEyeUsbCamera::initMemories( const CameraMode & mode )
	{
		for( size_t i = 0; i < _numImageBuffers; i++ ){
			_bufferIds[ 0 ] = 0;
			_buffers[ i ] = NULL;
			is_AllocImageMem( _camHandle, _width, _height, mode.format.bpp * 8, (char**)&_buffers[ i ], &_bufferIds[ i ] );
			is_AddToSequence( _camHandle, (char*)_buffers[ i ], _bufferIds[ i ] );
		}
	}

	void UEyeUsbCamera::freeMemories()
	{
		for( size_t i = 0; i < _numImageBuffers; i++ ){
			is_FreeImageMem( _camHandle, (char*)_buffers[ i ], _bufferIds[ i ] );
		}
	}

	void UEyeUsbCamera::startCapture()
	{		
        setRunMode( _runMode );
		enableEvents();
	}

	void UEyeUsbCamera::stopCapture()
	{
		if( _runMode == UEYE_MODE_FREERUN ){
			disableFreerun();
		}

		disableEvents();
	}

	bool UEyeUsbCamera::nextFrame( size_t timeout )
	{
        INT ret = IS_SUCCESS;

        switch( _runMode ){
            case UEYE_MODE_FREERUN:
                if( timeout )
                    return waitFrame( timeout );
                break;
            default:                
                ret = is_FreezeVideo( _camHandle, IS_DONT_WAIT );
                if( ret == IS_SUCCESS ){
                    if( timeout )
                        return waitFrame( timeout );
                }
        }
        return false;
	}

    bool UEyeUsbCamera::waitFrame( size_t ms )
    {
        if( ms && ms < 10 )
            ms = 10;
        else
            ms = ms * 0.1f;
        INT ret = is_WaitEvent( _camHandle, IS_SET_EVENT_FRAME, ms );

        if( ret == IS_SUCCESS ){
            bufferToFrame();
            return true;
        }
        return false;
    }

    bool UEyeUsbCamera::waitTriggerEvent( size_t timeout )
    {
        int tout = timeout / 10;
        INT ret = is_WaitEvent( _camHandle, IS_SET_EVENT_EXTTRIG, tout );

        if( ret == IS_SUCCESS )
            return true;
        return false;
    }

    void UEyeUsbCamera::bufferToFrame()
    {
        uint8_t* curBuffer  = NULL;
        uint8_t* lastBuffer = NULL;
        INT	bufferId = 0;
        if( is_GetActSeqBuf( _camHandle, &bufferId, ( char** )&curBuffer, ( char** )&lastBuffer ) == IS_SUCCESS ){
            int bufSeqNum = bufNumForAddr( curBuffer );
            is_LockSeqBuf( _camHandle, bufSeqNum, ( char* )curBuffer );

            size_t stride;
            uint8_t * framePtr = _frame.map( &stride );

            // TODO: use our memcpy here with _stride and stride!?
            is_CopyImageMem( _camHandle, (char*)curBuffer, bufSeqNum, (char*)framePtr );

            if( is_UnlockSeqBuf( _camHandle, bufSeqNum, (char*)curBuffer ) == IS_NO_SUCCESS ){
                std::cout << "UNLOCK FAILED" << std::endl;
            }

            _frame.unmap( framePtr );
        }
    }

	const Image & UEyeUsbCamera::frame() const
	{
        return _frame;
	}

	size_t UEyeUsbCamera::count()
	{
		INT ret = 0;

		if( is_GetNumberOfCameras( &ret ) != IS_SUCCESS )
			std::cout << "FAILURE IN GET NUM CAMS" << std::endl;

		return (size_t)ret;
	}

	void UEyeUsbCamera::cameraInfo( size_t index, CameraInfo & info )
	{
		size_t camCount = ( ULONG )UEyeUsbCamera::count();
		if( index > camCount ){
			throw CVTException( "Index out of bounds" );
		}

		// get all cameras
		PUEYE_CAMERA_LIST camList = 0;
        camList = ( PUEYE_CAMERA_LIST )new char[ sizeof( DWORD ) + camCount * sizeof( UEYE_CAMERA_INFO ) ];
        camList->dwCount = camCount;

        // Get CameraList and store it ...
		if( is_GetCameraList( camList ) == IS_SUCCESS ){
			UEYE_CAMERA_INFO & cinfo = camList->uci[ index ];
			info.setName( cinfo.Model );
			info.setIndex( cinfo.dwDeviceID );
			info.setType( CAMERATYPE_UEYE );
			HIDS handle = ( cinfo.dwDeviceID );
			handle |= IS_USE_DEVICE_ID;
			delete[] camList;

			if( is_InitCamera( &handle, 0 ) == IS_CANT_OPEN_DEVICE )
				throw CVTException( "Cannot initialize camera" );

			SENSORINFO sensorInfo;
			memset( &sensorInfo, 0, sizeof( SENSORINFO ) );
			if( is_GetSensorInfo( handle, &sensorInfo ) == IS_NO_SUCCESS )
				throw CVTException( "Could not get image information" );

			UINT pxClkRange[ 3 ] = { 0, 0, 0 };
			if( is_PixelClock( handle, IS_PIXELCLOCK_CMD_GET_RANGE, ( void* )pxClkRange, sizeof( pxClkRange ) ) == IS_NO_SUCCESS ){
				std::cout << "Could not get PixelClockRange" << std::endl;
			}

			if( is_PixelClock( handle, IS_PIXELCLOCK_CMD_SET, (void*)&pxClkRange[ 1 ]/**/, sizeof( unsigned int ) ) == IS_NO_SUCCESS ){
				std::cout << "Could not set PixelClock" << std::endl;
			}

			double min = 0, max = 0, interval = 0;
			if( is_GetFrameTimeRange( handle, &min, &max, &interval ) == IS_NO_SUCCESS ){
				std::cout << "Could not get TimeRange" << std::endl;
			}

			size_t width = (size_t)sensorInfo.nMaxWidth;
			size_t height = (size_t)sensorInfo.nMaxHeight;
			size_t fps = (size_t)( 1.0 / min );
			if( sensorInfo.nColorMode == IS_COLORMODE_BAYER )
				info.addMode( CameraMode( width, height, fps, IFormat::BAYER_RGGB_UINT8 ) );
			else if( sensorInfo.nColorMode == IS_COLORMODE_MONOCHROME )
				info.addMode( CameraMode( width, height, fps, IFormat::GRAY_UINT8 ) );
			else
				throw CVTException( "Invalid color mode" );

			if( is_ExitCamera( handle ) == IS_CANT_CLOSE_DEVICE )
				throw CVTException( "Could not exit camera" );
		} else {
			delete[] camList;
			throw CVTException( "Could not get camera list" );
		}
	}

    int	UEyeUsbCamera::bufNumForAddr( const uint8_t * buffAddr ) const
	{
		for( size_t i = 0; i < _numImageBuffers; i++ ){
			if( _buffers[ i ] == buffAddr ){
				return _bufferIds[ i ];
			}
		}

		return -1;
	}

    void UEyeUsbCamera::setRunMode( RunMode mode )
	{
        if( ( _runMode == UEYE_MODE_FREERUN ) &&
            ( mode     != UEYE_MODE_FREERUN ) ){
            disableFreerun();
        }

        switch( mode ){
            case UEYE_MODE_FREERUN:
                setTriggerMode( TRIGGER_OFF );
                enableFreerun();
                break;
            case UEYE_MODE_TRIGGERED:
                setTriggerMode( TRIGGER_SOFTWARE );
                //setTriggerMode( TRIGGER_OFF );
                break;
            case UEYE_MODE_HW_TRIGGER:
                setTriggerMode( TRIGGER_HI_LO );
                break;
		}
        _runMode = mode;
	}

    void UEyeUsbCamera::setTriggerMode( TriggerMode mode )
    {
        int ret = is_SetExternalTrigger( _camHandle, mode );

        if( ret != IS_SUCCESS ){
            std::cerr << "Could not set trigger mode!" << std::endl;
        }
    }

    void UEyeUsbCamera::setFlashMode( FlashMode mode )
    {
        INT nMode = mode;
        int ret = is_IO( _camHandle, IS_IO_CMD_FLASH_SET_MODE, ( void* )&nMode, sizeof( nMode ) );
        if( ret != IS_SUCCESS ){
            std::cerr << "Could not set flashmode" << std::endl;
        }
    }

    void UEyeUsbCamera::setTriggerDelay( size_t microSecs )
    {
        INT ret = is_SetTriggerDelay( _camHandle, ( INT )microSecs );
        if( ret != IS_SUCCESS ){
            std::cout << "Could not set Trigger Delay" << std::endl;
        }
    }

    void UEyeUsbCamera::setFlashDelayAndDuration( int32_t delayMuSecs, size_t durationMuSecs )
    {
        IO_FLASH_PARAMS params, min, max;

        memset( &params, 0, sizeof( params ) );
        memset( &min, 0, sizeof( min ) );
        memset( &max, 0, sizeof( max ) );

        INT ret = IS_SUCCESS;

        ret = is_IO( _camHandle, IS_IO_CMD_FLASH_GET_PARAMS_MIN, ( void* )&min, sizeof( min ) );
        if( ret == IS_SUCCESS ){
            if( min.s32Delay    > delayMuSecs )    delayMuSecs    = min.s32Delay;
            if( min.u32Duration > durationMuSecs ) durationMuSecs = min.u32Duration;
        }

        ret = is_IO( _camHandle, IS_IO_CMD_FLASH_GET_PARAMS_MAX, ( void* )&max, sizeof( max ) );
        if( ret == IS_SUCCESS ){
            if( max.s32Delay    < delayMuSecs )    delayMuSecs    = max.s32Delay;
            if( max.u32Duration < durationMuSecs ) durationMuSecs = max.u32Duration;
        }

        params.s32Delay    = delayMuSecs;
        params.u32Duration = durationMuSecs;

        std::cout << "Delay: " << delayMuSecs << " Duration: " << durationMuSecs << std::endl;
        std::cout << "Min Delay: " << min.s32Delay << " Duration: " << min.u32Duration << std::endl;
        std::cout << "Max Delay: " << max.s32Delay << " Duration: " << max.u32Duration << std::endl;

        ret = is_IO( _camHandle, IS_IO_CMD_FLASH_SET_PARAMS, ( void* )&params, sizeof( params ) );
        if( ret != IS_SUCCESS ){
            std::cerr << "Could not set delay and duration for Flash";
        }
    }

	void UEyeUsbCamera::saveParameters( const String& filename ) const
	{		
		int ret = is_ParameterSet( _camHandle, IS_PARAMETERSET_CMD_SAVE_FILE, ( void* )filename.c_str(), 0 );
		if( ret != IS_SUCCESS ){
			std::cout << "Could not save parameters to file" << std::endl;
		}
	}

	void UEyeUsbCamera::loadParameters( const String& filename )
	{		
		int ret = is_ParameterSet( _camHandle, IS_PARAMETERSET_CMD_LOAD_FILE, ( void* )filename.c_str(), 0 );
		if( ret != IS_SUCCESS )
			std::cout << "Error loading parameters from file" << std::endl;
	}

    void UEyeUsbCamera::testIOSupport()
    {
        UINT supportedIOs = 0;
        INT ret;
        ret = is_IO( _camHandle, IS_IO_CMD_GPIOS_GET_SUPPORTED, ( void* )&supportedIOs, sizeof( supportedIOs ) );


        if( ret != IS_SUCCESS ){
            std::cout << "IS_IO_CMD_GPIOS_GET_SUPPORTED function not supported by cam" << std::endl;
        } else {
            std::cout << "IS_IO_CMD_GPIOS_GET_SUPPORTED: ";
            if( supportedIOs & IO_GPIO_1 ){ std::cout << "IO_GPIO_1 "; }
            if( supportedIOs & IO_GPIO_2 ){ std::cout << "IO_GPIO_2 "; }
            if( supportedIOs & IO_GPIO_3 ){ std::cout << "IO_GPIO_3 "; }
            if( supportedIOs & IO_GPIO_4 ){ std::cout << "IO_GPIO_4 "; }
            std::cout << std::endl;
        }

        supportedIOs = 0;
        ret = is_IO( _camHandle, IS_IO_CMD_GPIOS_GET_SUPPORTED_INPUTS, ( void* )&supportedIOs, sizeof( supportedIOs ) );
        if( ret != IS_SUCCESS ){
            std::cout << "IS_IO_CMD_GPIOS_GET_SUPPORTED_INPUTS function not supported by cam" << std::endl;
        } else {
            std::cout << "IS_IO_CMD_GPIOS_GET_SUPPORTED_INPUTS: ";
            if( supportedIOs & IO_GPIO_1 ){ std::cout << "IO_GPIO_1 "; }
            if( supportedIOs & IO_GPIO_2 ){ std::cout << "IO_GPIO_2 "; }
            if( supportedIOs & IO_GPIO_3 ){ std::cout << "IO_GPIO_3 "; }
            if( supportedIOs & IO_GPIO_4 ){ std::cout << "IO_GPIO_4 "; }
            std::cout << std::endl;
        }

        supportedIOs = 0;
        ret = is_IO( _camHandle, IS_IO_CMD_GPIOS_GET_SUPPORTED_OUTPUTS, ( void* )&supportedIOs, sizeof( supportedIOs ) );
        if( ret != IS_SUCCESS ){
            std::cout << "IS_IO_CMD_GPIOS_GET_SUPPORTED_OUTPUTS function not supported by cam" << std::endl;
        } else {
            std::cout << "IS_IO_CMD_GPIOS_GET_SUPPORTED_OUTPUTS: ";
            if( supportedIOs & IO_GPIO_1 ){ std::cout << "IO_GPIO_1 "; }
            if( supportedIOs & IO_GPIO_2 ){ std::cout << "IO_GPIO_2 "; }
            if( supportedIOs & IO_GPIO_3 ){ std::cout << "IO_GPIO_3 "; }
            if( supportedIOs & IO_GPIO_4 ){ std::cout << "IO_GPIO_4 "; }
            std::cout << std::endl;
        }

        supportedIOs = 0;
        ret = is_IO( _camHandle, IS_IO_CMD_FLASH_GET_SUPPORTED_GPIOS, ( void* )&supportedIOs, sizeof( supportedIOs ) );
        if( ret != IS_SUCCESS ){
            std::cout << "IS_IO_CMD_FLASH_GET_SUPPORTED_GPIOS function not supported by cam" << std::endl;
        } else {
            std::cout << "IS_IO_CMD_FLASH_GET_SUPPORTED_GPIOS: ";
            if( supportedIOs & IO_FLASH_MODE_GPIO_1 ){
                std::cout << "IO_FLASH_MODE_GPIO_1 ";
                //ret = is_IO( _camHandle, IS_IO_CMD_GPIOS_GET_DIRECTION, ( void* )&gpio, sizeof( gpio ) );
            }
            if( supportedIOs & IO_FLASH_MODE_GPIO_2 ){
                std::cout << "IO_FLASH_MODE_GPIO_2 ";
            }
            if( supportedIOs & IO_FLASH_MODE_GPIO_3 ){
                std::cout << "IO_FLASH_MODE_GPIO_3 ";
            }
            if( supportedIOs & IO_FLASH_MODE_GPIO_4 ){
                std::cout << "IO_FLASH_MODE_GPIO_4 ";
            }
            std::cout << std::endl;
        }        
    }

	void UEyeUsbCamera::enableFreerun()
	{
		is_CaptureVideo( _camHandle, IS_DONT_WAIT );
	}

	void UEyeUsbCamera::disableFreerun()
	{
		if( is_StopLiveVideo( _camHandle, IS_WAIT ) == IS_NO_SUCCESS )
			throw CVTException( "Could not stop capture process" );
    }

	void UEyeUsbCamera::enableEvents()
	{
        is_EnableEvent( _camHandle, IS_SET_EVENT_FRAME );
        is_EnableEvent( _camHandle, IS_SET_EVENT_EXTTRIG );
	}

	void UEyeUsbCamera::disableEvents()
	{
        is_DisableEvent( _camHandle, IS_SET_EVENT_FRAME );
        is_DisableEvent( _camHandle, IS_SET_EVENT_EXTTRIG );
	}
}
