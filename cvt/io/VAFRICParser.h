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
#ifndef CVT_VAFRICPARSER_H
#define CVT_VAFRICPARSER_H

#include <cvt/util/String.h>
#include <cvt/io/FileSystem.h>
#include <cvt/util/Data.h>
#include <cvt/util/DataIterator.h>
#include <cvt/vision/CameraCalibration.h>

#include <vector>
#include <algorithm>

namespace cvt {

    class VAFRICSample {
        public:
            VAFRICSample( const CameraCalibration& cam, const String& imgpath );
            VAFRICSample( const VAFRICSample& );

            const CameraCalibration& calib() const;
            const String& imagePath() const;

        private:
            CameraCalibration _calib;
            String            _imgpath;
    };

    inline VAFRICSample::VAFRICSample( const CameraCalibration& cam, const String& imgpath ) :
        _calib( cam ),
        _imgpath( imgpath )
    {

    }

    inline VAFRICSample::VAFRICSample( const VAFRICSample& other ) :
        _calib( other._calib ),
        _imgpath( other._imgpath )
    {
    }

    const CameraCalibration& VAFRICSample::calib() const
    {
        return _calib;
    }

    const String& VAFRICSample::imagePath() const
    {
        return _imgpath;
    }

    class VAFRICParser {
        public:
            VAFRICParser( const String& imagepath, const String& basename );
            ~VAFRICParser();

            size_t size() const;

            const VAFRICSample& operator[]( size_t idx ) const;

            void findNearestNeighbours( std::vector<size_t>& neighbours, size_t index, size_t N, float minalloweddist = 1e-5f );

        private:
            void load( const String& paramfile, const String& imagefile );

            bool loadScalar( float& value, DataIterator& it );
            bool loadVector3f( Vector3f& value, DataIterator& it );

            String                    _path;
            std::vector<VAFRICSample> _data;
    };

    inline VAFRICParser::VAFRICParser( const String& path, const String& basename ) :
        _path( path )
    {
        if( ! _path.hasSuffix( "/" ) ) {
            _path += "/";
        }

        String filebase = path + basename + String( "_%04d.%s" );
        String paramfile, imagefile;
        int idx = 0;

        while( true ) {
            paramfile.sprintf( filebase.c_str(), idx, "txt" );
            imagefile.sprintf( filebase.c_str(), idx, "png" );

            if( FileSystem::exists( paramfile ) && FileSystem::exists( imagefile ) )
                load( paramfile, imagefile );
            else
                break;

            idx++;
        }
    }

    inline VAFRICParser::~VAFRICParser()
    {
    }

    inline bool VAFRICParser::loadScalar( float& value, DataIterator& it )
    {
        const String delimiter( " \t=" );
        String token;

        it.nextToken( token, delimiter );
        value = token.toFloat();
        return true;
    }

    inline bool VAFRICParser::loadVector3f( Vector3f& value, DataIterator& it )
    {
        const String delimiter( " \t=,[]" );
        String token;

        it.nextToken( token, delimiter );
        value.x = token.toFloat();
        it.nextToken( token, delimiter );
        value.y = token.toFloat();
        it.nextToken( token, delimiter );
        value.z = token.toFloat();
        return true;
    }

    inline void VAFRICParser::load( const String& file, const String& image )
    {
        Data data;
        FileSystem::load( data, file );
        DataIterator it( data );

        Matrix3f K, R;
        Vector3f t;
        CameraCalibration calib;
        String token;
        const String ws( " \r\n\t" );

        Vector3f camPos( 0.0, 0.0f, 0.0f );
        Vector3f camDir( 0.0, 0.0f, 0.0f );
        Vector3f camUp( 0.0, 0.0f, 0.0f );
        Vector3f camLookat( 0.0, 0.0f, 0.0f );
        Vector3f camSky( 0.0, 0.0f, 0.0f );
        Vector3f camRight( 0.0, 0.0f, 0.0f );
        Vector3f camFPoint( 0.0, 0.0f, 0.0f );
        float camAngle;

        /* load the data */
        while( it.hasNext() ) {
            it.nextToken( token, ws );
            if( token == "cam_pos" ) {
                loadVector3f( camPos, it );
            } else if( token == "cam_dir" ) {
                loadVector3f( camDir, it );
            } else if( token == "cam_up" ) {
                loadVector3f( camUp, it );
            } else if( token == "cam_lookat" ) {
                loadVector3f( camLookat, it );
            } else if( token == "cam_sky" ) {
                loadVector3f( camSky, it );
            } else if( token == "cam_right" ) {
                loadVector3f( camRight, it );
            } else if( token == "cam_fpoint" ) {
                loadVector3f( camFPoint, it );
            } else if( token == "cam_angle" ) {
                loadScalar( camAngle, it );
            } else {
                it.skipInverse("\n");
            }
        }

        /* calc the intrinsics */
        float focal  =  camDir.length();
        float aspect =  camRight.length() / camUp.length();
        float angle  =  2.0f * Math::atan( camRight.length() / 2.0f / camDir.length() );

        float M      = 480; //cam_height
        float N      = 640; //cam.width

        float width = N;
        float height = M;

        // pixel size
        float psx = 2.0f * focal * Math::tan( 0.5f * angle) / N ;
        float psy = 2.0f * focal * Math::tan( 0.5f * angle) / aspect / M;

        psx = psx / focal;
        psy = psy / focal;

//        float  Sx = psx;
//        float  Sy = psy;

        float Ox = (width + 1.0f  ) * 0.5f;
        float Oy = (height + 1.0f ) * 0.5f;

//        float f = focal;

        K = Matrix3f( 1.0f / psx, 0.0f,   Ox,
                      0.0f, 1.0f / psy,   Oy,
                      0.0f,       0.0f, 1.0f );

        K[ 1 ][ 1 ] = -K[ 1 ][ 1 ];
        calib.setIntrinsics( K );

        /* calc R and t */
        Vector3f z = camDir / camDir.length();

        Vector3f x = camUp.cross( z );
        x.normalize();

        Vector3f y = z.cross( x);

        R = Matrix3f( x, y, z ).transpose();

        t = camPos;

        Matrix4f E;
        E = R.toMatrix4();
        E[ 0 ][ 3 ] = t[ 0 ];
        E[ 1 ][ 3 ] = t[ 1 ];
        E[ 2 ][ 3 ] = t[ 2 ];
        calib.setExtrinsics( E );

        _data.push_back( VAFRICSample( calib, image ) );
    }

    inline const VAFRICSample& VAFRICParser::operator[]( size_t idx ) const
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

    inline void VAFRICParser::findNearestNeighbours( std::vector<size_t>& neighbours, size_t index, size_t N, float minalloweddist )
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

    inline size_t VAFRICParser::size() const
    {
        return _data.size();
    }

}
#endif
