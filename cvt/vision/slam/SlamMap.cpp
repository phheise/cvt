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

#include <cvt/vision/slam/SlamMap.h>

#include <set>

namespace cvt
{
    SlamMap::SlamMap() :
        _numMeas( 0 )
    {
    }

    SlamMap::~SlamMap()
    {
    }

    void SlamMap::clear()
    {
        _keyframes.clear();
        _features.clear();
        _numMeas = 0;
    }

    size_t SlamMap::addKeyframe( const Eigen::Matrix4d& pose )
    {
        size_t id = _keyframes.size();
        _keyframes.push_back( Keyframe( pose, id ) );
        return id;
    }


    size_t SlamMap::addFeature( const MapFeature& world )
    {
        _features.push_back( world );
        return _features.size()-1;
    }

    size_t SlamMap::addFeatureToKeyframe( const MapFeature& world,
                                          const MapMeasurement& feature,
                                          size_t keyframeId )
    {
        size_t pointId = addFeature( world );
        addMeasurement( pointId, keyframeId, feature );
        return pointId;
    }

    void SlamMap::addMeasurement( size_t pointId,
                                  size_t keyframeId,
                                  const  MapMeasurement& meas )
    {
        _features[ pointId ].addPointTrack( keyframeId );
        _keyframes[ keyframeId ].addFeature( meas, pointId );
        _numMeas++;
    }

    int SlamMap::findClosestKeyframe( const Eigen::Matrix4d& worldT ) const
    {
        double nearest = Math::MAXF;
        int kf = -1;

        for( size_t i = 0; i < _keyframes.size(); i++ ){
            double dist = _keyframes[ i ].distance( worldT );
            if( dist < nearest ){
                nearest = dist;
                kf = i;
            }
        }
        return kf;
    }

    void SlamMap::selectVisibleFeatures( std::vector<size_t> & visibleFeatureIds,
                                         std::vector<Vector2f> & projections,
                                         const Eigen::Matrix4d&	cameraPose /* this is the rig pose*/,
                                         const CameraCalibration& camCalib,
                                         double maxDistance ) const
    {
        Eigen::Vector4d pointInCam;
        Vector4f pic;
        Vector4f sp;
        Vector2f pointInScreen;

        // this is a hack: we should store the image width/height with the calibration object!
        size_t w = camCalib.width();
        size_t h = camCalib.height();

        std::set<size_t> usedPoints;
        for( size_t i = 0; i < _keyframes.size(); i++ ){
            double kfDistance = _keyframes[ i ].distance( cameraPose );
            if( kfDistance < maxDistance ){
                // check if the points of this keyframe project to this camera
                const Keyframe& kf = _keyframes[ i ];

                Keyframe::MeasurementIterator iter = kf.measurementsBegin();
                const Keyframe::MeasurementIterator measEnd = kf.measurementsEnd();

                while( iter != measEnd ){
                    size_t fId = iter->first;
                    const MapFeature& feature = _features[ fId ];

                    if( usedPoints.find( fId ) == usedPoints.end() ){
                        pointInCam = cameraPose * feature.estimate();
                        pointInCam /= pointInCam[ 3 ];

                        if( pointInCam[ 2 ] > 0.0 ){
                            pic[ 0 ] = ( float )pointInCam[ 0 ];
                            pic[ 1 ] = ( float )pointInCam[ 1 ];
                            pic[ 2 ] = ( float )pointInCam[ 2 ];
                            pic[ 3 ] = ( float )pointInCam[ 3 ];

                            // project it to the screen:
                            sp = camCalib.projectionMatrix() * pic;
                            pointInScreen.x = sp.x / sp.z;
                            pointInScreen.y = sp.y / sp.z;

                            if( pointInScreen.x > 0 &&
                                pointInScreen.x < w &&
                                pointInScreen.y > 0 &&
                                pointInScreen.y < h ){
                                usedPoints.insert( fId );
                                visibleFeatureIds.push_back( fId );
                                projections.push_back( pointInScreen );
                            }
                        }
                    }
                    ++iter;
                }
            }
        }
    }

    void SlamMap::deserialize( XMLNode* node )
    {
        if( node->name() != "SlamMap" ){
            throw CVTException( "This is not a SlamMap node" );
        }

        // get intrinsics:
        Matrix3d K;
        K = Matrix3d::fromString( node->childByName( "Intrinsics" )->child( 0 )->value() );
        EigenBridge::toEigen( _intrinsics, K );

        XMLNode* keyframes = node->childByName( "Keyframes" );
        if( keyframes == NULL ){
            throw CVTException( "No Keyframes in MapFile!" );
        }

        size_t numKF = keyframes->childSize();
        _keyframes.resize( numKF );
        _numMeas = 0;
        for( size_t i = 0; i < _keyframes.size(); i++ ){
            XMLNode* kfNode = keyframes->child( i );

            // get the id:
            size_t kfId = kfNode->childByName( "id" )->value().toInteger();

            _keyframes[ kfId ].deserialize( kfNode );
            _numMeas += _keyframes[ kfId ].numMeasurements();
        }

        XMLNode* featureNodes = node->childByName( "MapFeatures" );
        if( featureNodes == NULL ){
            throw CVTException( "No Features in MapFile!" );
        }

        _features.resize( featureNodes->childSize() );
        for( size_t i = 0; i < _features.size(); i++ ){
            XMLNode* fNode = featureNodes->child( i );
            _features[ i ].deserialize( fNode );
        }
    }

    XMLNode* SlamMap::serialize() const
    {
        XMLElement* mapNode = new XMLElement( "SlamMap");

        // Intrinsics of the Keyframe images
        {
            Matrix3d K;
            EigenBridge::toCVT( K, _intrinsics );
            XMLElement* camIntrinsics = new XMLElement( "Intrinsics" );
            camIntrinsics->addChild( new XMLText( K.toString() ) );
            mapNode->addChild( camIntrinsics );
        }

        // the keyframes: serialize each one
        XMLElement* keyframeNodes = new XMLElement( "Keyframes" );
        for( size_t i = 0; i < _keyframes.size(); i++ ){
            keyframeNodes->addChild( _keyframes[ i ].serialize() );
        }
        mapNode->addChild( keyframeNodes );


        // the mapfeatures
        XMLElement* mapFeatureNodes = new XMLElement( "MapFeatures" );
        for( size_t i = 0; i < _features.size(); i++ ){
            mapFeatureNodes->addChild( _features[ i ].serialize() );
        }
        mapNode->addChild( mapFeatureNodes );

        return mapNode;
    }

    void SlamMap::load( const String& filename )
    {
        XMLDocument xmlDoc;
        xmlDoc.load( filename );

        XMLNode* node = xmlDoc.nodeByName( "SlamMap" );
        this->deserialize( node );
    }

    void SlamMap::save( const String& filename ) const
    {
        XMLDocument doc;
        doc.addNode( this->serialize() );
        doc.save( filename );
    }

    void SlamMap::loadBinary( const cvt::String& filename )
    {
        if( !FileSystem::exists( filename ) ){
            throw CVTException( "File not found" );
        }
        std::ifstream file( filename.c_str(), std::ios_base::in | std::ios_base::binary );

        uint32_t nFeatures, nKeyframes, nMeas;

        file.read( ( char* )&nKeyframes, sizeof( uint32_t ) );
        file.read( ( char* )&nFeatures, sizeof( uint32_t ) );
        file.read( ( char* )&nMeas, sizeof( uint32_t ) );

        // write intrinsics:
        Matrix3d K;
        file.read( ( char* )K.ptr(), 9 * sizeof( double ) );
        EigenBridge::toEigen( _intrinsics, K );

        // keyframes
        cvt::Quaterniond q;
        Eigen::Matrix4d mat;
        for( size_t i = 0; i < nKeyframes; i++ ){
            file.read( ( char*)&q.x, sizeof( double ) );
            file.read( ( char*)&q.y, sizeof( double ) );
            file.read( ( char*)&q.z, sizeof( double ) );
            file.read( ( char*)&q.w, sizeof( double ) );
            EigenBridge::toEigen( mat, q.toMatrix4() );
            file.read( ( char*)&mat( 0, 3 ), sizeof( double ) );
            file.read( ( char*)&mat( 1, 3 ), sizeof( double ) );
            file.read( ( char*)&mat( 2, 3 ), sizeof( double ) );
            addKeyframe( mat );
        }

        // features
        MapFeature mapFeature;
        Matrix4d covariance;
        for( size_t i = 0; i < nFeatures; i++ ){
            file.read( ( char*)&mapFeature.estimate()[ 0 ], sizeof( double ) );
            file.read( ( char*)&mapFeature.estimate()[ 1 ], sizeof( double ) );
            file.read( ( char*)&mapFeature.estimate()[ 2 ], sizeof( double ) );
            file.read( ( char*)&mapFeature.estimate()[ 3 ], sizeof( double ) );
            file.read( ( char*)covariance.ptr(), sizeof( double ) * 16 );
            EigenBridge::toEigen( mapFeature.covariance(), covariance );

            addFeature( mapFeature );
        }

        std::vector<Vector2d> measurements( nMeas );
        std::vector<Matrix2d> measInformations( nMeas );
        std::vector<uint32_t> camIndices( nMeas );
        std::vector<uint32_t> pointIndices( nMeas );

        file.read( ( char* )&measurements[ 0 ].x, 2 * nMeas * sizeof( double ) );
        file.read( ( char* )&measInformations[ 0 ][ 0 ], 4 * nMeas * sizeof( double ) );
        file.read( ( char* )&camIndices[ 0 ], nMeas * sizeof( uint32_t ) );
        file.read( ( char* )&pointIndices[ 0 ], nMeas * sizeof( uint32_t ) );
        file.close();

        MapMeasurement mapMeasurement;
        for( size_t i = 0; i < nMeas; i++ ){
            EigenBridge::toEigen( mapMeasurement.point, measurements[ i ] );
            EigenBridge::toEigen( mapMeasurement.information, measInformations[ i ] );
            addMeasurement( pointIndices[ i ], camIndices[ i ], mapMeasurement );
        }
    }

    void SlamMap::saveBinary( const cvt::String& filename ) const
    {
        std::ofstream out( filename.c_str(), std::ios_base::out | std::ios_base::binary );

        uint32_t nFeatures  = ( uint32_t )_features.size();
        uint32_t nKeyframes = ( uint32_t )_keyframes.size();
        uint32_t nMeas      = ( uint32_t )_numMeas;

        // write a header: 3xuint32: #keyframes, #features, #measurements
        out.write( ( const char* )&nKeyframes, sizeof( nKeyframes ) );
        out.write( ( const char* )&nFeatures, sizeof( nFeatures ) );
        out.write( ( const char* )&nMeas, sizeof( nMeas ) );

        std::vector<Vector2d> measurements;
        std::vector<Matrix2d> measInformations;
        std::vector<uint32_t> camIdxForMeas;
        std::vector<uint32_t> featIdxForMeas;
        measurements.resize( nMeas );
        measInformations.resize( nMeas );
        camIdxForMeas.resize( nMeas );
        featIdxForMeas.resize( nMeas );

        // write intrinsics:
        Matrix3d K;
        EigenBridge::toCVT( K, intrinsics() );
        out.write( ( const char* )K.ptr(), 9 * sizeof( double ) );

        // write the cameras
        cvt::Matrix4d poseMat;
        size_t measId = 0;
        for( size_t i = 0; i < nKeyframes; ++i ){
            const Keyframe& k = _keyframes[ i ];
            const Eigen::Matrix4d& m = k.pose().transformation();
            cvt::EigenBridge::toCVT( poseMat, m );
            cvt::Quaterniond q( poseMat.toMatrix3() );
            out.write( ( const char* )&q.x, sizeof( double ) );
            out.write( ( const char* )&q.y, sizeof( double ) );
            out.write( ( const char* )&q.z, sizeof( double ) );
            out.write( ( const char* )&q.w, sizeof( double ) );
            out.write( ( const char* )&poseMat[ 0 ][ 3 ], sizeof( double ) );
            out.write( ( const char* )&poseMat[ 1 ][ 3 ], sizeof( double ) );
            out.write( ( const char* )&poseMat[ 2 ][ 3 ], sizeof( double ) );

            // get all features:
            Keyframe::MeasurementIterator measIter = k.measurementsBegin();
            while( measIter != k.measurementsEnd() ){
                const MapMeasurement& meas = measIter->second;

                Vector2d& currMeas = measurements[ measId ];
                currMeas[ 0 ] = meas.point[ 0 ];
                currMeas[ 1 ] = meas.point[ 1 ];

                EigenBridge::toCVT( measInformations[ measId ], meas.information );

                camIdxForMeas[ measId ]  = k.id();
                featIdxForMeas[ measId ] = measIter->first;

                ++measId;
                ++measIter;
            }
        }

        // write features & covariance
        Matrix4d covariance;
        for( size_t i = 0; i < nFeatures; ++i ){
            const MapFeature& m = _features[ i ];
            const Eigen::Vector4d& f = m.estimate();
            EigenBridge::toCVT( covariance, m.covariance() );
            out.write( ( const char* )&f[ 0 ], sizeof( double ) );
            out.write( ( const char* )&f[ 1 ], sizeof( double ) );
            out.write( ( const char* )&f[ 2 ], sizeof( double ) );
            out.write( ( const char* )&f[ 3 ], sizeof( double ) );
            out.write( ( const char* )covariance.ptr(), 16 * sizeof( double ) );
        }

        // measurements
        out.write( ( const char* )&measurements[ 0 ].x, 2 * sizeof( double ) * nMeas );
        out.write( ( const char* )&measInformations[ 0 ][ 0 ], 4 * sizeof( double ) * nMeas );
        out.write( ( const char* )&camIdxForMeas[ 0 ], sizeof( uint32_t ) * nMeas );
        out.write( ( const char* )&featIdxForMeas[ 0 ], sizeof( uint32_t ) * nMeas );
        out.close();
    }
}
