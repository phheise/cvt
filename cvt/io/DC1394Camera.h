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

#ifndef DC1394CAMERA_H
#define DC1394CAMERA_H

#include <cvt/io/Camera.h>
#include <dc1394/dc1394.h>
#include <cvt/gfx/Image.h>

namespace cvt
{

	class DC1394Camera : public Camera
	{

		public:
			enum RunMode {
				RUNMODE_CONTINUOUS,
				RUNMODE_SW_TRIGGER,
				RUNMODE_HW_TRIGGER
			};

			enum ExternalTriggerPolarity {
				TRIGGER_ON_FALLING_EDGE = DC1394_TRIGGER_ACTIVE_LOW,
				TRIGGER_ON_RISING_EDGE = DC1394_TRIGGER_ACTIVE_HIGH
				// there are even more options,
				// concerning the exposure time, multi trigger, etc.
			};

			enum ExternalTriggerMode {
				EDGE_TRIGGERED_FIXED_EXPOSURE = DC1394_TRIGGER_MODE_0,
				EDGE_TRIGGERED_EDGE_EXPOSURE,
				EDGE_TRIGGERED_STOP_NTH_EDGE,
				DC1394_TRIGGER_MODE_3,//TODO: lookup meaning of these modes
				DC1394_TRIGGER_MODE_4,
				DC1394_TRIGGER_MODE_5,
				VENDOR_SPEC_TRIGGER_0,
				VENDOR_SPEC_TRIGGER_1
			};

			enum ExternalTriggerSource {
				TRIGGER_SOURCE_0 = DC1394_TRIGGER_SOURCE_0,
				TRIGGER_SOURCE_1 = DC1394_TRIGGER_SOURCE_1,
				TRIGGER_SOURCE_2 = DC1394_TRIGGER_SOURCE_2,
				TRIGGER_SOURCE_3 = DC1394_TRIGGER_SOURCE_3,
				TRIGGER_SOURCE_SOFTWARE = DC1394_TRIGGER_SOURCE_SOFTWARE
			};
			typedef std::vector<ExternalTriggerSource> TriggerSourceVec;

			enum CameraPreset {
				PRESET_FACTORY = 0,
				PRESET_USER0,
				PRESET_USER1
			};

			enum FeatureMode {
                MANUAL = DC1394_FEATURE_MODE_MANUAL,
				AUTO = DC1394_FEATURE_MODE_AUTO,				
				ONE_SHOT = DC1394_FEATURE_MODE_ONE_PUSH_AUTO
			};

            struct Parameters {
                Parameters():
                    numDMABuf( 5 ),
                    runMode( RUNMODE_CONTINUOUS ),
                    usePreset( false ),
                    preset( PRESET_FACTORY ),
                    isoSpeed( 400 )
                {
                }

                Parameters( const Parameters& other ) :
                    numDMABuf( other.numDMABuf ),
                    runMode( other.runMode ),
                    usePreset( other.preset ),
                    preset( other.preset ),
                    isoSpeed( other.isoSpeed )
                {
                }

                uint32_t        numDMABuf;
                RunMode         runMode;
                bool            usePreset;
                CameraPreset    preset;
                uint32_t        isoSpeed;
            };

            DC1394Camera( size_t camIndex, const CameraMode & mode, const Parameters& params = Parameters() );

			~DC1394Camera();

			void startCapture();
			void stopCapture();

			/**
			 * @brief try to update the current frame with one from the ringbuffer
			 * @param timeout 0 -> return if no frame in ringbuffer, > 0 -> wait until a frame is in the buffer
			 * @return whether the current frame has been updated
			 */
			bool			nextFrame( size_t timeout = 30 );

			/**
			 *	@brief when using software triggering
			 *	request camera to capture a frame now
			 */
			void			triggerFrame();

			const Image&	frame() const;
			size_t			framesAvailable() const;

            size_t			width() const { return _frame.width(); }
            size_t			height() const { return _frame.height(); }
            const IFormat&	format() const { return _frame.format(); }
            const String&	identifier() const { return _identifier; }
			void            setRunMode( RunMode mode );
            RunMode         runMode() const;

			void			supportedTriggerSources( TriggerSourceVec& sources ) const;
			ExternalTriggerSource triggerSource() const;
			void			setTriggerSource( ExternalTriggerSource src ) const;

			void			setExternalTriggerMode( ExternalTriggerMode mode );
			ExternalTriggerMode externalTriggerMode() const;

			void			enableExternalTrigger( bool enable );
			bool			isExternalTriggered() const;

			// TODO: this actually is for triggering the camera
			bool			isSoftwareTriggered() const;
			void			setSoftwareTrigger( bool enable );

			void			setRegister( uint64_t address, uint32_t value );
			uint32_t		getRegister( uint64_t address ) const;

			/**
			 * @brief commandRegistersBase
			 * @return base address for command registers
			 */
			uint64_t		commandRegistersBase() const;

			/**
			 * @brief if camera supports switching of polarity from
			 *	      trigger on falling edge (default) to rising
			 * @return whether it's supported or not
			 */
			bool externalTriggerSupportsPolarity() const;

			/**
			 * @brief query polarity of the external trigger
			 * @return polarity of the external trigger
			 */
			ExternalTriggerPolarity externalTriggerPolarity() const;

			void setExternalTriggerPolarity( ExternalTriggerPolarity pol );

			void printAllFeatures();			

			void loadPreset( CameraPreset preset );
			void savePreset( CameraPreset preset = PRESET_USER0 );

            void whiteBalance( uint32_t* ubValue, uint32_t* vrValue );
            void setWhiteBalance( uint32_t ubValue, uint32_t vrValue );

			uint32_t shutter() const;
			float shutterAbs() const;
			void setShutter( uint32_t value );
			void setShutterAbs( float value );

			void enableIrisAuto( bool enable );			

			uint32_t exposureValue() const;
			float exposureValueAbs() const;
			void setExposureValue( uint32_t val );
			void setExposureValueAbs( float val );

			void setWhiteBalanceMode( FeatureMode mode );
			void setShutterMode( FeatureMode mode );
			void setExposureMode( FeatureMode mode );
			void setGainMode( FeatureMode mode );

			void enableAutoExposure( bool v );
			void enableAutoGain( bool v );
			void enableAutoShutter( bool v );
			void enableAutoWhiteBalance( bool v );

			FeatureMode whiteBalanceMode() const;
			FeatureMode shutterMode() const;
			FeatureMode exposureMode() const;
			FeatureMode gainMode() const;

			uint32_t gain() const;
			float gainAbs() const;
			void setGain( uint32_t );
			void setGainAbs( float value );

			void setFrameRate( float fps );
			float frameRate() const;

			void setPacketSize( size_t packetSize );
			size_t packetSize() const;

			void setAreaOfInterest( const Recti& rect );
			Recti areaOfInterest() const;

            void setISOSpeed( uint32_t speed );

            static size_t	count();
            static void		cameraInfo( size_t index, CameraInfo & info );

		private:
			void close();
			void init();
			void reset();			

			/* find fitting dc1394 settings for given camera mode */
			void dcSettings( const CameraMode & mode );
			dc1394video_mode_t dcMode( const CameraMode & mode );

			dc1394framerate_t closestFixedFrameRate( float fps );
			void setBandwidth( float fps );
			
            Parameters  _params;
            size_t      _camIndex;
            Image       _frame;
            float       _fps;
            size_t      _framesBehind;

			bool                 _capturing;
			dc1394_t*            _dcHandle;
			dc1394camera_t*      _camera;
			dc1394video_mode_t   _mode;
			dc1394color_coding_t _colorCoding;
			dc1394color_filter_t _colorFilter;
			dc1394framerate_t    _framerate;
			bool				 _isFormat7;
			String               _identifier;
			RunMode              _runMode;

			dc1394format7modeset_t	_format7Modeset;

            bool hasFeature( dc1394feature_t feature ) const;
            bool isVideoTransmitting() const;
            void setVideoTransmission( bool val ) const;
    };

}

#endif
