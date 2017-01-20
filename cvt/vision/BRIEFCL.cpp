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
#include <cvt/vision/BRIEFCL.h>
#include <cvt/cl/kernel/BRIEF.h>

namespace cvt {

    BRIEFCL::BRIEFCL() :
        _clbrief( _BRIEF_source, "brief" ),
        _clbriefstereomatch( _BRIEF_source, "brief_stereo_match" )
    {
    }

    BRIEFCL::~BRIEFCL()
    {
    }

    void BRIEFCL::extract( CLBuffer& output, const CLBuffer& features, int numfeatures, const Image& image )
    {
        _clbrief.setArg( 0, output );
        _clbrief.setArg( 1, features );
        _clbrief.setArg( 2, numfeatures );
        _clbrief.setArg( 3, image );
        _clbrief.run( CLNDRange( Math::pad( numfeatures, 256 ) ), CLNDRange( 256 ) );
    }

    void BRIEFCL::stereoMatch( CLBuffer& matches, int& matches_size, const CLBuffer& features0, const CLBuffer& brief0, int size_features0,
                               const CLBuffer& features1, const CLBuffer& brief1, int size_features1, const CLBuffer& sclidx1, int threshold )
    {
        matches_size = 0;
        CLBuffer nummatches( &matches_size, sizeof( cl_int ) );

        _clbriefstereomatch.setArg( 0, matches );
        _clbriefstereomatch.setArg( 1, nummatches );
        _clbriefstereomatch.setArg( 2, ( int ) ( matches.size() / sizeof( cl_float3 ) ) );
        _clbriefstereomatch.setArg( 3, features0 );
        _clbriefstereomatch.setArg( 4, brief0 );
        _clbriefstereomatch.setArg( 5, size_features0 );
        _clbriefstereomatch.setArg( 6, features1 );
        _clbriefstereomatch.setArg( 7, brief1 );
        _clbriefstereomatch.setArg( 8, size_features1 );
        _clbriefstereomatch.setArg( 9, sclidx1 );
        _clbriefstereomatch.setArg( 10, threshold );
        _clbriefstereomatch.run( CLNDRange( Math::pad( size_features0, 256 ) ), CLNDRange( 256 ) );

        nummatches.read( &matches_size );
    }
}
