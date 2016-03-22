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


#include <cvt/io/KittiVOParser.h>
#include <cvt/util/DataIterator.h>
#include <cvt/util/Exception.h>
#include <cvt/io/FileSystem.h>

namespace cvt {

    KittiVOParser::KittiVOParser( const cvt::String& folder, bool useColorCams ) :
        _useColor( useColorCams ),
        _iter( 0 )
    {
        cvt::String leftFolder( folder );
        cvt::String rightFolder( folder );
        cvt::String calibFile( folder );
        cvt::String timesFile( folder );
        cvt::String poseFile( folder );

        if( !_useColor ){
            leftFolder  += "/image_0/";
            rightFolder += "/image_1/";
        } else {
            leftFolder  += "/image_2/";
            rightFolder += "/image_3/";
        }

        calibFile   += "/calib.txt";
        timesFile   += "/times.txt";
        poseFile    += "/poses.txt";

        checkFileExistence( leftFolder );
        checkFileExistence( rightFolder );
        checkFileExistence( calibFile );
        checkFileExistence( timesFile );

        bool hasPoseFile = true;
        try {
            checkFileExistence( poseFile );
        } catch( const cvt::Exception&e ){
            hasPoseFile = false;
        }

        std::vector<cvt::String>   filesLeft;
        std::vector<cvt::String>   filesRight;
        std::vector<cvt::Matrix4d> poses;
        std::vector<double>        stamps;

        loadImageNames( filesLeft, leftFolder );
        loadImageNames( filesRight, rightFolder );

        if( hasPoseFile ){
            std::cout << "Loading poses: " << std::endl;
            loadPoses( poses, poseFile );
        }
        std::cout << "Loading stamps: " << std::endl;
        loadStamps( stamps, timesFile );

        // check sizes:
        size_t n = filesLeft.size();
        if( n != filesRight.size() ){
            throw CVTException( "Unequal number of images in subfolders" );
        }

        if( hasPoseFile && ( n != poses.size() ) ){
            throw CVTException( "Unequal number of images and poses" );
        }

        if( n != stamps.size() ){
            throw CVTException( "Unequal number of images and stamps" );
        }

        _sequence.resize( n );
        for( size_t i = 0; i < n; i++ ){
            _sequence[ i ].timestamp    = stamps[ i ];
            _sequence[ i ].leftFile     = filesLeft[ i ];
            _sequence[ i ].rightFile    = filesRight[ i ];
            if( hasPoseFile ){
                _sequence[ i ].hasPose      = true;
                _sequence[ i ].pose         = poses[ i ];
            } else {
                _sequence[ i ].hasPose      = false;
            }
        }

        _curSample = &_sequence[ _iter ];
        loadImages();

        // get the stereo calibration: we need the images sizes, that's why it has to be done after the load
        std::cout << "Loading calibFile: " << std::endl;
        loadCalibration( calibFile );
    }

    KittiVOParser::~KittiVOParser()
    {
    }

    bool KittiVOParser::nextFrame( size_t /*timeout*/ )
    {
        if( hasNext() ){
            _iter++;
            _curSample = &_sequence[ _iter ];
            loadImages();
            return true;
        }
        return false;
    }

    bool KittiVOParser::hasNext() const
    {
        return ( _iter + 1 ) < _sequence.size();
    }    

    void KittiVOParser::checkFileExistence( const cvt::String& file )
    {
        if( !cvt::FileSystem::exists( file ) ){
            cvt::String msg;
            msg.sprintf( "File <%s> not found!\n", file.c_str() );
            throw CVTException( msg.c_str() );
        }
    }

    void KittiVOParser::loadImages()
    {
        _left.load( _curSample->leftFile );
        _right.load( _curSample->rightFile );
    }

    void KittiVOParser::loadImageNames( std::vector<cvt::String>& names, const cvt::String& folder )
    {
        cvt::FileSystem::filesWithExtension( folder, names, "png" );

        std::sort( names.begin(), names.end() );
    }

    void KittiVOParser::loadPoses( std::vector<cvt::Matrix4d>& poses, const cvt::String& file )
    {
        cvt::Data d;
        if( !cvt::FileSystem::load( d, file ) ){
            throw CVTException( "Error when loading pose file!" );
        }

        cvt::DataIterator iter( d );

        cvt::Matrix4d p;
        p.setIdentity();

        cvt::String line;
        while( iter.nextLine( line ) ){
            std::vector<cvt::String> lineTokens;
            line.tokenize( lineTokens, ' ' );

            if( lineTokens.size() != 12 ){
                throw CVTException( "Corrupt Pose File" );
            }

            p[ 0 ][ 0 ] = lineTokens[ 0  ].toDouble();
            p[ 0 ][ 1 ] = lineTokens[ 1  ].toDouble();
            p[ 0 ][ 2 ] = lineTokens[ 2  ].toDouble();
            p[ 0 ][ 3 ] = lineTokens[ 3  ].toDouble();

            p[ 1 ][ 0 ] = lineTokens[ 4  ].toDouble();
            p[ 1 ][ 1 ] = lineTokens[ 5  ].toDouble();
            p[ 1 ][ 2 ] = lineTokens[ 6  ].toDouble();
            p[ 1 ][ 3 ] = lineTokens[ 7  ].toDouble();

            p[ 2 ][ 0 ] = lineTokens[ 8  ].toDouble();
            p[ 2 ][ 1 ] = lineTokens[ 9  ].toDouble();
            p[ 2 ][ 2 ] = lineTokens[ 10 ].toDouble();
            p[ 2 ][ 3 ] = lineTokens[ 11 ].toDouble();

            poses.push_back( p );
        }
    }

    void KittiVOParser::loadStamps( std::vector<double>& stamps, const cvt::String& file )
    {
        cvt::Data d;
        if( !cvt::FileSystem::load( d, file ) ){
            throw CVTException( "Error when loading times file!" );
        }

        cvt::DataIterator iter( d );
        cvt::String line;
        while( iter.nextLine( line ) ){
            stamps.push_back( line.toDouble() );
        }
    }

    void KittiVOParser::loadCalibration( const cvt::String& calibFile )
    {
        cvt::Data d;
        if( !cvt::FileSystem::load( d, calibFile ) ){
            throw CVTException( "Error when loading times file!" );
        }

        cvt::DataIterator iter( d );
        cvt::String line;

        cvt::String query0, query1;
        if( !_useColor ){
            query0 = "P0:";
            query1 = "P1:";
        } else {
            query0 = "P2:";
            query1 = "P3:";
        }

        bool found0 = false;
        bool found1 = false;
        while( iter.nextLine( line ) && !( found0 && found1 ) ){
            std::vector<cvt::String> lineTokens;
            line.tokenize( lineTokens, ' ' );

            if( lineTokens.size() != 13 ){
                throw CVTException( "Corrupt calib File" );
            }

            if( !found0 && lineTokens[ 0 ] == query0 ){
                found0 = true;
                _calibLeft.setIdentity();
                _calibLeft[ 0 ][ 0 ] = lineTokens[ 1  ].toDouble();
                _calibLeft[ 0 ][ 1 ] = lineTokens[ 2  ].toDouble();
                _calibLeft[ 0 ][ 2 ] = lineTokens[ 3  ].toDouble();
                _calibLeft[ 0 ][ 3 ] = lineTokens[ 4  ].toDouble();

                _calibLeft[ 1 ][ 0 ] = lineTokens[ 5  ].toDouble();
                _calibLeft[ 1 ][ 1 ] = lineTokens[ 6  ].toDouble();
                _calibLeft[ 1 ][ 2 ] = lineTokens[ 7  ].toDouble();
                _calibLeft[ 1 ][ 3 ] = lineTokens[ 8  ].toDouble();

                _calibLeft[ 2 ][ 0 ] = lineTokens[ 9  ].toDouble();
                _calibLeft[ 2 ][ 1 ] = lineTokens[ 10 ].toDouble();
                _calibLeft[ 2 ][ 2 ] = lineTokens[ 11 ].toDouble();
                _calibLeft[ 2 ][ 3 ] = lineTokens[ 12 ].toDouble();                
                continue;
            }

            if( !found1 && lineTokens[ 0 ] == query1 ){
                found1 = true;
                _calibRight.setIdentity();
                _calibRight[ 0 ][ 0 ] = lineTokens[ 1  ].toDouble();
                _calibRight[ 0 ][ 1 ] = lineTokens[ 2  ].toDouble();
                _calibRight[ 0 ][ 2 ] = lineTokens[ 3  ].toDouble();
                _calibRight[ 0 ][ 3 ] = lineTokens[ 4  ].toDouble();

                _calibRight[ 1 ][ 0 ] = lineTokens[ 5  ].toDouble();
                _calibRight[ 1 ][ 1 ] = lineTokens[ 6  ].toDouble();
                _calibRight[ 1 ][ 2 ] = lineTokens[ 7  ].toDouble();
                _calibRight[ 1 ][ 3 ] = lineTokens[ 8  ].toDouble();

                _calibRight[ 2 ][ 0 ] = lineTokens[ 9  ].toDouble();
                _calibRight[ 2 ][ 1 ] = lineTokens[ 10 ].toDouble();
                _calibRight[ 2 ][ 2 ] = lineTokens[ 11 ].toDouble();
                _calibRight[ 2 ][ 3 ] = lineTokens[ 12 ].toDouble();                
                continue;
            }
        }

        if( !found0 || !found1 ){
            throw CVTException( "Could not find calibration file!" );
        }

        // update the StereoCalibration
        Vector3f    radialDist( 0.0f, 0.0f, 0.0f );
        Vector2f    tangentialDist( 0.0f, 0.0f );
        Matrix4f    extrinsics; extrinsics.setIdentity();
        Matrix3f intr = _calibLeft.toMatrix3();

        CameraCalibration cLeft, cRight;
        cLeft.setDistortion( radialDist, tangentialDist );
        cLeft.setExtrinsics( extrinsics );
        cLeft.setIntrinsics( intr );
        cLeft.setWidth( _left.width() );
        cLeft.setHeight( _left.height() );
        _calib.setFirstCamera( cLeft );

        intr = _calibRight.toMatrix3();
        extrinsics[ 0 ][ 3 ] = _calibRight[ 0 ][ 3 ] / intr[ 0 ][ 0 ];
        cRight.setDistortion( radialDist, tangentialDist );
        cRight.setIntrinsics( intr );
        cRight.setExtrinsics( extrinsics );
        cRight.setWidth( _right.width() );
        cRight.setHeight( _right.height() );
        _calib.setSecondCamera( cRight );
        _calib.setExtrinsics( extrinsics );
    }


}
