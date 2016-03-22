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

#ifndef CVT_INTEGRAL_IMAGE_H
#define CVT_INTEGRAL_IMAGE_H

#include <cvt/gfx/Image.h>
#include <cvt/gfx/IMapScoped.h>
#include <cvt/util/Flags.h>
#include <cvt/vision/Patch.h>

namespace cvt
{
    enum IntImgFlagTypes {
		SUMMED_AREA = ( 1 << 0 ),
		SQUARED_SUMMED_AREA = ( 1 << 1 )
	};

	CVT_ENUM_TO_FLAGS( IntImgFlagTypes, IntegralImageFlags )

	class IntegralImage
	{
		public:
            IntegralImage( const Image & img, IntegralImageFlags flags = SUMMED_AREA );
            IntegralImage( IntegralImageFlags flags = SUMMED_AREA );
            
            ~IntegralImage();

            void    update( const Image & img );
            float   area( const Recti & r ) const;
            float   sqrArea( const Recti & r ) const;

            /**
             * normalized cross correlation:
             * calculates the normalized cross correlation of the Rectangle rOther from other
             * with pos in this image
             */
            float   ncc( const Image & img,
                         const Image & otherI,
                         const IntegralImage & otherII,
                         const Recti & rOther,
                         const Vector2i & pos ) const;

            /**
             * compute normalized cross correlation of the patch with
             * this image at position pos
             */
            float   ncc( const Image & img, const Patch & patch, const Vector2i & pos ) const;

            const Image & sumImage()	const { return _sum; };
            const Image & sqrSumImage() const { return _sqrSum; };
            IntegralImageFlags flags()  const { return _flags; };

            static inline float area( const Image & img, const Recti & r );

            static inline float area( const float * p, size_t w, size_t h, size_t stride );
            static inline float area( const float * ptr, size_t x, size_t y, size_t w, size_t h, size_t stride );

            static inline float area( const IMapScoped<const float>& map, size_t x, size_t y, size_t w, size_t h );

        private:
            Image              _sum;
            Image              _sqrSum;
            IntegralImageFlags _flags;

	};

    inline float IntegralImage::area( const Image & img, const Recti & r )
    {
        size_t stride;
        const float * p = img.map<float>( &stride );

        int xOffset, yOffset;

        yOffset = r.y - 1;
        xOffset = r.x - 1;

        float sum = p[ ( yOffset + r.height ) * stride + xOffset + r.width ];

        if( yOffset >= 0 ){
            if( xOffset >= 0 ){ // +a
                sum += p[ yOffset * stride + xOffset ];
                sum -= p[ ( yOffset + r.height ) * stride + xOffset ];
            }

            xOffset = Math::min( xOffset + r.width, ( int )img.width() - 1 );
            sum -= p[ yOffset * stride + xOffset ];
        } else {
            yOffset = yOffset + r.height;
            if( xOffset >= 0 ){ // -c
                sum -= p[ yOffset * stride + xOffset ];
            }
        }

        img.unmap( p );
        return sum;
    }

    inline float IntegralImage::area( const float * p, size_t w, size_t h, size_t stride )
    {
        w--; h--;
        return  p[ stride * h + w ]
               -p[ w - stride ]
               -p[ stride * h - 1 ]
               +p[ -stride - 1 ];
    }

    inline float IntegralImage::area( const float* ptr, size_t x, size_t y, size_t w, size_t h, size_t widthstep )
    {
		const float* p = ptr + y * widthstep + x;
        w--; h--;
        return  p[ widthstep * h + w ]
               -p[ w - widthstep ]
               -p[ widthstep * h - 1 ]
               +p[ -widthstep - 1 ];
    }

     inline float IntegralImage::area( const IMapScoped<const float>& map, size_t x, size_t y, size_t w, size_t h )
     {
        x--; y--;
        return map( x + w, y + h ) - map( x + w, y ) - map( x , y + h ) + map( x, y );
     }

}

#endif
