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

#include "DC1394Camera.h"
#include <cvt/util/Exception.h>

#include <cvt/util/String.h>

namespace cvt
{    
	static IFormat _formatForDC( dc1394color_filter_t filter, dc1394color_coding_t coding ){        
		switch( coding ){
			case DC1394_COLOR_CODING_MONO8:
				return IFormat::GRAY_UINT8;
			case DC1394_COLOR_CODING_YUV422:
				return IFormat::YUYV_UINT8;
			case DC1394_COLOR_CODING_RGB8:
				return IFormat::RGBA_UINT8;
			case DC1394_COLOR_CODING_MONO16:
				return IFormat::RGBA_UINT16;
			case DC1394_COLOR_CODING_RGB16:
				return IFormat::RGBA_UINT16;
			case DC1394_COLOR_CODING_MONO16S:
				return IFormat::GRAY_INT16;
			case DC1394_COLOR_CODING_RGB16S:
				return IFormat::RGBA_INT16;
			case DC1394_COLOR_CODING_RAW8:
				switch( filter ){
					case DC1394_COLOR_FILTER_RGGB:
						return IFormat::BAYER_RGGB_UINT8;
					case DC1394_COLOR_FILTER_GRBG:
						return IFormat::BAYER_GRBG_UINT8;
					case DC1394_COLOR_FILTER_GBRG:
                        return IFormat::BAYER_GBRG_UINT8;
					case DC1394_COLOR_FILTER_BGGR:                    
					default:
                        std::cout << "filter" << ( int )filter << std::endl;
						throw CVTException( "unsupported bayer format" );
				}
            default:
                std::cout << "coding " << ( int )coding << std::endl;
				throw CVTException( "unsupported format" );
		}
	}

    static bool _isFormat7Mode( dc1394video_mode_t mode )
    {
        switch( mode ){
            case DC1394_VIDEO_MODE_FORMAT7_0:
            case DC1394_VIDEO_MODE_FORMAT7_1:
            case DC1394_VIDEO_MODE_FORMAT7_2:
            case DC1394_VIDEO_MODE_FORMAT7_3:
            case DC1394_VIDEO_MODE_FORMAT7_4:
            case DC1394_VIDEO_MODE_FORMAT7_5:
            case DC1394_VIDEO_MODE_FORMAT7_6:
            case DC1394_VIDEO_MODE_FORMAT7_7:
                return true;
            default:
                return false;
        }
    }

    static float _calcFormat7FPS( size_t bytesPerPacket, size_t width, size_t height, float bytesPerPixel )
    {
        return ( float )( bytesPerPacket * 8000 /* 8000 packets per second */ ) / ( float )( width * height * bytesPerPixel );
    }

    DC1394Camera::DC1394Camera( size_t camIndex, const CameraMode & mode, const Parameters& params ) :
        _params( params ),
		_camIndex( camIndex ),
		_frame( mode.width, mode.height, mode.format ),        
		_fps( mode.fps ),
		_capturing( false ),
		_dcHandle( NULL ),
		_camera( NULL ),        
        _runMode( params.runMode )
	{
        _dcHandle = dc1394_new();
		dc1394camera_list_t* list;
		dc1394_camera_enumerate( _dcHandle, &list );

		if( list->num == 0 ) {
			throw CVTException( "No Firewire cameras found" );
		}

		if( list->num <= _camIndex ) {
			throw CVTException(	"No camera with this index found" );
		}

		_camera = dc1394_camera_new( _dcHandle, list->ids[ _camIndex ].guid );
		dc1394_camera_free_list( list );

		if( !_camera )
			throw CVTException( "Could not open camera" );

		// update the cameras format7 modeset:
		for( int i = 0; i < DC1394_VIDEO_MODE_FORMAT7_NUM; i++ ) {
		  _format7Modeset.mode[ i ].present = DC1394_FALSE;
		}
		if( dc1394_format7_get_modeset( _camera, &_format7Modeset ) != DC1394_SUCCESS ){
			throw CVTException( "Could not query format7 modeset from camera" );
		}

		_mode = dcMode( mode );
		setRunMode( _params.runMode );

        if( _params.usePreset ){
            loadPreset( _params.preset );
        }

        setFrameRate( _fps );



		_identifier.sprintf( "%llu", _camera->guid );
	}

	DC1394Camera::~DC1394Camera( )
	{
		close( );
		dc1394_camera_free( _camera );
		dc1394_free( _dcHandle );
	}

	void DC1394Camera::close( )
	{
		if( _capturing ) {
			stopCapture();
		}
	}

	void DC1394Camera::init( )
	{
		if( _capturing ) {
			stopCapture();
			//reset();
			startCapture();
		} else {
			//reset();
		}
	}

	void DC1394Camera::reset( )
	{
		dc1394_camera_reset( _camera );
	}

    void DC1394Camera::setISOSpeed( uint32_t speed )
    {
        dc1394speed_t isoSpeed = DC1394_ISO_SPEED_400;
        switch( speed ){
            case 100:
                isoSpeed = DC1394_ISO_SPEED_100;
                break;
            case 200:
                isoSpeed = DC1394_ISO_SPEED_200;
                break;
            case 400:
                isoSpeed = DC1394_ISO_SPEED_400;
                break;
            case 800:
                isoSpeed = DC1394_ISO_SPEED_800;
                break;
            case 1600:
                isoSpeed = DC1394_ISO_SPEED_1600;
                break;
            case 3200:
                isoSpeed = DC1394_ISO_SPEED_3200;
                break;
        default:
            break;
        }

        dc1394error_t error = dc1394_video_set_iso_speed( _camera, isoSpeed );
        if( error == DC1394_FAILURE ){
            throw CVTException( dc1394_error_get_string( error ) );
        }
    }

	void DC1394Camera::startCapture( )
	{
		if( _capturing )
			return;        

		dc1394error_t error;
		error = dc1394_video_set_mode( _camera, _mode );
		if( error == DC1394_FAILURE ){
			throw CVTException( dc1394_error_get_string( error ) );
		}

        error = dc1394_capture_setup( _camera, _params.numDMABuf, DC1394_CAPTURE_FLAGS_DEFAULT );
		if( error == DC1394_FAILURE ){
			throw CVTException( dc1394_error_get_string( error ) );
		}

		error = dc1394_video_set_transmission( _camera, DC1394_ON );

		if( error == DC1394_FAILURE ){
			throw CVTException( dc1394_error_get_string( error ) );
		}

		_capturing = true;
	}

	void DC1394Camera::stopCapture( )
	{
		if( !_capturing )
			return;

        dc1394error_t error;
        if( _runMode == RUNMODE_CONTINUOUS ){
            error = dc1394_video_set_transmission( _camera, DC1394_OFF );
            if( error != DC1394_SUCCESS ){
                throw CVTException( dc1394_error_get_string( error ) );
            }
        }

        error = dc1394_capture_stop( _camera );
        if( error != DC1394_SUCCESS ){
            throw CVTException( dc1394_error_get_string( error ) );
        }
		_capturing = false;
	}

	bool DC1394Camera::nextFrame( size_t timeout )
	{
		if( !_capturing )
			return false;

        dc1394video_frame_t* frame = 0;
        dc1394error_t error = DC1394_SUCCESS;
		if( timeout > 0 ) {
			int fd = dc1394_capture_get_fileno( _camera );
			fd_set fdset;
			struct timeval tv;

			FD_ZERO( &fdset );
			FD_SET( fd, &fdset );

			tv.tv_sec = timeout / 1000;
            tv.tv_usec = ( timeout % 1000 ) * 1000;
			select( fd + 1, &fdset, 0, 0, &tv );

			if( FD_ISSET( fd, &fdset ) ){
				// can read without blocking
				error = dc1394_capture_dequeue( _camera, DC1394_CAPTURE_POLICY_WAIT, &frame );
				if( error != DC1394_SUCCESS || frame == 0 ){
					return false;
				}
				FD_ZERO( &fdset );
			} else {
				FD_ZERO( &fdset );
				return false;
			}
		} else {
			// don't wait
			error = dc1394_capture_dequeue( _camera, DC1394_CAPTURE_POLICY_POLL, &frame );
			if( error != DC1394_SUCCESS || frame == 0 ){
				return false;
			}
		}

		_framesBehind = frame->frames_behind;
		if( frame->color_coding != DC1394_COLOR_CODING_RAW8 &&
			( _frame.format() == IFormat::BAYER_GRBG_UINT8 || _frame.format() == IFormat::BAYER_RGGB_UINT8 ) ){
			// reallocate to MONO
			_frame.reallocate( frame->size[ 0 ], frame->size[ 1 ], _formatForDC( frame->color_filter, frame->color_coding ) );
		} else if( frame->color_coding == DC1394_COLOR_CODING_RAW8 &&
			( _frame.format() != IFormat::BAYER_GRBG_UINT8 ||
			  _frame.format() != IFormat::BAYER_RGGB_UINT8 ||
			  _frame.format() != IFormat::BAYER_GBRG_UINT8 ) ){
			// reallocate to BAYER
			_frame.reallocate( frame->size[ 0 ], frame->size[ 1 ], _formatForDC( frame->color_filter, frame->color_coding ) );
		}

		size_t stride;
		uint8_t* dst = _frame.map( &stride );

        /* numpixels * bytesperpixel*/
        uint32_t bitsPerPixel;
        error = dc1394_get_color_coding_bit_size( frame->color_coding, &bitsPerPixel );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

        size_t bytesPerRow = ( frame->size[ 0 ] * bitsPerPixel ) >> 3;
        for( size_t i = 0; i < _frame.height(); i++ )
			memcpy( dst + i * stride, frame->image + i * frame->stride, bytesPerRow );
		_frame.unmap( dst );

        error = dc1394_capture_enqueue( _camera, frame );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

		return true;
	}

	size_t DC1394Camera::framesAvailable() const
	{
		return _framesBehind;
	}

	void DC1394Camera::triggerFrame()
	{
		setSoftwareTrigger( true );
		setSoftwareTrigger( false );
	}

    const Image& DC1394Camera::frame() const
	{
		if( !_capturing)
			throw CVTException( "Camera is not in capturing mode!" );
		return _frame;
	}

	void DC1394Camera::setRegister( uint64_t offset, uint32_t value )
	{
		dc1394error_t status = dc1394_set_registers( _camera, offset, &value, 1 );
        if( status != DC1394_SUCCESS ){
            throw CVTException( dc1394_error_get_string( status ) );
        }
	}

	uint32_t DC1394Camera::getRegister( uint64_t offset ) const
	{
		uint32_t val;
		dc1394error_t status = dc1394_get_registers( _camera, offset, &val, 1 );
        if( status != DC1394_SUCCESS ){
            throw CVTException( dc1394_error_get_string( status ) );
        }

		return val;
	}

	uint64_t DC1394Camera::commandRegistersBase() const
	{
		return _camera->command_registers_base;
	}

	void DC1394Camera::loadPreset( CameraPreset preset )
	{
        dc1394error_t error = dc1394_memory_load( _camera, ( uint32_t )preset );
        if( error != DC1394_SUCCESS ){
            throw CVTException( dc1394_error_get_string( error ) );
        }
	}

	void DC1394Camera::savePreset( CameraPreset preset )
	{
		if( preset == PRESET_FACTORY ){
			throw CVTException( "Factory channel is read-only!" );
		}

        dc1394error_t error = dc1394_memory_save( _camera, ( uint32_t )preset );
        if( error != DC1394_SUCCESS ){
            throw CVTException( dc1394_error_get_string( error ) );
        }
	}

	void DC1394Camera::whiteBalance( uint32_t *ubValue, uint32_t *vrValue )
	{
        dc1394error_t error = dc1394_feature_whitebalance_get_value( _camera, ubValue, vrValue );
        if( error != DC1394_SUCCESS ){
            throw CVTException( dc1394_error_get_string( error ) );
        }
	}

    void DC1394Camera::setWhiteBalance( uint32_t ubValue, uint32_t vrValue )
	{
        dc1394error_t error = dc1394_feature_whitebalance_set_value( _camera, ubValue, vrValue );
        if( error != DC1394_SUCCESS ){
            throw CVTException( dc1394_error_get_string( error ) );
        }
	}

	uint32_t DC1394Camera::shutter() const
	{
		uint32_t val;
        dc1394error_t error = dc1394_feature_get_value( _camera, DC1394_FEATURE_SHUTTER, &val );
        if( error != DC1394_SUCCESS ){
            throw CVTException( dc1394_error_get_string( error ) );
        }
		return val;
	}

	float DC1394Camera::shutterAbs() const
	{
		float value;
        dc1394error_t error = DC1394_SUCCESS;

        error = dc1394_feature_set_absolute_control( _camera, DC1394_FEATURE_SHUTTER, DC1394_ON );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

        error = dc1394_feature_get_absolute_value( _camera, DC1394_FEATURE_SHUTTER, &value );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

        error = dc1394_feature_set_absolute_control( _camera, DC1394_FEATURE_SHUTTER, DC1394_OFF );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

		return value;
	}

	void DC1394Camera::setShutter( uint32_t value )
	{
        dc1394error_t error = dc1394_feature_set_value( _camera, DC1394_FEATURE_SHUTTER, value );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
	}

	void DC1394Camera::setShutterAbs( float value )
	{
        dc1394error_t error = DC1394_SUCCESS;
        error = dc1394_feature_set_absolute_control( _camera, DC1394_FEATURE_SHUTTER, DC1394_ON );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

        error = dc1394_feature_set_absolute_value( _camera, DC1394_FEATURE_SHUTTER, value );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

        error = dc1394_feature_set_absolute_control( _camera, DC1394_FEATURE_SHUTTER, DC1394_OFF );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
	}

	void DC1394Camera::enableIrisAuto( bool enable )
	{
		dc1394feature_mode_t mode;

		if( enable ) {
			mode = DC1394_FEATURE_MODE_AUTO;
		} else   {
			mode = DC1394_FEATURE_MODE_MANUAL;
		}

        dc1394error_t error = dc1394_feature_set_mode( _camera, DC1394_FEATURE_IRIS, mode );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
	}

	uint32_t DC1394Camera::exposureValue() const
	{
		uint32_t val;

        dc1394error_t error = dc1394_feature_get_value( _camera, DC1394_FEATURE_EXPOSURE, &val );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

		return val;
	}

	float DC1394Camera::exposureValueAbs() const
	{
		float value;
        dc1394error_t error = DC1394_SUCCESS;
        error = dc1394_feature_set_absolute_control( _camera, DC1394_FEATURE_EXPOSURE, DC1394_ON );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

        error = dc1394_feature_get_absolute_value( _camera, DC1394_FEATURE_EXPOSURE, &value );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

        error = dc1394_feature_set_absolute_control( _camera, DC1394_FEATURE_EXPOSURE, DC1394_OFF );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

		return value;
	}

	void DC1394Camera::setExposureValue( uint32_t val )
	{
        dc1394error_t error = dc1394_feature_set_value( _camera, DC1394_FEATURE_EXPOSURE, val );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
	}

	void DC1394Camera::setExposureValueAbs( float value )
	{
        dc1394error_t error = DC1394_SUCCESS;

        error = dc1394_feature_set_absolute_control( _camera, DC1394_FEATURE_EXPOSURE, DC1394_ON );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

        error = dc1394_feature_set_absolute_value( _camera, DC1394_FEATURE_EXPOSURE, value );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

        error = dc1394_feature_set_absolute_control( _camera, DC1394_FEATURE_EXPOSURE, DC1394_OFF );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
	}

	float DC1394Camera::gainAbs() const
	{
		float value;
        dc1394error_t error = DC1394_SUCCESS;
        error = dc1394_feature_set_absolute_control( _camera, DC1394_FEATURE_GAIN, DC1394_ON );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

        error = dc1394_feature_get_absolute_value( _camera, DC1394_FEATURE_GAIN, &value );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

        error = dc1394_feature_set_absolute_control( _camera, DC1394_FEATURE_GAIN, DC1394_OFF );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

		return value;
	}

	void DC1394Camera::setGainAbs( float value )
	{
        dc1394error_t error = DC1394_SUCCESS;
        error = dc1394_feature_set_absolute_control( _camera, DC1394_FEATURE_GAIN, DC1394_ON );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

        error = dc1394_feature_set_absolute_value( _camera, DC1394_FEATURE_GAIN, value );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

        error = dc1394_feature_set_absolute_control( _camera, DC1394_FEATURE_GAIN, DC1394_OFF );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
	}

	uint32_t DC1394Camera::gain() const
	{
		uint32_t value;        
        dc1394error_t error = dc1394_feature_get_value( _camera, DC1394_FEATURE_GAIN, &value );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
		return value;
	}

	void DC1394Camera::setGain( uint32_t value )
	{
        dc1394error_t error = dc1394_feature_set_value( _camera, DC1394_FEATURE_GAIN, value );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
	}

	void DC1394Camera::setWhiteBalanceMode( FeatureMode mode )
	{
        dc1394error_t error = dc1394_feature_set_mode( _camera, DC1394_FEATURE_WHITE_BALANCE, ( dc1394feature_mode_t )mode );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
	}

	void DC1394Camera::setShutterMode( FeatureMode mode )
	{
        dc1394error_t error = dc1394_feature_set_mode( _camera, DC1394_FEATURE_SHUTTER, ( dc1394feature_mode_t )mode );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
	}

	void DC1394Camera::setExposureMode( FeatureMode mode )
	{
        dc1394error_t error = dc1394_feature_set_mode( _camera, DC1394_FEATURE_EXPOSURE, ( dc1394feature_mode_t )mode );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
	}

	void DC1394Camera::setGainMode( FeatureMode mode )
	{
        dc1394error_t error = dc1394_feature_set_mode( _camera, DC1394_FEATURE_GAIN, ( dc1394feature_mode_t )mode );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
	}

	void DC1394Camera::enableAutoExposure( bool v )
	{
		dc1394switch_t sw = DC1394_OFF;
		if( v )
			sw = DC1394_ON;
		dc1394error_t error = dc1394_feature_set_power( _camera, DC1394_FEATURE_EXPOSURE, sw );
		if( error != DC1394_SUCCESS )
			throw CVTException( dc1394_error_get_string( error ) );
	}

	void DC1394Camera::enableAutoGain( bool v )
	{
		dc1394switch_t sw = DC1394_OFF;
		if( v )
			sw = DC1394_ON;
		dc1394error_t error = dc1394_feature_set_power( _camera, DC1394_FEATURE_GAIN, sw );
		if( error != DC1394_SUCCESS )
			throw CVTException( dc1394_error_get_string( error ) );
	}

	void DC1394Camera::enableAutoShutter( bool v )
	{
		dc1394switch_t sw = DC1394_OFF;
		if( v )
			sw = DC1394_ON;
		dc1394error_t error = dc1394_feature_set_power( _camera, DC1394_FEATURE_SHUTTER, sw );
		if( error != DC1394_SUCCESS )
			throw CVTException( dc1394_error_get_string( error ) );
	}

	void DC1394Camera::enableAutoWhiteBalance( bool v )
	{
		dc1394switch_t sw = DC1394_OFF;
		if( v )
			sw = DC1394_ON;
		dc1394error_t error = dc1394_feature_set_power( _camera, DC1394_FEATURE_WHITE_BALANCE, sw );
		if( error != DC1394_SUCCESS )
			throw CVTException( dc1394_error_get_string( error ) );
	}

	DC1394Camera::FeatureMode DC1394Camera::whiteBalanceMode() const
	{
		dc1394feature_mode_t m;
        dc1394error_t error = dc1394_feature_get_mode( _camera, DC1394_FEATURE_WHITE_BALANCE, &m );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
		return ( FeatureMode )m;
	}

	DC1394Camera::FeatureMode DC1394Camera::shutterMode() const
	{
		dc1394feature_mode_t m;
        dc1394error_t error = dc1394_feature_get_mode( _camera, DC1394_FEATURE_SHUTTER, &m );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
		return ( FeatureMode )m;
	}

	DC1394Camera::FeatureMode DC1394Camera::exposureMode() const
	{
		dc1394feature_mode_t m;
        dc1394error_t error = dc1394_feature_get_mode( _camera, DC1394_FEATURE_EXPOSURE, &m );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
		return ( FeatureMode )m;
	}

	DC1394Camera::FeatureMode DC1394Camera::gainMode() const
	{
		dc1394feature_mode_t m;
        dc1394error_t error = dc1394_feature_get_mode( _camera, DC1394_FEATURE_GAIN, &m );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
		return ( FeatureMode )m;
	}

	void DC1394Camera::setFrameRate( float fps )
	{
		if( _isFormat7 ){
			setBandwidth( fps );
		} else {
			dc1394framerate_t newFR = closestFixedFrameRate( fps );
			dc1394error_t status = dc1394_video_set_framerate( _camera, newFR );
			if( status == DC1394_SUCCESS ){
				_framerate = newFR;
				float f;
				dc1394_framerate_as_float( _framerate, &f );
				_fps = f;
			} else {                
                throw CVTException( dc1394_error_get_string( status ) );
			}
		}
	}

	void DC1394Camera::setBandwidth( float fps )
	{
		// this should use the current AOI later on
		uint32_t w = width();
		uint32_t h = height();

        dc1394error_t error = DC1394_SUCCESS;

        error = dc1394_video_set_mode( _camera, _mode );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

        error = dc1394_format7_set_color_coding( _camera, _mode, _colorCoding );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

        error = dc1394_format7_set_image_position(_camera, _mode, 0, 0 );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

        error = dc1394_format7_set_image_size( _camera, _mode, w, h );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

        error = dc1394_format7_set_color_coding( _camera, _mode, _colorCoding );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

		uint32_t bitsPerPixel = 0;
        error = dc1394_get_color_coding_data_depth( _colorCoding, &bitsPerPixel );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

		size_t bytesPerFrame = ( w * h * bitsPerPixel ) >> 3;

		// we need to calculate the packet size needed to reach the frame rate
		uint32_t pacSize = fps * bytesPerFrame / 8000.0f;        
		setPacketSize( pacSize );
	}

	void DC1394Camera::setPacketSize( size_t pacSize )
	{
		if( !_isFormat7 ){
			return;
		}

		bool wasCapturing = _capturing;

		if( _capturing )
			stopCapture();

		dc1394error_t error;
		uint32_t packetUnits, maxBytes;		
		int modeIdx = _mode - DC1394_VIDEO_MODE_FORMAT7_MIN;
		packetUnits = _format7Modeset.mode[ modeIdx ].unit_packet_size;
		maxBytes = _format7Modeset.mode[ modeIdx ].max_packet_size;

		if( error != DC1394_SUCCESS ){
			if( wasCapturing ) startCapture();
			throw CVTException( dc1394_error_get_string( error ) );
		}

		// pad to unit				
		uint32_t pSize = pacSize;
		uint32_t nPacks = pSize / packetUnits;
		uint32_t min = packetUnits * nPacks;
		uint32_t max = min + packetUnits;
		if( ( pSize - min ) < ( max - pSize ) )
			max = min;
		pSize = Math::min( max, maxBytes );

        error = dc1394_format7_set_packet_size( _camera, _mode, pSize );
        if( error != DC1394_SUCCESS ){
            if( wasCapturing ) startCapture();
            throw CVTException( dc1394_error_get_string( error ) );
        }

		uint32_t actualSize = 0;
		error = dc1394_format7_get_packet_size( _camera, _mode, &actualSize );
		if( error != DC1394_SUCCESS ){
			if( wasCapturing ) startCapture();
			throw CVTException( dc1394_error_get_string( error ) );
		}

		_format7Modeset.mode[ modeIdx ].packet_size = actualSize;

        uint32_t bitsPerPixel = 0;
        error = dc1394_get_color_coding_data_depth( _colorCoding, &bitsPerPixel );
        if( error != DC1394_SUCCESS ){
            if( wasCapturing ) startCapture();
            throw CVTException( dc1394_error_get_string( error ) );
        }
        _fps = _calcFormat7FPS( actualSize, width(), height(), bitsPerPixel >> 3 );

        std::cout << "PacketSize - Req = " << pacSize << " Adj = " << pSize << " actual = " << actualSize << std::endl;

        if( wasCapturing ) startCapture();
    }

	size_t DC1394Camera::packetSize() const
	{
		return _format7Modeset.mode[ _mode - DC1394_VIDEO_MODE_FORMAT7_MIN ].packet_size;
	}

	float DC1394Camera::frameRate() const
	{
		return _fps;
	}

	dc1394framerate_t DC1394Camera::closestFixedFrameRate( float fps )
	{
		dc1394framerates_t framerates;
        dc1394error_t error = DC1394_SUCCESS;

        error = dc1394_video_get_supported_framerates( _camera, _mode, &framerates );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

		float floatFps;
        error = dc1394_framerate_as_float( framerates.framerates[ 0 ], &floatFps );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

		size_t idx = 0;
		float bestDistance = Math::abs( floatFps - fps );
		for( size_t i = 1; i < framerates.num; i++ ){
            error = dc1394_framerate_as_float( framerates.framerates[ i ], &floatFps );
            if( error != DC1394_SUCCESS )
                throw CVTException( dc1394_error_get_string( error ) );

			float currDist = Math::abs( floatFps - fps );

			if( currDist < bestDistance ){
				bestDistance = currDist;
				idx = i;				
			}
		}
		return framerates.framerates[ idx ];
	}

	bool DC1394Camera::isSoftwareTriggered() const
	{
		dc1394switch_t sw;
		dc1394error_t error = dc1394_software_trigger_get_power( _camera, &sw );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );


		return ( sw == DC1394_ON ) ? true : false;
	}

	void DC1394Camera::setSoftwareTrigger( bool enable )
	{
		dc1394switch_t sw = enable ? DC1394_ON : DC1394_OFF;
		dc1394error_t error = dc1394_software_trigger_set_power( _camera, sw );

        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
	}

	void DC1394Camera::enableExternalTrigger( bool enable )
	{
		dc1394switch_t sw = enable ? DC1394_ON : DC1394_OFF;

		dc1394error_t error = dc1394_feature_set_power( _camera, DC1394_FEATURE_TRIGGER, sw );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
	}

	bool DC1394Camera::isExternalTriggered() const
	{
		dc1394switch_t sw;
		dc1394error_t error = dc1394_external_trigger_get_power( _camera, &sw );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
		return ( sw == DC1394_ON ) ? true : false;
	}

	bool DC1394Camera::externalTriggerSupportsPolarity() const
	{
		dc1394bool_t v;
		dc1394error_t error = dc1394_external_trigger_has_polarity( _camera, &v );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

		return ( v == DC1394_TRUE ) ? true : false;
	}

	DC1394Camera::ExternalTriggerPolarity DC1394Camera::externalTriggerPolarity() const
	{
		dc1394trigger_polarity_t pol;
		dc1394error_t error = dc1394_external_trigger_get_polarity( _camera, &pol );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

		return ( ExternalTriggerPolarity )pol;
	}

	void DC1394Camera::setExternalTriggerPolarity( ExternalTriggerPolarity pol )
	{
		dc1394error_t error = dc1394_external_trigger_set_polarity( _camera, ( dc1394trigger_polarity_t )pol );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
	}

	void DC1394Camera::supportedTriggerSources( TriggerSourceVec& srcVec ) const
	{
		dc1394trigger_sources_t sources;
		sources.num = 0;

		dc1394error_t error = dc1394_external_trigger_get_supported_sources( _camera, &sources );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

		for( size_t i = 0; i < sources.num; i++ ){
			srcVec.push_back( ( ExternalTriggerSource )sources.sources[ i ] );
		}
	}

	DC1394Camera::ExternalTriggerSource DC1394Camera::triggerSource() const
	{
		dc1394trigger_source_t src;
		dc1394error_t error = dc1394_external_trigger_get_source( _camera, &src );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

		return ( ExternalTriggerSource )src;
	}

	void DC1394Camera::setTriggerSource( ExternalTriggerSource src ) const
	{
		dc1394error_t error = dc1394_external_trigger_set_source( _camera, ( dc1394trigger_source_t )src );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
	}

	void DC1394Camera::setAreaOfInterest( const Recti& rect )
	{
		// only format 7 modes support this
		if( _isFormat7 ){
			uint32_t packSize = 0;
            dc1394error_t error = dc1394_format7_get_packet_size( _camera, _mode, &packSize );
            if( error != DC1394_SUCCESS )
                throw CVTException( dc1394_error_get_string( error ) );

            error = dc1394_format7_set_roi( _camera,
                                            _mode,
                                            _colorCoding,
                                            packSize,
                                            ( uint32_t )rect.x,
                                            ( uint32_t )rect.y,
                                            ( uint32_t )rect.width,
                                            ( uint32_t )rect.height );
            if( error != DC1394_SUCCESS )
                throw CVTException( dc1394_error_get_string( error ) );
		}
	}

	Recti DC1394Camera::areaOfInterest() const
	{
		Recti aoi( 0, 0, width(), height() );
		if( _isFormat7 ){
			uint32_t l, t, w, h;
			dc1394error_t error = dc1394_format7_get_image_position( _camera, _mode, &l, &t );
			if( error != DC1394_SUCCESS ){
				throw CVTException( dc1394_error_get_string( error ) );
			}

			error = dc1394_format7_get_image_size( _camera, _mode, &w, &h );
			if( error != DC1394_SUCCESS ){
				throw CVTException( dc1394_error_get_string( error ) );
			}
			aoi.x = l;
			aoi.y = t;
			aoi.width = w;
			aoi.height = h;
		}
		return aoi;
	}

	void DC1394Camera::setExternalTriggerMode( ExternalTriggerMode mode )
	{
		dc1394error_t error = dc1394_external_trigger_set_mode( _camera, ( dc1394trigger_mode_t )mode );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
	}

	DC1394Camera::ExternalTriggerMode DC1394Camera::externalTriggerMode() const
	{
		dc1394trigger_mode_t tm;
		dc1394error_t error = dc1394_external_trigger_get_mode( _camera, &tm );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

		return ( ExternalTriggerMode )tm;
	}

	dc1394video_mode_t DC1394Camera::dcMode( const CameraMode & mode ) {
		/* first try to use the format7 mode if possible */
        dc1394error_t error = DC1394_SUCCESS;
		dc1394video_modes_t videoModes;
		error = dc1394_video_get_supported_modes( _camera, &videoModes );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

		for( size_t m = 0; m < videoModes.num; m++ ){
			if( _isFormat7Mode( videoModes.modes[ m ] ) ){
				// get modeinfo
				dc1394format7mode_t f7m;
                error = dc1394_format7_get_mode_info( _camera, videoModes.modes[ m ], &f7m );
                if( error != DC1394_SUCCESS )
                    throw CVTException( dc1394_error_get_string( error ) );

				for( size_t c = 0; c < f7m.color_codings.num; c++ ){
					try {
						IFormat cvtFormat = _formatForDC( f7m.color_filter, f7m.color_codings.codings[ c ] );
						if( f7m.max_size_x == mode.width &&
							f7m.max_size_y == mode.height &&
							mode.format == cvtFormat ){
							_colorCoding = f7m.color_codings.codings[ c ];
							_colorFilter = f7m.color_filter;
							_isFormat7 = true;
							return videoModes.modes[ m ];
						}
					} catch( const cvt::Exception& /*e*/ ){
						//std::cout << "IGNORING UNSUPPORTED CODING" << std::endl;
					}
				}
			}
		}

        // if we reached here, it's a non format 7 mode
		_isFormat7 = false;
		/* check fixed dc sizes */
		if( mode.width == 320 && mode.height == 240 ){
			if ( mode.format == IFormat::YUYV_UINT8 ) {
				return DC1394_VIDEO_MODE_320x240_YUV422;
			} else {
				throw CVTException( "No equivalent DC video mode for requested CameraMode" );
			}
		}

		if( mode.width == 640 && mode.height == 480 ){
			switch ( mode.format.formatID ) {
				case IFORMAT_UYVY_UINT8:
					_colorCoding = DC1394_COLOR_CODING_YUV422;
					return DC1394_VIDEO_MODE_640x480_YUV422;
				case IFORMAT_BAYER_RGGB_UINT8:
					_colorCoding = DC1394_COLOR_CODING_RAW8;
					_colorFilter = DC1394_COLOR_FILTER_RGGB;
					return DC1394_VIDEO_MODE_640x480_MONO8;
				default:
					throw CVTException( "No equivalent dc1394 mode for given CameraMode" );
					break;
			}
		}

		if( mode.width == 800 && mode.height == 600 ){
			switch ( mode.format.formatID ) {
				case IFORMAT_YUYV_UINT8:
					_colorCoding = DC1394_COLOR_CODING_YUV422;
					return DC1394_VIDEO_MODE_800x600_YUV422;
				case IFORMAT_BAYER_RGGB_UINT8:
					_colorCoding = DC1394_COLOR_CODING_RAW8;
					_colorFilter = DC1394_COLOR_FILTER_RGGB;
					return DC1394_VIDEO_MODE_800x600_MONO8;
				default:
					throw CVTException( "No equivalent dc1394 mode for given CameraMode" );
					break;
			}
		}

		if( mode.width == 1024 && mode.height == 768 ){
			switch ( mode.format.formatID ) {
				case IFORMAT_YUYV_UINT8:
					_colorCoding = DC1394_COLOR_CODING_YUV422;
					return DC1394_VIDEO_MODE_1024x768_YUV422;
				case IFORMAT_BAYER_RGGB_UINT8:
					_colorCoding = DC1394_COLOR_CODING_RAW8;
					_colorFilter = DC1394_COLOR_FILTER_RGGB;
					return DC1394_VIDEO_MODE_1024x768_MONO8;
				default:
					throw CVTException( "No equivalent dc1394 mode for given CameraMode" );
					break;
			}
		}

		if( mode.width == 1280 && mode.height == 960 ){
			switch ( mode.format.formatID ) {
				case IFORMAT_YUYV_UINT8:
					_colorCoding = DC1394_COLOR_CODING_YUV422;
					return DC1394_VIDEO_MODE_1280x960_YUV422;
				case IFORMAT_BAYER_RGGB_UINT8:
					_colorCoding = DC1394_COLOR_CODING_RAW8;
					_colorFilter = DC1394_COLOR_FILTER_RGGB;
					return DC1394_VIDEO_MODE_1280x960_MONO8;
				default:
					throw CVTException( "No equivalent dc1394 mode for given CameraMode" );
					break;
			}
		}
		if( mode.width == 1600 && mode.height == 1200 ){
			switch ( mode.format.formatID ) {
				case IFORMAT_YUYV_UINT8:
					_colorCoding = DC1394_COLOR_CODING_YUV422;
					return DC1394_VIDEO_MODE_1600x1200_YUV422;
				case IFORMAT_GRAY_UINT8:
					_colorCoding = DC1394_COLOR_CODING_RAW8;
					_colorFilter = DC1394_COLOR_FILTER_RGGB;
					return DC1394_VIDEO_MODE_1600x1200_MONO8;
				default:
					throw CVTException( "No equivalent dc1394 mode for given CameraMode" );
					break;
			}
		}

		throw CVTException( "No equivalent dc1394 mode for given CameraMode" );
	}

	void DC1394Camera::setRunMode( RunMode mode )
	{
		if( mode == _runMode )
			return;

        dc1394error_t error = DC1394_SUCCESS;
		switch( mode ){
			case RUNMODE_CONTINUOUS:
                error = dc1394_video_set_transmission( _camera, DC1394_ON );
				break;
			case RUNMODE_SW_TRIGGER:
				dc1394_video_set_transmission( _camera, DC1394_OFF );
				break;
			case RUNMODE_HW_TRIGGER:
				dc1394_video_set_transmission( _camera, DC1394_OFF );
				break;
			default:
				throw CVTException( "unkown runmode!" );
		}
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
		_runMode = mode;
	}

	DC1394Camera::RunMode DC1394Camera::runMode() const
	{
		return _runMode;
	}

	size_t DC1394Camera::count()
	{
		size_t numCameras = 0;

		dc1394camera_list_t* list;
        dc1394error_t error = DC1394_SUCCESS;
		dc1394_t* handle = dc1394_new( );
        if( !handle )
            throw CVTException( "Could not create dc1394 handle" );

        error = dc1394_camera_enumerate( handle, &list );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

		numCameras = list->num;
		dc1394_camera_free_list( list );
		dc1394_free( handle );
		return numCameras;
	}

	void DC1394Camera::cameraInfo( size_t index, CameraInfo & info )
	{
		info.setIndex( index );
		info.setType( CAMERATYPE_DC1394 );

		dc1394camera_list_t* list;
        dc1394error_t error = DC1394_SUCCESS;

        dc1394_t* handle = dc1394_new();
        if( !handle )
            throw CVTException( "Could not create dc1394 handle" );

        error = dc1394_camera_enumerate( handle, &list );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

		if( index > list->num )
			throw CVTException( "Camera index out of bounds" );

		dc1394camera_t * cam = 0;
		cam = dc1394_camera_new( handle, list->ids[ index ].guid );

		if( !cam )
			throw CVTException( "Could not create camera handle" );

		String name;
		name += cam->vendor;
		name += " ";
		name += cam->model;
		info.setName( name );

		String camId;
		camId.sprintf( "%llu", cam->guid );
		info.setIdentifier( camId );

		// get supported frame formats + speeds		
		dc1394video_modes_t videoModes;
		error = dc1394_video_get_supported_modes( cam, &videoModes );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

		for( unsigned int i = 0; i < videoModes.num; i++ ){
			IFormat cvtFormat = IFormat::BGRA_UINT8;
            uint32_t width = 0, height = 0;
			bool fixedFrameRate = true;
			switch ( videoModes.modes[ i ] ) {
				case DC1394_VIDEO_MODE_320x240_YUV422:
					cvtFormat = IFormat::UYVY_UINT8;
					width = 320; height = 240;
					break;
				case DC1394_VIDEO_MODE_640x480_YUV422:
					cvtFormat = IFormat::UYVY_UINT8;
					width = 640; height = 480;
					break;
				case DC1394_VIDEO_MODE_640x480_MONO8:{
					cvtFormat = IFormat::BAYER_RGGB_UINT8;
					width = 640; height = 480;
					break;
					}
				case DC1394_VIDEO_MODE_800x600_YUV422:
					cvtFormat = IFormat::UYVY_UINT8;
					width = 800; height = 600;
					break;
				case DC1394_VIDEO_MODE_800x600_MONO8:
					cvtFormat = IFormat::BAYER_RGGB_UINT8;
					width = 800; height = 600;
					break;
				case DC1394_VIDEO_MODE_1024x768_YUV422:
					cvtFormat = IFormat::UYVY_UINT8;
					width = 1024; height = 768;
					break;
				case DC1394_VIDEO_MODE_1024x768_MONO8:
					cvtFormat = IFormat::BAYER_RGGB_UINT8;
					width = 1024; height = 768;
					break;
				case DC1394_VIDEO_MODE_1280x960_YUV422:
					cvtFormat = IFormat::UYVY_UINT8;
					width = 1280; height = 960;
					break;
				case DC1394_VIDEO_MODE_1280x960_MONO8:
					cvtFormat = IFormat::BAYER_RGGB_UINT8;
					width = 1280; height = 960;
					break;
				case DC1394_VIDEO_MODE_1600x1200_YUV422:
					cvtFormat = IFormat::UYVY_UINT8;
					width = 1600; height = 1200;
					break;
				case DC1394_VIDEO_MODE_1600x1200_MONO8:
					cvtFormat = IFormat::BAYER_RGGB_UINT8;
					width = 1600; height = 1200;
					break;
				case DC1394_VIDEO_MODE_1600x1200_MONO16:
					cvtFormat = IFormat::GRAYALPHA_UINT16;
					width = 1600; height = 1200;
					break;
				case DC1394_VIDEO_MODE_FORMAT7_0:
					fixedFrameRate = false;
					break;
				case DC1394_VIDEO_MODE_FORMAT7_1:
					fixedFrameRate = false;
					break;
				case DC1394_VIDEO_MODE_FORMAT7_2:
					fixedFrameRate = false;
					break;
				case DC1394_VIDEO_MODE_FORMAT7_3:
					fixedFrameRate = false;
					break;
				case DC1394_VIDEO_MODE_FORMAT7_4:
					fixedFrameRate = false;
					break;
				case DC1394_VIDEO_MODE_FORMAT7_5:
					fixedFrameRate = false;
					break;
				case DC1394_VIDEO_MODE_FORMAT7_6:
					fixedFrameRate = false;
					break;
				case DC1394_VIDEO_MODE_FORMAT7_7:
					fixedFrameRate = false;
					break;
				case DC1394_VIDEO_MODE_160x120_YUV444:
				case DC1394_VIDEO_MODE_640x480_YUV411:
				case DC1394_VIDEO_MODE_EXIF:
				default:
					continue;
					break;
			}

			if( fixedFrameRate ){
				dc1394framerates_t framerates;
                error = dc1394_video_get_supported_framerates( cam, videoModes.modes[ i ], &framerates );
                if( error != DC1394_SUCCESS )
                    throw CVTException( dc1394_error_get_string( error ) );

				size_t fps = 0;
				for( size_t f = 0; f < framerates.num; f++ ){
					switch ( framerates.framerates[ f ]) {
						case DC1394_FRAMERATE_1_875:
							fps = 2;
							break;
						case DC1394_FRAMERATE_3_75:
							fps = 4;
							break;
						case DC1394_FRAMERATE_7_5:
							fps = 8;
							break;
						case DC1394_FRAMERATE_15:
							fps = 15;
							break;
						case DC1394_FRAMERATE_30:
							fps = 30;
							break;
						case DC1394_FRAMERATE_60:
							fps = 60;
							break;
						case DC1394_FRAMERATE_120:
							fps = 120;
							break;
						case DC1394_FRAMERATE_240:
							fps = 240;
							break;
						default:
							break;
					}
					info.addMode( CameraMode( width, height, fps, cvtFormat ) );
				}
			} else {
                dc1394color_codings_t codings;
                dc1394color_filter_t filter;
                uint32_t packetUnit, maxPacket;

                error = dc1394_format7_get_max_image_size( cam, videoModes.modes[ i ], &width, &height );
                if( error != DC1394_SUCCESS )
                    throw CVTException( dc1394_error_get_string( error ) );

                error = dc1394_format7_get_packet_parameters( cam, videoModes.modes[ i ], &packetUnit, &maxPacket );
                if( error != DC1394_SUCCESS )
                    throw CVTException( dc1394_error_get_string( error ) );

                error = dc1394_format7_get_color_codings( cam, videoModes.modes[ i ], &codings );
                if( error != DC1394_SUCCESS )
                    throw CVTException( dc1394_error_get_string( error ) );

                error = dc1394_format7_get_color_filter( cam, videoModes.modes[ i ], &filter );
                if( error != DC1394_SUCCESS )
                    throw CVTException( dc1394_error_get_string( error ) );

                for( size_t c = 0; c < codings.num; c++ ){
					try {
						// get the matching cvt format
                        cvtFormat = _formatForDC( filter, codings.codings[ c ] );

						// get the bits per pixel that go over the bus for this color coding
						uint32_t bitsPerPixel = 0;
                        error = dc1394_get_color_coding_data_depth( codings.codings[ c ], &bitsPerPixel );
                        if( error != DC1394_SUCCESS )
                            throw CVTException( dc1394_error_get_string( error ) );

						// compute the maximum possible framerate
                        float fps = _calcFormat7FPS( maxPacket, width, height, ( float )bitsPerPixel / 8.0f );

						info.addMode( CameraMode( width, height, fps, cvtFormat ) );
					} catch( const cvt::Exception& /*e*/ ){
						//std::cout << "Skipping dc1394 video mode: " << e.what() << std::endl;
					}
				}
			}
		}

		dc1394_camera_free( cam );
		dc1394_camera_free_list( list );
		dc1394_free( handle );
	}

	bool DC1394Camera::hasFeature( dc1394feature_t feature ) const
	{
		dc1394bool_t value;
		dc1394error_t error = dc1394_feature_is_present( _camera, feature, &value );
		if( error != DC1394_SUCCESS )
			throw CVTException( dc1394_error_get_string( error ) );
		if( value == DC1394_TRUE )
			return true;
		return false;
	}

	void DC1394Camera::printAllFeatures()
	{
		dc1394featureset_t featureSet;
		dc1394error_t error = dc1394_feature_get_all( _camera, &featureSet );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );

        error = dc1394_feature_print_all( &featureSet, stdout );
        if( error != DC1394_SUCCESS )
            throw CVTException( dc1394_error_get_string( error ) );
	}

	bool DC1394Camera::isVideoTransmitting() const
	{
		dc1394error_t error;
		dc1394switch_t ison;
		error = dc1394_video_get_transmission( _camera, &ison );
		return ( ison == DC1394_ON ) ? true : false;
	}

	void DC1394Camera::setVideoTransmission( bool val ) const
	{
		bool isOn = isVideoTransmitting();
		dc1394error_t error;
		if( val ){
			if( !isOn ){
				// turn on
				error = dc1394_video_set_transmission( _camera, DC1394_ON );
				if( error == DC1394_FAILURE ){
					throw CVTException( dc1394_error_get_string( error ) );
				}
			}
		} else {
			if( isOn ){
				error = dc1394_video_set_transmission( _camera, DC1394_OFF );
				if( error == DC1394_FAILURE ){
					throw CVTException( dc1394_error_get_string( error ) );
				}
			}
		}
	}
}
