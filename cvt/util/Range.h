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
 *  Range.h
 *  CVTools
 *
 *  Created by Sebastian Klose on 16.08.10.
 *  Copyright 2010. All rights reserved.
 *
 */

#ifndef RANGE_H
#define RANGE_H

#include <iostream>

namespace cvt {


template <typename T>
class Range
{
public:
	Range(T min, T max);

	T size();

	T min;
	T max;
};

template<typename T>
inline std::ostream& operator<<(std::ostream &out, const Range<T> &range)
{
	return out << "[" << range.min << ":" << range.max << "]" << std::endl;
}

typedef Range<float> Rangef;
typedef Range<double> Ranged;
typedef Range<int> Rangei;

template<typename T>
Range<T>::Range(T min, T max):
	min(min), max(max)
{
}

template<typename T>
T Range<T>::size()
{
	return ( max - min );
}

}

#endif

