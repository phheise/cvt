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

constant int _edgeTable[ 256 ]={
	0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
	0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
	0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
	0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
	0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
	0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
	0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
	0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
	0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
	0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
	0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
	0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
	0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
	0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
	0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
	0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
	0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
	0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
	0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
	0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
	0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
	0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
	0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
	0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
	0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
	0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
	0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
	0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
	0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
	0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
	0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
	0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0   };

constant char _triTable[ 256 ][ 16 ] =
	{{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1},
		{3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1},
		{3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1},
		{3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1},
		{9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1},
		{1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1},
		{9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
		{2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1},
		{8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1},
		{9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
		{4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1},
		{3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1},
		{1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1},
		{4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1},
		{4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1},
		{9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1},
		{1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
		{5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1},
		{2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1},
		{9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
		{0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
		{2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1},
		{10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1},
		{4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1},
		{5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1},
		{5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1},
		{9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1},
		{0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1},
		{1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1},
		{10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1},
		{8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1},
		{2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1},
		{7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1},
		{9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1},
		{2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1},
		{11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1},
		{9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1},
		{5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1},
		{11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1},
		{11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
		{1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1},
		{9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1},
		{5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1},
		{2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
		{0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
		{5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1},
		{6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1},
		{0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1},
		{3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1},
		{6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1},
		{5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1},
		{1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
		{10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1},
		{6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1},
		{1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1},
		{8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1},
		{7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1},
		{3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
		{5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1},
		{0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1},
		{9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1},
		{8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1},
		{5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1},
		{0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1},
		{6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1},
		{10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1},
		{10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1},
		{8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1},
		{1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1},
		{3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1},
		{0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1},
		{10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1},
		{0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1},
		{3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1},
		{6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1},
		{9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1},
		{8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1},
		{3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1},
		{6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1},
		{0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1},
		{10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1},
		{10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1},
		{1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1},
		{2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1},
		{7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1},
		{7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1},
		{2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1},
		{1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1},
		{11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1},
		{8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1},
		{0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1},
		{7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
		{10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
		{2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
		{6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1},
		{7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1},
		{2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1},
		{1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1},
		{10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1},
		{10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1},
		{0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1},
		{7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1},
		{6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1},
		{8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1},
		{9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1},
		{6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1},
		{1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1},
		{4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1},
		{10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1},
		{8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1},
		{0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1},
		{1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1},
		{8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1},
		{10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1},
		{4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1},
		{10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
		{5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
		{11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1},
		{9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
		{6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1},
		{7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1},
		{3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1},
		{7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1},
		{9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1},
		{3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1},
		{6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1},
		{9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1},
		{1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1},
		{4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1},
		{7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1},
		{6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1},
		{3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1},
		{0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1},
		{6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1},
		{1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1},
		{0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1},
		{11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1},
		{6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1},
		{5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1},
		{9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1},
		{1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1},
		{1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1},
		{10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1},
		{0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1},
		{5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1},
		{10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1},
		{11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1},
		{0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1},
		{9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1},
		{7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1},
		{2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1},
		{8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1},
		{9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1},
		{9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1},
		{1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1},
		{9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1},
		{9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1},
		{5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1},
		{0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1},
		{10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1},
		{2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1},
		{0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1},
		{0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1},
		{9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1},
		{5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1},
		{3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1},
		{5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1},
		{8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1},
		{0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1},
		{9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1},
		{0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1},
		{1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1},
		{3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1},
		{4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1},
		{9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1},
		{11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1},
		{11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1},
		{2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1},
		{9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1},
		{3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1},
		{1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1},
		{4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1},
		{4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1},
		{0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1},
		{3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1},
		{3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1},
		{0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1},
		{9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1},
		{1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}};

constant unsigned int _triSizeTable[ 256 ] =
{ 0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,2,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,3,1,2,2,3,
  2,3,3,4,2,3,3,4,3,4,4,3,2,3,3,2,3,4,4,3,3,4,4,3,4,5,5,2,1,2,2,3,2,3,3,4,
  2,3,3,4,3,4,4,3,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,4,2,3,3,4,3,4,2,3,3,4,4,5,
  4,5,3,2,3,4,4,3,4,5,3,2,4,5,5,4,5,2,4,1,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,3,
  2,3,3,4,3,4,4,5,3,2,4,3,4,3,5,2,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,4,3,4,4,3,
  4,5,5,4,4,3,5,2,5,4,2,1,2,3,3,4,3,4,4,5,3,4,4,5,2,3,3,2,3,4,4,5,4,5,5,2,
  4,3,5,4,3,2,4,1,3,4,4,5,4,5,3,4,4,5,5,2,3,4,2,1,2,3,3,2,3,4,2,1,3,2,4,1,
  2,1,1,0 };


#define VOLUME( a, b, c )	( *( cv + ( ( c ) * height + ( b ) ) * width + ( a ) ) )

inline void vertexInterp( float3* vtx, const float3 p1, const float3 p2, float val1, float val2, float isolevel )
{
	/*
	const float ISO_EPSILON = 1e-6f;

	if( fabs( isolevel - val1 ) <  ISO_EPSILON ) {
		*vtx = p1;
		return;
	}

	if( fabs( isolevel - val2 ) <  ISO_EPSILON ) {
		*vtx = p2;
		return;
	}

	if( fabs( val1 - val2 ) <  ISO_EPSILON ) {
		*vtx = p2;
		return;
	}
	*/

	float alpha = ( isolevel - val1 ) / ( val2 - val1 );
	*vtx = mix( p1, p2, alpha );
}

inline void vertexNormalInterp( float3* vtx, const float3 p1, const float3 p2, float3* normal, const float3 n1, const float3 n2, float val1, float val2, float isolevel )
{
	/*
	const float ISO_EPSILON = 1e-6f;

	if( fabs( isolevel - val1 ) <  ISO_EPSILON ) {
		*vtx = p1;
		return;
	}

	if( fabs( isolevel - val2 ) <  ISO_EPSILON ) {
		*vtx = p2;
		return;
	}

	if( fabs( val1 - val2 ) <  ISO_EPSILON ) {
		*vtx = p2;
		return;
	}
	*/

	float alpha = ( isolevel - val1 ) / ( val2 - val1 );
	*vtx	    = mix( p1, p2, alpha );
	*normal		= mix( n1, n2, alpha );
}

inline float MC_weighted_trilinearValue( global float2* cv, int width, int height, int depth, float3 pos )
{
	pos = fmin( fmax( ( float3 ) 0.0f, pos ), ( float3 ) ( width - 2, height - 2, depth - 2 ) );

	float3 base  = floor( pos );
	float3 alpha = pos - base;
	int3   ibase = ( int3 )( base.x, base.y, base.z );

	float2 tmp;
	float8 values;
	tmp		  = VOLUME( ibase.x    , ibase.y    , ibase.z );
	if( tmp.y < 1.0f ) return 1e10f;
	values.s0 = tmp.x;
	tmp		  = VOLUME( ibase.x + 1, ibase.y    , ibase.z );
	if( tmp.y < 1.0f ) return 1e10f;
	values.s1 = tmp.x;
	tmp		  = VOLUME( ibase.x    , ibase.y + 1, ibase.z );
	if( tmp.y < 1.0f ) return 1e10f;
	values.s2 = tmp.x;
	tmp		  = VOLUME( ibase.x + 1, ibase.y + 1, ibase.z );
	if( tmp.y < 1.0f ) return 1e10f;
	values.s3 = tmp.x;

	tmp		  = VOLUME( ibase.x    , ibase.y    , ibase.z + 1 );
	if( tmp.y < 1.0f ) return 1e10f;
	values.s4 = tmp.x;
	tmp		  = VOLUME( ibase.x + 1, ibase.y    , ibase.z + 1 );
	if( tmp.y < 1.0f ) return 1e10f;
	values.s5 = tmp.x;
	tmp		  = VOLUME( ibase.x    , ibase.y + 1, ibase.z + 1 );
	if( tmp.y < 1.0f ) return 1e10f;
	values.s6 = tmp.x;
	tmp		  = VOLUME( ibase.x + 1, ibase.y + 1, ibase.z + 1 );
	if( tmp.y < 1.0f ) return 1e10f;
	values.s7 = tmp.x;

	float4 interpz = mix( values.s0123, values.s4567, alpha.z );
	float2 interpy = mix( interpz.s01, interpz.s23, alpha.y );
	float  interpx = mix( interpy.s0, interpy.s1, alpha.x );

	return interpx;
}



kernel void MC_weighted_triangleSize( global unsigned int* size, global float2* cv, int width, int height, int depth, float isolevel )
{
	const float wepsilon = 20.0f;
	const int x = get_global_id( 0 );
	const int y = get_global_id( 1 );
	const int z = get_global_id( 2 );
	const int lx = get_local_id( 0 );
	const int ly = get_local_id( 1 );
	const int lz = get_local_id( 2 );

	if( x < width - 1 && y < height - 1 && z < depth - 1 ) {

		unsigned int cubeindex = 0;
		float2 val[ 8 ];

		/* get the values of the current grid voxel */
		val[ 0 ] = VOLUME( x, y, z );
		val[ 1 ] = VOLUME( x + 1, y, z );
		val[ 2 ] = VOLUME( x + 1, y + 1, z );
		val[ 3 ] = VOLUME( x, y + 1, z );
		val[ 4 ] = VOLUME( x, y, z + 1 );
		val[ 5 ] = VOLUME( x + 1, y, z + 1 );
		val[ 6 ] = VOLUME( x + 1, y + 1, z + 1 );
		val[ 7 ] = VOLUME( x, y + 1, z + 1 );

		if(  val[ 0 ].y > wepsilon
		  && val[ 1 ].y > wepsilon
		  && val[ 2 ].y > wepsilon
		  && val[ 3 ].y > wepsilon
		  && val[ 4 ].y > wepsilon
		  && val[ 5 ].y > wepsilon
		  && val[ 6 ].y > wepsilon
		  && val[ 7 ].y > wepsilon ) {
			if( val[ 0 ].x < isolevel ) cubeindex |= ( 1 << 0 );
			if( val[ 1 ].x < isolevel ) cubeindex |= ( 1 << 1 );
			if( val[ 2 ].x < isolevel ) cubeindex |= ( 1 << 2 );
			if( val[ 3 ].x < isolevel ) cubeindex |= ( 1 << 3 );
			if( val[ 4 ].x < isolevel ) cubeindex |= ( 1 << 4 );
			if( val[ 5 ].x < isolevel ) cubeindex |= ( 1 << 5 );
			if( val[ 6 ].x < isolevel ) cubeindex |= ( 1 << 6 );
			if( val[ 7 ].x < isolevel ) cubeindex |= ( 1 << 7 );

			atomic_add( size, ( unsigned int ) _triSizeTable[ cubeindex ] );
		}
	}
}

kernel void MC_weighted_triangleSizeWNormals( global unsigned int* size, global float2* cv, int width, int height, int depth, float isolevel )
{
	const float wepsilon = 20.0f;
	const int x = get_global_id( 0 );
	const int y = get_global_id( 1 );
	const int z = get_global_id( 2 );
	const int lx = get_local_id( 0 );
	const int ly = get_local_id( 1 );
	const int lz = get_local_id( 2 );

	if( x > 0 && x < width  - 2 &&
	    y > 0 && y < height - 2 &&
	    z > 0 && z < depth  - 2 ) {

		unsigned int cubeindex = 0;
		float2 val[ 8 ];

		/* get the values of the current grid voxel */
		val[ 0 ] = VOLUME( x, y, z );
		val[ 1 ] = VOLUME( x + 1, y, z );
		val[ 2 ] = VOLUME( x + 1, y + 1, z );
		val[ 3 ] = VOLUME( x, y + 1, z );
		val[ 4 ] = VOLUME( x, y, z + 1 );
		val[ 5 ] = VOLUME( x + 1, y, z + 1 );
		val[ 6 ] = VOLUME( x + 1, y + 1, z + 1 );
		val[ 7 ] = VOLUME( x, y + 1, z + 1 );

		if(  val[ 0 ].y > wepsilon
		  && val[ 1 ].y > wepsilon
		  && val[ 2 ].y > wepsilon
		  && val[ 3 ].y > wepsilon
		  && val[ 4 ].y > wepsilon
		  && val[ 5 ].y > wepsilon
		  && val[ 6 ].y > wepsilon
		  && val[ 7 ].y > wepsilon ) {
			if( val[ 0 ].x < isolevel ) cubeindex |= ( 1 << 0 );
			if( val[ 1 ].x < isolevel ) cubeindex |= ( 1 << 1 );
			if( val[ 2 ].x < isolevel ) cubeindex |= ( 1 << 2 );
			if( val[ 3 ].x < isolevel ) cubeindex |= ( 1 << 3 );
			if( val[ 4 ].x < isolevel ) cubeindex |= ( 1 << 4 );
			if( val[ 5 ].x < isolevel ) cubeindex |= ( 1 << 5 );
			if( val[ 6 ].x < isolevel ) cubeindex |= ( 1 << 6 );
			if( val[ 7 ].x < isolevel ) cubeindex |= ( 1 << 7 );

			atomic_add( size, ( unsigned int ) _triSizeTable[ cubeindex ] );
		}
	}
}

kernel void MC_weighted_extractTriangles( global float3* output, global unsigned int* pos, global float2* cv, int width, int height, int depth, float isolevel )
{
	const float wepsilon = 20.0f;
	const int x = get_global_id( 0 );
	const int y = get_global_id( 1 );
	const int z = get_global_id( 2 );
	const int lx = get_local_id( 0 );
	const int ly = get_local_id( 1 );
	const int lz = get_local_id( 2 );

	if( x < width - 1 && y < height - 1 && z < depth - 1 ) {

		unsigned int cubeindex = 0;
		float2 gridval[ 8 ];

		/* get the values of the current grid voxel */
		gridval[ 0 ] = VOLUME( x, y, z );
		gridval[ 1 ] = VOLUME( x + 1, y, z );
		gridval[ 2 ] = VOLUME( x + 1, y + 1, z );
		gridval[ 3 ] = VOLUME( x, y + 1, z );
		gridval[ 4 ] = VOLUME( x, y, z + 1 );
		gridval[ 5 ] = VOLUME( x + 1, y, z + 1 );
		gridval[ 6 ] = VOLUME( x + 1, y + 1, z + 1 );
		gridval[ 7 ] = VOLUME( x, y + 1, z + 1 );

		if(  gridval[ 0 ].y > wepsilon
		   && gridval[ 1 ].y > wepsilon
		   && gridval[ 2 ].y > wepsilon
		   && gridval[ 3 ].y > wepsilon
		   && gridval[ 4 ].y > wepsilon
		   && gridval[ 5 ].y > wepsilon
		   && gridval[ 6 ].y > wepsilon
		   && gridval[ 7 ].y > wepsilon ) {
			if( gridval[ 0 ].x < isolevel ) cubeindex |= ( 1 << 0 );
			if( gridval[ 1 ].x < isolevel ) cubeindex |= ( 1 << 1 );
			if( gridval[ 2 ].x < isolevel ) cubeindex |= ( 1 << 2 );
			if( gridval[ 3 ].x < isolevel ) cubeindex |= ( 1 << 3 );
			if( gridval[ 4 ].x < isolevel ) cubeindex |= ( 1 << 4 );
			if( gridval[ 5 ].x < isolevel ) cubeindex |= ( 1 << 5 );
			if( gridval[ 6 ].x < isolevel ) cubeindex |= ( 1 << 6 );
			if( gridval[ 7 ].x < isolevel ) cubeindex |= ( 1 << 7 );

			unsigned int numtri = ( unsigned int ) _triSizeTable[ cubeindex ];
			if( numtri ) {
				float3 gridvtx[ 8 ];
				float3 vertlist[ 12 ];
				unsigned int lpos = atomic_add( pos, numtri * 3 );

				gridvtx[ 0 ] = ( float3 ) ( x, y, z );
				gridvtx[ 1 ] = ( float3 ) ( x + 1, y, z );
				gridvtx[ 2 ] = ( float3 ) ( x + 1, y + 1, z );
				gridvtx[ 3 ] = ( float3 ) ( x, y + 1, z );
				gridvtx[ 4 ] = ( float3 ) ( x, y, z + 1 );
				gridvtx[ 5 ] = ( float3 ) ( x + 1, y, z + 1 );
				gridvtx[ 6 ] = ( float3 ) ( x + 1, y + 1, z + 1);
				gridvtx[ 7 ] = ( float3 ) ( x, y + 1, z + 1 );

				/* Find the vertices where the surface intersects the cube */
				if ( _edgeTable[ cubeindex ] & 1)
					vertexInterp( &vertlist[ 0 ], gridvtx[ 0 ], gridvtx[ 1 ], gridval[ 0 ].x, gridval[ 1 ].x, isolevel );
				if ( _edgeTable[ cubeindex ] & 2)
					vertexInterp( &vertlist[ 1 ], gridvtx[ 1 ], gridvtx[ 2 ], gridval[ 1 ].x, gridval[ 2 ].x, isolevel );
				if ( _edgeTable[ cubeindex ] & 4)
					vertexInterp( &vertlist[ 2 ], gridvtx[ 2 ], gridvtx[ 3 ], gridval[ 2 ].x, gridval[ 3 ].x, isolevel );
				if ( _edgeTable[ cubeindex ] & 8)
					vertexInterp( &vertlist[ 3 ], gridvtx[ 3 ], gridvtx[ 0 ], gridval[ 3 ].x, gridval[ 0 ].x, isolevel );
				if ( _edgeTable[ cubeindex ] & 16)
					vertexInterp( &vertlist[ 4 ], gridvtx[ 4 ], gridvtx[ 5 ], gridval[ 4 ].x, gridval[ 5 ].x, isolevel );
				if ( _edgeTable[ cubeindex ] & 32)
					vertexInterp( &vertlist[ 5 ], gridvtx[ 5 ], gridvtx[ 6 ], gridval[ 5 ].x, gridval[ 6 ].x, isolevel );
				if ( _edgeTable[ cubeindex ] & 64)
					vertexInterp( &vertlist[ 6 ], gridvtx[ 6 ], gridvtx[ 7 ], gridval[ 6 ].x, gridval[ 7 ].x, isolevel );
				if ( _edgeTable[ cubeindex ] & 128)
					vertexInterp( &vertlist[ 7 ], gridvtx[ 7 ], gridvtx[ 4 ], gridval[ 7 ].x, gridval[ 4 ].x, isolevel );
				if ( _edgeTable[ cubeindex ] & 256)
					vertexInterp( &vertlist[ 8 ], gridvtx[ 0 ], gridvtx[ 4 ], gridval[ 0 ].x, gridval[ 4 ].x, isolevel );
				if ( _edgeTable[ cubeindex ] & 512)
					vertexInterp( &vertlist[ 9 ], gridvtx[ 1 ], gridvtx[ 5 ], gridval[ 1 ].x, gridval[ 5 ].x, isolevel );
				if ( _edgeTable[ cubeindex ] & 1024)
					vertexInterp( &vertlist[ 10 ], gridvtx[ 2 ], gridvtx[ 6 ], gridval[ 2 ].x, gridval[ 6 ].x, isolevel );
				if ( _edgeTable[ cubeindex ] & 2048)
					vertexInterp( &vertlist[ 11 ], gridvtx[ 3 ], gridvtx[ 7 ], gridval[ 3 ].x, gridval[ 7 ].x, isolevel );


				/* Create the triangles */
				for( unsigned int i = 0; i < numtri; i++ ) {
					const unsigned int i3 = i * 3;
					output[ lpos + i3 + 0 ].xyz = vertlist[ _triTable[ cubeindex ][ i3     ] ];
					output[ lpos + i3 + 1 ].xyz = vertlist[ _triTable[ cubeindex ][ i3 + 1 ] ];
					output[ lpos + i3 + 2 ].xyz = vertlist[ _triTable[ cubeindex ][ i3 + 2 ] ];
				}
			}
		}
	}
}

kernel void MC_weighted_extractTrianglesNormals( global float3* outputvtx, global float3* outputnormal, global unsigned int* pos, global float2* cv, int width, int height, int depth, float isolevel )
{
	const float wepsilon = 20.0f;
	const int x = get_global_id( 0 );
	const int y = get_global_id( 1 );
	const int z = get_global_id( 2 );
	const int lx = get_local_id( 0 );
	const int ly = get_local_id( 1 );
	const int lz = get_local_id( 2 );

	if( x > 0 && x < width  - 2 &&
	    y > 0 && y < height - 2 &&
	    z > 0 && z < depth  - 2 ) {
		unsigned int cubeindex = 0;
		float2 gridval[ 8 ];

		/* get the values of the current grid voxel */
		gridval[ 0 ] = VOLUME( x, y, z );
		gridval[ 1 ] = VOLUME( x + 1, y, z );
		gridval[ 2 ] = VOLUME( x + 1, y + 1, z );
		gridval[ 3 ] = VOLUME( x, y + 1, z );
		gridval[ 4 ] = VOLUME( x, y, z + 1 );
		gridval[ 5 ] = VOLUME( x + 1, y, z + 1 );
		gridval[ 6 ] = VOLUME( x + 1, y + 1, z + 1 );
		gridval[ 7 ] = VOLUME( x, y + 1, z + 1 );

		if(  gridval[ 0 ].y > wepsilon
		   && gridval[ 1 ].y > wepsilon
		   && gridval[ 2 ].y > wepsilon
		   && gridval[ 3 ].y > wepsilon
		   && gridval[ 4 ].y > wepsilon
		   && gridval[ 5 ].y > wepsilon
		   && gridval[ 6 ].y > wepsilon
		   && gridval[ 7 ].y > wepsilon ) {

			if( gridval[ 0 ].x < isolevel ) cubeindex |= ( 1 << 0 );
			if( gridval[ 1 ].x < isolevel ) cubeindex |= ( 1 << 1 );
			if( gridval[ 2 ].x < isolevel ) cubeindex |= ( 1 << 2 );
			if( gridval[ 3 ].x < isolevel ) cubeindex |= ( 1 << 3 );
			if( gridval[ 4 ].x < isolevel ) cubeindex |= ( 1 << 4 );
			if( gridval[ 5 ].x < isolevel ) cubeindex |= ( 1 << 5 );
			if( gridval[ 6 ].x < isolevel ) cubeindex |= ( 1 << 6 );
			if( gridval[ 7 ].x < isolevel ) cubeindex |= ( 1 << 7 );

			unsigned int numtri = ( unsigned int ) _triSizeTable[ cubeindex ];
			if( numtri ) {
				float3 gridvtx[ 8 ];
				float3 gridnormal[ 8 ];
				float3 vertlist[ 12 ];
				float3 normallist[ 12 ];
				unsigned int lpos = atomic_add( pos, numtri * 3 );

				gridvtx[ 0 ] = ( float3 ) ( x, y, z );
				gridvtx[ 1 ] = ( float3 ) ( x + 1, y, z );
				gridvtx[ 2 ] = ( float3 ) ( x + 1, y + 1, z );
				gridvtx[ 3 ] = ( float3 ) ( x, y + 1, z );
				gridvtx[ 4 ] = ( float3 ) ( x, y, z + 1 );
				gridvtx[ 5 ] = ( float3 ) ( x + 1, y, z + 1 );
				gridvtx[ 6 ] = ( float3 ) ( x + 1, y + 1, z + 1);
				gridvtx[ 7 ] = ( float3 ) ( x, y + 1, z + 1 );

#define VOLNORMAL( x, y, z )   -( float3 )( VOLUME( ( x ) + 1, ( y ), ( z ) ).s0 - VOLUME( ( x ) - 1, ( y ), ( z ) ).s0, \
										  VOLUME( ( x ), ( y ) + 1, ( z ) ).s0 - VOLUME( ( x ) , ( y ) - 1, ( z ) ).s0, \
										  VOLUME( ( x ), ( y ), ( z ) + 1 ).s0 - VOLUME( ( x ) , ( y ), ( z ) - 1 ).s0 )

				gridnormal[ 0 ] = VOLNORMAL( x, y, z );
				gridnormal[ 1 ] = VOLNORMAL( x + 1, y, z );
				gridnormal[ 2 ] = VOLNORMAL( x + 1, y + 1, z );
				gridnormal[ 3 ] = VOLNORMAL( x, y + 1, z );
				gridnormal[ 4 ] = VOLNORMAL( x, y, z + 1 );
				gridnormal[ 5 ] = VOLNORMAL( x + 1, y, z + 1 );
				gridnormal[ 6 ] = VOLNORMAL( x + 1, y + 1, z + 1);
				gridnormal[ 7 ] = VOLNORMAL( x, y + 1, z + 1 );

#undef VOLNORMAL

				/* Find the vertices where the surface intersects the cube */
				if ( _edgeTable[ cubeindex ] & 1)
					vertexNormalInterp( &vertlist[ 0 ], gridvtx[ 0 ], gridvtx[ 1 ],
									    &normallist[ 0 ], gridnormal[ 0 ], gridnormal[ 1 ],
										gridval[ 0 ].x, gridval[ 1 ].x, isolevel );
				if ( _edgeTable[ cubeindex ] & 2)
					vertexNormalInterp( &vertlist[ 1 ], gridvtx[ 1 ], gridvtx[ 2 ],
										&normallist[ 1 ], gridnormal[ 1 ], gridnormal[ 2 ],
										gridval[ 1 ].x, gridval[ 2 ].x, isolevel );
				if ( _edgeTable[ cubeindex ] & 4)
					vertexNormalInterp( &vertlist[ 2 ], gridvtx[ 2 ], gridvtx[ 3 ],
										&normallist[ 2 ], gridnormal[ 2 ], gridnormal[ 3 ],
										gridval[ 2 ].x, gridval[ 3 ].x, isolevel );
				if ( _edgeTable[ cubeindex ] & 8)
					vertexNormalInterp( &vertlist[ 3 ], gridvtx[ 3 ], gridvtx[ 0 ],
										&normallist[ 3 ], gridnormal[ 3 ], gridnormal[ 0 ],
										gridval[ 3 ].x, gridval[ 0 ].x, isolevel );
				if ( _edgeTable[ cubeindex ] & 16)
					vertexNormalInterp( &vertlist[ 4 ], gridvtx[ 4 ], gridvtx[ 5 ],
										&normallist[ 4 ], gridnormal[ 4 ], gridnormal[ 5 ],
										gridval[ 4 ].x, gridval[ 5 ].x, isolevel );
				if ( _edgeTable[ cubeindex ] & 32)
					vertexNormalInterp( &vertlist[ 5 ], gridvtx[ 5 ], gridvtx[ 6 ],
										&normallist[ 5 ], gridnormal[ 5 ], gridnormal[ 6 ],
										gridval[ 5 ].x, gridval[ 6 ].x, isolevel );
				if ( _edgeTable[ cubeindex ] & 64)
					vertexNormalInterp( &vertlist[ 6 ], gridvtx[ 6 ], gridvtx[ 7 ],
										&normallist[ 6 ], gridnormal[ 6 ], gridnormal[ 7 ],
										gridval[ 6 ].x, gridval[ 7 ].x, isolevel );
				if ( _edgeTable[ cubeindex ] & 128)
					vertexNormalInterp( &vertlist[ 7 ], gridvtx[ 7 ], gridvtx[ 4 ],
										&normallist[ 7 ], gridnormal[ 7 ], gridnormal[ 4 ],
										gridval[ 7 ].x, gridval[ 4 ].x, isolevel );
				if ( _edgeTable[ cubeindex ] & 256)
					vertexNormalInterp( &vertlist[ 8 ], gridvtx[ 0 ], gridvtx[ 4 ],
										&normallist[ 8 ], gridnormal[ 0 ], gridnormal[ 4 ],
										gridval[ 0 ].x, gridval[ 4 ].x, isolevel );
				if ( _edgeTable[ cubeindex ] & 512)
					vertexNormalInterp( &vertlist[ 9 ], gridvtx[ 1 ], gridvtx[ 5 ],
										&normallist[ 9 ], gridnormal[ 1 ], gridnormal[ 5 ],
										gridval[ 1 ].x, gridval[ 5 ].x, isolevel );
				if ( _edgeTable[ cubeindex ] & 1024)
					vertexNormalInterp( &vertlist[ 10 ], gridvtx[ 2 ], gridvtx[ 6 ],
										&normallist[ 10 ], gridnormal[ 2 ], gridnormal[ 6 ],
										gridval[ 2 ].x, gridval[ 6 ].x, isolevel );
				if ( _edgeTable[ cubeindex ] & 2048)
					vertexNormalInterp( &vertlist[ 11 ], gridvtx[ 3 ], gridvtx[ 7 ],
										&normallist[ 11 ], gridnormal[ 3 ], gridnormal[ 7 ],
										gridval[ 3 ].x, gridval[ 7 ].x, isolevel );


				/* Create the triangles */
				for( unsigned int i = 0; i < numtri; i++ ) {
					const unsigned int i3 = i * 3;
					outputvtx[ lpos + i3 + 0 ].xyz = vertlist[ _triTable[ cubeindex ][ i3     ] ];
					outputvtx[ lpos + i3 + 1 ].xyz = vertlist[ _triTable[ cubeindex ][ i3 + 1 ] ];
					outputvtx[ lpos + i3 + 2 ].xyz = vertlist[ _triTable[ cubeindex ][ i3 + 2 ] ];
					outputnormal[ lpos + i3 + 0 ].xyz = normallist[ _triTable[ cubeindex ][ i3     ] ];
					outputnormal[ lpos + i3 + 1 ].xyz = normallist[ _triTable[ cubeindex ][ i3 + 1 ] ];
					outputnormal[ lpos + i3 + 2 ].xyz = normallist[ _triTable[ cubeindex ][ i3 + 2 ] ];

				}
			}
		}
	}
}
