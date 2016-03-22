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

#include <cvt/io/OpenNIManager.h>

namespace cvt {

	std::ostream& operator<<( std::ostream& out, const OpenNIManager::DeviceInformation& dInfo )
	{
		out << "Name: " << dInfo.name;
		out << " Serial: " << dInfo.serial;
		out << " Vendor Specific: " << dInfo.vendorSpecific;
		return out;
	}

	OpenNIManager& OpenNIManager::instance()
	{
		static OpenNIManager _instance;
		return _instance;
	}

	OpenNIManager::OpenNIManager()
	{
		updateDeviceList();
	}

	OpenNIManager::~OpenNIManager()
	{
	}

	void OpenNIManager::updateDeviceList()
	{
		xn::Context context;
		XnStatus status = context.Init();

		if( status != XN_STATUS_OK ){
			throw CVTException( "Error when initializing OpenNI context" );
		}

		xn::NodeInfoList devices;
		status = context.EnumerateProductionTrees( XN_NODE_TYPE_DEVICE, NULL, devices );

		if( status != XN_STATUS_OK ){
            // this causes a problem if no device is connected!
            //throw CVTException( "Error enumerating production trees" );
            return;
		}

		xn::NodeInfoList::Iterator it= devices.Begin();
		xn::NodeInfoList::Iterator itEnd = devices.End();
		
		while( it!= itEnd ){
			xn::Device device;
			
			xn::NodeInfo nodeInfo( *it );
			status = context.CreateProductionTree( nodeInfo, device );

			DeviceInformation devInfo;
			nameAndSerialForDevice( devInfo.name, devInfo.serial, devInfo.vendorSpecific, device );
			
			if( isNewDevice( devInfo.name, devInfo.serial ) ){
				std::cout << "Found new device: kill" << devInfo << std::endl;
				size_t index = _deviceList.size();
				_deviceList.push_back( devInfo );
				fillCameraModes( index, device );
			}

			device.Release();

			it++;
		}
		context.Release();
	}

	void OpenNIManager::nameAndSerialForDevice( String& name, String& serial, String& vendorSpec, xn::Device& device ) const
	{
        const size_t bufsize = 1024;
		char buffer[ bufsize ];

		xn::DeviceIdentificationCapability idCap = device.GetIdentificationCap();

		XnStatus status = idCap.GetDeviceName( buffer, bufsize );
		if( status == XN_STATUS_OK ){
			name = buffer;
		} else {
			name = "UNKNOWN";
		}

		status = idCap.GetSerialNumber( buffer, bufsize );
		if( status == XN_STATUS_OK ){
			serial = buffer;
		} else {
			serial = "UNKNOWN";
		}

		status = idCap.GetVendorSpecificData( buffer, bufsize );
		if( status == XN_STATUS_OK ){
			vendorSpec = buffer;
		} else {
			vendorSpec = "UNKNOWN";
		}
	}

	void OpenNIManager::fillCameraModes( size_t idx, xn::Device& device )
	{
		XnStatus status = XN_STATUS_OK;

		xn::Context			context;
		device.GetContext( context );
		
		// handle image generators
		xn::ImageGenerator	imgGen;
		if( createImageGeneratorForDevice( imgGen, idx, context ) ){
			XnUInt32 numModes = imgGen.GetSupportedMapOutputModesCount();
			XnMapOutputMode modes[ numModes ];

			status = imgGen.GetSupportedMapOutputModes( modes, numModes );
			if( status == XN_STATUS_OK ){
				std::vector<XnMapOutputMode> uniqueModes;
				getUniqueModeList( modes, numModes, uniqueModes );

				String name;
				name += _deviceList[ idx ].name;
				_deviceList[ idx ].rgbInfo.setName( name );
				_deviceList[ idx ].rgbInfo.setIndex( idx );
				_deviceList[ idx ].rgbInfo.setType( CAMERATYPE_OPENNI );
				findSupportedModes( _deviceList[ idx ].rgbInfo , uniqueModes, imgGen );
			}			
		}
		imgGen.Release();
		
		// check all supported image formats etc:
		xn::DepthGenerator	depthGen;
		if( createDepthGeneratorForDevice( depthGen, idx, context ) ){
			// find out about supported format etc.
			XnUInt32 numModes = depthGen.GetSupportedMapOutputModesCount();
			XnMapOutputMode modes[ numModes ];

			status = depthGen.GetSupportedMapOutputModes( modes, numModes );
			if( status == XN_STATUS_OK ){
				std::vector<XnMapOutputMode> uniqueModes;
				getUniqueModeList( modes, numModes, uniqueModes );

				std::cout << "Depth ModeS: " << std::endl;
				for( size_t i = 0; i < uniqueModes.size(); i++ ){
					/*std::cout << uniqueModes[ i ].nXRes << "x"
						      << uniqueModes[ i ].nYRes << "@"
						      << uniqueModes[ i ].nFPS << std::endl;*/
					_deviceList[ idx ].depthInfo.addMode( CameraMode( ( size_t )uniqueModes[ i ].nXRes, 
																	  ( size_t )uniqueModes[ i ].nYRes,
																      ( size_t )uniqueModes[ i ].nFPS, 
																	  IFormat::GRAY_UINT16 ) );
				}
			}
		}
		depthGen.Release();

		xn::IRGenerator	irGen;
		if( createIRGeneratorForDevice( irGen, idx, context ) ){
			// find out about supported format etc.
			// find out about supported format etc.
			XnUInt32 numModes = irGen.GetSupportedMapOutputModesCount();
			XnMapOutputMode modes[ numModes ];

			status = irGen.GetSupportedMapOutputModes( modes, numModes );
			if( status == XN_STATUS_OK ){
				std::vector<XnMapOutputMode> uniqueModes;
				getUniqueModeList( modes, numModes, uniqueModes );
				for( size_t i = 0; i < uniqueModes.size(); i++ ){
					std::cout << uniqueModes[ i ].nXRes << "x"
						      << uniqueModes[ i ].nYRes << "@"
						      << uniqueModes[ i ].nFPS << std::endl;
					_deviceList[ idx ].irInfo.addMode( CameraMode( uniqueModes[ i ].nXRes, uniqueModes[ i ].nYRes,
																   uniqueModes[ i ].nFPS, IFormat::GRAY_UINT16 ) );
				}
			}
		}
		irGen.Release();
		context.Release();
	}
			
	void OpenNIManager::getUniqueModeList( const XnMapOutputMode* modes, size_t num, std::vector<XnMapOutputMode> & unique )
	{
		for( size_t i = 0; i < num; i++ ){
			bool isNew = true;
			for( size_t x = 0; x < unique.size(); x++ ){
				if( unique[ x ].nXRes == modes[ i ].nXRes &&
				    unique[ x ].nYRes == modes[ i ].nYRes &&
				    unique[ x ].nFPS  == modes[ i ].nFPS ){
					isNew = false;
					break;
				}
			}
			if( isNew ){
				unique.push_back( modes[ i ] );
			}
		}
	}
	
	void OpenNIManager::findSupportedModes( CameraInfo& camInfo, const std::vector<XnMapOutputMode> & possibleModes, xn::MapGenerator & gen )
	{
		std::vector<OpenNIImageFormats> formatsToCheck;
		formatsToCheck.push_back( OPENNI_FORMAT_UNCOMPRESSED_YUV422 );
		formatsToCheck.push_back( OPENNI_FORMAT_UNCOMPRESSED_BAYER );
		formatsToCheck.push_back( OPENNI_FORMAT_UNCOMPRESSED_GRAY8 );

		IFormat format = IFormat::GRAY_UINT8;
		for( size_t i = 0; i < formatsToCheck.size(); i++ ){
			if( formatsToCheck[ i ] == OPENNI_FORMAT_UNCOMPRESSED_YUV422 )
				format = IFormat::UYVY_UINT8;
			else if( formatsToCheck[ i ] == OPENNI_FORMAT_UNCOMPRESSED_BAYER )
				format = IFormat::BAYER_GRBG_UINT8;
			else
				format = IFormat::GRAY_UINT8;

			if( gen.SetIntProperty( "InputFormat", formatsToCheck[ i ] ) == XN_STATUS_OK ){
				for( size_t m = 0; m < possibleModes.size(); m++ ){
					if( gen.SetMapOutputMode( possibleModes[ m ] ) == XN_STATUS_OK ){
						camInfo.addMode( CameraMode( possibleModes[ m ].nXRes,
													 possibleModes[ m ].nYRes,
													 possibleModes[ m ].nFPS,
													 format ) );
					}
				}
			}
		}

	}

	bool OpenNIManager::isNewDevice( const String& name, const String& serial ) const
	{
		for( size_t i = 0; i < _deviceList.size(); i++ ){
			if( _deviceList[ i ].name == name &&
				_deviceList[ i ].serial == serial )
				return false;
		}
		return true;
	}
    
    bool OpenNIManager::createDeviceForIdx( xn::Device& device, size_t idx, xn::Context& context )
    {
        xn::NodeInfoList devices;
		XnStatus status = context.EnumerateProductionTrees( XN_NODE_TYPE_DEVICE, NULL, devices );
        
        if( status != XN_STATUS_OK )
            throw CVTException( "Could not enumerate devices" );
        
		xn::NodeInfoList::Iterator it= devices.Begin();
		xn::NodeInfoList::Iterator itEnd = devices.End();
        
		while( it!= itEnd ){            
            xn::NodeInfo n( *it );
			if( nodeBelongsToDeviceIdx( idx, n, context ) ){
                status = context.CreateProductionTree( n, device );
                if( status != XN_STATUS_OK ){
                    throw CVTException( "Could not create production tree for device" ); 
                }
                return true;
            }

			it++;
		}
        return false;
    }


	bool OpenNIManager::createImageGeneratorForDevice( xn::ImageGenerator& generator, size_t idx, xn::Context& context )
	{
		XnStatus status = XN_STATUS_OK;

		xn::NodeInfoList nodeList;
		status = context.EnumerateProductionTrees( XN_NODE_TYPE_IMAGE, NULL, nodeList );

		xn::NodeInfoList::Iterator it = nodeList.Begin();
		xn::NodeInfoList::Iterator itEnd = nodeList.End();

		std::cout << __FUNCTION__ << std::endl;

		while( it != itEnd ){
			xn::NodeInfo n( *it );
			if( nodeBelongsToDeviceIdx( idx, n, context ) ){
				std::cout << "Found node for device " << std::endl;
				// create the image generator from this node and return
				status = context.CreateProductionTree( n, generator );				
				if( status == XN_STATUS_OK ){
					std::cout << "Successfully created production tree" << std::endl;
					return true;
				} else {
					throw CVTException( "Could not create Image Generator for device" );
				}
			}

			it++;
		}

		return false;
	}

	bool OpenNIManager::createDepthGeneratorForDevice( xn::DepthGenerator& generator, size_t idx, xn::Context& context )
	{
		XnStatus status = XN_STATUS_OK;

		xn::NodeInfoList nodeList;
		status = context.EnumerateProductionTrees( XN_NODE_TYPE_DEPTH, NULL, nodeList );

		xn::NodeInfoList::Iterator it = nodeList.Begin();
		xn::NodeInfoList::Iterator itEnd = nodeList.End();

		while( it != itEnd ){
			xn::NodeInfo n( *it );

			if( nodeBelongsToDeviceIdx( idx, n, context ) ){
				// create the image generator from this node and return
				status = context.CreateProductionTree( n, generator );
				if( status == XN_STATUS_OK )
					return true;
				else
					throw CVTException( "Could not create Image Generator for device" );
			}

			it++;
		}

		return false;
	}

	bool OpenNIManager::createIRGeneratorForDevice( xn::IRGenerator& generator, size_t idx, xn::Context& context )
	{
		XnStatus status = XN_STATUS_OK;

		xn::NodeInfoList nodeList;
		status = context.EnumerateProductionTrees( XN_NODE_TYPE_IR, NULL, nodeList );

		xn::NodeInfoList::Iterator it = nodeList.Begin();
		xn::NodeInfoList::Iterator itEnd = nodeList.End();

		while( it != itEnd ){
			xn::NodeInfo n( *it );

			if( nodeBelongsToDeviceIdx( idx, n, context ) ){
				// create the image generator from this node and return
				status = context.CreateProductionTree( n, generator );
				if( status == XN_STATUS_OK )
					return true;
				else
					throw CVTException( "Could not create Image Generator for device" );
			}

			it++;
		}

		return false;
	}
    
    void OpenNIManager::initializeImageGenerator( xn::ImageGenerator& generator, const CameraMode& mode )
    {
        OpenNIImageFormats format = OPENNI_FORMAT_UNCOMPRESSED_YUV422;
        if( mode.format == IFormat::UYVY_UINT8 ){
            format = OPENNI_FORMAT_UNCOMPRESSED_YUV422;
            generator.SetPixelFormat( XN_PIXEL_FORMAT_YUV422 );
        } else if( mode.format == IFormat::BAYER_GRBG_UINT8 ){
            format = OPENNI_FORMAT_UNCOMPRESSED_BAYER;
            generator.SetPixelFormat( XN_PIXEL_FORMAT_GRAYSCALE_8_BIT );
        } else {
            format = OPENNI_FORMAT_UNCOMPRESSED_GRAY8;
            generator.SetPixelFormat( XN_PIXEL_FORMAT_GRAYSCALE_8_BIT );
        }
        
        if( generator.SetIntProperty( "InputFormat", format ) != XN_STATUS_OK ){
            throw CVTException( "Cannot set requested InputFormat for device" );
        }
                
        XnMapOutputMode outputMode;
        outputMode.nXRes = mode.width;
        outputMode.nYRes = mode.height;
        outputMode.nFPS  = mode.fps;
    
        if( generator.SetMapOutputMode( outputMode ) != XN_STATUS_OK ){
            throw CVTException( "Could not set requested Map Outputmode" );
        }
    }
    
    void OpenNIManager::initializeDepthGenerator( xn::DepthGenerator& generator, const CameraMode& mode )
    {
        XnMapOutputMode outputMode;
        outputMode.nXRes = mode.width;
        outputMode.nYRes = mode.height;
        outputMode.nFPS  = mode.fps;
        
        if( generator.SetMapOutputMode( outputMode ) != XN_STATUS_OK ){
            throw CVTException( "Could not set requested Map Outputmode" );
        }   
    }
    
    void OpenNIManager::initializeIRGenerator( xn::IRGenerator& generator, const CameraMode& mode )
    {
        XnMapOutputMode outputMode;
        outputMode.nXRes = mode.width;
        outputMode.nYRes = mode.height;
        outputMode.nFPS  = mode.fps;
        
        if( generator.SetMapOutputMode( outputMode ) != XN_STATUS_OK ){
            throw CVTException( "Could not set requested Map Outputmode" );
        }
    }

	bool OpenNIManager::nodeBelongsToDeviceIdx( size_t idx, xn::NodeInfo& nodeInfo, xn::Context& context ) const 
	{
		xn::NodeInfoList& neededNodes = nodeInfo.GetNeededNodes();
		xn::NodeInfoList::Iterator it = neededNodes.Begin();
		xn::NodeInfoList::Iterator itEnd = neededNodes.End();

		const DeviceInformation& neededDevice = _deviceList[ idx ];

		String currName, currSerial, currVendor;
		XnStatus status = XN_STATUS_OK;
		while( it != itEnd ){
			xn::NodeInfo n( *it );
			const XnProductionNodeDescription& desc = n.GetDescription();

			if( desc.Type == XN_NODE_TYPE_DEVICE ){
				xn::Device dev;
				status = context.CreateProductionTree( n, dev );
                if( status == XN_STATUS_OK ){
                    // get the name
                    nameAndSerialForDevice( currName, currSerial, currVendor, dev );
                    if( currName == neededDevice.name &&
                        currSerial == neededDevice.serial &&
                        currVendor == neededDevice.vendorSpecific ){
                        return true;
                    }
                }
			}

			it++;
		}

		return false;

	}

}
