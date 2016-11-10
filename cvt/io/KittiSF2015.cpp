/*
   The MIT License (MIT)

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
#include <cvt/io/KittiSF2015.h>
#include <cvt/gfx/IMapScoped.h>

namespace cvt {

    KittiSF2015::KittiSF2015( const String& path, const String& multiviewpath ) :
        _path( path ),
        _multipath( multiviewpath )
    {
        if( _path.hasSuffix( "/") )
            _path.truncate( _path.length() - 1 );
        if( _multipath.hasSuffix( "/") )
            _multipath.truncate( _multipath.length() - 1 );
        load();
    }

    KittiSF2015::~KittiSF2015()
    {
    }

    size_t KittiSF2015::size() const
    {
        return _data.size();
    }

    const KittiSF2015Sample& KittiSF2015::operator[]( size_t idx ) const
    {
        return _data[ idx ];
    }

    void KittiSF2015::load()
    {
        const size_t N = 200;
        const String* mvpath = FileSystem::exists( _multipath ) ? &_multipath : NULL;

        for( size_t i = 0; i < N; i++ ) {
            KittiSF2015Sample sample( _path, mvpath, i );
            if( sample.isValid() )
                _data.push_back( sample );
        }
    }

    KittiSF2015Sample::KittiSF2015Sample( const String& path, const String* mvpath, size_t id ) :
        _id( id ),
        _hasMV( mvpath )
    {
        _left.sprintf("%s/image_2/%06zu_10.png", path.c_str(), id );
        _left2.sprintf("%s/image_2/%06zu_11.png", path.c_str(), id );
        _right.sprintf("%s/image_3/%06zu_10.png", path.c_str(), id );
        _right2.sprintf("%s/image_3/%06zu_11.png", path.c_str(), id );
        _obj.sprintf("%s/obj_map/%06zu_10.png", path.c_str(), id );
        _disp_noc_left.sprintf("%s/disp_noc_0/%06zu_10.png", path.c_str(), id );
        _disp_occ_left.sprintf("%s/disp_occ_0/%06zu_10.png", path.c_str(), id );
        _disp_noc_right.sprintf("%s/disp_noc_1/%06zu_10.png", path.c_str(), id );
        _disp_occ_right.sprintf("%s/disp_occ_1/%06zu_10.png", path.c_str(), id );
        _flow_noc.sprintf("%s/flow_noc/%06zu_10.png", path.c_str(), id );
        _flow_occ.sprintf("%s/flow_occ/%06zu_10.png", path.c_str(), id );

        if( hasMultiView() ) {
            for( size_t subid = 0; subid <= 20; subid++ ) {
                _mv_left[ subid ].sprintf("%s/image_2/%06zu_%02zu.png", mvpath->c_str(), id, subid );
                _mv_right[ subid ].sprintf("%s/image_3/%06zu_%02zu.png", mvpath->c_str(), id, subid );
            }
        } else {
            for( size_t subid = 0; subid <= 20; subid++ ) {
                _mv_left[ subid ] = "";
                _mv_right[ subid ] = "";
            }
        }
    }

    KittiSF2015Sample::~KittiSF2015Sample()
    {
    }

    bool KittiSF2015Sample::isValid() const
    {
        if( !FileSystem::exists( _left ) )
            return false;
        if( !FileSystem::exists( _right ) )
            return false;
        if( !FileSystem::exists( _left2 ) )
            return false;
        if( !FileSystem::exists( _right2 ) )
            return false;
        if( !FileSystem::exists( _obj ) )
            return false;
        if( !FileSystem::exists( _disp_noc_left ) )
            return false;
        if( !FileSystem::exists( _disp_occ_left ) )
            return false;
        if( !FileSystem::exists( _disp_noc_right ) )
            return false;
        if( !FileSystem::exists( _disp_occ_right ) )
            return false;
        if( !FileSystem::exists( _flow_noc ) )
            return false;
        if( !FileSystem::exists( _flow_occ ) )
            return false;

        if( hasMultiView() ) {
            for( size_t i = 0; i <= 20; i++ ) {
                if( !FileSystem::exists( _mv_left[ i ] ) )
                    return false;
                if( !FileSystem::exists( _mv_right[ i ] ) )
                    return false;
            }
        }

        return true;
    }

    void KittiSF2015Sample::print( std::ostream &out ) const
    {
        out << "KITTI 2015 Stereo/Flow/SceneFlow Sample " << _id << std::endl;
        out << "left    : " << _left << std::endl;
        out << "right   : " << _right << std::endl;
        out << "left2   : " << _left2 << std::endl;
        out << "right2  : " << _right2 << std::endl;
        out << "obj-map : " << _obj << std::endl;
        out << "disp NOC left : " << _disp_noc_left << std::endl;
        out << "disp NOC right: " << _disp_noc_right << std::endl;
        out << "disp OCC left : " << _disp_occ_left << std::endl;
        out << "disp OCC right: " << _disp_occ_right << std::endl;
        out << "flow NOC      : " << _flow_noc << std::endl;
        out << "flow OCC      : " << _flow_occ << std::endl;
        out << "multiview     : " << ( hasMultiView() ? "yes" : "no" ) << std::endl;
    }

}
