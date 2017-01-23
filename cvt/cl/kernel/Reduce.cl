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

/* The following defines have to be there to use the reduce kernel: */

//#ifndef LOCAL_SIZE
//#define LOCAL_SIZE 1024
//#endif
//
//#ifndef TYPE
//#define TYPE int
//#endif
//
//#ifndef SET_ZERO
//#define SET_ZERO(x) x = 0
//#endif

//#ifndef REDUCE
//#define REDUCE( a, b ) a += b
//#endif

kernel void reduce( global TYPE* output, global TYPE* input, unsigned int size )
{
    const unsigned int lid = get_local_id( 0 );
    const unsigned int grpid = get_group_id( 0 );
    const unsigned int stride = get_num_groups( 0 ) * 2 * LOCAL_SIZE;
    unsigned int idx = grpid * 2 * LOCAL_SIZE + lid;
    local TYPE localbuf[ LOCAL_SIZE ];

    SET_ZERO( localbuf[ lid ] );

    while( idx < size ) {
        REDUCE( localbuf[ lid ], input[ idx ] );
        if( idx + LOCAL_SIZE < size )
            REDUCE( localbuf[ lid ], input[ idx + LOCAL_SIZE ] );
        idx += stride;
    }
    barrier( CLK_LOCAL_MEM_FENCE );

#if ( LOCAL_SIZE >= 1024 )
    if( lid < 512 ) REDUCE( localbuf[ lid ], localbuf[ lid + 512 ] );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif
#if ( LOCAL_SIZE >= 512 )
    if( lid < 256 ) REDUCE( localbuf[ lid ], localbuf[ lid + 256 ] );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif
#if ( LOCAL_SIZE >= 256 )
    if( lid < 128 ) REDUCE( localbuf[ lid ], localbuf[ lid + 128 ] );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif
#if ( LOCAL_SIZE >= 128 )
    if( lid < 64 ) REDUCE( localbuf[ lid ], localbuf[ lid + 64 ] );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif

    // we have the barrier because not all platforms can guarante synchronous operations for less than 32 threads

#if ( LOCAL_SIZE >= 64 )
    if( lid < 32 ) REDUCE( localbuf[ lid ], localbuf[ lid + 32 ] );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif
#if ( LOCAL_SIZE >= 32 )
    if( lid < 16 ) REDUCE( localbuf[ lid ], localbuf[ lid + 16 ] );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif
#if ( LOCAL_SIZE >= 16 )
    if( lid < 8 ) REDUCE( localbuf[ lid ], localbuf[ lid + 8 ] );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif
#if ( LOCAL_SIZE >= 8 )
    if( lid < 4 ) REDUCE( localbuf[ lid ], localbuf[ lid + 4 ] );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif
#if ( LOCAL_SIZE >= 4 )
    if( lid < 2 ) REDUCE( localbuf[ lid ], localbuf[ lid + 2 ] );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif
#if ( LOCAL_SIZE >= 2 )
    if( lid < 1 ) REDUCE( localbuf[ lid ], localbuf[ lid + 1 ] );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif

    if( lid == 0 ) output[ grpid ] = localbuf[ 0 ];
}
