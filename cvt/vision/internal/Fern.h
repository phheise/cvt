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

#ifndef CVT_FERN_H
#define CVT_FERN_H

/* a fern is a set of pixel tests */
namespace cvt {
	class Fern
	{
		public:
			Fern( uint32_t numTests, uint32_t patchSize ) : 
				_regPrior( 1.0 ),
					_patchSize( patchSize )
			{					
				_tests.reserve( numTests );
				_numProbs = ( 1 << numTests );
			
				// one result per class
				_probsForResult.resize( _numProbs );
			}
		
			~Fern()
			{
			}			
		
			// add a test to the group
			void addTest( const Eigen::Vector2i & p0, const Eigen::Vector2i & p1 )
			{
				_tests.push_back( std::make_pair( p0, p1 ) );
			}
		
			/* imP must point to the top left corner of the patch */
			uint32_t test( const uint8_t * imP, size_t stride )
			{										
				uint32_t idx = 0;
				for( size_t i = 0; i < _tests.size(); i++ ){						
					Eigen::Vector2i & x0 = _tests[ i ].first;
					Eigen::Vector2i & x1 = _tests[ i ].second;
				
					if( imP[ x0[ 1 ] * stride + x0[ 0 ] ] < imP[ x1[ 1 ] * stride + x1[ 0 ] ] )
						idx++;
					idx <<= 1;
				}
				idx >>= 1;
			
				return idx;
			}
	
			void normalizeStatistics( uint32_t numSamples )
			{
				// for each possible testResult
				for( size_t t = 0; t < _probsForResult.size(); t++ ){					
					// for each class
					for( size_t c = 0; c < _probsForResult[ t ].size(); c++ ){
						_probsForResult[ t ][ c ] = log( ( _probsForResult[ t ][ c ] + _regPrior ) / ( ( double )numSamples + _numProbs * _regPrior ) );
					}
				}		
			}
		
			/* update statistics for a class with a new patch information */
			void trainClass( size_t idx, const uint8_t * p, size_t pStride )
			{	
				size_t classIndex = _probsForResult[ 0 ].size();
			
				if( classIndex <= idx ){
					// it's a new class: initialize probabilities with 0.0
					for( size_t i = 0; i < _probsForResult.size(); i++ ){
						_probsForResult[ i ].push_back( 0.0 );
					}					
				}				
						
				uint32_t testResult;
					
				testResult = test( p, pStride );
				_probsForResult[ testResult ][ idx ] += 1.0;
			}
		
			const std::vector<double> & probsForResult( uint32_t result )
			{
				return _probsForResult[ result ];					
			}
				
			void setProbabilityForTest( size_t p, size_t c, double prob )
			{
				if( _probsForResult[ p ].size() <= c ){
					_probsForResult[ p ].resize( c + 1 );
				}
								
				_probsForResult[ p ][ c ] = prob;
			}
		
			void serialize( std::ofstream & out )
			{
				out << _numProbs << std::endl;
				out << _regPrior << std::endl;
			
				for( size_t t = 0; t < _tests.size(); t++){
					out << _tests[ t ].first.x() << " "
						<< _tests[ t ].first.y() << " "
						<< _tests[ t ].second.x() << " "
						<< _tests[ t ].second.y() << std::endl;
				}
			
				for( size_t i = 0; i <  _probsForResult.size(); i++ ){
					for( size_t c = 0; c < _probsForResult[ i ].size(); c++ ){
						out << _probsForResult[ i ][ c ] << " ";
					}
					out << std::endl;
				}
			}
		
			private:				
				// tests in a fern: two points w.r.t. upper left corner of patch
				typedef std::pair<Eigen::Vector2i, Eigen::Vector2i> PixelPair;
				std::vector<PixelPair>	_tests;
				uint32_t				_numProbs;
				float					_regPrior;
				uint32_t				_patchSize;
			
				/* trained probabilities for each possible result and classes (2^S per class) */
				std::vector< std::vector<double> > _probsForResult;			
	};
}

#endif