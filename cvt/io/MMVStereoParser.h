/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2013, Philipp Heise and Sebastian Klose
   Copyright (c) 2016, BMW Car IT GmbH, Philipp Heise (philipp.heise@bmw.de)

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

#ifndef CVT_MMVSTEREOPARSER_H
#define CVT_MMVSTEREOPARSER_H

#include <cvt/util/String.h>
#include <cvt/io/FileSystem.h>
#include <cvt/util/Data.h>
#include <cvt/util/DataIterator.h>
#include <cvt/vision/CameraCalibration.h>

#include <vector>
#include <algorithm>

namespace cvt {

    class MMVStereoSample {
        public:
            MMVStereoSample( const CameraCalibration& cam, const String& imgpath, const String& filename );
            MMVStereoSample( const MMVStereoSample& );

            CameraCalibration& calib();
            const String& imagePath() const;
            const String& filename() const;

        private:
            CameraCalibration _calib;
            String            _imgpath;
            String            _filename;
    };

    inline MMVStereoSample::MMVStereoSample( const CameraCalibration& cam, const String& imgpath, const String& filename ) :
        _calib( cam ),
        _imgpath( imgpath ),
        _filename( filename )
    {

    }

    inline MMVStereoSample::MMVStereoSample( const MMVStereoSample& other ) :
        _calib( other._calib ),
        _imgpath( other._imgpath ),
        _filename( other._filename )
    {
    }

    CameraCalibration& MMVStereoSample::calib()
    {
        return _calib;
    }

    const String& MMVStereoSample::imagePath() const
    {
        return _imgpath;
    }

    const String& MMVStereoSample::filename() const
    {
        return _filename;
    }

    class MMVStereoParser {
        public:
            MMVStereoParser( const String& paramsfile, const String& imagepath );
            ~MMVStereoParser();

            size_t size() const;

            MMVStereoSample& operator[]( size_t idx );

            void findNearestNeighbours( std::vector<size_t>& neighbours, size_t index, size_t N, float minalloweddist = 1e-5f );

        private:
            void load( const String& paramfile );
            String                       _imgpath;

            std::vector<MMVStereoSample> _data;
    };

    inline MMVStereoParser::MMVStereoParser( const String& paramsfile, const String& imagepath ) :
        _imgpath( imagepath )
    {
        if( ! _imgpath.hasSuffix( "/" ) ) {
            _imgpath += "/";
        }
        load( paramsfile );
    }

    inline MMVStereoParser::~MMVStereoParser()
    {
    }

    inline void MMVStereoParser::load( const String& file )
    {
        Data data;
        FileSystem::load( data, file );
        DataIterator it( data );

        size_t numentries = it.nextLong();

        for( size_t i = 0; i < numentries; i++ ) {
            String file;
            Matrix3f K, R;
            Vector3f t;
            CameraCalibration calib;

            it.nextToken( file, " \n\t\r" );

            for( int y = 0; y < 3; y++ ) {
                for( int x = 0; x < 3; x++ ) {
                    K[ y ][ x ] = it.nextDouble();
                }
            }
            K[ 2 ][ 2 ] = 1.0f;
            calib.setIntrinsics( K );

            for( int y = 0; y < 3; y++ ) {
                for( int x = 0; x < 3; x++ ) {
                    R[ y ][ x ] = it.nextDouble();
                }
            }

            t[ 0 ] = it.nextDouble();
            t[ 1 ] = it.nextDouble();
            t[ 2 ] = it.nextDouble();

            Matrix4f E;
            E = R.toMatrix4();
            E[ 0 ][ 3 ] = t[ 0 ];
            E[ 1 ][ 3 ] = t[ 1 ];
            E[ 2 ][ 3 ] = t[ 2 ];
            calib.setExtrinsics( E );

            _data.push_back( MMVStereoSample( calib, _imgpath + file, file ) );
        }
    }


    inline MMVStereoSample& MMVStereoParser::operator[]( size_t idx )
    {
        return _data[ idx ];
    }

    struct IdDist {
        float  distance;
        size_t idx;

        bool operator<( const IdDist& other ) const
        {
            return distance < other.distance;
        }
    };

    inline void MMVStereoParser::findNearestNeighbours( std::vector<size_t>& neighbours, size_t index, size_t N, float minalloweddist )
    {
        std::vector<IdDist> all;
        Matrix4f pose = _data[ index ].calib().extrinsics().inverse();
        Vector4f t( pose.col( 3 ) );

        for( size_t i = 0; i < _data.size(); i++ ) {
           if( i == index ) continue;
           IdDist idd;
           idd.distance = ( _data[ i ].calib().extrinsics().inverse().col( 3 ) -t ).length();
           idd.idx = i;
           all.push_back( idd );
        }

        std::sort( all.begin(), all.end() );
        neighbours.clear();
        size_t i = 0, n = 0;
        while( n < N && i < all.size() ) {
            if( all[ i ].distance > minalloweddist ) {
                neighbours.push_back( all[ i ].idx );
                n++;
            }
            i++;
        }
    }

    inline size_t MMVStereoParser::size() const
    {
        return _data.size();
    }

}

#endif
