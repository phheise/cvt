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

#ifndef CVT_RDFORESTTRAINERCLASSIFICATION_H
#define CVT_RDFORESTTRAINERCLASSIFICATION_H

#include <vector>
#include <cvt/ml/rdf/RDFNode.h>
#include <cvt/ml/rdf/RDFTest.h>
#include <cvt/ml/rdf/RDFClassHistogram.h>
#include <cvt/ml/rdf/RDFClassificationTree.h>

namespace cvt {

	template<typename DATA, typename DATACOLLECTION, size_t N>
	class RDFClassificationTrainer
	{
		public:
			RDFClassificationTrainer();
			~RDFClassificationTrainer();

			size_t				   classCount() const { return N; }
			virtual size_t		   dataSize( const DATACOLLECTION& data ) = 0;
			virtual RDFTest<DATA>* randomTest() = 0;
			virtual size_t		   classLabel( const DATACOLLECTION& data, size_t index ) = 0;
			virtual DATA&		   trainingData( const DATACOLLECTION& data, size_t index ) = 0;

			RDFClassificationTree<DATA,N>* train( const DATACOLLECTION& data, size_t maxdepth, size_t randTries );

		private:
			RDFNode<DATA,RDFClassHistogram<N> >* trainRecursive( const DATACOLLECTION& data, const RDFClassHistogram<N>& hist, std::vector<size_t>& indices, size_t level, size_t randTries );

			static float IG( const RDFClassHistogram<N>& parent, const RDFClassHistogram<N>& left, const RDFClassHistogram<N>& right );
	};

	template<typename DATA, typename DATACOLLECTION, size_t N>
	inline RDFClassificationTrainer<DATA,DATACOLLECTION,N>::RDFClassificationTrainer()
	{
	}

	template<typename DATA, typename DATACOLLECTION, size_t N>
	inline RDFClassificationTrainer<DATA,DATACOLLECTION,N>::~RDFClassificationTrainer()
	{
	}


	template<typename DATA, typename DATACOLLECTION, size_t N>
	inline float RDFClassificationTrainer<DATA,DATACOLLECTION,N>::IG( const RDFClassHistogram<N>& parent, const RDFClassHistogram<N>& left, const RDFClassHistogram<N>& right )
	{
		float entropyParent = parent.entropy();
		size_t allSamples = left.sampleCount() + right.sampleCount();

		if( allSamples <= 1 )
			return 0.0;

		float entropyChildren = ( left.sampleCount() * left.entropy() + right.sampleCount() * right.entropy()) / allSamples;
		return entropyParent - entropyChildren;
	}

	template<typename DATA, typename DATACOLLECTION, size_t N>
	inline RDFClassificationTree<DATA,N>* RDFClassificationTrainer<DATA,DATACOLLECTION,N>::train( const DATACOLLECTION& data, size_t level, size_t randTries )
	{
		RDFTest<DATA>* best = NULL;
		RDFNode<DATA,RDFClassHistogram<N> >* left;
		RDFNode<DATA,RDFClassHistogram<N> >* right;
		float IGmax = 0.0f;
		RDFClassHistogram<N> histparent, histleft, histright;
		RDFClassHistogram<N> histbestleft, histbestright;

		// get histogram for the root
		const size_t size = dataSize( data );
		for( size_t i = 0; i < size; i++ ) {
			histparent.addSample( classLabel( data, i ) );
		}

		for( size_t t = 0; t < randTries; t++ ) {
			// new random test
			RDFTest<DATA>* test = randomTest();
			histright.clear();
			histleft.clear();

			for( size_t i = 0; i < size; i++ ) {
				if( test->operator()( trainingData( data, i ) ) ) {
					histright.addSample( classLabel( data, i ) );
				} else {
					histleft.addSample( classLabel( data, i ) );
				}
			}

			// how good is the information gain
			// keep the best otherwise die like the rest
			float ig = IG( histparent, histleft, histright);
			if( ig > IGmax ) {
				IGmax = ig;
				if( best )
					delete best;
				best = test;
				histbestleft = histleft;
				histbestright = histright;
				std::cout << ( 100.0f * ( float ) t / ( float ) randTries) << " \%  IG:" << IGmax << std::endl;
			} else {
				delete test;
			}
		}

		// split the data into two sets containing the indices
		std::vector<size_t> leftindices, rightindices;
		for( size_t i = 0; i < size; i++ ) {
			if( best->operator()( trainingData( data, i ) ) ) {
				rightindices.push_back( i );
			} else {
				leftindices.push_back( i );
			}
		}

		left  = trainRecursive( data, histbestleft, leftindices, level - 1, randTries );
		right = trainRecursive( data, histbestright, rightindices, level - 1, randTries );
		return new RDFClassificationTree<DATA,N>( new RDFNode<DATA,RDFClassHistogram<N> >( NULL, best, left, right ) );
	}

	template<typename DATA, typename DATACOLLECTION, size_t N>
	inline RDFNode<DATA,RDFClassHistogram<N> >* RDFClassificationTrainer<DATA,DATACOLLECTION,N>::trainRecursive( const DATACOLLECTION& data, const RDFClassHistogram<N>& hist, std::vector<size_t>& indices, size_t level, size_t randTries )
	{
		RDFTest<DATA>* best = NULL;
		RDFNode<DATA,RDFClassHistogram<N> >* left;
		RDFNode<DATA,RDFClassHistogram<N> >* right;
		float IGmax = 0.0f;
		RDFClassHistogram<N> histparent, histleft, histright;
		RDFClassHistogram<N> histbestleft, histbestright;

		if( !level ) {
			return new RDFNode<DATA,RDFClassHistogram<N> >( new RDFClassHistogram<N>( hist ), NULL, NULL, NULL );
		}

		const size_t size = indices.size();
		for( size_t t = 0; t < randTries; t++ ) {
			// new random test
			RDFTest<DATA>* test = randomTest();
			histright.clear();
			histleft.clear();

			for( size_t i = 0; i < size; i++ ) {
				size_t idx = indices[ i ];
				if( test->operator()( trainingData( data, idx ) ) ) {
					histright.addSample( classLabel( data, idx ) );
				} else {
					histleft.addSample( classLabel( data, idx ) );
				}
			}

			// how good is the information gain
			// keep the best otherwise die like the rest
			float ig = IG( hist, histleft, histright);
			if( ig > IGmax ) {
				IGmax = ig;
				if( best )
					delete best;
				best = test;
				histbestleft = histleft;
				histbestright = histright;
				std::cout << ( 100.0f * ( float ) t / ( float ) randTries) << " \%  IG:" << IGmax << " Level: " << level << std::endl;
			} else {
				delete test;
			}
		}

		if( !best ) {
			return new RDFNode<DATA,RDFClassHistogram<N> >( new RDFClassHistogram<N>( hist ), NULL, NULL, NULL );
		}

		// split the data into two sets containing the indices
		std::vector<size_t> leftindices, rightindices;
		for( size_t i = 0; i < size; i++ ) {
			size_t idx = indices[ i ];
			if( best->operator()( trainingData( data, idx ) ) ) {
				rightindices.push_back( idx );
			} else {
				leftindices.push_back( idx );
			}
		}

		left  = trainRecursive( data, histbestleft, leftindices, level - 1, randTries );
		right = trainRecursive( data, histbestright, rightindices, level - 1, randTries );
		return new RDFNode<DATA,RDFClassHistogram<N> >( NULL, best, left, right );
	}

}

#endif
