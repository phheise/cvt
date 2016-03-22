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

#ifndef CVTDELEGATE_H
#define CVTDELEGATE_H

namespace cvt {
    template<typename> class Delegate;
    template<typename> class DelegateImpl;
    template<typename> class DelegateFunction;
    template<class, typename> class DelegateMember;
    template<class, typename> class DelegateMemberConst;

/* ", typename T1, typename T2, ... , typename T10 " */
#define TYPENAMELIST_N0
#define TYPENAMELIST_N1 TYPENAMELIST_N0, typename T1
#define TYPENAMELIST_N2 TYPENAMELIST_N1, typename T2
#define TYPENAMELIST_N3 TYPENAMELIST_N2, typename T3
#define TYPENAMELIST_N4 TYPENAMELIST_N3, typename T4
#define TYPENAMELIST_N5 TYPENAMELIST_N4, typename T5
#define TYPENAMELIST_N6 TYPENAMELIST_N5, typename T6
#define TYPENAMELIST_N7 TYPENAMELIST_N6, typename T7
#define TYPENAMELIST_N8 TYPENAMELIST_N7, typename T8
#define TYPENAMELIST_N9 TYPENAMELIST_N8, typename T9
#define TYPENAMELIST_N10 TYPENAMELIST_N9, typename T10

/* " T1, T2, ... , T10 " */
#define TYPELIST_N0
#define TYPELIST_N1	 T1
#define TYPELIST_N2  TYPELIST_N1, T2
#define TYPELIST_N3  TYPELIST_N2, T3
#define TYPELIST_N4  TYPELIST_N3, T4
#define TYPELIST_N5  TYPELIST_N4, T5
#define TYPELIST_N6  TYPELIST_N5, T6
#define TYPELIST_N7  TYPELIST_N6, T7
#define TYPELIST_N8  TYPELIST_N7, T8
#define TYPELIST_N9  TYPELIST_N8, T9
#define TYPELIST_N10 TYPELIST_N9, T10

/* " T1 arg1, T2 arg2, ..., T10 arg10 " */
#define TYPEARGLIST_N0
#define TYPEARGLIST_N1	T1  arg1
#define TYPEARGLIST_N2  TYPEARGLIST_N1, T2  arg2
#define TYPEARGLIST_N3  TYPEARGLIST_N2, T3  arg3
#define TYPEARGLIST_N4  TYPEARGLIST_N3, T4  arg4
#define TYPEARGLIST_N5  TYPEARGLIST_N4, T5  arg5
#define TYPEARGLIST_N6  TYPEARGLIST_N5, T6  arg6
#define TYPEARGLIST_N7  TYPEARGLIST_N6, T7  arg7
#define TYPEARGLIST_N8  TYPEARGLIST_N7, T8  arg8
#define TYPEARGLIST_N9  TYPEARGLIST_N8, T9  arg9
#define TYPEARGLIST_N10 TYPEARGLIST_N9, T10 arg10

/* " arg1, arg2, ..., arg10 " */
#define ARGLIST_N0
#define ARGLIST_N1	arg1
#define ARGLIST_N2  ARGLIST_N1, arg2
#define ARGLIST_N3  ARGLIST_N2, arg3
#define ARGLIST_N4  ARGLIST_N3, arg4
#define ARGLIST_N5  ARGLIST_N4, arg5
#define ARGLIST_N6  ARGLIST_N5, arg6
#define ARGLIST_N7  ARGLIST_N6, arg7
#define ARGLIST_N8  ARGLIST_N7, arg8
#define ARGLIST_N9  ARGLIST_N8, arg9
#define ARGLIST_N10 ARGLIST_N9, arg10

/* 0 arguments */

#define TYPENAMELIST TYPENAMELIST_N0
#define TYPELIST TYPELIST_N0
#define TYPEARGLIST TYPEARGLIST_N0
#define ARGLIST ARGLIST_N0
#include "Delegate.inl"
#undef TYPENAMELIST
#undef TYPELIST
#undef TYPEARGLIST
#undef ARGLIST

/* 1 argument ... */
#define TYPENAMELIST TYPENAMELIST_N1
#define TYPELIST TYPELIST_N1
#define TYPEARGLIST TYPEARGLIST_N1
#define ARGLIST ARGLIST_N1
#include "Delegate.inl"
#undef TYPENAMELIST
#undef TYPELIST
#undef TYPEARGLIST
#undef ARGLIST

#define TYPENAMELIST TYPENAMELIST_N2
#define TYPELIST TYPELIST_N2
#define TYPEARGLIST TYPEARGLIST_N2
#define ARGLIST ARGLIST_N2
#include "Delegate.inl"
#undef TYPENAMELIST
#undef TYPELIST
#undef TYPEARGLIST
#undef ARGLIST

#define TYPENAMELIST TYPENAMELIST_N3
#define TYPELIST TYPELIST_N3
#define TYPEARGLIST TYPEARGLIST_N3
#define ARGLIST ARGLIST_N3
#include "Delegate.inl"
#undef TYPENAMELIST
#undef TYPELIST
#undef TYPEARGLIST
#undef ARGLIST

#define TYPENAMELIST TYPENAMELIST_N4
#define TYPELIST TYPELIST_N4
#define TYPEARGLIST TYPEARGLIST_N4
#define ARGLIST ARGLIST_N4
#include "Delegate.inl"
#undef TYPENAMELIST
#undef TYPELIST
#undef TYPEARGLIST
#undef ARGLIST

#define TYPENAMELIST TYPENAMELIST_N5
#define TYPELIST TYPELIST_N5
#define TYPEARGLIST TYPEARGLIST_N5
#define ARGLIST ARGLIST_N5
#include "Delegate.inl"
#undef TYPENAMELIST
#undef TYPELIST
#undef TYPEARGLIST
#undef ARGLIST

#define TYPENAMELIST TYPENAMELIST_N6
#define TYPELIST TYPELIST_N6
#define TYPEARGLIST TYPEARGLIST_N6
#define ARGLIST ARGLIST_N6
#include "Delegate.inl"
#undef TYPENAMELIST
#undef TYPELIST
#undef TYPEARGLIST
#undef ARGLIST

#define TYPENAMELIST TYPENAMELIST_N7
#define TYPELIST TYPELIST_N7
#define TYPEARGLIST TYPEARGLIST_N7
#define ARGLIST ARGLIST_N7
#include "Delegate.inl"
#undef TYPENAMELIST
#undef TYPELIST
#undef TYPEARGLIST
#undef ARGLIST

#define TYPENAMELIST TYPENAMELIST_N8
#define TYPELIST TYPELIST_N8
#define TYPEARGLIST TYPEARGLIST_N8
#define ARGLIST ARGLIST_N8
#include "Delegate.inl"
#undef TYPENAMELIST
#undef TYPELIST
#undef TYPEARGLIST
#undef ARGLIST

#define TYPENAMELIST TYPENAMELIST_N9
#define TYPELIST TYPELIST_N9
#define TYPEARGLIST TYPEARGLIST_N9
#define ARGLIST ARGLIST_N9
#include "Delegate.inl"
#undef TYPENAMELIST
#undef TYPELIST
#undef TYPEARGLIST
#undef ARGLIST

/* 10 arguments */

#define TYPENAMELIST TYPENAMELIST_N10
#define TYPELIST TYPELIST_N10
#define TYPEARGLIST TYPEARGLIST_N10
#define ARGLIST ARGLIST_N10
#include "Delegate.inl"
#undef TYPENAMELIST
#undef TYPELIST
#undef TYPEARGLIST
#undef ARGLIST

#undef TYPENAMELIST_N0
#undef TYPENAMELIST_N1
#undef TYPENAMELIST_N2
#undef TYPENAMELIST_N3
#undef TYPENAMELIST_N4
#undef TYPENAMELIST_N5
#undef TYPENAMELIST_N6
#undef TYPENAMELIST_N7
#undef TYPENAMELIST_N8
#undef TYPENAMELIST_N9
#undef TYPENAMELIST_N10

#undef TYPELIST_N0
#undef TYPELIST_N1
#undef TYPELIST_N2
#undef TYPELIST_N3
#undef TYPELIST_N4
#undef TYPELIST_N5
#undef TYPELIST_N6
#undef TYPELIST_N7
#undef TYPELIST_N8
#undef TYPELIST_N9
#undef TYPELIST_N10

#undef TYPEARGLIST_N0
#undef TYPEARGLIST_N1
#undef TYPEARGLIST_N2
#undef TYPEARGLIST_N3
#undef TYPEARGLIST_N4
#undef TYPEARGLIST_N5
#undef TYPEARGLIST_N6
#undef TYPEARGLIST_N7
#undef TYPEARGLIST_N8
#undef TYPEARGLIST_N9
#undef TYPEARGLIST_N10

#undef ARGLIST_N0
#undef ARGLIST_N1
#undef ARGLIST_N2
#undef ARGLIST_N3
#undef ARGLIST_N4
#undef ARGLIST_N5
#undef ARGLIST_N6
#undef ARGLIST_N7
#undef ARGLIST_N8
#undef ARGLIST_N9
#undef ARGLIST_N10
}

#endif
