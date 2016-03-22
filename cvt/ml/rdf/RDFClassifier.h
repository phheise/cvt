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

#ifndef CVT_RDFCLASSIFIER_H
#define CVT_RDFCLASSIFIER_H

#include <vector>
#include <cvt/ml/rdf/RDFClassificationTree.h>

namespace cvt {
	template<typename DATA,size_t N>
	class RDFClassifier {
		public:
			RDFClassifier();
			~RDFClassifier();

			void    addTree( RDFClassificationTree<DATA,N>* tree );
			size_t  treeCount() const;

			void    classify( RDFClassHistogram<N>& classhist, const DATA& data ) const;

		private:
			std::vector<RDFClassificationTree<DATA,N>*> _trees;
	};

	template<typename DATA, size_t N>
	inline RDFClassifier<DATA,N>::RDFClassifier()
	{
	}

	template<typename DATA, size_t N>
	inline RDFClassifier<DATA,N>::~RDFClassifier()
	{
	}

	template<typename DATA, size_t N>
	inline void RDFClassifier<DATA,N>::addTree( RDFClassificationTree<DATA,N>* tree )
	{
		_trees.push_back( tree );
	}

	template<typename DATA, size_t N>
	inline size_t RDFClassifier<DATA,N>::treeCount() const
	{
		return _trees.size();
	}

	template<typename DATA, size_t N>
	inline void RDFClassifier<DATA,N>::classify( RDFClassHistogram<N>& chist, const DATA& data ) const
	{
		size_t iend = _trees.size();
		chist.clear();
		for( size_t i = 0; i < iend; i++ ) {
			chist += _trees[ i ]->classify( data );
		}
	}

}

#endif
