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

#ifndef CVT_GRAPHEDGE_H
#define CVT_GRAPHEDGE_H

namespace cvt {
	template<typename,typename> class GraphNode;
	template<typename,typename> class Graph;

	/* Edge */
	template<typename TNODE, typename TEDGE>
		class GraphEdge {
			template<typename,typename> friend class Graph;
			template<typename,typename> friend class GraphNode;
			public:
				typedef GraphNode<TNODE,TEDGE> NODETYPE;

				GraphEdge(  NODETYPE* src, NODETYPE* dst, const TEDGE& data );
				~GraphEdge();

				TEDGE& data();
				const TEDGE& data() const;
				NODETYPE* source() const;
				NODETYPE* dest() const;

			public:
				NODETYPE* _src;
				NODETYPE* _dst;
				TEDGE	  _data;
		};


	template<typename TNODE,typename TEDGE>
		inline GraphEdge<TNODE,TEDGE>::GraphEdge( NODETYPE* src, NODETYPE* dst, const TEDGE& data ) : _src( src ), _dst( dst ), _data( data )
	{
	}

	template<typename TNODE,typename TEDGE>
		inline GraphEdge<TNODE,TEDGE>::~GraphEdge()
		{
		}

	template<typename TNODE,typename TEDGE>
		inline TEDGE& GraphEdge<TNODE,TEDGE>::data()
		{
			return _data;
		}

	template<typename TNODE,typename TEDGE>
		inline const TEDGE& GraphEdge<TNODE,TEDGE>::data() const
		{
			return _data;
		}

	template<typename TNODE,typename TEDGE>
		inline GraphNode<TNODE,TEDGE>* GraphEdge<TNODE,TEDGE>::source() const
		{
			return _src;
		}

	template<typename TNODE,typename TEDGE>
		inline GraphNode<TNODE,TEDGE>* GraphEdge<TNODE,TEDGE>::dest() const
		{
			return _dst;
		}

}

#endif
