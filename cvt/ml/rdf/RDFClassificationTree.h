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

#ifndef CVT_RDFCLASSIFICATIONTREE_H
#define CVT_RDFCLASSIFICATIONTREE_H

#include <cvt/ml/rdf/RDFNode.h>
#include <cvt/ml/rdf/RDFClassHistogram.h>

namespace cvt {
	template<typename DATA, size_t N>
	class RDFClassificationTree
	{
		public:
			RDFClassificationTree( RDFNode<DATA,RDFClassHistogram<N> >* root );
			~RDFClassificationTree();

			const RDFClassHistogram<N>& classify( const DATA& d );
		private:
			RDFClassificationTree( const RDFClassificationTree<DATA,N>& );

			RDFNode<DATA,RDFClassHistogram<N> >* _root;
	};


	template<typename DATA, size_t N>
	inline RDFClassificationTree<DATA,N>::RDFClassificationTree( RDFNode<DATA,RDFClassHistogram<N> >* root ) : _root( root )
	{
	}

	template<typename DATA, size_t N>
	inline RDFClassificationTree<DATA,N>::~RDFClassificationTree( )
	{
	}


	template<typename DATA, size_t N>
	inline const RDFClassHistogram<N>& RDFClassificationTree<DATA,N>::classify( const DATA& d )
	{
		RDFNode<DATA,RDFClassHistogram<N> >* node = _root;
		while( !node->isLeaf() ) {
			if( node->test( d ) )
				node = node->right();
			else
				node = node->left();
		}
		return *node->data();
	}

}


#endif
