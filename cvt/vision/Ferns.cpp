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

#include <cvt/vision/Ferns.h>

#include <cvt/vision/FAST.h>

namespace cvt
{

	Ferns::Ferns( uint32_t patchSize, uint32_t numOverallTests, uint32_t numFerns ) :
		_patchSize( patchSize ),
		_numFerns( numFerns ),
		_nTests( numOverallTests ),
		_trainingSamples( 15000 ),
		_featureDetector( 0 )
	{
		_ferns.reserve( numFerns );

		while( ( _nTests % _numFerns ) != 0 ){
			_nTests++;
		}
		_testsPerFern = _nTests / _numFerns;

		_featureDetector = new FAST( SEGMENT_9 );
		_featureDetector->setThreshold( 40 );
	}

	Ferns::Ferns( const std::string & fileName ) : _featureDetector( 0 )
	{
		std::ifstream file;
		std::string line;
		std::istringstream strTok;

		file.open( fileName.c_str(), std::ifstream::in );

		getline( file, line );
		strTok.str( line );
		strTok >> _patchSize;

		getline( file, line );
		strTok.clear();
		strTok.str( line );
		strTok >> _numFerns;

		getline( file, line );
		strTok.clear();
		strTok.str( line );
		strTok >> _nTests;

		getline( file, line );
		strTok.clear();
		strTok.str( line );
		strTok >> _testsPerFern;

		uint32_t numFeatures;
		getline( file, line );
		strTok.clear();
		strTok.str( line );
		strTok >> numFeatures;

        int x, y;
		std::string lineVal;
		for( size_t i = 0; i < numFeatures; i++ ){
			getline( file, line );

			strTok.clear();
			strTok.str( line );
			strTok >> x;
			strTok >> y;

			_modelFeatures.push_back( Feature2Df( x, y ) );
		}

		uint32_t numProbs;
		float	 regPrior;
		for( size_t i = 0; i < _numFerns; i++ ){
			getline( file, line );
			strTok.clear();
			strTok.str( line );
			strTok >> numProbs;

			getline( file, line );
			strTok.clear();
			strTok.str( line );
			strTok >> regPrior;

			_ferns.push_back( Fern( _testsPerFern, _patchSize ) );
			Eigen::Vector2i p0, p1;
			for( uint32_t t = 0; t < _testsPerFern; t++ ){
				getline( file, line );

				strTok.clear();
				strTok.str( line );

				strTok >> p0[ 0 ];
				strTok >> p0[ 1 ];
				strTok >> p1[ 0 ];
				strTok >> p1[ 1 ];
				_ferns.back().addTest( p0, p1 );
			}

			for( size_t p = 0; p < numProbs; p++ ){
				double prob;
				getline( file, line );
				strTok.clear();
				strTok.str( line );
				for( size_t c = 0; c < _modelFeatures.size(); c++ ){
					strTok >> prob;
					_ferns.back().setProbabilityForTest( p, c, prob );
				}

			}
		}
	}

	Ferns::~Ferns()
	{
		if( _featureDetector )
			delete _featureDetector;
	}

	void Ferns::train( const Image & img )
	{
		RNG rng( time( NULL ) );

		Eigen::Vector2i x0, x1;
		for( uint32_t i = 0; i < _numFerns; i++ ){
			_ferns.push_back( Fern( _testsPerFern, _patchSize ) );

			for( uint32_t t = 0; t < _testsPerFern; t++ ){
				x0[ 0 ] = rng.uniform( 0, _patchSize );
				x0[ 1 ] = rng.uniform( 0, _patchSize );
				x1[ 0 ] = rng.uniform( 0, _patchSize );
				x1[ 1 ] = rng.uniform( 0, _patchSize );

				_ferns.back().addTest( x0, x1 );
			}
		}

		// detect features in the "model"-image
		std::vector<Feature2Df> features;
        VectorFeature2DInserter<float> inserter( features );
		_featureDetector->extract( img, inserter );

		int32_t patchHalfSize = _patchSize >> 1;

		/* train the class */
		PatchGenerator patchGen( Rangef( 0.0f, Math::TWO_PI ), Rangef( 0.6f, 1.5f ), _patchSize, 3.0 /* noise */ );

		int x, y;
		for( size_t i = 0; i < features.size(); i++ ){
			std::cout << "FEATURE " << i+1 << " / " << features.size() << std::endl;
			x = features[ i ].pt.x;
			y = features[ i ].pt.y;

			if( ( x - patchHalfSize ) < 0 ||
			 	( x + patchHalfSize ) >= ( int32_t )img.width() ||
				( y - patchHalfSize ) < 0 ||
				( y + patchHalfSize ) >= ( int32_t )img.height() )
				continue;

			_modelFeatures.push_back( features[ i ] );

			this->trainClass( _modelFeatures.size() - 1, patchGen, img );
		}

		for( size_t i = 0; i < _ferns.size(); i++ ){
			_ferns[ i ].normalizeStatistics( _trainingSamples );
		}
	}

	void Ferns::match( const::std::vector<Eigen::Vector2i> & features,
					   const Image & img,
					   std::vector<Eigen::Vector2d> & matchedModel,
					   std::vector<Eigen::Vector2d> & matchedFeatures )
	{
		std::vector<double> bestProbsForPoint( _modelFeatures.size(), 0.0 );
		std::vector<size_t> featureIndicesForPoint( _modelFeatures.size(), 0 );

		int32_t patchHalfSize = _patchSize >> 1;

		size_t stride;
		const uint8_t * p = img.map( &stride );

		uint32_t testResult;
		for( size_t i = 0; i < features.size(); i++ ){
			if( features[ i ][ 0 ] - patchHalfSize < 0 ||
			    features[ i ][ 0 ] + patchHalfSize > ( int32_t )img.width() ||
			    features[ i ][ 1 ] - patchHalfSize < 0 ||
			    features[ i ][ 1 ] + patchHalfSize > ( int32_t )img.height() ){
				continue;
			}

			const uint8_t * uL = p + stride * ( features[ i ][ 1 ] - patchHalfSize ) + features[ i ][ 0 ] - patchHalfSize;
			std::vector<double> classProbs( _modelFeatures.size(), 0.0 );


			for( size_t f = 0; f < _ferns.size(); f++) {
				testResult = _ferns[ f ].test( uL, stride );
				const std::vector<double> & trainedProbabilities = _ferns[ f ].probsForResult( testResult );
				for( size_t k = 0; k < trainedProbabilities.size(); k++ ){
					classProbs[ k ] += trainedProbabilities[ k ];
				}
			}

			size_t bestIdx = 0;
			double probSum = exp( classProbs[ 0 ] );
			for( size_t k = 1; k < classProbs.size(); k++ ){
				probSum += exp( classProbs[ k ] );
				if( classProbs[ k ] > classProbs[ bestIdx ] )
					bestIdx = k;
			}

			double probability = exp( classProbs[ bestIdx ] ) / probSum;

			if( probability > bestProbsForPoint[ bestIdx ] ){
				bestProbsForPoint[ bestIdx ] = probability;
				featureIndicesForPoint[ bestIdx ] = i;
			}
		}
		img.unmap( p );

		// no check the result:
		for( size_t i = 0; i < bestProbsForPoint.size(); i++ ){
			std::cout << "Prob: " << bestProbsForPoint[ i ] << std::endl;
			if( bestProbsForPoint[ i ] > 0.96 ){
				matchedModel.push_back( Eigen::Vector2d( _modelFeatures[ i ].pt.x, _modelFeatures[ i ].pt.y ) );
				matchedFeatures.push_back( features[ featureIndicesForPoint[ i ] ].cast<double>() );
			}
		}

	}

	double Ferns::classify( Eigen::Vector2i & bestClass, const Image & img, const Eigen::Vector2i & p )
	{
		size_t imStride;

        const uint8_t * imP = img.map( &imStride );
		int32_t patchHalfSize = _patchSize >> 1;

        if( p[ 0 ] - patchHalfSize < 0 ||
            p[ 0 ] + patchHalfSize > ( int32_t )img.width() ||
            p[ 1 ] - patchHalfSize < 0 ||
            p[ 1 ] + patchHalfSize > ( int32_t )img.height() )
            return 0.0f;

		const uint8_t * uL = imP + imStride * ( p[ 1 ] - patchHalfSize ) + p[ 0 ] - patchHalfSize;

		std::vector<double> classProbs( _modelFeatures.size(), 0.0 );

		uint32_t testResult;

		// for each fern
		for( size_t i = 0; i < _ferns.size(); i++ ){
			testResult = _ferns[ i ].test( uL, imStride );
			const std::vector<double> & trainedProbs = _ferns[ i ].probsForResult( testResult );

			for( size_t f = 0; f < trainedProbs.size(); f++ ){
				classProbs[ f ] += trainedProbs[ f ];
			}
		}

		img.unmap( imP );

		uint32_t bestIdx = 0;
		double probSum = 0.0;
		for( size_t i = 0; i < classProbs.size(); i++ ){
			probSum += exp( classProbs[ i ] );
			if( classProbs[ i ] > classProbs[ bestIdx ] )
				bestIdx = i;
		}

		bestClass[ 0 ] = _modelFeatures[ bestIdx ].pt.x;
		bestClass[ 1 ] = _modelFeatures[ bestIdx ].pt.y;

		return exp( classProbs[ bestIdx ] ) / probSum;
	}

	void Ferns::save( const std::string & fileName )
	{
		std::ofstream out;

		out.open( fileName.c_str(), std::ios_base::out );

		out << _patchSize << std::endl;
		out << _numFerns << std::endl;
		out << _nTests << std::endl;
		out << _testsPerFern << std::endl;

		out << _modelFeatures.size() << std::endl;
		for( size_t i = 0; i < _modelFeatures.size(); i++ ){
			out << _modelFeatures[ i ].pt.x << " " << _modelFeatures[ i ].pt.y << std::endl;
		}

		for( size_t i = 0; i < _ferns.size(); i++ ){
			_ferns[ i ].serialize( out );
		}

		out.close();
	}

	void Ferns::trainClass( size_t idx, PatchGenerator & patchGen, const Image & img )
	{
		Image patch( _patchSize, _patchSize, img.format() );
		size_t pStride;
		uint8_t * p;

		for( size_t i = 0; i < _trainingSamples; i++ ) {
			// generate a new patch
			patchGen.next( patch, img, _modelFeatures[ idx ].pt );

			p = patch.map( &pStride );

			// train each fern
			for( size_t f = 0; f < _ferns.size(); f++ ){
				_ferns[ f ].trainClass( idx , p,  pStride );
			}

			patch.unmap( p );
		}
	}
}