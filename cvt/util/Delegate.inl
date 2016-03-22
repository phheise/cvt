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

#ifdef CVTDELEGATE_H

/*
   Mediocre Delegate implementation:
   - needs heap memory
   - additional indirection
   + but readable code
 */

/* General delegate interface */
template<typename T0 TYPENAMELIST>
class DelegateImpl<T0 ( TYPELIST )>
{
	public:
		virtual ~DelegateImpl() {}
		virtual T0 invoke( TYPELIST ) const = 0;
		virtual DelegateImpl<T0 ( TYPELIST )>* clone() const = 0;
		virtual bool operator==( const DelegateImpl& dimp ) const = 0;
		virtual bool operator!=( const DelegateImpl& dimp ) const { return !( *this == dimp );}
};

/* Delegate implementation for pointer to member */
template<class T, typename T0 TYPENAMELIST >
class DelegateMember<T, T0 ( TYPELIST )> : public DelegateImpl<T0 ( TYPELIST )>
{
	typedef T0 (T::*MemberPtr)( TYPELIST );

	public:
		DelegateMember( T* obj, MemberPtr mptr ) : _obj( obj ), _mptr( mptr ) {}

		virtual T0 invoke( TYPEARGLIST ) const
		{
			return (_obj->*_mptr)( ARGLIST );
		}

		virtual DelegateImpl<T0 ( TYPELIST )>* clone() const
		{
			return new DelegateMember<T, T0 ( TYPELIST )>( _obj, _mptr );
		}

		virtual bool operator==( const DelegateImpl<T0 ( TYPELIST )>& dimp ) const
		{
			const DelegateMember<T, T0 ( TYPELIST )>* other = dynamic_cast<const DelegateMember<T, T0 ( TYPELIST )>*>( &dimp );
			if( !other )
				return false;
			return ( _obj == other->_obj && _mptr == other->_mptr );
		}

	private:
		T* _obj;
		MemberPtr _mptr;
};

/* Delegate implementation for pointer to const member */
template<class T, typename T0 TYPENAMELIST >
class DelegateMemberConst<T, T0 ( TYPELIST )> : public DelegateImpl<T0 ( TYPELIST )>
{
	typedef T0 (T::*MemberPtr)( TYPELIST ) const;

	public:
		DelegateMemberConst( T* obj, MemberPtr mptr ) : _obj( obj ), _mptr( mptr ) {}

		virtual T0 invoke( TYPEARGLIST ) const
		{
			return (_obj->*_mptr)( ARGLIST );
		}

		virtual DelegateImpl<T0 ( TYPELIST )>* clone() const
		{
			return new DelegateMemberConst<T, T0 ( TYPELIST )>( _obj, _mptr );
		}

		virtual bool operator==( const DelegateImpl<T0 ( TYPELIST )>& dimp ) const
		{
			const DelegateMemberConst<T, T0 ( TYPELIST )>* other = dynamic_cast<const DelegateMemberConst<T, T0 ( TYPELIST )>*>( &dimp );
			if( !other )
				return false;
			return ( _obj == other->_obj && _mptr == other->_mptr );
		}

	private:
		T* _obj;
		MemberPtr _mptr;
};

/* Delegate implementation for function pointers */
template<typename T0 TYPENAMELIST >
class DelegateFunction<T0 ( TYPELIST )> : public DelegateImpl<T0 ( TYPELIST )>
{
	typedef T0 (*FuncPtr)( TYPELIST );

	public:
		DelegateFunction( FuncPtr fptr ) : _fptr( fptr ) {}

		virtual T0 invoke( TYPEARGLIST ) const
		{
			return _fptr( ARGLIST );
		}

		virtual DelegateImpl<T0 ( TYPELIST )>* clone() const
		{
			return new DelegateFunction<T0 ( TYPELIST )>( _fptr );
		}

		virtual bool operator==( const DelegateImpl<T0 ( TYPELIST )>& dimp ) const
		{
			const DelegateFunction<T0 ( TYPELIST )>* other = dynamic_cast<const DelegateFunction<T0 ( TYPELIST )>*>( &dimp );
			if( !other )
				return false;
			return _fptr == other->_fptr;
		}
	private:
		FuncPtr _fptr;
};

/* The real delegate */
template<typename T0 TYPENAMELIST >
class Delegate<T0 ( TYPELIST )>
{
	public:
		template<class T>
			Delegate( T* t, T0 ( T::*ptr )( TYPELIST ) )
			{
				_impl = new DelegateMember<T, T0 ( TYPELIST )>( t, ptr );
			}

		template<class T>
			Delegate( T* t, T0 ( T::*ptr )( TYPELIST ) const )
			{
				_impl = new DelegateMemberConst<T, T0 ( TYPELIST )>( t, ptr );
			}

		Delegate( const Delegate<T0 ( TYPELIST )>& d )
		{
			_impl = d._impl->clone();
		}

		Delegate( T0 ( *func )( TYPELIST ) )
		{
			_impl = new DelegateFunction<T0 ( TYPELIST )>( func );
		}

		~Delegate()
		{
			delete _impl;
		}

		T0 operator()( TYPEARGLIST ) const
		{
			return _impl->invoke( ARGLIST );
		}

		bool operator==( const Delegate& other ) const
		{
			return *_impl ==  *( other._impl );
		}

		bool operator!=( const Delegate& other ) const
		{
			return *_impl !=  *( other._impl );
		}

	private:
		DelegateImpl<T0 ( TYPELIST )>* _impl;
};


#endif
