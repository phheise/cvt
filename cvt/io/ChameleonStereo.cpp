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


#include <cvt/io/ChameleonStereo.h>

#define PG_STROBE_PRESENCE_BIT ( 1 << 31 )
#define PG_STROBE_ON_OFF_BIT ( 1 << 25 )
#define PG_SIG_POLARITY_BIT ( 1 << 24 )

namespace cvt {

    ChameleonStereo::ChameleonStereo( const Parameters &params ) :
        StereoInput(),
        _leftCam( 0 ),
        _rightCam( 0 )
    {
        // init cams
        size_t n = DC1394Camera::count();
        if( n < 2 ){
            throw CVTException( "not enough cameras connected" );
        }

        size_t leftIdx = idxForId( params.leftId );
        size_t rightIdx = idxForId( params.rightId );

        DC1394Camera::Parameters camParams;
        camParams.isoSpeed = 400;
        camParams.numDMABuf = 2;
        camParams.usePreset = true;
        camParams.preset = DC1394Camera::PRESET_USER0;

        // leftcam should run continuous
        camParams.runMode = DC1394Camera::RUNMODE_CONTINUOUS;
        CameraInfo cInfo;
        DC1394Camera::cameraInfo( leftIdx, cInfo );
        CameraMode mode = cInfo.bestMatchingMode( IFormat::BAYER_GBRG_UINT8, 1296, 960, 18 );
        std::cout << mode << std::endl;

        _leftCam = new DC1394Camera( leftIdx, mode, camParams );

        camParams.runMode = DC1394Camera::RUNMODE_HW_TRIGGER;
        _rightCam = new DC1394Camera( rightIdx, mode, camParams );

        _leftCam->startCapture();
        _rightCam->startCapture();
    }

    ChameleonStereo::~ChameleonStereo()
    {
        if( _leftCam ){
            _leftCam->stopCapture();
            delete _leftCam;
        }
        if( _rightCam ){
            _rightCam->stopCapture();
            delete _rightCam;
        }
    }

    bool ChameleonStereo::nextFrame( size_t timeout )
    {
        if( _leftCam->nextFrame( timeout ) ){
            _leftRect   = _leftCam->frame();
            while( !_rightCam->nextFrame( timeout ) )
                ;

            // TODO: undistort rectify with calibration data
            _rightRect  = _rightCam->frame();
            return true;

        }
        return false;
    }

    size_t ChameleonStereo::idxForId( const cvt::String& id ) const
    {
        size_t n = DC1394Camera::count();
        CameraInfo info;

        for( size_t i = 0; i < n; ++i ){
            DC1394Camera::cameraInfo( i, info );
            if( info.identifier() == id ){
                // get this camera
                return info.index();
            }
        }
        throw CVTException( "Could not find requested camera id" );
        return 0;
    }

    void ChameleonStereo::configureLeft( int strobePin )
    {
        // set to asynchronous triggering
        _leftCam->startCapture();
        _leftCam->setRunMode( cvt::DC1394Camera::RUNMODE_SW_TRIGGER );
        _leftCam->enableExternalTrigger( true );
        _leftCam->setExternalTriggerMode( cvt::DC1394Camera::EDGE_TRIGGERED_FIXED_EXPOSURE );
        // trigger from SW
        _leftCam->setTriggerSource( cvt::DC1394Camera::TRIGGER_SOURCE_SOFTWARE );
        configureStrobe( strobePin );
    }

    void ChameleonStereo::configureStrobe( int pin )
    {
        // configure strobePin to output strobe, as long as exposure time
        uint64_t baseAddress = _leftCam->commandRegistersBase();
        static const uint64_t StrobeOutputInq = 0x48C;
        uint32_t strobeAddress = _leftCam->getRegister( baseAddress + StrobeOutputInq );

        strobeAddress = ( strobeAddress << 2 ) & 0xFFFFF;

        uint32_t strobeCtrlInq = _leftCam->getRegister( baseAddress + strobeAddress );

        uint32_t pinOffset = strobeAddress + 0x200 + 4 * pin;

        if( pin > 3 || pin < 0 ){
            throw CVTException( "unknown pin!" );
        }

        if( !( strobeCtrlInq & ( 1 << ( 31 - pin ) ) ) ){
            throw CVTException( "Strobe not present for requested pin" );
        }

        uint32_t strobeReg = _leftCam->getRegister( baseAddress + pinOffset );

        // stop strobe when streaming stops
        strobeReg |= PG_STROBE_ON_OFF_BIT;

        // trigger on rising edge
        strobeReg |= PG_SIG_POLARITY_BIT;
        //strobeReg &= ~PG_SIG_POLARITY_BIT;

        // no delay, strobe length = expose length
        strobeReg &= 0xFF000000;
        _leftCam->setRegister( baseAddress + pinOffset, strobeReg );
    }

    void ChameleonStereo::configureRight( int triggerPin )
    {
        // slave shall trigger asynchronous
        _rightCam->startCapture();
        _rightCam->setRunMode( cvt::DC1394Camera::RUNMODE_HW_TRIGGER );
        _rightCam->enableExternalTrigger( true );
        _rightCam->setExternalTriggerMode( cvt::DC1394Camera::EDGE_TRIGGERED_EDGE_EXPOSURE );
        _rightCam->setExternalTriggerPolarity( cvt::DC1394Camera::TRIGGER_ON_RISING_EDGE );

        // configure to capture frame on signal change on triggerpin
        cvt::DC1394Camera::ExternalTriggerSource triggerSource = cvt::DC1394Camera::TRIGGER_SOURCE_0;
        switch( triggerPin ){
            case 0: triggerSource = cvt::DC1394Camera::TRIGGER_SOURCE_0; break;
            case 1: triggerSource = cvt::DC1394Camera::TRIGGER_SOURCE_1; break;
            case 2: triggerSource = cvt::DC1394Camera::TRIGGER_SOURCE_2; break;
            case 3: triggerSource = cvt::DC1394Camera::TRIGGER_SOURCE_3; break;
            default:
                throw CVTException( "unknown pin number for trigger source" );
        }
        _rightCam->setTriggerSource( triggerSource );
    }

    void ChameleonStereo::setShutter( float val )
    {
        _leftCam->setShutterAbs( val );
        _rightCam->setShutterAbs( val );
    }

    void ChameleonStereo::setGain( float val )
    {
        _leftCam->setGainAbs( val );
        _rightCam->setGainAbs( val );
    }

    void ChameleonStereo::setExposure( float val )
    {
        _leftCam->setExposureValueAbs( val );
        _rightCam->setExposureValueAbs( val );
    }

    void ChameleonStereo::setWhiteBalance( uint32_t ubValue, uint32_t vrValue )
    {
        _leftCam->setWhiteBalance( ubValue, vrValue );
        _rightCam->setWhiteBalance( ubValue, vrValue );
    }

    void ChameleonStereo::setFps( float fps )
    {
        _leftCam->setFrameRate( fps );
        _rightCam->setFrameRate( fps );
    }

    float ChameleonStereo::fps() const
    {
        return _leftCam->frameRate();
    }

    void ChameleonStereo::setPacketSize( size_t n, SubCamera cam )
    {
        switch( cam ){
            case LEFT:
                _leftCam->setPacketSize( n );
                break;
            case RIGHT:
                _rightCam->setPacketSize( n );
                break;
            case BOTH:
                _leftCam->setPacketSize( n );
                _rightCam->setPacketSize( n );
                break;
        }
    }

    size_t ChameleonStereo::packetSize( SubCamera cam ) const
    {
        if( cam == RIGHT )
            return _rightCam->packetSize();
        else
            return _leftCam->packetSize();
    }

    void ChameleonStereo::setAreaOfInterest( const Recti& rect )
    {
        _leftCam->setAreaOfInterest( rect );
        _rightCam->setAreaOfInterest( rect );
    }

    Recti ChameleonStereo::areaOfInterest() const
    {
        return _leftCam->areaOfInterest();
    }

    void ChameleonStereo::setAutoShutter( bool val )
    {
        DC1394Camera::FeatureMode mode = DC1394Camera::MANUAL;
        if( val ){
            mode = DC1394Camera::AUTO;
        }
        _leftCam->setShutterMode( mode );
        _rightCam->setShutterMode( mode );
    }

    void ChameleonStereo::setAutoGain( bool val )
    {
        DC1394Camera::FeatureMode mode = DC1394Camera::MANUAL;
        if( val ){
            mode = DC1394Camera::AUTO;
        }
        _leftCam->setGainMode( mode );
        _rightCam->setGainMode( mode );
    }

    void ChameleonStereo::setAutoExposure( bool val )
    {
        DC1394Camera::FeatureMode mode = DC1394Camera::MANUAL;
        if( val ){
            mode = DC1394Camera::AUTO;
        }
        _leftCam->setExposureMode( mode );
        _rightCam->setExposureMode( mode );
    }

    void ChameleonStereo::setAutoWhiteBalance( bool val )
    {
        DC1394Camera::FeatureMode mode = DC1394Camera::MANUAL;
        if( val ){
            mode = DC1394Camera::AUTO;
        }
        _leftCam->setWhiteBalanceMode( mode );
        _rightCam->setWhiteBalanceMode( mode );
    }

    void ChameleonStereo::enableAutoExposure( bool val )
    {
        _leftCam->enableAutoExposure( val );
        _rightCam->enableAutoExposure( val );
    }

    void ChameleonStereo::enableAutoGain( bool val )
    {
        _leftCam->enableAutoGain( val );
        _rightCam->enableAutoGain( val );
    }

    void ChameleonStereo::enableAutoShutter( bool val )
    {
        _leftCam->enableAutoShutter( val );
        _rightCam->enableAutoShutter( val );
    }

    void ChameleonStereo::enableAutoWhiteBalance( bool val )
    {
        _leftCam->enableAutoWhiteBalance( val );
        _rightCam->enableAutoWhiteBalance( val );
    }

    bool ChameleonStereo::gainMode() const
    {
        return _leftCam->gainMode() == DC1394Camera::AUTO &&
               _rightCam->gainMode() == DC1394Camera::AUTO;
    }

    void ChameleonStereo::loadPreset( DC1394Camera::CameraPreset preset )
    {
        _leftCam->loadPreset( preset );
        _rightCam->loadPreset( preset );
    }
}
