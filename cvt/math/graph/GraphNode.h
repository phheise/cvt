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

#ifndef CVT_GRAPHNODE_H
#define CVT_GRAPHNODE_H

#include <cvt/math/graph/GraphEdge.h>
#include <vector>
#include <iostream>

namespace cvt {
	template<typename,typename> class Graph;

	template<typename TNODE, typename TEDGE>
	class GraphNode {
		template<typename,typename> friend class Graph;
		public:
			typedef GraphEdge<TNODE,TEDGE> EDGETYPE;
			typedef GraphNode<TNODE,TEDGE> NODETYPE;

			GraphNode( const TNODE& data );
			~GraphNode();

			TNODE& data();
			const TNODE& data() const;

			size_t inSize() const;
			size_t outSize() const;
			size_t size() const;

			EDGETYPE* addEdgeTo( NODETYPE* node, const TEDGE& edata );
			EDGETYPE* addEdgeFrom( NODETYPE* node, const TEDGE& edata );

			EDGETYPE* inEdge( size_t index );
			EDGETYPE* outEdge( size_t index );

		public:
			TNODE _data;
			std::vector< EDGETYPE* > _inEdges;
			std::vector< EDGETYPE* > _outEdges;
			int _visited;
	};

	template<typename TNODE,typename TEDGE>
	inline GraphNode<TNODE,TEDGE>::GraphNode( const TNODE& data ) : _data( data ), _visited( false )
	{
	}

	template<typename TNODE,typename TEDGE>
	inline GraphNode<TNODE,TEDGE>::~GraphNode()
	{
		/* only clear outgoing edges, incoming edges are cleared by the source node */
		_inEdges.clear();
		for( typename std::vector< GraphEdge<TNODE,TEDGE>* >::const_iterator it = _outEdges.begin(), end = _outEdges.end(); it != end; ++it )
			delete *it;
		_outEdges.clear();
	}

	template<typename TNODE,typename TEDGE>
	inline const TNODE& GraphNode<TNODE,TEDGE>::data() const
	{
		return _data;
	}

	template<typename TNODE,typename TEDGE>
	inline TNODE& GraphNode<TNODE,TEDGE>::data()
	{
		return _data;
	}

	template<typename TNODE,typename TEDGE>
	inline size_t GraphNode<TNODE,TEDGE>::inSize() const
	{
		return _inEdges.size();
	}

	template<typename TNODE,typename TEDGE>
	inline size_t GraphNode<TNODE,TEDGE>::outSize() const
	{
		return _outEdges.size();
	}

	template<typename TNODE,typename TEDGE>
	inline size_t GraphNode<TNODE,TEDGE>::size() const
	{
		return _inEdges.size() + _outEdges.size();
	}

	template<typename TNODE,typename TEDGE>
	inline GraphEdge<TNODE,TEDGE>* GraphNode<TNODE,TEDGE>::addEdgeTo( GraphNode<TNODE,TEDGE>* dst, const TEDGE& edata )
	{
		if( dst == this )
			return NULL;
		GraphEdge<TNODE,TEDGE>* edge = new GraphEdge<TNODE,TEDGE>( this, dst, edata );
		_outEdges.push_back( edge );
		dst->_inEdges.push_back( edge );
		return edge;
	}

	template<typename TNODE,typename TEDGE>
	inline GraphEdge<TNODE,TEDGE>* GraphNode<TNODE,TEDGE>::addEdgeFrom( GraphNode<TNODE,TEDGE>* src, const TEDGE& edata )
	{
		if( src == this )
			return NULL;
		GraphEdge<TNODE,TEDGE>* edge = new GraphEdge<TNODE,TEDGE>( src, this, edata );
		_inEdges.push_back( edge );
		src->_outEdges.push_back( edge );
		return edge;
	}

	template<typename TNODE,typename TEDGE>
	inline GraphEdge<TNODE,TEDGE>* GraphNode<TNODE,TEDGE>::inEdge( size_t index )
	{
		return _inEdges[ index ];
	}


	template<typename TNODE,typename TEDGE>
	inline GraphEdge<TNODE,TEDGE>* GraphNode<TNODE,TEDGE>::outEdge( size_t index )
	{
		return _outEdges[ index ];
	}

}

#endif
