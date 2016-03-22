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

#ifndef CVT_UEYEUSBCAMERA_H
#define CVT_UEYEUSBCAMERA_H

#include <cvt/io/Camera.h>
#include <cvt/util/String.h>
#include <ueye.h>

namespace cvt
{

class UEyeUsbCamera : public Camera
    {
        public:
            enum RunMode {
                UEYE_MODE_TRIGGERED,
                UEYE_MODE_FREERUN,
                UEYE_MODE_HW_TRIGGER
            };

            enum FlashMode {
                FLASH_PERMANENT_HIGH = IO_FLASH_MODE_CONSTANT_HIGH,
                FLASH_PERMANENT_LOW = IO_FLASH_MODE_CONSTANT_LOW,
                FLASH_HIGH_ON_EXPOSURE = IO_FLASH_MODE_TRIGGER_HI_ACTIVE,
                FLASH_LOW_ON_EXPOSURE = IO_FLASH_MODE_TRIGGER_LO_ACTIVE,
                FLASH_HIGH_ON_EXPOSURE_FR = IO_FLASH_MODE_FREERUN_HI_ACTIVE,
                FLASH_LOW_ON_EXPOSURE_FR = IO_FLASH_MODE_FREERUN_LO_ACTIVE
            };

            enum TriggerMode {
                TRIGGER_OFF         = IS_SET_TRIGGER_OFF,
                TRIGGER_SOFTWARE    = IS_SET_TRIGGER_SOFTWARE,
                TRIGGER_HI_LO       = IS_SET_TRIGGER_HI_LO, /* Trigger on falling edge */
                TRIGGER_LO_HI       = IS_SET_TRIGGER_LO_HI, /* Trigger on raising edge */                
                TRIGGER_HI_LO_SYNC  = IS_SET_TRIGGER_HI_LO_SYNC, /* Trigger on falling edge in freerun */
                TRIGGER_LO_HI_SYNC  = IS_SET_TRIGGER_LO_HI_SYNC /* Trigger on raising edge in freerun */
            };

            UEyeUsbCamera( size_t camIndex, const CameraMode& mode );
            virtual ~UEyeUsbCamera();

            void			startCapture();
            void			stopCapture();
			bool			nextFrame( size_t timeout = 50 );
            bool            waitFrame( size_t timeout = 50 );
            const Image&	frame() const;

            static size_t	count();
			static void		cameraInfo( size_t index, CameraInfo & info );
			size_t			width() const;
			size_t			height() const;
			const IFormat&	format() const;
			const String&	identifier() const;
            
			void	setFramerate( double fps );
			void	setAutoGain( bool value );
			void	setGainBoost( bool value );
            void	setAutoWhiteBalance( bool value );
			void	setWhiteBalanceOnce();
            void	setAutoShutter( bool value );
            void	setAutoSensorShutter( bool value );
            void	setMaxAutoShutter( double value );
            void	setExposureTime( double value );
            void	setPixelClock( unsigned int value );
            void    pixelClockRange( unsigned int& min, unsigned int& max, unsigned int& step ) const;
            void	setHorizontalMirror( bool value );
            void	setVerticalMirror( bool value );

			void	getHardwareGains( int& master, int& red, int& green, int& blue );
			void	setHardwareGains( int master, int red, int green, int blue );
			
            void	setRunMode( RunMode mode );            

            void    setTriggerMode( TriggerMode mode );
            void    setFlashMode( FlashMode mode );

            void    setTriggerDelay( size_t microSecs );
            void    setFlashDelayAndDuration( int32_t delayMuSecs, size_t durationMuSecs );

			void	saveParameters( const String& filename ) const;
			void	loadParameters( const String& filename );

            void    testIOSupport();

            bool    waitTriggerEvent( size_t timeout );

            void    setAreaOfInterest( const Recti& roi );

        private:
            bool	initCam();
            void	open( const CameraMode & mode );
			void	initMemories( const CameraMode & mode );
			void	freeMemories();
			void	exposureRange( double& min, double& max );
            int		bufNumForAddr( const uint8_t * buffAddr ) const;
			void	setIdentifier();


			void	enableFreerun();
			void	disableFreerun();			
			void	enableEvents();
			void	disableEvents();

            void    bufferToFrame();

            int             _camIndex;

            // uEye variables
            HIDS            _camHandle;		// handle to camera
            INT             _width;			// width of image
            INT             _height;		// height of image
            INT             _stride;		// stride of image

            static const size_t	_numImageBuffers = 5;
            uint8_t*		_buffers[ _numImageBuffers ];
            INT				_bufferIds[ _numImageBuffers ];

            Image           _frame;
            String          _identifier;
            RunMode         _runMode;
    };

	inline size_t UEyeUsbCamera::width() const
	{
		return _frame.width();
	}

	inline size_t UEyeUsbCamera::height() const
	{
		return _frame.height();
	}

	inline const IFormat & UEyeUsbCamera::format() const
	{
		return _frame.format();
	}
}
#endif
