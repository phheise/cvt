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

#ifndef CVT_LIST_H
#define CVT_LIST_H

#include <stdint.h>
#include <string.h>
#include <iostream>
#include <functional>

namespace cvt {

	template<typename T>
	class List {

		private:
			struct NodeBase {
				NodeBase( NodeBase* next = NULL , NodeBase* prev = NULL ) : _next( next ), _prev( prev ) {}
				NodeBase( const NodeBase& it ) : _next( it._next ), _prev( it._prev )  {}
				NodeBase& operator=( const NodeBase& it ) { if( &it != this ) { _next = it._next; _prev = it._prev; } return *this; }

				NodeBase* _next;
				NodeBase* _prev;
			};

			struct Node : public NodeBase {
				Node( const T& data, NodeBase* next = NULL, NodeBase* prev = NULL ) : NodeBase( next, prev ), _data( data ) {}
				Node( const Node& node ) : NodeBase( node._next, node._prev ), _data( node._data ) {}
				Node& operator=( const Node& it ) { if( &it != this ) { NodeBase::_next = it._next; NodeBase::_prev = it._prev; _data = it._data; } return *this; }

				T _data;
			};


		public:
			class Iterator;
			class ReverseIterator;

			List();
			~List();
			List( const List<T>& list );
			List& operator=( const List<T>& list );

			void append( const T& data );
			void prepend( const T& data );
			void clear();
			size_t size() const;
			bool isEmpty() const;

			T& front();
			T& back();
			const T& front() const;
			const T& back() const;

			Iterator find( const T& data ) const;
			template<typename T2> Iterator find( const T2& data ) const;

			Iterator begin() { return Iterator( _anchor._next ); }
			Iterator end() { return Iterator( &_anchor ); }
			Iterator remove( Iterator it );

			Iterator rbegin() { return Iterator( _anchor._prev ); }
			Iterator rend() { return Iterator( &_anchor ); }
			Iterator remove( ReverseIterator it );

			void sort();

			template<typename CMP >
			void sort( CMP cmp );

			class Iterator {
				friend class List;
				public:
					Iterator() : _it( NULL ) {}
					Iterator( const Iterator& it ) : _it( it._it ) {}
					Iterator& operator=( const Iterator& it ) { _it = it._it; return *this; }

					bool operator==( const Iterator& it ) { return _it == it._it; }
					bool operator!=( const Iterator& it ) { return _it != it._it; }

					Iterator& operator++() { _it = _it->_next; return *this; }
					Iterator operator++( int ) { Iterator ret( *this ); _it = _it->_next; return ret; }
					Iterator& operator--() { _it = _it->_prev; return *this; }
					Iterator operator--( int ) { Iterator ret( *this ); _it = _it->_prev; return ret; }

					T& operator*() const { return ( ( Node* ) _it )->_data; }
					T* operator->() const { return  &( ( Node* ) _it )->_data; }

				private:
					Iterator( NodeBase* it ) : _it( it ) {}
					NodeBase* _it;
			};

			class ReverseIterator {
				friend class List;
				public:
					ReverseIterator() : _it( NULL ) {}
					ReverseIterator( const ReverseIterator& it ) : _it( it._it ) {}
					ReverseIterator& operator=( const ReverseIterator& it ) { _it = it._it; return *this; }

					bool operator==( const ReverseIterator& it ) { return _it == it._it; }
					bool operator!=( const ReverseIterator& it ) { return _it != it._it; }

					ReverseIterator& operator++() { _it = _it->_prev; return *this; }
					ReverseIterator operator++( int ) { ReverseIterator ret( *this ); _it = _it->_prev; return ret; }
					ReverseIterator& operator--() { _it = _it->_next; return *this; }
					ReverseIterator operator--( int ) { ReverseIterator ret( *this ); _it = _it->_next; return ret; }

					T& operator*() const { return ( ( Node* ) _it )->_data; }
					T* operator->() const { return  &( ( Node* ) _it )->_data; }

				private:
					ReverseIterator( NodeBase* it ) : _it( it ) {}
					NodeBase* _it;
			};


		private:
			void unlinkNode( NodeBase* node );
			void swapNodes( NodeBase* a, NodeBase* b );
			template<typename CMP>
			NodeBase* mergeSort( NodeBase* a, NodeBase* b, CMP cmp );

			NodeBase  _anchor;
			size_t	  _size;
	};

	template<typename T>
	inline List<T>::List() : _anchor( &_anchor, &_anchor ), _size( 0 )
	{
	}

	template<typename T>
	inline List<T>::~List()
	{
		clear();
	}

	template<typename T>
	inline List<T>::List( const List<T>& list )
	{
		clear();
		Node* it = list._anchor._next;
		while( it != &list._anchor ) {
			append( it->_data );
			it = it->_next;
		}
	}

	template<typename T>
	inline size_t List<T>::size() const
	{
		return _size;
	}

	template<typename T>
	inline bool List<T>::isEmpty() const
	{
		return _size == 0;
	}

	template<typename T>
	inline T& List<T>::front()
	{
		return ( ( Node* ) _anchor._next )->_data;
	}

	template<typename T>
	inline T& List<T>::back()
	{
		return ( ( Node* ) _anchor._prev )->_data;
	}

	template<typename T>
	inline const T& List<T>::front() const
	{
		return ( ( Node* ) _anchor._next )->_data;
	}

	template<typename T>
	inline const T& List<T>::back() const
	{
		return ( ( Node* ) _anchor._prev )->_data;
	}


	template<typename T>
	inline List<T>& List<T>::operator=( const List<T>& list )
	{
		if( this != &list ) {
			clear();
			Node* it = list._anchor._next;
			while( it != &list._anchor ) {
				append( it->_data );
				it = it->_next;
			}
		}
		return *this;
	}

	template<typename T>
	inline void List<T>::append( const T& data )
	{
		if( _anchor._next == &_anchor ) {
			Node* node = new Node( data, &_anchor, &_anchor );
			_anchor._prev = node;
			_anchor._next = node;
		} else {
			Node* node = new Node( data, &_anchor, _anchor._prev );
			_anchor._prev->_next = node;
			_anchor._prev = node;
		}
		_size++;
	}

	template<typename T>
	inline void List<T>::prepend( const T& data )
	{
		if( _anchor._prev == &_anchor ) {
			Node* node = new Node( data, &_anchor, &_anchor );
			_anchor._next = node;
			_anchor._prev = node;
		} else {
			Node* node = new Node( data, _anchor._next, &_anchor );
			_anchor._next->_prev = node;
			_anchor._next = node;
		}
		_size++;
	}

	template<typename T>
	template<typename T2>
	inline typename List<T>::Iterator List<T>::find( const T2& data ) const
	{
		Node* it = ( Node* ) _anchor._next;
		while( it != &_anchor ) {
			if( it->_data == data )
				break;
			it = ( Node* ) it->_next;
		}
		return Iterator( it );
	}


	template<typename T>
	inline typename List<T>::Iterator List<T>::find( const T& data ) const
	{
		Node* it = ( Node* ) _anchor._next;
		while( it != &_anchor ) {
			if( it->_data == data )
				break;
			it = ( Node* ) it->_next;
		}
		return Iterator( it );
	}

	template<typename T>
	inline typename List<T>::Iterator List<T>::remove( Iterator it )
	{
		Iterator ret( it._it->_next );
		unlinkNode( it._it );
		_size--;
		delete ( Node* ) it._it;
		return ret;
	}

	template<typename T>
	inline typename List<T>::Iterator List<T>::remove( ReverseIterator it )
	{
		Iterator ret( it._it->_prev );
		unlinkNode( it._it );
		_size--;
		delete ( Node* ) it._it;
		return ret;
	}

	template<typename T>
	template<typename CMP>
	inline typename List<T>::NodeBase* List<T>::mergeSort( NodeBase* a, NodeBase* b, CMP cmp )
	{
		NodeBase* dst, *ret;

		/* set start first element of a or b */
		if( cmp( ( ( Node* ) a )->_data, ( ( Node* ) b )->_data ) ) {
			dst = a;
			dst->_prev = b->_prev;
			a = a->_next;
		} else {
			dst = b;
			b = b->_next;
		}
		ret = dst;

		/* merge the lists */
		while( a && b ) {
			if( cmp( ( ( Node* ) a )->_data, ( ( Node* ) b )->_data ) ) {
				dst->_next = a;
				dst->_next->_prev = dst;
				dst = dst->_next;
				a = a->_next;
			} else {
				dst->_next = b;
				dst->_next->_prev = dst;
				dst = dst->_next;
				b = b->_next;
			}
		}

		/* append the rest of either a or b */
		if( a ) {
			dst->_next = a;
			dst->_next->_prev = dst;
		} else if( b ) {
			dst->_next = b;
			dst->_next->_prev = dst;
		}

		/* find end of list */
		if( !ret->_prev ) {
			_anchor._prev = dst;
			while( _anchor._prev->_next )
				_anchor._prev = _anchor._prev->_next;
		}
		return ret;
	}

	template<typename T>
	inline void List<T>::sort()
	{
		sort( std::less<T>() );
	}

	template<typename T>
	template<typename CMP>
	inline void List<T>::sort( CMP cmp )
	{
		if( _size <= 1 )
			return;

		NodeBase* head = _anchor._next;
		NodeBase* tail = _anchor._prev;
		head->_prev = NULL;
		tail->_next = NULL;

		NodeBase* it = head;

		/*
			Transform double-linked list

			a <-> b <-> c <-> d <-> e

			to list of single-linked lists with a < b, d < c, -> is _prev and | is _next
																			  v
			a -> d -> e
			|	 |    |
		    v	 v    v
			b	 c   NULL
			|	 |
			v	 v
		   NULL NULL
		 */
		while( it && it->_next ) {
			if( cmp( ( ( Node* ) it->_next )->_data, ( ( Node* ) it )->_data ) ) {
				NodeBase *tmp;

				// previous element or head points to next
				if( it->_prev )
					it->_prev->_prev = it->_next;
				else
					head = it->_next;

				tmp = it->_next; // next element

				it->_prev = it->_next; // previous of it is next
				it->_next = NULL; // next of it is NUL

				tmp->_prev = tmp->_next;
				tmp->_next = it;
				it = tmp->_prev;

			} else {
				// the previous of the next element of next is it
				if( it->_next->_next )
					it->_next->_next->_prev = it;
				else
					tail = it;

				it->_prev = it->_next->_next; // prev points to next after next
				it->_next->_next = NULL; // next pointer of next element points to NULL
				it = it->_prev; // go to next after next
			}
		}

		if( it && !it->_next ) { // if the size is odd, then the prev pointer of the last element is set to NULL
			it->_prev = NULL;
			tail = it;
		}

		/* Now merge-sort the single-linked lists to double-linked list */
		NodeBase* next;
		while( head && head->_prev ) {
			next = head = mergeSort( head, head->_prev, cmp );
			while( next->_prev && next->_prev->_prev ) {
				next->_prev = mergeSort( next->_prev, next->_prev->_prev, cmp );
				next = next->_prev;
			}
		}

		_anchor._next = head;
		_anchor._next->_prev = &_anchor;
		// _anchor._prev = _anchor._next; // already set in mergeSort
		_anchor._prev->_next = &_anchor;
	}

	template<typename T>
	inline void List<T>::clear()
	{
		if( isEmpty() )
			return;

		Iterator it = begin();
		Iterator iend = end();
		while( it != iend ) {
			Iterator cur = it;
			++it;
			delete ( Node* ) cur._it;
		}
		_anchor._prev = &_anchor;
		_anchor._next = &_anchor;
		_size = 0;
	}

	template<typename T>
	inline void List<T>::unlinkNode( NodeBase* node )
	{
		node->_prev->_next = node->_next;
		node->_next->_prev = node->_prev;
	}


	template<typename T>
	inline void List<T>::swapNodes( NodeBase* a, NodeBase* b )
	{
		NodeBase* temp;

		temp = a->_next;
		a->_next = b->_next;
		b->_next = temp;

		a->_next->_prev = a;
		b->_next->_prev = b;

		temp = a->_prev;
		a->_prev = b->_prev;
		b->_prev = temp;

		a->_prev->_next = a;
		b->_prev->_next = b;
	}

}

#endif
