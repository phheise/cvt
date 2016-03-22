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

#include <cvt/vision/IntegralImage.h>
#include <cvt/util/Time.h>
#include <cvt/util/CVTTest.h>
#include <cvt/io/Resources.h>

#define DUMP(v) std::cout << #v << " = " << v << std::endl;

namespace cvt
{
	IntegralImage::IntegralImage( const Image & img, IntegralImageFlags flags ) : _flags( flags )
	{
        update( img );
	}

    IntegralImage::IntegralImage( IntegralImageFlags flags ) : _flags( flags )
    {
    }

    IntegralImage::~IntegralImage()
    {
    }

    void IntegralImage::update( const Image & img )
    {
        if( _flags & SUMMED_AREA ){
            _sum.reallocate( img.width(), img.height(), IFormat::floatEquivalent( img.format() ), img.memType() );
            img.integralImage( _sum );
        }
        if( _flags & SQUARED_SUMMED_AREA ){
            _sqrSum.reallocate( img.width(), img.height(), IFormat::floatEquivalent( img.format() ), img.memType() );
            img.squaredIntegralImage( _sqrSum );
        }
    }

    float IntegralImage::area( const Recti & r ) const
    {
        if( !_flags & SUMMED_AREA )
            throw CVTException( "Summed Area Table is not computed -> cannot calculate area" );

        return IntegralImage::area( _sum, r );
    }

    float IntegralImage::sqrArea( const Recti & r ) const
    {
        if( !_flags & SQUARED_SUMMED_AREA )
            throw CVTException( "Squared Summed Area Table is not computed -> cannot calculate area" );

        return IntegralImage::area( _sqrSum, r );
    }

    float IntegralImage::ncc( const Image & img,
                              const Image & otherI,
                              const IntegralImage & otherII,
                              const Recti & rOther,
                              const Vector2i & pos ) const
    {
        if( !_flags & SUMMED_AREA || !_flags & SQUARED_SUMMED_AREA ||
            !otherII.flags() & SUMMED_AREA ||!otherII.flags() & SQUARED_SUMMED_AREA ){
            throw CVTException( "NCC needs SUMMED_AREA and SQUARED_SUMMED_AREA" );
        }

        // corresponding rect in img:
        Recti iRect( pos.x, pos.y, rOther.width, rOther.height );

        float sumI = IntegralImage::area( _sum, iRect );
        float ssumI = IntegralImage::area( _sqrSum, iRect );

        float sumO = IntegralImage::area( otherII.sumImage(), rOther );
        float ssumO = IntegralImage::area( otherII.sqrSumImage(), rOther );
        float size = rOther.width * rOther.height;

        float meanP = sumO / size;
        float meanI = sumI / size;
        float sigmaPSigmaI = Math::invSqrt( ( ssumO/size - Math::sqr( meanP ) ) * ( ssumI / size - Math::sqr( meanI ) ) );

        // calc SUM( I_i * P_i )
        size_t istride, pstride;
        const uint8_t* i = img.map( &istride );
        const uint8_t* p = otherI.map( &pstride );

        const uint8_t* iPtr = i + iRect.y * istride + iRect.x;
        const uint8_t* pPtr = p + rOther.y * pstride + rOther.x;
        float mulSum = 0;
        for( int y = 0; y < iRect.height; y++ ){
            for( int x = 0; x < iRect.width; x++ ){
                mulSum += ( ( float )iPtr[ x ] * ( float )pPtr[ x ] );
            }
            iPtr += istride;
            pPtr += pstride;
        }
        img.unmap( i );
        otherI.unmap( p );

        return ( mulSum - meanP * sumI ) * sigmaPSigmaI / ( size - 1.0f);
    }

    float IntegralImage::ncc( const Image & img, const Patch & patch, const Vector2i & pos ) const
    {
        if( !_flags & SUMMED_AREA || !_flags & SQUARED_SUMMED_AREA ){
            throw CVTException( "NCC needs SUMMED_AREA and SQUARED_SUMMED_AREA" );
        }

        // corresponding rect in this img:
        Recti iRect( pos.x, pos.y, patch.width(), patch.height() );

        float sumI = IntegralImage::area( _sum, iRect );
        float ssumI = IntegralImage::area( _sqrSum, iRect );

        float size = iRect.width * iRect.height;

        float sigmaPSigmaI = Math::invSqrt( patch.variance() * ( ssumI / size - Math::sqr( sumI / size ) ) );

        // calc SUM( I_i * P_i )
        size_t istride, pstride;
        const uint8_t* i = img.map( &istride );
        const uint8_t* p = patch.data().map( &pstride );

        const uint8_t* iPtr = i + iRect.y * istride + iRect.x;
        const uint8_t* pPtr = p;
        float mulSum = 0;
        for( int y = 0; y < iRect.height; y++ ){
            for( int x = 0; x < iRect.width; x++ ){
                mulSum += ( ( float )iPtr[ x ] * ( float )pPtr[ x ] );
            }
            iPtr += istride;
            pPtr += pstride;
        }
        img.unmap( i );
        patch.data().unmap( p );

        return ( mulSum - patch.mean() * sumI ) * sigmaPSigmaI / ( size - 1.0f );
    }

    /************************INTEGRAL IMAGE TESTS *********************************/

    static float areaTest( const Image & img, const Recti & r )
    {
        size_t stride;
        const uint8_t * p = img.map( &stride );
        float sum = 0;
        for( int y = 0; y < r.height && r.y + y < ( int )img.height(); y++ ){
            for( int x = 0; x < r.width && r.x + x < ( int )img.width(); x++ ){
                sum += p[ ( r.y + y ) * stride + r.x + x ];
            }
        }
        img.unmap( p );
        return sum;
    }

    static float squaredAreaTest( const Image & img, const Recti & r )
    {
        size_t stride;
        const uint8_t * p = img.map( &stride );
        float sum = 0;
        for( int y = 0; y < r.height && y < ( int )img.height(); y++ ){
            for( int x = 0; x < r.width && x < ( int )img.width(); x++ ){
                sum += Math::sqr( ( float )p[ ( r.y + y ) * stride + r.x + x ] );
            }
        }
        img.unmap( p );
        return sum;
    }

    static void _genImage( Image & img )
    {
        size_t s0;
        uint8_t * p0 = img.map( &s0 );
        uint8_t * p = p0;

        uint8_t v = 1;
        for( size_t i = 0; i < img.height(); i++ ){
            for( size_t k = 0; k < img.width(); k++ ){
                p0[ k ] = v++;
            }
            p0+=s0;
        }
        img.unmap( p );
    }

	static void _genColorImage( Image & img )
	{
		size_t s0;
		uint8_t * p0 = img.map( &s0 );
		uint8_t * p = p0;

		uint8_t v = 1;
		for( size_t i = 0; i < img.height(); i++ ){
			for( size_t k = 0; k < img.width(); k++ ){
				p0[ k * 4 ]		= v;
				p0[ k * 4 + 1 ] = v;
				p0[ k * 4 + 2 ] = v;
				p0[ k * 4 + 3 ] = v++;
			}
			p0+=s0;
		}
		img.unmap( p );
	}

    template <typename T>
    static void _dumpImage( const Image & img )
    {
        size_t stride;

        const T * p = img.map<T>( &stride );
		const T *ptr = p;
        size_t h = img.height();
        while( h-- ){
            for( size_t x = 0; x < img.width(); x++ ){
				for( size_t c = 0; c < img.channels(); c++ )
					std::cout << std::setw( 3 ) << ( int )p[ img.channels() * x + c ] << "  ";
            }
            std::cout << std::endl;
            p +=stride;
        }
        img.unmap( ptr );
    }

    static void _tableSum( Image & img, Image & sumImage ){
        size_t iStride, sStride;
        uint8_t * ip = img.map( &iStride );
        float * is = sumImage.map<float>( &sStride );

        for( size_t y = 0; y < img.height(); y++ ){
            float rowSum = 0;
            for( size_t x = 0; x < img.width(); x++ ){
                rowSum += ip[ y * iStride + x ];
                if( y > 0 )
                    is[ y * sStride + x ] = rowSum + is[ ( y-1 ) * sStride + x ];
                else
                    is[ y * sStride + x ] = rowSum;
            }
        }
        img.unmap( ip );
        sumImage.unmap( is );
    }

    static void _tableSqrSum( Image & img, Image & sumImage ){
        size_t iStride, sStride;
        uint8_t * ip = img.map( &iStride );
        float * is = sumImage.map<float>( &sStride );

        for( size_t y = 0; y < img.height(); y++ ){
            float rowSum = 0.0f;
            for( size_t x = 0; x < img.width(); x++ ){
                rowSum += Math::sqr( ( float )ip[ y * iStride + x ] );
                if( y > 0 )
                    is[ y * sStride + x ] = rowSum + is[ ( y - 1 ) * sStride + x ];
                else
                    is[ y * sStride + x ] = rowSum;
            }
        }
        img.unmap( ip );
        sumImage.unmap( is );
    }

    static bool _compare( const Image & gt, const Image & comp )
    {
        size_t iStride, sStride;
        const float * ip = gt.map<float>( &iStride );
        const float * is = comp.map<float>( &sStride );

        bool ret = true;
        for( size_t y = 0; y < gt.height(); y++ ){
            for( size_t x = 0; x < gt.width(); x++ ){

                if( Math::abs( ip[ y * iStride + x ] - is[ y * sStride + x ] ) > Math::EPSILONF ){
                    std::cout << "Error: Pos = " << y << ", " << x << ": GT = " << ip[ y * iStride + x ] << " Computed: " << is[ y * iStride + x ] << std::endl;
                    ret = false;
                    break;
                }
            }
            if( !ret )
                break;
        }
        gt.unmap( ip );
        comp.unmap( is );
        return ret;
    }

    static bool _rectSum( const Image & img, const Image & sum, const Recti & rect )
    {
        bool success = true;
        float iiArea;
        iiArea = IntegralImage::area( sum, rect );

        float iArea;
        iArea = areaTest( img, rect );

        bool area1 = Math::abs( iArea - iiArea ) < Math::EPSILONF;
        success &= area1;
		if( !area1 )
			CVTTEST_PRINT( "IntegralImage::area( Image, Rect )", area1 );

        size_t stride;
        const float * ptr = sum.map<float>( &stride );
        float a2 = IntegralImage::area( ( ptr + stride * rect.y + rect.x ), rect.width, rect.height, stride );

        bool area2 = Math::abs( iArea - a2 ) < Math::EPSILONF;
        if( !area2 ) {
			std::cout << "GT: " << iArea << " Result: " << a2 << std::endl;
			CVTTEST_PRINT( "IntegralImage::area( float*, width, height, stride )", area2 );
		}
        success &= area2;

        float a3 = IntegralImage::area( ptr, rect.x, rect.y, rect.width, rect.height, stride );
        bool area3 = Math::abs( iArea - a3 ) < Math::EPSILONF;
        if( !area3 ) {
			std::cout << "GT: " << iArea << " Result: " << a3 << std::endl;
			CVTTEST_PRINT( "IntegralImage::area( float*, x, y, width, height, stride )", area3 );
		}
        success &= area3;

        {
            IMapScoped<const float> map( sum );
            float a4 = IntegralImage::area( map, rect.x, rect.y, rect.width, rect.height );
            bool area4 = Math::abs( iArea - a4 ) < Math::EPSILONF;
            if( !area4 ) {
                std::cout << "GT: " << iArea << " Result: " << a4 << std::endl;
                CVTTEST_PRINT( "IntegralImage::area( IMapScoped, x, y, width, height )", area4 );
            }
            success &= area4;
        }

        sum.unmap( ptr );

        return success;
    }

    static bool _sqrRectSum( const Image & img, const Image & sum, const Recti & rect )
    {
        float iiArea;
        iiArea = IntegralImage::area( sum, rect );

        float iArea;
        iArea = squaredAreaTest( img, rect );

        //std::cout << "Area Test -> Rect = " << rect << " " << iiArea << " ?== " << iArea << std::endl;
        return Math::abs( iArea - iiArea ) < Math::EPSILONF;
    }

    BEGIN_CVTTEST( IntegralImage )

    Image img( 20, 20, IFormat::GRAY_UINT8 );
    Image imgC( 10, 10, IFormat::RGBA_UINT8 );

    _genImage( img );
	_genColorImage( imgC );

    IntegralImage ii( img, ( SUMMED_AREA | SQUARED_SUMMED_AREA ) );

    Image gtSum( img.width(), img.height(), IFormat::GRAY_FLOAT );
    _tableSum( img, gtSum );
    Image gtSqrSum( img.width(), img.height(), IFormat::GRAY_FLOAT );
    _tableSqrSum( img, gtSqrSum );

    bool result = true;
    bool test;

    test = _compare( gtSum, ii.sumImage() );
    if( !test ){
        std::cout << "GT: \n";
        _dumpImage<float>( gtSum );
        std::cout << "\nIntImage: \n";
        _dumpImage<float>( ii.sumImage() );
    }
    CVTTEST_PRINT("::summedImage( ... )",  test );
    result &= test;

    test = _compare( gtSqrSum, ii.sqrSumImage() );
    if( !test ){
        std::cout << "GT: \n";
        _dumpImage<float>( gtSqrSum );
        std::cout << "\nIntImage: \n";
        _dumpImage<float>( ii.sqrSumImage() );
    }
    CVTTEST_PRINT("::sqrSummedImage( ... )",  test );
    result &= test;

	CVTTEST_PRINT( "::integralImage( COLOR ) ", true );
	Image iic;
	imgC.integralImage( iic );
	
	//_dumpImage<float>( iic );

    Recti rect( 0, 0, 4, 4 );
    test = true;
    for( int y = 1; y < 10; y++ ){
        rect.y = y;
        for( int x = 1; x < 15; x++ ){
            rect.x = x;
            test &= _rectSum( img, ii.sumImage(), rect );
        }
    }
    CVTTEST_PRINT("::area( ... )",  test );
    result &= test;

    test = true;
    for( int y = 0; y < 10; y++ ){
        rect.y = y;
        for( int x = 0; x < 10; x++ ){
            rect.x = x;
            test &= _sqrRectSum( img, ii.sqrSumImage(), rect );
        }
    }
    CVTTEST_PRINT("::squaredArea( ... )", test );
    result &= test;

    return result;

    END_CVTTEST

}
