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

#ifndef CVT_RDFORESTTRAINERCLASSIFICATION2D_H
#define CVT_RDFORESTTRAINERCLASSIFICATION2D_H

#include <cvt/math/Vector.h>
#include <cvt/ml/rdf/RDFClassificationTrainer.h>

#include <cvt/gfx/Image.h>
#include <cvt/gfx/IMapScoped.h>
#include <cvt/ml/rdf/RDFClassifier.h>

namespace cvt {

		class RDFTestLinear2D : public RDFTest<Vector2f>
		{
			public:
				RDFTestLinear2D( const Vector2f& vec, float threshold  ) : _norm( vec ), _threshold( threshold ) {}

				bool operator()( const Vector2f& other )
				{
					return Math::abs( _norm.x *  other.x + _norm.y * other.y ) < _threshold;
				}

			private:
				Vector2f _norm;
				float	 _threshold;
		};

		class RDFClassificationTrainer2D : public RDFClassificationTrainer<Vector2f,std::vector<Vector3f>,2>
		{
			public:
				RDFClassificationTrainer2D( size_t numberOfClasses ) : _numClasses( numberOfClasses )
				{
				}

				~RDFClassificationTrainer2D()
				{
				}

				virtual size_t dataSize( const std::vector<Vector3f>& data )
				{
					return data.size();
				}

				virtual RDFTest<Vector2f>*  randomTest()
				{
					float x = Math::rand( -1.0f, 1.0f );
					float y = Math::sqrt( 1.0f - Math::sqr( x ) );
					return new RDFTestLinear2D( Vector2f( x, y ), Math::rand( -10000.0f, 10000.0f  ) );
				}

				virtual size_t classLabel( const std::vector<Vector3f>& data, size_t index )
				{
					return ( size_t ) data[ index ].z;
				}


				virtual Vector2f& trainingData(  const std::vector<Vector3f>& data, size_t index ) {
					return *( ( Vector2f*) ( &data[ index ] ) );
				}

				static void visualizeClassifier( Image& dst, const RDFClassifier<Vector2f,2>& classifier, const Rectf& range, size_t width, size_t height );

			private:
				size_t _numClasses;
		};



		void RDFClassificationTrainer2D::visualizeClassifier( Image& dst, const RDFClassifier<Vector2f,2>& classifier, const Rectf& rect, size_t width, size_t height )
		{
			dst.reallocate( width, height, IFormat::RGBA_FLOAT );
			IMapScoped<float> map( dst );
			Vector2f pt;
			RDFClassHistogram<2> hist;
			Color ctmp, c, gray( 0.5f );

			for( size_t y = 0; y < height; y++ ) {
				float* ptr = map.ptr();
				pt.y = 1.0f - ( float ) y / ( ( float ) height - 1.0f );
				pt.y = rect.height * pt.y + rect.y;
				for( size_t x = 0; x < width; x++ ) {
					pt.x = ( float ) x / ( ( float ) width - 1.0f );
					pt.x = rect.width * pt.x + rect.x;
					classifier.classify( hist, pt );
					ctmp.mix( Color::BLUE, Color::RED, hist.probability( 0 ) );
					c.mix( ctmp, gray, hist.entropy() * 0.5 );
					*ptr++ = c.red();
					*ptr++ = c.green();
					*ptr++ = c.blue();
					*ptr++ = c.alpha();
				}
				map++;
			}
		}

}

#endif
