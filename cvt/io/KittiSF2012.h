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
#ifndef CVT_KITTISF2012_H
#define CVT_KITTISF2012_H

#include <cvt/util/String.h>
#include <cvt/io/FileSystem.h>
#include <cvt/gfx/Image.h>

#include <vector>
#include <algorithm>

namespace cvt {

    class KittiSF2012Sample
    {
        public:
            KittiSF2012Sample( const String& path, const String* mvpath, size_t id );
            ~KittiSF2012Sample();

            size_t id() const { return _id; }
            bool   hasMultiView() const { return _hasMV; }

            const String& leftImagePath() const { return _left; }
            const String& leftImage2Path() const { return _left2; }

            const String& disparityNOCPath() const { return _disp_noc; }
            const String& disparityOCCPath() const { return _disp_occ; }

            const String& rightImagePath() const { return _right; }
            const String& rightImage2Path() const { return _right2; }

            const String& leftImagePath( size_t mvidx ) const { return _mv_left[ mvidx ]; }
            const String& rightImagePath( size_t mvidx ) const { return _mv_right[ mvidx ]; }

            const String& flowNOCPath() const { return _flow_noc; }
            const String& flowOCCPath() const { return _flow_occ; }

            void    print( std::ostream &out ) const;

            bool    isValid() const;
        private:
            size_t  _id;
            bool    _hasMV;
            String  _left;
            String  _right;
            String  _left2;
            String  _right2;
            String  _disp_noc;
            String  _disp_occ;
            String  _flow_noc;
            String  _flow_occ;
            String  _mv_left[ 21 ];
            String  _mv_right[ 21 ];
    };

    inline std::ostream& operator<<( std::ostream &out, const KittiSF2012Sample& sample )
    {
        sample.print( out );
        return out;
    }

    class KittiSF2012 {
        public:
            KittiSF2012( const String& path, const String& multiviewpath = "" );
            ~KittiSF2012();

            size_t size() const;
            const KittiSF2012Sample& operator[]( size_t idx ) const;

        private:
            void load();

            String                         _path;
            String                         _multipath;
            std::vector<KittiSF2012Sample> _data;

    };
};
#endif
