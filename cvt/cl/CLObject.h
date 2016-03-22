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

#ifndef CVT_CLOBJECT_H
#define CVT_CLOBJECT_H

#include <cvt/cl/OpenCL.h>

namespace cvt {
	template<typename T>
	class CLObject
	{
		public:
			CLObject();
			CLObject( const T& other );
			CLObject( const CLObject<T>& other );
			~CLObject();

			CLObject<T>& operator=( const CLObject<T>& other );
			operator T () const { return _object; }

		protected:
			cl_int retain() const;
			cl_int release() const;
			T _object;
	};

	template<typename T>
	inline CLObject<T>::CLObject() : _object( NULL )
	{
	}

	template<typename T>
	inline CLObject<T>::CLObject( const T& other ) : _object( NULL )
	{
		_object = other;
		retain();
	}

	template<typename T>
	inline CLObject<T>::CLObject( const CLObject<T>& other ) : _object( NULL )
	{
		_object = other._object;
		retain();
	}

	template<typename T>
    CLObject<T>& CLObject<T>::operator=( const CLObject<T>& other )
	{
		if( &other == this )
			return *this;
		release();
		_object = other._object;
		retain();
		return *this;
	}


	template<typename T>
	inline CLObject<T>::~CLObject()
	{
		release();
	}

#define X( t, ref, unref ) \
	template<> \
	inline cl_int CLObject<t>::retain() const\
	{ \
		if( _object == NULL ) \
			return CL_SUCCESS; \
		return ref( _object ); \
	} \
	\
	template<> \
	inline cl_int CLObject<t>::release() const\
	{ \
		if( _object == NULL ) \
			return CL_SUCCESS; \
		return unref( _object ); \
	}
#include "CLObject.inl"
#undef X

	typedef CLObject<cl_mem> CLMemory;
};

#endif
