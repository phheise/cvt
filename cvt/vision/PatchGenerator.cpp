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

#include <cvt/vision/PatchGenerator.h>


namespace cvt
{
	
	PatchGenerator::PatchGenerator( const Rangef & angleRange,
									const Rangef & scaleRange, 
									uint32_t patchSize,
									double whiteNoiseSigma ) :
		_patchSize( patchSize ), 
		_angleRange( angleRange ), 
		_scaleRange( scaleRange ),		
		_whiteNoiseSigma( whiteNoiseSigma ),
		_rng( time( NULL ) )
	{
	}
	
	PatchGenerator::~PatchGenerator()
	{
	}
	
	void PatchGenerator::randomizeAffine()
	{
		float phi, theta, s1, s2;
		Eigen::Matrix2f R_phi, R_theta, S;
		
		phi = _rng.uniform( _angleRange.min, _angleRange.max );
		theta = _rng.uniform( _angleRange.min, _angleRange.max );
		s1 = _rng.uniform( _scaleRange.min, _scaleRange.max );
		s2 = _rng.uniform( _scaleRange.min, _scaleRange.max );	
		
		S << s1, 0.0f, 0.0f, s2;
		
		float c = Math::cos( phi );
		float s = Math::sin( phi );		
		R_phi << c, -s, s, c; 
		
		c = Math::cos( theta );
		s = Math::sin( theta );		
		R_theta << c, -s, s, c;
		
		_affine = R_theta * R_phi.transpose() * S * R_phi;
	}
	
	void PatchGenerator::next( Image & outputPatch, const cvt::Image & inputImage, const Vector2f & patchCenter )
	{
		// generate new random affine Transform
		randomizeAffine();
		
		outputPatch.reallocate( _patchSize, _patchSize, inputImage.format() );
		
		size_t inStride, outStride;
		const uint8_t * in = inputImage.map( &inStride );
		uint8_t * out = outputPatch.map( &outStride );
		uint8_t * outSave = out;
		
		Eigen::Vector2f currentP, pPrime;
		
		int32_t x0, y0, x1, y1;	
		uint32_t numChannels = outputPatch.channels();
		
		currentP[ 1 ] = -(float)_patchSize / 2.0f;	
		float fracX, fracY;
		double pixelNoise;
		int32_t tmp0, tmp1, tmp;
		for( uint32_t i = 0; i < _patchSize; i++ ){
			currentP[ 0 ] = -(float)_patchSize / 2.0f;
			for( uint32_t j = 0; j < _patchSize; j++ ){	
				pPrime = _affine * currentP;
												
				x0 = int32_t( pPrime[ 0 ] + float( patchCenter[ 0 ] ) );
				y0 = int32_t( pPrime[ 1 ] + float( patchCenter[ 1 ] ) );
				x1 = x0++;
				y1 = y0++;
				fracX = pPrime[ 0 ] - Math::floor( pPrime[ 0 ] );
				fracY = pPrime[ 1 ] - Math::floor( pPrime[ 1 ] );
				
				if( x0 < 0 ||
				    y0 < 0 ||
				    ( uint32_t )x0 >= inputImage.width() ||
					( uint32_t )y0 >= inputImage.height() ){
					for( size_t c = 0; c < numChannels; c++ )
						out[ numChannels * j  + c ] = 0;
				} else {
					pixelNoise = Math::clamp( _rng.gaussian( _whiteNoiseSigma ), -20.0, 20.0 );
					
					for( size_t c = 0; c < numChannels; c++ ){						
						if( ( uint32_t )x1 < inputImage.width() )
							tmp0 = Math::mix( in[ y0 * inStride + x0 * numChannels + c ], in[ y0 * inStride + x1 * numChannels + c ], fracX );
						else
							tmp0 = in[ y0 * inStride + x0 * numChannels + c ];
						if( ( uint32_t )y1 < inputImage.height() )
							tmp1 = Math::mix( in[ y1 * inStride + x0 * numChannels + c ], in[ y1 * inStride + x1 * numChannels + c ], fracX );
						else
							tmp1 = in[ y1 * inStride + x0 * numChannels + c ];
						tmp = Math::mix( tmp0, tmp1, fracY ) + pixelNoise;
						
						out[ numChannels * j + c ] = ( uint8_t )Math::clamp( tmp, 0, 255 );
					}
				}
				currentP[ 0 ] += 1.0f;
			}
			
			out += outStride;
			currentP[ 1 ] += 1.0f;
		}
		
		inputImage.unmap( in );
		outputPatch.unmap( outSave );
		
		Image tmpPatch( outputPatch );
		
		outputPatch.convolve( tmpPatch, IKernel::GAUSS_HORIZONTAL_3 );
		tmpPatch.convolve( outputPatch, IKernel::GAUSS_VERTICAL_3 );
	}
	
}