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
#include <cvt/io/KittiSF2012.h>
#include <cvt/gfx/IMapScoped.h>

namespace cvt {

    KittiSF2012::KittiSF2012( const String& path, const String& multiviewpath ) :
        _path( path ),
        _multipath( multiviewpath )
    {
        if( _path.hasSuffix( "/") )
            _path.truncate( _path.length() - 1 );
        if( _multipath.hasSuffix( "/") )
            _multipath.truncate( _multipath.length() - 1 );
        load();
    }

    KittiSF2012::~KittiSF2012()
    {
    }

    size_t KittiSF2012::size() const
    {
        return _data.size();
    }

    const KittiSF2012Sample& KittiSF2012::operator[]( size_t idx ) const
    {
        return _data[ idx ];
    }

    void KittiSF2012::load()
    {
        const size_t N = 198;
        const String* mvpath = FileSystem::exists( _multipath ) ? &_multipath : NULL;

        for( size_t i = 0; i < N; i++ ) {
            KittiSF2012Sample sample( _path, mvpath, i );
            if( sample.isValid() )
                _data.push_back( sample );
        }
    }

    KittiSF2012Sample::KittiSF2012Sample( const String& path, const String* mvpath, size_t id ) :
        _id( id ),
        _hasMV( mvpath )
    {
        _left.sprintf("%s/image_0/%06zu_10.png", path.c_str(), id );
        _left2.sprintf("%s/image_0/%06zu_11.png", path.c_str(), id );
        _right.sprintf("%s/image_1/%06zu_10.png", path.c_str(), id );
        _right2.sprintf("%s/image_1/%06zu_11.png", path.c_str(), id );
        _disp_noc.sprintf("%s/disp_noc/%06zu_10.png", path.c_str(), id );
        _disp_occ.sprintf("%s/disp_occ/%06zu_10.png", path.c_str(), id );
        _flow_noc.sprintf("%s/flow_noc/%06zu_10.png", path.c_str(), id );
        _flow_occ.sprintf("%s/flow_occ/%06zu_10.png", path.c_str(), id );

        if( hasMultiView() ) {
            int mvinvalid = 0;
            for( size_t subid = 0; subid <= 20; subid++ ) {
                _mv_left[ subid ].sprintf("%s/image_0/%06zu_%02zu.png", mvpath->c_str(), id, subid );
                _mv_right[ subid ].sprintf("%s/image_1/%06zu_%02zu.png", mvpath->c_str(), id, subid );
                if( !FileSystem::exists( _mv_left[ subid ] ) ||
                   !FileSystem::exists( _mv_right[ subid ] ) ) {
                    _mv_left[ subid ] = "";
                    _mv_right[ subid ] = "";
                    mvinvalid++;
                }
            }
            if( mvinvalid == 21 )
                _hasMV = false;
        } else {
            for( size_t subid = 0; subid <= 20; subid++ ) {
                _mv_left[ subid ] = "";
                _mv_right[ subid ] = "";
            }
        }
    }

    KittiSF2012Sample::~KittiSF2012Sample()
    {
    }

    bool KittiSF2012Sample::isValid() const
    {
        if( !FileSystem::exists( _left ) )
            return false;
        if( !FileSystem::exists( _right ) )
            return false;
        if( !FileSystem::exists( _left2 ) )
            return false;
        if( !FileSystem::exists( _right2 ) )
            return false;
        if( !FileSystem::exists( _disp_noc ) )
            return false;
        if( !FileSystem::exists( _disp_occ ) )
            return false;
        if( !FileSystem::exists( _flow_noc ) )
            return false;
        if( !FileSystem::exists( _flow_occ ) )
            return false;

        if( hasMultiView() ) {
            for( size_t i = 0; i <= 20; i++ ) {
                if( !_mv_left[ i ].isEmpty() && !FileSystem::exists( _mv_left[ i ] ) )
                    return false;
                if( !_mv_right[ i ].isEmpty() && !FileSystem::exists( _mv_right[ i ] ) )
                    return false;
            }
        }

        return true;
    }

    void KittiSF2012Sample::print( std::ostream &out ) const
    {
        out << "KITTI 2012 Stereo/Flow/SceneFlow Sample " << _id << std::endl;
        out << "left    : " << _left << std::endl;
        out << "right   : " << _right << std::endl;
        out << "left2   : " << _left2 << std::endl;
        out << "right2  : " << _right2 << std::endl;
        out << "disp NOC  : " << _disp_noc << std::endl;
        out << "disp NOC  : " << _disp_noc << std::endl;
        out << "flow NOC  : " << _flow_noc << std::endl;
        out << "flow OCC  : " << _flow_occ << std::endl;
        out << "multiview : " << ( hasMultiView() ? "yes" : "no" ) << std::endl;
    }

}
