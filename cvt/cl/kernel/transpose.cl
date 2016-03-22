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

/*
	This kernel needs the defines TYPE and BLOCK_DIM
	the local memory 'block' must have the size ( BLOCK_DIM + 1) * BLOCK_DIM * sizeof( TYPE )
 */

__kernel void transpose( __global TYPE* odata, __global TYPE* idata, int width, int height, __local TYPE* block )
{
	// read the matrix tile into shared memory
	unsigned int xIndex = get_global_id( 0 );
	unsigned int yIndex = get_global_id( 1 );

	if( ( xIndex < width ) && ( yIndex < height ) ) {
		unsigned int index_in = yIndex * width + xIndex;
		block[ get_local_id( 1 ) * ( BLOCK_DIM + 1 ) + get_local_id( 0 ) ] = idata[ index_in ];
	}

	barrier( CLK_LOCAL_MEM_FENCE );

	// write the transposed matrix tile to global memory
	xIndex = get_group_id( 1 ) * BLOCK_DIM + get_local_id( 0 );
	yIndex = get_group_id( 0 ) * BLOCK_DIM + get_local_id( 1 );
	if( ( xIndex < height ) && ( yIndex < width ) ) {
		unsigned int index_out = yIndex * height + xIndex;
		odata[ index_out ] = block[ get_local_id( 0 ) * ( BLOCK_DIM + 1 ) + get_local_id( 1 ) ];
	}
}

