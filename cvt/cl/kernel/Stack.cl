/*
   The MIT License (MIT)

   Copyright (c) 2016, BMW Car IT GmbH, Philipp Heise (philipp.heise@bmw.de)

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
#ifndef CVT_STACK_CL
#define CVT_STACK_CL

typedef uint Stackui;

/* WARNING: only push or pop operations are allowed in one kernel.
   Otherwise the state may become inconsistent ( e.g. pop returns newly pushed value ).
   No checking is performed if too many pushs/pops are performed and the actual size is exceeded.
 */
kernel void stackui_fill_values( global Stackui* stack, uint size, int offset )
{
    uint id = get_global_id( 0 );

    if( id >= size )
        return;

    if( id == 0 )
        stack[ id ] = 1; // the stack point points to the first element
    else
        stack[ id ] = offset + id;
}

inline uint stackui_pop( global Stackui* stack )
{
    uint pos = atomic_inc( stack );
    return stack[ pos ];
}

inline void stackui_push( global Stackui* stack, uint value )
{
    uint pos = atomic_dec( stack );
    stack[ pos - 1 ] = value;
}

#endif
