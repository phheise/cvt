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

#ifndef CVT_RDFNODE_H
#define CVT_RDFNODE_H

#include <string.h>
#include <cvt/ml/rdf/RDFTest.h>

namespace cvt {

	template<typename DATA, typename NODEDATA>
	class RDFNode
	{
		public:
			RDFNode();
			RDFNode( NODEDATA* data, RDFTest<DATA>*, RDFNode<DATA,NODEDATA>* left, RDFNode<DATA,NODEDATA>* right );
			~RDFNode();

			bool				    test( const DATA& data );
			RDFNode<DATA,NODEDATA>*	left();
			RDFNode<DATA,NODEDATA>* right();
			RDFTest<DATA>*			test();
			NODEDATA*				data();
			const NODEDATA*			data() const;

			bool					isLeaf() const;

		private:
			NODEDATA*				_data;
			RDFTest<DATA>*			_test;
			RDFNode<DATA,NODEDATA>* _left;
			RDFNode<DATA,NODEDATA>* _right;
	};


	template<typename DATA, typename NODEDATA>
	inline RDFNode<DATA, NODEDATA>::RDFNode() :
		_data( NULL ),
		_test( NULL ),
		_left( NULL ),
		_right( NULL )
	{
	}

	template<typename DATA, typename NODEDATA>
	inline RDFNode<DATA, NODEDATA>::RDFNode( NODEDATA* data, RDFTest<DATA>* test, RDFNode<DATA,NODEDATA>* left, RDFNode<DATA,NODEDATA>* right  ) :
		_data( data ),
		_test( test ),
		_left( left ),
		_right( right )
	{
	}


	template<typename DATA, typename NODEDATA>
	inline RDFNode<DATA, NODEDATA>::~RDFNode()
	{
	}

	template<typename DATA, typename NODEDATA>
	inline bool RDFNode<DATA, NODEDATA>::test( const DATA& data )
	{
		return _test->operator()( data );
	}


	template<typename DATA, typename NODEDATA>
	inline RDFTest<DATA>* RDFNode<DATA, NODEDATA>::test()
	{
		return _test;
	}

	template<typename DATA, typename NODEDATA>
	inline RDFNode<DATA,NODEDATA>* RDFNode<DATA, NODEDATA>::left()
	{
		return _left;
	}

	template<typename DATA, typename NODEDATA>
	inline RDFNode<DATA,NODEDATA>* RDFNode<DATA, NODEDATA>::right()
	{
		return _right;
	}

	template<typename DATA, typename NODEDATA>
	inline NODEDATA* RDFNode<DATA, NODEDATA>::data()
	{
		return _data;
	}

	template<typename DATA, typename NODEDATA>
	inline const NODEDATA* RDFNode<DATA, NODEDATA>::data() const
	{
		return _data;
	}

	template<typename DATA, typename NODEDATA>
	inline bool RDFNode<DATA, NODEDATA>::isLeaf() const
	{
		return _test == NULL;
	}
}

#endif
