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

#include <cvt/io/OpenNICamera.h>

#include <cvt/io/OpenNIManager.h>

namespace cvt
{
    OpenNICamera::OpenNICamera( size_t idx, const CameraMode& mode ) :
        _captureMode( DEPTH_RGB )
    {
        // initialize a context
        XnStatus status = _context.Init();
        if( status != XN_STATUS_OK ){
            throw CVTException( "Could not initialize context" );
        }

        OpenNIManager& manager = OpenNIManager::instance();

        // create the device
        manager.createDeviceForIdx( _device, idx, _context );

        const OpenNIManager::DeviceInformation& devInfo = manager.deviceInfoForCam( idx );
        _identifier = devInfo.name;
        _identifier += "_";
        _identifier += devInfo.serial;

        if( !manager.createImageGeneratorForDevice( _imageGen, idx, _context ) ){
            // device does not support imageGen
            throw CVTException( "Device cannot generate rgb images" );
        }

        if( !manager.createDepthGeneratorForDevice( _depthGen, idx, _context ) ){
            // device does not support imageGen
            throw CVTException( "Device cannot generate depth images" );
        }

        _rgb.reallocate( mode.width, mode.height, mode.format );
        _depth.reallocate( mode.width, mode.height, IFormat::GRAY_UINT16 );

        manager.initializeImageGenerator( _imageGen, mode );
        manager.initializeDepthGenerator( _depthGen, mode );
    }

    OpenNICamera::~OpenNICamera()
    {
        stopCapture();
        _context.StopGeneratingAll();
        _context.Release();
    }


    void OpenNICamera::startCapture()
    {
        switch ( _captureMode ) {
            case DEPTH_ONLY:
                startDepthCapture();
                break;
            case DEPTH_RGB:
                startImageCapture();
                startDepthCapture();
                break;
            case RGB_ONLY:
                startImageCapture();
                break;
            default:
                throw CVTException( "Capture Mode not supported" );
                break;
        }
    }

    void OpenNICamera::stopCapture()
    {
        switch ( _captureMode ) {
            case DEPTH_ONLY:
                stopDepthCapture();
                break;
            case DEPTH_RGB:
                stopDepthCapture();
                stopImageCapture();
                break;
            case RGB_ONLY:
                stopImageCapture();
                break;

            default:
                throw CVTException( "Capture Mode not supported" );
                break;
        }
    }

    bool OpenNICamera::nextFrame( size_t /*timeout*/ )
    {
        XnStatus status = XN_STATUS_OK;

        status = _imageGen.WaitAndUpdateData();
        //status = _context.WaitAndUpdateAll();
        if( status != XN_STATUS_OK ){
            throw CVTException( "Error in WaitAndUpdateData for img" );
        } else {
            copyImage();
        }

        status = _depthGen.WaitAndUpdateData();
        if( status != XN_STATUS_OK ){
            throw CVTException( "Error in WaitAndUpdateData for depth" );
        } else {
            copyDepth();
        }

		return true;
    }

    void OpenNICamera::setSyncRGBDepth( bool val )
    {
        xn::FrameSyncCapability syncCap = _imageGen.GetFrameSyncCap();
        if( syncCap.CanFrameSyncWith( _depthGen ) ){
            bool isSynced = syncCap.IsFrameSyncedWith( _depthGen );
            if( isSynced && !val ){
                syncCap.StopFrameSyncWith( _depthGen );
            }
            if( !isSynced && val ){
                syncCap.FrameSyncWith( _depthGen );
            }
        } else {
            syncCap = _depthGen.GetFrameSyncCap();
            if( syncCap.CanFrameSyncWith( _imageGen ) ){
                bool isSynced = syncCap.IsFrameSyncedWith( _imageGen );
                if( isSynced && !val ){
                    syncCap.StopFrameSyncWith( _imageGen );
                }
                if( !isSynced && val ){
                    syncCap.FrameSyncWith( _imageGen );
                }
            } else {
                std::cout << "Cannot Sync RGB and Depth" << std::endl;
            }
        }
    }

    void OpenNICamera::setRegisterDepthToRGB( bool val )
    {
        xn::AlternativeViewPointCapability cap = _depthGen.GetAlternativeViewPointCap();
        bool canRegisterToRGB = cap.IsViewPointSupported( _imageGen );
        if( canRegisterToRGB ){
            bool isRegistered = cap.IsViewPointAs( _imageGen );
            if( isRegistered && !val ){
                cap.ResetViewPoint();
            } else if( !isRegistered && val ) {
                cap.SetViewPoint( _imageGen );
            }
        } else {
            std::cout << "Cannot register depth to rgb" << std::endl;
        }
    }

    void OpenNICamera::copyDepth()
    {
        _depthGen.GetMetaData( _depthData );

        if( _depthData.XRes() != _depth.width() ||
            _depthData.YRes() != _depth.height() ){
            _depth.reallocate( _depthData.XRes(),
                               _depthData.YRes(),
                               IFormat::GRAY_UINT16 );
        }

        size_t stride;
        uint16_t* ptr = _depth.map<uint16_t>( &stride );

        const XnDepthPixel* dpixel = _depthData.Data();

        SIMD* simd = SIMD::instance();
        if( _depthData.XRes() == stride ){
            simd->Memcpy( ( uint8_t* )ptr, ( const uint8_t* )dpixel, _depth.width() * _depth.height() * sizeof( uint16_t ) );
        } else {
            uint16_t* p = ptr;
            size_t h = _depth.height();
            size_t w = _depth.width();
            size_t bytesPerLine = w * sizeof( uint16_t );

            while( h-- ){
                simd->Memcpy( ( uint8_t* )p, ( const uint8_t* )dpixel, bytesPerLine );
                p += stride;
                dpixel += w;
            }
        }

        _depth.unmap( ptr );
    }

    void OpenNICamera::copyImage()
    {
        _imageGen.GetMetaData( _rgbData );

        size_t w = _rgbData.XRes();
        size_t h = _rgbData.YRes();

        if( w != _rgb.width() ||
            h != _rgb.height() ){
            _depth.reallocate( w, h, IFormat::GRAY_UINT16 );
        }

        size_t stride;
        uint8_t* ptr = _rgb.map( &stride );

        const uint8_t* iptr = _rgbData.Data();
        size_t bytesPerLine = w * _rgbData.BytesPerPixel();

        SIMD* simd = SIMD::instance();
        if( bytesPerLine == stride ){
            simd->Memcpy( ptr, iptr, h * bytesPerLine );
        } else {
            uint8_t* p = ptr;
            while( h-- ){
                simd->Memcpy( p, iptr, bytesPerLine );
                p += stride;
                iptr += bytesPerLine;
            }
        }

        _rgb.unmap( ptr );
    }

    void OpenNICamera::depthFocalLength() const
    {
        XnFieldOfView fov;
        XnStatus status = _depthGen.GetFieldOfView( fov );
        if( status != XN_STATUS_OK )
            throw CVTException( "Could not get FOV" );

        double fx =  _depth.width() / ( 2 * Math::tan( fov.fHFOV / 2.0 ) );
        double fy =  _depth.height() / ( 2 * Math::tan( fov.fVFOV / 2.0 ) );
        std::cout << fx << ", " << fy << std::endl;

        // pix size in mm
        XnDouble pixelSize;
        status = _depthGen.GetRealProperty( "ZPPS", pixelSize );
        if( status != XN_STATUS_OK ){
            String msg;
            msg.sprintf( "Could not get pixel size: %s", xnGetStatusString( status ) );
            throw CVTException( msg.c_str() );
        }

        // focal length in mm
        XnUInt64 depthFocalLengthFullRes;
        status = _depthGen.GetIntProperty( "ZPD", depthFocalLengthFullRes );
        if( status != XN_STATUS_OK ){
            String msg;
            msg.sprintf( "Could not get depth focal length: %s", xnGetStatusString( status ) );
            throw CVTException( msg.c_str() );
        }

        // baseline in cm
        XnDouble baseline;
        status = _depthGen.GetRealProperty( "LDDIS", baseline );
        if( status != XN_STATUS_OK ){
            String msg;
            msg.sprintf( "Could not get baseline: %s", xnGetStatusString( status ) );
            throw CVTException( msg.c_str() );
        }

        //focal length from mm -> pixels (valid for 1280x1024)
        std::cout << "Pixel Size: " << pixelSize << "mm" << std::endl;
        std::cout << "Focal len: " << depthFocalLengthFullRes / pixelSize << "px" << std::endl;
        std::cout << "Baseline: " << baseline << "cm" << std::endl;
    }

    float OpenNICamera::maxDepthRange() const
    {
        XnDepthPixel d = _depthGen.GetDeviceMaxDepth();
        return d / 1000.0f; // in meters
    }

    void OpenNICamera::setCaptureMode( CaptureMode mode )
    {
        if( mode == _captureMode )
            return;
        stopCapture();
        _captureMode = mode;
        startCapture();
    }


    void OpenNICamera::startDepthCapture()
    {
        if( _depthGen.IsGenerating() )
            return;

        XnStatus status = XN_STATUS_OK;
        status = _depthGen.StartGenerating();
        if( status != XN_STATUS_OK ){
            String msg;
            msg.sprintf( "Could not start generating Depth: %s", xnGetStatusString( status ) );
            throw CVTException( msg.c_str() );
        }
    }

    void OpenNICamera::startImageCapture()
    {
        if( _imageGen.IsGenerating() )
            return;

        XnStatus status = XN_STATUS_OK;
        status = _imageGen.StartGenerating();
        if( status != XN_STATUS_OK ){
            String msg;
            msg.sprintf( "Could not start generating Image: %s", xnGetStatusString( status ) );
            throw CVTException( msg.c_str() );
        }
    }

    void OpenNICamera::startIRCapture()
    {
        if( _irGen.IsGenerating() )
            return;

        XnStatus status = XN_STATUS_OK;
        status = _irGen.StartGenerating();
        if( status != XN_STATUS_OK ){
            String msg;
            msg.sprintf( "Could not start generating IR Image: %s", xnGetStatusString( status ) );
            throw CVTException( msg.c_str() );
        }
    }

    void OpenNICamera::stopDepthCapture()
    {
        if( !_depthGen.IsGenerating() )
            return;

        XnStatus status = XN_STATUS_OK;
        status = _depthGen.StopGenerating();
        if( status != XN_STATUS_OK ){
            String msg;
            msg.sprintf( "Could not stop generating Depth: %s", xnGetStatusString( status ) );
            throw CVTException( msg.c_str() );
        }
    }

    void OpenNICamera::stopImageCapture()
    {
        if( !_imageGen.IsGenerating() )
            return;

        XnStatus status = XN_STATUS_OK;
        status = _imageGen.StopGenerating();
        if( status != XN_STATUS_OK ){
            String msg;
            msg.sprintf( "Could not stop generating Image: %s", xnGetStatusString( status ) );
            throw CVTException( msg.c_str() );
        }
    }

    void OpenNICamera::stopIRCapture()
    {
        if( !_irGen.IsGenerating() )
            return;

        XnStatus status = XN_STATUS_OK;
        status = _irGen.StopGenerating();
        if( status != XN_STATUS_OK ){
            String msg;
            msg.sprintf( "Could not stop generating IR Image: %s", xnGetStatusString( status ) );
            throw CVTException( msg.c_str() );
        }
    }

    void OpenNICamera::setAutoExposure( bool )
    {
        if( _imageGen.IsCapabilitySupported( XN_CAPABILITY_EXPOSURE ) ){

            xn::GeneralIntCapability ecap = _imageGen.GetExposureCap();

            XnInt32 min, max, step, def;
            XnBool autoSupport;
            ecap.GetRange( min, max, step, def, autoSupport);

            std::cout << "Exposure: " << min << ", " << max << ", "
                      << step << ", " << def << ", " << autoSupport << std::endl;

        } else {
            std::cout << "Device does not support exposure capability" << std::endl;
        }
    }

    void OpenNICamera::setBacklightCompensation( bool val )
    {
        if( _imageGen.IsCapabilitySupported( XN_CAPABILITY_BACKLIGHT_COMPENSATION ) ){

            xn::GeneralIntCapability cap = _imageGen.GetBacklightCompensationCap();

            XnInt32 min, max, step, def;
            XnBool autoSupport;
            cap.GetRange( min, max, step, def, autoSupport);

            std::cout << "Backlight: " << min << ", " << max << ", "
                      << step << ", " << def << ", " << autoSupport << std::endl;

        } else {
            std::cout << "Device does not support backlight compensation" << std::endl;
        }
    }

    void OpenNICamera::setAntiFlicker( OpenNICamera::AntiFlickerMode mode )
    {
        if( _imageGen.IsCapabilitySupported( XN_CAPABILITY_ANTI_FLICKER ) ){

            xn::AntiFlickerCapability cap = _imageGen.GetAntiFlickerCap();

            XnPowerLineFrequency plf = cap.GetPowerLineFrequency();

            if( ( AntiFlickerMode )plf != mode ){
                cap.SetPowerLineFrequency( ( XnPowerLineFrequency )mode );
            }

        } else {
            std::cout << "Device does not support anti flickering" << std::endl;
        }
    }

    void OpenNICamera::setGain()
    {
        if( _imageGen.IsCapabilitySupported( XN_CAPABILITY_GAIN ) ){

            xn::GeneralIntCapability cap = _imageGen.GetGainCap();
            XnInt32 min, max, step, def;
            XnBool autoSupport;
            cap.GetRange( min, max, step, def, autoSupport );

            std::cout << "Gain: " << min << ", " << max << ", "
                      << step << ", " << def << ", " << autoSupport << std::endl;
        } else {
            std::cout << "Device does not support Gain Capability" << std::endl;
        }

    }

    const Image& OpenNICamera::frame() const
    {
        return _rgb;
    }

    const Image& OpenNICamera::depth() const
    {
        return _depth;
    }

    const String& OpenNICamera::identifier() const
    {
        return _identifier;
    }

    size_t OpenNICamera::count()
    {
        return OpenNIManager::instance().deviceCount();
    }

    void OpenNICamera::cameraInfo( size_t index, CameraInfo& info )
    {
        info = OpenNIManager::instance().cameraInfoForDevice( index );
    }

}
