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

#ifndef CVT_UEYE_STEREO_H
#define	CVT_UEYE_STEREO_H

#include <cvt/util/String.h>
#include <cvt/util/Exception.h>
#include <cvt/io/UEyeUsbCamera.h>

namespace cvt
{
	/* convenience class for synced Ueye stereo */
	class UEyeStereo 
	{
		public:
			UEyeStereo( const String& masterId, const String& slaveId, size_t flashDelay = 20, size_t flashDuration = 200 );
			~UEyeStereo();

			/**
			 *	\brief capture the next frame in both cameras
		   	 *	\return	true if a frame could be captured in both views
			 */
			bool nextFrame();
			
			const Image&  masterFrame() const { return _master->frame(); }	
			const Image&  slaveFrame()  const { return _slave->frame(); }	
			
			const String& masterId()    const { return _master->identifier(); }	
            const String& slaveId()     const { return _slave->identifier(); }

			void setPixelClock( size_t mhz );

            void setHorizontalMirror( bool value );
            void setVerticalMirror( bool value );
            void setAutoShutter( bool value );
            void setAutoSensorShutter( bool value );
            void setFramerate( double value );
            void setExposureTime( double value );
            void setAutoGain( bool value );
            void setGainBoost( bool value );

			void setTimeout( size_t to ){ _timeout = to; }

            const UEyeUsbCamera& master() const { return *_master; }
            const UEyeUsbCamera& slave() const { return *_slave; }

		private:
			UEyeUsbCamera*	_master;
			UEyeUsbCamera*	_slave;

            size_t _nM, _nS;
			size_t _timeout;

			void syncCameraSettings();
			void setupSyncing( size_t flashDelay, size_t flashDuration );
	};

	inline UEyeStereo::UEyeStereo( const String& masterId, 
								   const String& slaveId,
								   size_t flashDelay,
								   size_t flashDuration ):
		_master( 0 ),
        _slave( 0 ),
        _nM( 0 ), 
		_nS( 0 ),
		_timeout( 500 )
	{
		size_t numCams = UEyeUsbCamera::count();
		if( numCams < 2 ){
			throw CVTException( "Could not find two ueye cameras!" );
		}

		for( size_t i = 0; i < numCams; i++ ){
			if( _master && _slave )
				break;

            CameraInfo camInfo;
			UEyeUsbCamera::cameraInfo( i, camInfo );            
            UEyeUsbCamera* tmp = new UEyeUsbCamera( camInfo.index(), camInfo.mode( 0 ) );

			if( _master == 0 ){
				if( tmp->identifier() == masterId ){
                    std::cout << "Master FOUND" << std::endl;
					_master = tmp;
					continue;	
				}
			}
			if( _slave == 0 ){
				if( tmp->identifier() == slaveId ){
                    std::cout << "Slave FOUND" << std::endl;
					_slave = tmp;
					continue;	
				}
			}

            delete tmp;
		}

		if( !_master )
			throw CVTException( "Could not find UEye Cam for master id" );
		if( !_slave )
			throw CVTException( "Could not find UEye Cam for slave id" );
			
        syncCameraSettings();
        setupSyncing( flashDelay, flashDuration );

        _slave->startCapture();
        _master->startCapture();

        _slave->nextFrame( 0 );
        _master->nextFrame( 0 );
	}

	UEyeStereo::~UEyeStereo()
	{
		if( _master )
			delete _master;
		if( _slave )
			delete _slave;
	}

	inline void UEyeStereo::setPixelClock( size_t mhz )
	{
		_master->setPixelClock( mhz );
		_slave->setPixelClock( mhz );
	}

    inline void UEyeStereo::setHorizontalMirror( bool value )
    {
        _master->setHorizontalMirror( value );
        _slave->setHorizontalMirror( value );
    }

    inline void UEyeStereo::setVerticalMirror( bool value )
    {
        _master->setVerticalMirror( value );
        _slave->setVerticalMirror( value );
    }

    inline void UEyeStereo::setAutoShutter( bool value )
    {
        _master->setAutoShutter( value );
        _slave->setAutoShutter( value );
    }

    inline void UEyeStereo::setAutoSensorShutter( bool value )
    {
        _master->setAutoSensorShutter( value );
        _slave->setAutoSensorShutter( value );
    }

    inline void UEyeStereo::setFramerate( double value )
    {
        _master->setFramerate( value );
        _slave->setFramerate( value );
    }

    inline void UEyeStereo::setExposureTime( double value )
    {
        _master->setExposureTime( value );
        _slave->setExposureTime( value );
    }


	inline bool UEyeStereo::nextFrame()
    {        
        bool gotM = _master->waitFrame( _timeout );
        bool gotS = _slave->waitFrame( _timeout );

        if( gotM ) _nM++;
        if( gotS ) _nS++;

        if( _nM != _nS )
            std::cout << "Capture Difference: " << _nM - _nS << std::endl;

        _slave->nextFrame( 0 );
        _master->nextFrame( 0 );

        return ( gotM & gotS );
	}

	inline void UEyeStereo::syncCameraSettings()
	{
		// same gains:
		{
			int m, r, g, b;
			_master->getHardwareGains( m, r, g, b );
			_slave->setHardwareGains( m, r, g, b );
		}

		{
            //_master->saveParameters( "tmp.ini" );
            //_slave->loadParameters( "tmp.ini" );
		}
	}

	inline void UEyeStereo::setupSyncing( size_t flashDelay, size_t flashDuration )
	{
		// master is triggered by software!
        _master->setRunMode( UEyeUsbCamera::UEYE_MODE_TRIGGERED );
        _master->setFlashMode( UEyeUsbCamera::FLASH_LOW_ON_EXPOSURE );
        _master->setFlashDelayAndDuration( flashDelay, flashDuration );

		// slave is triggered by hardware (master)!
		_slave->setRunMode( UEyeUsbCamera::UEYE_MODE_HW_TRIGGER );
        _slave->setTriggerMode( UEyeUsbCamera::TRIGGER_HI_LO );
        _slave->setTriggerDelay( 0 );
	}

	inline void UEyeStereo::setAutoGain( bool value )
	{
		_master->setAutoGain( value );
		_slave->setAutoGain( value );
	}

	inline void UEyeStereo::setGainBoost( bool value )
	{
		_master->setGainBoost( value );
		_slave->setGainBoost( value );
	}

}

#endif
