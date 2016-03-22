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


#include <cvt/vision/features/RowLookupTable.h>
#include <cvt/vision/features/FeatureSet.h>
#include <cvt/vision/features/FeatureDescriptorExtractor.h>

namespace cvt 
{

	RowLookupTable::RowLookupTable( const FeatureSet& fset ) :
		_maxY( -1 ), _minY ( -1 )
	{
		buildIndex( fset );
	}

	RowLookupTable::RowLookupTable( const FeatureDescriptorExtractor& fset ) :
		_maxY( -1 ), _minY ( -1 )
	{
		buildIndex( fset );
	}

	RowLookupTable::~RowLookupTable()
	{
	}
			

	const RowLookupTable::Row& RowLookupTable::row( size_t r ) const
	{
		return _rowIndex[ r ];
	}

	bool RowLookupTable::isValidRow( int r ) const
	{
		if( r > _maxY || r < _minY )
			return false;

		return row( r ).valid();
	}

	void RowLookupTable::buildIndex( const FeatureSet & fset )
	{
		size_t n = fset.size();
		if( !n )
			return;
		_maxY = ( int )fset[ n - 1 ].pt.y;
		
		_rowIndex.resize( _maxY + 1, Row() );

		int cy = ( int )fset[ 0 ].pt.y;
		_minY = cy;
		_rowIndex[ cy ].start = 0;

		FeatureSet::CmpYi cmp;
		while( cy < _maxY ){
			int prevStart = _rowIndex[ cy ].start;
			// calculate the upper bound for the previous y coord
			int start = std::upper_bound( &fset[ prevStart ],
										  &fset[ n - 1 ],
										  fset[ prevStart ], cmp ) - &fset[ 0 ];

			if( start == prevStart ) {
				break;
			}
			_rowIndex[ cy ].len = start - prevStart;

			cy = ( int ) fset[ start ].pt.y;
			_rowIndex[ cy ].start = start;
		}
		_rowIndex[ cy ].len = fset.size() - _rowIndex[ cy ].start;
	}

	void RowLookupTable::buildIndex( const FeatureDescriptorExtractor& fset )
	{
		size_t n = fset.size();
		if( !n )
			return;
		_maxY = ( int )fset[ n - 1 ].pt.y;
		_rowIndex.resize( _maxY + 1, Row() );

		int cy = ( int )fset[ 0 ].pt.y;
		_minY = cy;
		_rowIndex[ cy ].start = 0;

		while( cy < _maxY ){
			int prevStart = _rowIndex[ cy ].start;
			int start = prevStart;

			while( cy == ( int )fset[ start ].pt.y ){
				start++;
				if( start == n )
					break;
			}

			if( start == prevStart ) {
				break;
			}
			_rowIndex[ cy ].len = start - prevStart;

			cy = ( int ) fset[ start ].pt.y;
			_rowIndex[ cy ].start = start;
		}
		_rowIndex[ cy ].len = fset.size() - _rowIndex[ cy ].start;
	}
}
