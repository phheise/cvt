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

#include <cvt/io/OpenNI2Camera.h>
#include <cvt/gfx/IMapScoped.h>

namespace cvt {

    class Openni2Helper {
        public:
            static Openni2Helper& instance()
            {
                static Openni2Helper _instance;
                return _instance;
            }

            ~Openni2Helper()
            {
                openni::OpenNI::shutdown();
            }

            size_t numDevices() const { return _camInfos.size(); }
            const CameraInfo& cameraInfo( size_t idx ) const { return _camInfos[ idx ]; }
            const openni::DeviceInfo& deviceInfo( size_t idx ) const { return _deviceInfos[ idx ];}

            static const openni::VideoMode& findMatchingVideoMode(const CameraMode& cm, openni::Device &dev , openni::SensorType sensor );

            static openni::PixelFormat toPixelFormat( const IFormat& iformat )
            {
                switch( iformat.formatID ){
                    case IFORMAT_RGBA_UINT8:  return openni::PIXEL_FORMAT_RGB888;
                    case IFORMAT_GRAY_UINT8:  return openni::PIXEL_FORMAT_GRAY8;
                    case IFORMAT_GRAY_UINT16: return openni::PIXEL_FORMAT_GRAY16;
                    case IFORMAT_UYVY_UINT8:  return openni::PIXEL_FORMAT_YUV422;
                    //case IFORMAT_YUYV_UINT8:  return openni::PIXEL_FORMAT_YUYV;
                    default: throw CVTException( "no matching openni format" );
                }
            }

            static const IFormat& toIFormat( openni::PixelFormat fmt )
            {
                switch( fmt ){
                    case openni::PIXEL_FORMAT_DEPTH_1_MM:
                    case openni::PIXEL_FORMAT_DEPTH_100_UM:
                    case openni::PIXEL_FORMAT_GRAY16:   return IFormat::GRAY_UINT16;
                    case openni::PIXEL_FORMAT_GRAY8:    return IFormat::GRAY_UINT8;
                    case openni::PIXEL_FORMAT_RGB888:   return IFormat::RGBA_UINT8;
                    case openni::PIXEL_FORMAT_YUV422:   return IFormat::UYVY_UINT8;
                    //case openni::PIXEL_FORMAT_YUYV:     return IFormat::YUYV_UINT8;
                    case openni::PIXEL_FORMAT_SHIFT_9_2: // TODO: support the shift formats?
                    case openni::PIXEL_FORMAT_SHIFT_9_3:
                    case openni::PIXEL_FORMAT_JPEG:
                    default:    throw CVTException( "unsupported pixelformat" );
                }
            }

            static void toCVTImage( Image& dst, const openni::VideoFrameRef& frame )
            {
                dst.reallocate( frame.getWidth(), frame.getHeight(), Openni2Helper::toIFormat( frame.getVideoMode().getPixelFormat() ) );

                switch( frame.getVideoMode().getPixelFormat() ){
                    case openni::PIXEL_FORMAT_RGB888:
                        copyRGB( dst, ( const uint8_t* )frame.getData(), frame.getStrideInBytes() );
                        break;
                    default:
                        copyData( dst, ( const uint8_t* )frame.getData(), frame.getStrideInBytes() );
                }
            }

        private:
            Openni2Helper()
            {
                openni::OpenNI::initialize();
                openni::OpenNI::enumerateDevices( &_deviceInfos );

                _camInfos.resize( _deviceInfos.getSize() );
                for( size_t i = 0; i < _deviceInfos.getSize(); ++i ){
                    CameraInfo& c = _camInfos[ i ];
                    const openni::DeviceInfo& devInfo = _deviceInfos[ i ];
                    c.setName(  devInfo.getName() );
                    c.setIndex( i );
                    c.setType( CAMERATYPE_OPENNI2 );
                    String id;
                    id.sprintf( "%s_%s", devInfo.getVendor(), devInfo.getUri() );
                    c.setIdentifier( id );

                    // add the supported modes
                    openni::Device d;
                    d.open( devInfo.getUri() );
                    const openni::SensorInfo* sinfo = d.getSensorInfo( openni::SENSOR_COLOR );
                    const openni::Array<openni::VideoMode>& vmodes = sinfo->getSupportedVideoModes();
                    for( size_t i = 0; i < vmodes.getSize(); ++i ){
                        const openni::VideoMode& vm = vmodes[ i ];
                        try {
                            c.addMode( CameraMode( vm.getResolutionX(), vm.getResolutionY(), vm.getFps(), toIFormat( vm.getPixelFormat() ) ) );
                        } catch( const cvt::Exception& e ){}
                    }
                    d.close();
                }
            }

            Openni2Helper( const Openni2Helper& );
            Openni2Helper& operator=( const Openni2Helper& );

            openni::Array<openni::DeviceInfo> _deviceInfos;
            std::vector<CameraInfo>           _camInfos;

            static void copyRGB( Image& dst, const uint8_t* p, size_t pStride )
            {
                IMapScoped<uint8_t> map( dst );
                size_t h = dst.height();
                SIMD* simd = SIMD::instance();
                size_t n = dst.width() * 3;
                while( h-- ){
                    simd->Conv_XXXu8_to_XXXAu8( map.ptr(), p, n );
                    map++;
                    p += pStride;
                }
            }

            static void copyData( Image& dst, const uint8_t* p, size_t pStride )
            {
                IMapScoped<uint8_t> map( dst );
                size_t h = dst.height();
                SIMD* simd = SIMD::instance();
                size_t cStride = Math::min( map.stride(), pStride );
                while( h-- ){
                    simd->Memcpy( map.ptr(), p, cStride );
                    map++;
                    p += pStride;
                }
            }

            static const openni::VideoMode& findExactMode( const CameraMode& cm, const openni::Array<openni::VideoMode>& vm );
            static const openni::VideoMode& findApproximateMode( const CameraMode& cm, const openni::Array<openni::VideoMode>& vm );
    };

    static void dumpVM( const openni::VideoMode& vm ){
        std::cout << "VideoMode: [" << vm.getResolutionX() << ", " << vm.getResolutionY() << "] @ " << vm.getFps();
        switch( vm.getPixelFormat() ){
            case openni::PIXEL_FORMAT_DEPTH_100_UM: std::cout << " DEPTH_100_UM" << std::endl; return;
            case openni::PIXEL_FORMAT_DEPTH_1_MM:   std::cout << " DEPTH_1_MM" << std::endl; return;
            case openni::PIXEL_FORMAT_SHIFT_9_2:    std::cout << " PIXEL_FORMAT_SHIFT_9_2" << std::endl;
            case openni::PIXEL_FORMAT_SHIFT_9_3:    std::cout << " PIXEL_FORMAT_SHIFT_9_3" << std::endl;
            case openni::PIXEL_FORMAT_GRAY8:        std::cout << " GRAY_U8" << std::endl; return;
            case openni::PIXEL_FORMAT_GRAY16:       std::cout << " GRAY_U16" << std::endl; return;
            case openni::PIXEL_FORMAT_RGB888:       std::cout << " RGB_888" << std::endl; return;
            //case openni::PIXEL_FORMAT_YUYV:       std::cout << " YUYV" << std::endl; return;
            case openni::PIXEL_FORMAT_YUV422:       std::cout << " YUV422" << std::endl; return;
            case openni::PIXEL_FORMAT_JPEG:         std::cout << " JPEG" << std::endl; return;
            default:
                std::cout << " unknown pxformat" << std::endl;
        }
    }

    const openni::VideoMode& Openni2Helper::findMatchingVideoMode( const CameraMode& cm, openni::Device& dev, openni::SensorType sensor )
    {
        const openni::Array<openni::VideoMode>& vm = dev.getSensorInfo( sensor )->getSupportedVideoModes();
        if( sensor == openni::SENSOR_COLOR )
            return findExactMode( cm, vm);
        else
            return findApproximateMode( cm, vm );
    }

    const openni::VideoMode& Openni2Helper::findExactMode( const CameraMode& cm, const openni::Array<openni::VideoMode>& vm )
    {
        for( size_t i = 0; i < vm.getSize(); i++ ){
            if( vm[ i ].getResolutionX() == cm.width &&
                vm[ i ].getResolutionY() == cm.height &&
                vm[ i ].getFps() == cm.fps &&
                vm[ i ].getPixelFormat() == toPixelFormat( cm.format ) )
                return vm[ i ];
        }
        throw CVTException( "Could not find matching openni videomode for requested cvt CameraMode" );
    }

    const openni::VideoMode& Openni2Helper::findApproximateMode( const CameraMode& cm, const openni::Array<openni::VideoMode>& vm )
    {
        size_t bestIdx = 0;
        int bestDist = 1000000;
        for( size_t i = 0; i < vm.getSize(); i++ ){
            if( vm[ i ].getPixelFormat() == openni::PIXEL_FORMAT_DEPTH_1_MM ){
            //if( vm[ i ].getPixelFormat() == openni::PIXEL_FORMAT_DEPTH_100_UM ){
                int dist = Math::abs( vm[ i ].getResolutionX() - ( int )cm.width ) +
                           Math::abs( vm[ i ].getResolutionY() - ( int )cm.height ) +
                           Math::abs( vm[ i ].getFps() - ( int )cm.fps );
                if( dist < bestDist ){
                    bestDist = dist;
                    bestIdx = i;
                }
            }
        }
        std::cout << "DepthMode: ";
        dumpVM( vm[ bestIdx ] );
        return vm[ bestIdx ];
    }

    OpenNI2Camera::OpenNI2Camera( size_t idx, const CameraMode& mode )
    {
        const openni::DeviceInfo& dinfo = Openni2Helper::instance().deviceInfo( idx );
        _device.open( dinfo.getUri() );

        if( _device.hasSensor( openni::SENSOR_COLOR ) ){
            _rgbStream.create( _device, openni::SENSOR_COLOR );
            openni::Status status = _rgbStream.setVideoMode( Openni2Helper::findMatchingVideoMode( mode, _device, openni::SENSOR_COLOR ) );
            if( status != openni::STATUS_OK ){
                std::cout << "Error: " << openni::OpenNI::getExtendedError() << std::endl;
            }
            setRGBMirroring( false );
        }
        if( _device.hasSensor( openni::SENSOR_DEPTH ) ){
            _depthStream.create( _device, openni::SENSOR_DEPTH );
            openni::Status status = _depthStream.setVideoMode( Openni2Helper::findMatchingVideoMode( mode, _device, openni::SENSOR_DEPTH ) );
            if( status != openni::STATUS_OK ){
                std::cout << "Error: " << openni::OpenNI::getExtendedError() << std::endl;
            }
            setDepthMirroring( false );
        }

        _identifier.sprintf( "%s_%s_%02d", dinfo.getVendor(), dinfo.getName(), idx );
    }
    
    OpenNI2Camera::~OpenNI2Camera()
    {
        _rgbStream.destroy();
        _depthStream.destroy();
        _device.close();
    }

    void OpenNI2Camera::startCapture()
    {
        if( _rgbStream.isValid() ){
            std::cout << "Start capture" << std::endl;
            openni::Status status = _rgbStream.start();
            if( status != openni::STATUS_OK )
                throw CVTException( "ERROR starting RGB stream" );
        }
        if( _depthStream.isValid() ){
            openni::Status status = _depthStream.start();
            if( status != openni::STATUS_OK )
                throw CVTException( "ERROR starting depth stream" );
        }
    }

    void OpenNI2Camera::stopCapture()
    {
        _rgbStream.stop();
        _depthStream.stop();
    }

    bool OpenNI2Camera::nextFrame( size_t timeOut )
    {
        openni::VideoStream* streams[] = { &_rgbStream, &_depthStream };
        int readyIdx = 0;
        openni::Status status = openni::OpenNI::waitForAnyStream( streams, 2, &readyIdx, ( int )timeOut );
        if( status == openni::STATUS_TIME_OUT )
            return false;
        else if( status == openni::STATUS_OK ){
            openni::VideoFrameRef frameRef;
            _rgbStream.readFrame( &frameRef );
            Openni2Helper::toCVTImage( _rgb, frameRef );
            _rgbStamp = frameRef.getTimestamp();

            _depthStream.readFrame( &frameRef );
            Openni2Helper::toCVTImage( _depth, frameRef );
            _depthStamp = frameRef.getTimestamp();
            return true;
        } else {
            return false;
        }
    }
    
    void OpenNI2Camera::setSyncRGBDepth( bool val )
    {
        openni::Status status = _device.setDepthColorSyncEnabled( val );
        if( status != openni::STATUS_OK ){
            std::cout << "Could not set syncronization - maybe device does not support frame syncronization";
        }
    }
    
    void OpenNI2Camera::setRegisterDepthToRGB( bool val )
    {
        openni::ImageRegistrationMode mode = val ? openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR : openni::IMAGE_REGISTRATION_OFF;
        openni::Status status = _device.setImageRegistrationMode( mode );
        if( status != openni::STATUS_OK ){
            std::cout << "Could not set registration - maybe device does not support frame registration";
        }
    }
    
    size_t OpenNI2Camera::count()
    {
        return Openni2Helper::instance().numDevices();
    }
    
    void OpenNI2Camera::cameraInfo( size_t index, CameraInfo & info )
    {
        info = Openni2Helper::instance().cameraInfo( index );
    }

    void OpenNI2Camera::setAutoExposure( bool val )
    {
        openni::Status status = _rgbStream.getCameraSettings()->setAutoExposureEnabled( val );
        if( status != openni::STATUS_OK ){
            std::cout << "Error: " << openni::OpenNI::getExtendedError() << std::endl;
        }
    }

    void OpenNI2Camera::setRGBMirroring( bool val )
    {
        openni::Status status = _rgbStream.setMirroringEnabled( val );
        if( status != openni::STATUS_OK ){
            std::cout << "Error: " << openni::OpenNI::getExtendedError() << std::endl;
        }
    }
    
    void OpenNI2Camera::setDepthMirroring( bool val )
    {
        openni::Status status = _depthStream.setMirroringEnabled( val );
        if( status != openni::STATUS_OK ){
            std::cout << "Error: " << openni::OpenNI::getExtendedError() << std::endl;
        }
    }

    bool OpenNI2Camera::isRGBMirroring() const
    {
        return _rgbStream.getMirroringEnabled();
    }
    
    bool OpenNI2Camera::isDepthMirroring() const
    {
        return _depthStream.getMirroringEnabled();
    }

    void OpenNI2Camera::setAutoWhiteBalance( bool val )
    {
        openni::Status status = _rgbStream.getCameraSettings()->setAutoWhiteBalanceEnabled( val );
        if( status != openni::STATUS_OK ){
            std::cout << "Error: " << openni::OpenNI::getExtendedError() << std::endl;
        }
    }

    bool OpenNI2Camera::autoExposure()
    {
        return _rgbStream.getCameraSettings()->getAutoExposureEnabled();
    }

    bool OpenNI2Camera::autoWhiteBalance()
    {
        return _rgbStream.getCameraSettings()->getAutoWhiteBalanceEnabled();
    }

    int OpenNI2Camera::exposure()
    {
        //return _rgbStream.getCameraSettings()->getExposure();
        return 0;
    }

    void OpenNI2Camera::setExposure( int val )
    {
//        openni::Status status = _rgbStream.getCameraSettings()->setExposure( val );
//        if( status != openni::STATUS_OK ){
//            std::cout << "Error: " << openni::OpenNI::getExtendedError() << std::endl;
//        }
    }

    int OpenNI2Camera::gain()
    {
//        return _rgbStream.getCameraSettings()->getGain();
        return 0;
    }

    void OpenNI2Camera::setGain( int val )
    {
//        openni::Status status = _rgbStream.getCameraSettings()->setGain( val );
//        if( status != openni::STATUS_OK ){
//            std::cout << "Error: " << openni::OpenNI::getExtendedError() << std::endl;
//        }
    }
}
