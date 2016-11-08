/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2014, Philipp Heise and Sebastian Klose
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
#ifndef CVT_CLMATRIX_H
#define CVT_CLMATRIX_H

#include <cvt/math/Matrix.h>

namespace cvt {
    class CLMatrix3f
    {
        public:
            CLMatrix3f() {}

            CLMatrix3f( const Matrix3f& mat )
            {
                set( mat );
            }

            void toMatrix3f( Matrix3f& mat )
            {
                mat[ 0 ][ 0 ] = m[ 0 ].x;
                mat[ 0 ][ 1 ] = m[ 0 ].y;
                mat[ 0 ][ 2 ] = m[ 0 ].z;

                mat[ 1 ][ 0 ] = m[ 1 ].x;
                mat[ 1 ][ 1 ] = m[ 1 ].y;
                mat[ 1 ][ 2 ] = m[ 1 ].z;

                mat[ 2 ][ 0 ] = m[ 2 ].x;
                mat[ 2 ][ 1 ] = m[ 2 ].y;
                mat[ 2 ][ 2 ] = m[ 2 ].z;
            }

            void set( const Matrix3f& mat )
            {
                m[ 0 ].x = mat[ 0 ][ 0 ];
                m[ 0 ].y = mat[ 0 ][ 1 ];
                m[ 0 ].z = mat[ 0 ][ 2 ];

                m[ 1 ].x = mat[ 1 ][ 0 ];
                m[ 1 ].y = mat[ 1 ][ 1 ];
                m[ 1 ].z = mat[ 1 ][ 2 ];

                m[ 2 ].x = mat[ 2 ][ 0 ];
                m[ 2 ].y = mat[ 2 ][ 1 ];
                m[ 2 ].z = mat[ 2 ][ 2 ];
            }

            cl_float3 m[ 3 ];
    };

    class CLMatrix4f
    {
        public:
            CLMatrix4f() {}

            CLMatrix4f( const Matrix4f& mat )
            {
                set( mat );
            }

            void toMatrix4f( Matrix4f& mat )
            {
                mat[ 0 ][ 0 ] = m[ 0 ].x;
                mat[ 0 ][ 1 ] = m[ 0 ].y;
                mat[ 0 ][ 2 ] = m[ 0 ].z;
                mat[ 0 ][ 3 ] = m[ 0 ].w;

                mat[ 1 ][ 0 ] = m[ 1 ].x;
                mat[ 1 ][ 1 ] = m[ 1 ].y;
                mat[ 1 ][ 2 ] = m[ 1 ].z;
                mat[ 1 ][ 3 ] = m[ 1 ].w;

                mat[ 2 ][ 0 ] = m[ 2 ].x;
                mat[ 2 ][ 1 ] = m[ 2 ].y;
                mat[ 2 ][ 2 ] = m[ 2 ].z;
                mat[ 2 ][ 3 ] = m[ 2 ].w;

                mat[ 3 ][ 0 ] = m[ 3 ].x;
                mat[ 3 ][ 1 ] = m[ 3 ].y;
                mat[ 3 ][ 2 ] = m[ 3 ].z;
                mat[ 3 ][ 3 ] = m[ 3 ].w;
            }

            void set( const Matrix4f& mat )
            {
                m[ 0 ].x = mat[ 0 ][ 0 ];
                m[ 0 ].y = mat[ 0 ][ 1 ];
                m[ 0 ].z = mat[ 0 ][ 2 ];
                m[ 0 ].w = mat[ 0 ][ 3 ];

                m[ 1 ].x = mat[ 1 ][ 0 ];
                m[ 1 ].y = mat[ 1 ][ 1 ];
                m[ 1 ].z = mat[ 1 ][ 2 ];
                m[ 1 ].w = mat[ 1 ][ 3 ];

                m[ 2 ].x = mat[ 2 ][ 0 ];
                m[ 2 ].y = mat[ 2 ][ 1 ];
                m[ 2 ].z = mat[ 2 ][ 2 ];
                m[ 2 ].w = mat[ 2 ][ 3 ];

                m[ 3 ].x = mat[ 3 ][ 0 ];
                m[ 3 ].y = mat[ 3 ][ 1 ];
                m[ 3 ].z = mat[ 3 ][ 2 ];
                m[ 3 ].w = mat[ 3 ][ 3 ];
            }

            cl_float4 m[ 4 ];
    };
}
#endif
