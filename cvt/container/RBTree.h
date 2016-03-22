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


#ifndef CVT_RBTREE_H
#define CVT_RBTREE_H

#include <stdio.h>
#include <cvt/util/Exception.h>
#include <functional>

namespace cvt {

	template<typename T, typename CMP = std::less<T> >
	class RBTree {
		private:
			enum RBNodeColor { RBNODE_RED = 1, RBNODE_BLACK = 2 };
			struct RBNode;

		public:
			class Iterator;
			class ReverseIterator;

							RBTree();
							RBTree( const RBTree& tree );
							~RBTree();

			size_t			size() const;
			void			clear();
			bool			isEmpty() const;

			Iterator		begin();
			Iterator		end();

			ReverseIterator rbegin();
			ReverseIterator rend();

			Iterator		insert( const T& data );
			Iterator		find( const T& data );
			void			remove( Iterator& it );
			void			remove( ReverseIterator& it );

			T&				front();
			const T&		front() const;

			T&				back();
			const T&		back() const;

			class Iterator {
				friend class RBTree;
				public:
				Iterator() : _it( NULL ) {}
				Iterator( const Iterator& it ) : _it( it._it ) {}
				Iterator& operator=( const Iterator& it ) { _it = it._it; return *this; }

				bool operator==( const Iterator& it ) { return _it == it._it; }
				bool operator!=( const Iterator& it ) { return _it != it._it; }

				Iterator& operator++() { _it = _it->next(); return *this; }
				Iterator operator++( int ) { Iterator ret( *this ); _it = _it->next(); return ret; }
				Iterator& operator--() { _it = _it->prev(); return *this; }
				Iterator operator--( int ) { Iterator ret( *this ); _it = _it->prev(); return ret; }

				T& operator*() { return _it->_data; }

				private:
				Iterator( RBNode* node ) : _it( node ) {}

				RBNode* _it;
			};

			class ReverseIterator {
				friend class RBTree;
				public:
				ReverseIterator() : _it( NULL ) {}
				ReverseIterator( const ReverseIterator& it ) : _it( it._it ) {}
				ReverseIterator& operator=( const ReverseIterator& it ) { _it = it._it; return *this; }

				bool operator==( const ReverseIterator& it ) { return _it == it._it; }
				bool operator!=( const ReverseIterator& it ) { return _it != it._it; }

				ReverseIterator& operator++() { _it = _it->prev(); return *this; }
				ReverseIterator operator++( int ) { ReverseIterator ret( *this ); _it = _it->prev(); return ret; }
				ReverseIterator& operator--() { _it = _it->next(); return *this; }
				ReverseIterator operator--( int ) { Iterator ret( *this ); _it = _it->next(); return ret; }

				T& operator*() { return _it->_data; }

				private:
				ReverseIterator( RBNode* node ) : _it( node ) {}

				RBNode* _it;
			};

		private:
			struct RBNode {
				RBNode( const T& data ) :
					_color( RBNODE_RED ),
					_parent( NULL ),
					_left( NULL ),
					_right( NULL ),
					_data( data ) {}
				~RBNode() {}

				RBNode* next();
				RBNode* prev( );

				RBNodeColor _color;
				RBNode*		_parent;
				RBNode*		_left;
				RBNode*		_right;
				T			_data;
			};

			RBNode* minimum();
			RBNode* maximum();
			void remove( RBNode* node );
			void rotateLeft( RBNode* node );
			void rotateRight( RBNode* node );
			void insertFixup( RBNode* node );
			void deleteFixup( RBNode* node, RBNode* parent );

			size_t		_size;
			RBNode*		_root;
			CMP			_cmp;
	};


	template<typename T, typename CMP>
	inline typename RBTree<T,CMP>::RBNode* RBTree<T,CMP>::RBNode::next()
	{
		RBNode* it, *it2;

		if( _right ) {
			it = _right;
			while( it->_left )
				it = it->_left;
			return it;
		}

		it2 = this;
		it = _parent;
		while( it && it2 == it->_right ) {
			it2 = it;
			it = it->_parent;
		}
		return it;
	}

	template<typename T, typename CMP>
	inline typename RBTree<T,CMP>::RBNode* RBTree<T,CMP>::RBNode::prev()
	{
		RBNode* it, *it2;

		if( _left ) {
			it = _left;
			while( it->_right )
				it = it->_right;
			return it;
		}

		it2 = this;
		it = _parent;
		while( it && it2 == it->_left ) {
			it2 = it;
			it = it->_parent;
		}
		return it;
	}

	template<typename T, typename CMP>
	inline RBTree<T,CMP>::RBTree() : _size( 0 ), _root( 0 )
	{
	}

	template<typename T, typename CMP>
	inline RBTree<T,CMP>::~RBTree()
	{
		clear();
	}

	template<typename T, typename CMP>
	inline size_t RBTree<T, CMP>::size() const
	{
		return _size;
	}

	template<typename T, typename CMP>
	inline bool RBTree<T,CMP>::isEmpty() const
	{
		return _size == 0;
	}

	template<typename T, typename CMP>
	inline void RBTree<T,CMP>::clear()
	{
		RBNode* n = _root, * tmp;

		while( n ) {
			if( n->_left ) {
				n = n->_left;
			} else if( n->_right ) {
				n = n->_right;
			} else {
				tmp = n->_parent;
				if( tmp ) {
					if( tmp->_left == n )
						tmp->_left = NULL;
					else
						tmp->_right = NULL;
				}
				delete n;
				n = tmp;
			}
		}
		_root = NULL;
		_size = 0;
	}

	template<typename T, typename CMP>
	T& RBTree<T,CMP>::front()
	{
		return minimum()->_data;
	}

	template<typename T, typename CMP>
	const T& RBTree<T,CMP>::front() const
	{
		return minimum()->_data;
	}

	template<typename T, typename CMP>
	T& RBTree<T,CMP>::back()
	{
		return maximum()->_data;
	}

	template<typename T, typename CMP>
	const T& RBTree<T,CMP>::back() const
	{
		return maximum()->_data;
	}

	template<typename T, typename CMP>
	inline typename RBTree<T,CMP>::Iterator RBTree<T,CMP>::begin( )
	{
		return Iterator( minimum() );
	}

	template<typename T, typename CMP>
	inline typename RBTree<T,CMP>::Iterator RBTree<T,CMP>::end( )
	{
		return Iterator( NULL );
	}

	template<typename T, typename CMP>
	inline typename RBTree<T,CMP>::ReverseIterator RBTree<T,CMP>::rbegin( )
	{
		return ReverseIterator( maximum() );
	}

	template<typename T, typename CMP>
	inline typename RBTree<T,CMP>::ReverseIterator RBTree<T,CMP>::rend( )
	{
		return ReverseIterator( NULL );
	}

	template<typename T, typename CMP>
	inline typename RBTree<T,CMP>::Iterator RBTree<T,CMP>::find( const T& data )
	{
		RBNode* cnode;

		cnode = _root;
		while( cnode ) {
			if( _cmp( data, cnode->_data  ) )
				cnode = cnode->_left;
			else if( _cmp( cnode->_data, data ) )
				cnode = cnode->_right;
			else
				return Iterator( cnode );
		}
		return Iterator( NULL );
	}

	template<typename T, typename CMP>
	inline typename RBTree<T,CMP>::Iterator RBTree<T,CMP>::insert( const T& data )
	{
		RBNode* newnode, *cnode, *dnode = NULL;

		_size += 1;
		newnode = new RBNode( data );

		cnode = _root;
		while( cnode ) {
			dnode = cnode;
			if( _cmp( data, cnode->_data ) )
				cnode = cnode->_left;
			else
				cnode = cnode->_right;
		}
		newnode->_parent = dnode;
		if( !dnode )
			_root = newnode;
		else if( _cmp( data, dnode->_data ) )
			dnode->_left = newnode;
		else
			dnode->_right = newnode;

		insertFixup( newnode );

		return Iterator( newnode );
	}


	template<typename T, typename CMP>
	inline void RBTree<T,CMP>::remove( Iterator& it )
	{
		remove( it._it );
	}


	template<typename T, typename CMP>
	inline void RBTree<T,CMP>::remove( ReverseIterator& it )
	{
		remove( it._it );
	}


	template<typename T, typename CMP>
	inline void RBTree<T,CMP>::remove( RBNode* node )
	{
		RBNode* y;
		RBNode* x;

		if( !node )
			throw CVTException( "Can't remove end()-Iterator!");

		_size -= 1;

		if( !node->_left || !node->_right ) {
			y = node;
			if( y->_left )
				x = y->_left;
			else
				x = y->_right;

			if( x )
				x->_parent = y->_parent;

			if( !y->_parent )
				_root = x;
			else {
				if( y == y->_parent->_left )
					y->_parent->_left = x;
				else
					y->_parent->_right = x;
			}

		} else {
			RBNode* parent;
			RBNodeColor color;

			y = node->_right;
			while( y->_left )
				y = y->_left;

			x = y->_right;
			parent = y->_parent;
			color = y->_color;

			if( x )
				x->_parent = y->_parent;

			if( !y->_parent )
				_root = x;
			else if( y == y->_parent->_left )
				y->_parent->_left = x;
			else
				y->_parent->_right = x;

			if( y->_parent == node )
				parent = y;

			y->_parent = node->_parent;
			y->_color = node->_color;
			y->_right = node->_right;
			y->_left = node->_left;

			if( !node->_parent )
				_root = y;
			else if( node == node->_parent->_left )
				node->_parent->_left = y;
			else
				node->_parent->_right = y;

			y->_left->_parent = y;
			if( y->_right )
				y->_right->_parent = y;

			y = node;
			y->_parent = parent;
			y->_color = color;
		}

		if( y->_color == RBNODE_BLACK ) {
			deleteFixup( x, y->_parent );
		}

		delete y;
	}

	template<typename T, typename CMP>
	inline typename RBTree<T,CMP>::RBNode* RBTree<T,CMP>::minimum()
	{
		RBNode* it;

		if( !_root )
			return NULL;

		it = _root;
		while( it->_left )
			it = it->_left;
		return it;
	}


	template<typename T, typename CMP>
	inline typename RBTree<T,CMP>::RBNode* RBTree<T,CMP>::maximum()
	{
		RBNode* it;
		if( !_root )
			return NULL;

		it = _root;
		while( it->_right )
			it = it->_right;
		return it;
	}

	template<typename T, typename CMP>
	inline void RBTree<T,CMP>::rotateLeft( RBNode* node )
	{
		RBNode* y = node->_right;

		node->_right = y->_left;
		if( y->_left )
			y->_left->_parent = node;
		y->_parent = node->_parent;
		if( !node->_parent )
			_root = y;
		else if( node == node->_parent->_left )
			node->_parent->_left = y;
		else
			node->_parent->_right = y;
		y->_left = node;
		node->_parent = y;
	}

	template<typename T, typename CMP>
	inline void RBTree<T,CMP>::rotateRight( RBNode* node )
	{
		RBNode* y = node->_left;

		node->_left = y->_right;
		if( y->_right )
			y->_right->_parent = node;
		y->_parent = node->_parent;
		if( !node->_parent )
			_root = y;
		else if( node == node->_parent->_left )
			node->_parent->_left = y;
		else
			node->_parent->_right = y;
		y->_right = node;
		node->_parent = y;
	}

	template<typename T, typename CMP>
	inline void RBTree<T,CMP>::insertFixup( RBNode* node )
	{
	    RBNode* y;

		while( _root != node && node->_parent->_color == RBNODE_RED ) {
			if( node->_parent == node->_parent->_parent->_left ) {
				y = node->_parent->_parent->_right;
				if( y && y->_color == RBNODE_RED ) {
					node->_parent->_color = RBNODE_BLACK;
					y->_color = RBNODE_BLACK;
					node->_parent->_parent->_color = RBNODE_RED;
					node = node->_parent->_parent;
				} else {
					if( node == node->_parent->_right ) {
						node = node->_parent;
						rotateLeft( node );
					}
					node->_parent->_color = RBNODE_BLACK;
					node->_parent->_parent->_color = RBNODE_RED;
					rotateRight( node->_parent->_parent );
				}
			} else {
				y = node->_parent->_parent->_left;
				if( y && y->_color == RBNODE_RED ) {
					node->_parent->_color = RBNODE_BLACK;
					y->_color = RBNODE_BLACK;
					node->_parent->_parent->_color = RBNODE_RED;
					node = node->_parent->_parent;
				} else {
					if( node == node->_parent->_left ) {
						node = node->_parent;
						rotateRight( node );
					}
					node->_parent->_color = RBNODE_BLACK;
					node->_parent->_parent->_color = RBNODE_RED;
					rotateLeft( node->_parent->_parent );
				}
			}
		}
	    _root->_color = RBNODE_BLACK;
	}

	template<typename T, typename CMP>
	inline void RBTree<T,CMP>::deleteFixup( RBNode* node, RBNode* parent )
	{
		RBNode* w;

		while( node != _root && ( !node || node->_color == RBNODE_BLACK ) ) {
			if( node == parent->_left ) {
				w = parent->_right;
				if( w->_color == RBNODE_RED ) {
					w->_color = RBNODE_BLACK;
					parent->_color = RBNODE_RED;
					rotateLeft( parent );
					w = parent->_right;
				}
				if( ( !w->_left || w->_left->_color == RBNODE_BLACK ) && ( !w->_right || w->_right->_color == RBNODE_BLACK ) ) {
					w->_color = RBNODE_RED;
					node = parent;
					parent = node->_parent;
				} else {
					if( !w->_right || w->_right->_color == RBNODE_BLACK ) {
						if( w->_left )
							w->_left->_color = RBNODE_BLACK;
						w->_color = RBNODE_RED;
						rotateRight( w );
						w = parent->_right;
					}
					w->_color = parent->_color;
					parent->_color = RBNODE_BLACK;
					if( w->_right )
						w->_right->_color = RBNODE_BLACK;
					rotateLeft( parent );
					node = _root;
				}
			} else {
				w = parent->_left;
				if( w->_color == RBNODE_RED ) {
					w->_color = RBNODE_BLACK;
					parent->_color = RBNODE_RED;
					rotateRight( parent );
					w = parent->_left;
				}
				if( ( !w->_left || w->_left->_color == RBNODE_BLACK ) && ( !w->_right || w->_right->_color == RBNODE_BLACK ) ) {
					w->_color = RBNODE_RED;
					node = parent;
					parent = node->_parent;
				} else {
					if( !w->_left || w->_left->_color == RBNODE_BLACK ) {
						if( w->_right )
							w->_right->_color = RBNODE_BLACK;
						w->_color = RBNODE_RED;
						rotateLeft( w );
						w = parent->_left;
					}
					w->_color = parent->_color;
					parent->_color = RBNODE_BLACK;
					if( w->_left )
						w->_left->_color = RBNODE_BLACK;
					rotateRight( parent );
					node = _root;
				}

			}
		}
		if( node )
			node->_color = RBNODE_BLACK;
	}
}

#endif
