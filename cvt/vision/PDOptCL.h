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

#ifndef CVT_PDOPTCL_H
#define CVT_PDOPTCL_H

#include <cvt/gfx/Image.h>
#include <cvt/cl/CLKernel.h>

namespace cvt {
    class PDOptCL {
        public:
            PDOptCL();
            ~PDOptCL();

            void edgeWeight( Image& dst, const Image& input, float alpha, float beta ) const;
            void edgeWeightConfidence( Image& dst, const Image& input, const Image& c, float alpha, float beta ) const;

            void diffusionTensor( Image& dst, const Image& input, float alpha, float beta ) const;
            void diffusionTensorConfidence( Image& dst, const Image& input, const Image& c, float alpha, float beta ) const;

            void denoiseROF( Image& dst, const Image& input, float lambda, size_t iter ) const;
            void denoiseROF_PDD( Image& dst, const Image& input, float lambda, size_t iter ) const;
            void denoiseHuber( Image& dst, const Image& input, float lambda, float hubereps, size_t iterations ) const;
            void denoiseHuber_PDD( Image& dst, const Image& input, float lambda, float hubereps, size_t iterations ) const;
            void denoiseHL( Image& dst, const Image& input, float lambda, float alpha, size_t iterations ) const;
            void denoiseHL_cwise( Image& dst, const Image& input, float lambda, float alpha, size_t iterations ) const;
            void denoiseLOG1( Image& dst, const Image& input, float lambda, float beta, size_t iterations ) const;

            void denoiseLOG1( Image& dst1, Image& dst2, const Image& input1, const Image& input2, float lambda, float beta, size_t iterations ) const;

            void denoiseLOG1_cwise( Image& dst, const Image& input, float lambda, float beta, size_t iterations ) const;

            void mumfordShahConst( Image& dst, const Image& input, float lambda, size_t iterations ) const;
            void mumfordShahConst_cwise( Image& dst, const Image& input, float lambda, size_t iterations ) const;

            void denoiseWeightedROF( Image& dst, const Image& input, const Image& weight, float lambda, size_t iterations ) const;
            void denoiseWeightedROF_PDD( Image& dst, const Image& input, const Image& weight, float lambda, size_t iterations ) const;
            void denoiseWeightedROF_PDD( Image& dst, const Image& input, const Image& weight, const Vector4f& lambda, size_t iterations ) const;

            void denoiseWeightedHuber( Image& dst, const Image& input, const Image& weight, float lambda, float hubereps, size_t iterations ) const;
            void denoiseWeightedHuber_PDD( Image& dst, const Image& input, const Image& weight, float lambda, float hubereps, size_t iterations ) const;
            void denoiseWeightedHuber_PDD( Image& dst, const Image& input, const Image& weight, const Vector4f& lambda, float hubereps, size_t iterations ) const;


            void denoiseDiffusionTensorROF_PDD( Image& dst, const Image& input, const Image& DT, float lambda, size_t iterations ) const;
            void denoiseDiffusionTensorROF_PDD( Image& dst, const Image& input, const Image& DT, const Vector4f& lambda, size_t iterations ) const;

            void denoiseDiffusionTensorHuber_PDD( Image& dst, const Image& input, const Image& DT, float lambda, float hubereps, size_t iterations ) const;
            void denoiseDiffusionTensorHuber_PDD( Image& dst, const Image& input, const Image& DT, const Vector4f& lambda, float hubereps, size_t iterations ) const;

            void denoiseNDDiffusionTensorROF_PDD( Image& dst, const Image& input, const Matrix3f& kinv, const Image& DT, const Vector4f& lambda, size_t iterations ) const;
            void denoisePOSDiffusionTensorROF_PDD( Image& dst, const Image& input, const Image& DT, const Vector4f& lambda, size_t iterations ) const;

            void inpaintWeightedHuber( Image& dst, const Image& input, const Image& weight, const Image& mask, float lambda, float alpha, size_t iterations ) const;
            void inpaintDiffusionTensorHuber_PDD( Image& dst, const Image& input, const Image& DT, const Image& mask, float lambda, float hubereps, size_t iterations ) const;
            void inpaintDiffusionTensorHuber_PDD( Image& dst, const Image& input, const Image& DT, const Image& mask, const Vector4f& lambda, float hubereps, size_t iterations ) const;

        private:
            void fill( Image& dst, const cl_float4& value ) const;

            CLKernel _clfill;
            CLKernel _clweightColor;
            CLKernel _clweightGray;
            CLKernel _clweightColorConfid;
            CLKernel _cldiffTensorColor;
            CLKernel _cldiffTensorGray;
            CLKernel _cldiffTensorColorConfid;
            CLKernel _cldiffTensorGrayConfid;
            CLKernel _clrof;
            CLKernel _clrofpdd;
            CLKernel _clhuber;
            CLKernel _clhuberpdd;
            CLKernel _clwrof;
            CLKernel _clwrofpdd;
            CLKernel _clwhuber;
            CLKernel _clwhuberpdd;
            CLKernel _cldtrofpdd;
            CLKernel _cldthuberpdd;
            CLKernel _clinpaintdthuberpdd;
            CLKernel _clnddtrofpdd;
            CLKernel _clposdtrofpdd;
            CLKernel _clinpaintwhuber;
            CLKernel _clhl;
            CLKernel _clhlcwise;
            CLKernel _clmsc;
            CLKernel _clmsccwise;
            CLKernel _cllog1;
            CLKernel _cllog1cwise;
            CLKernel _cllog18;
    };

    inline void PDOptCL::denoiseWeightedHuber_PDD( Image& dst, const Image& input, const Image& weight, float lambda, float hubereps, size_t iterations ) const
    {
        const Vector4f l( lambda, lambda, lambda, lambda );
        denoiseWeightedHuber_PDD( dst, input, weight, l, hubereps, iterations );
    }

    inline void PDOptCL::denoiseWeightedROF_PDD( Image& dst, const Image& input, const Image& weight, float lambda, size_t iterations ) const
    {
        const Vector4f l( lambda, lambda, lambda, lambda );
        denoiseWeightedROF_PDD( dst, input, weight, l, iterations );
    }

    inline void PDOptCL::denoiseDiffusionTensorROF_PDD( Image& dst, const Image& input, const Image& DT, float lambda, size_t iterations ) const
    {
        const Vector4f l( lambda, lambda, lambda, lambda );
        denoiseDiffusionTensorROF_PDD( dst, input, DT, l, iterations );
    }

    inline void PDOptCL::denoiseDiffusionTensorHuber_PDD( Image& dst, const Image& input, const Image& DT, float lambda, float hubereps, size_t iterations ) const
    {
        const Vector4f l( lambda, lambda, lambda, lambda );
        denoiseDiffusionTensorHuber_PDD( dst, input, DT, l, hubereps, iterations );
    }

    inline void PDOptCL::inpaintDiffusionTensorHuber_PDD( Image& dst, const Image& input, const Image& DT, const Image& mask, float lambda, float hubereps, size_t iterations ) const
    {
        const Vector4f l( lambda, lambda, lambda, lambda );
        inpaintDiffusionTensorHuber_PDD( dst, input, DT, mask, l, hubereps, iterations );
    }
}

#endif
