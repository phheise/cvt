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

#include <cvt/vision/ProxPrecondCL.h>
#include <cvt/cl/kernel/ProxPrecond.h>

namespace cvt {
#define KX 32
#define KY 32

    ProxPrecondCL::ProxPrecondCL() :
        _clprecond_edgeclamp_sep3( _ProxPrecond_source, "prox_precond_edge_clamp_separable3" ),
        _clprecond_edgezero_sep3( _ProxPrecond_source, "prox_precond_edge_zero_separable3" )
    {
    }

    ProxPrecondCL::~ProxPrecondCL()
    {
    }

    void ProxPrecondCL::precondEdgeClampSeparable3( Image& output, const Vector3f& kx, const Vector3f& ky )
    {
        cl_float3 clkx, clky;

        clkx.x = kx.x;
        clkx.y = kx.y;
        clkx.z = kx.z;

        clky.x = ky.x;
        clky.y = ky.y;
        clky.z = ky.z;

        _clprecond_edgeclamp_sep3.setArg( 0, output );
        _clprecond_edgeclamp_sep3.setArg( 1, sizeof( cl_float3 ), &clkx );
        _clprecond_edgeclamp_sep3.setArg( 2, sizeof( cl_float3 ), &clky );
        _clprecond_edgeclamp_sep3.run( CLNDRange( Math::pad( output.width(), KX ), Math::pad( output.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void ProxPrecondCL::precondEdgeZeroSeparable3( Image& output, const Vector3f& kx, const Vector3f& ky )
    {
        cl_float3 clkx, clky;

        clkx.x = kx.x;
        clkx.y = kx.y;
        clkx.z = kx.z;

        clky.x = ky.x;
        clky.y = ky.y;
        clky.z = ky.z;

        _clprecond_edgezero_sep3.setArg( 0, output );
        _clprecond_edgezero_sep3.setArg( 1, sizeof( cl_float3 ), &clkx );
        _clprecond_edgezero_sep3.setArg( 2, sizeof( cl_float3 ), &clky );
        _clprecond_edgezero_sep3.run( CLNDRange( Math::pad( output.width(), KX ), Math::pad( output.height(), KY ) ), CLNDRange( KX, KY ) );
    }

}
