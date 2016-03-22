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


#ifndef CVT_ROWLOOKUPTABLE_H
#define CVT_ROWLOOKUPTABLE_H

#include <vector>
#include <cvt/vision/features/FeatureSet.h>
#include <cvt/vision/features/FeatureDescriptorExtractor.h>

namespace cvt 
{
	class RowLookupTable {
		public:
			struct Row
			{
				Row() : start( -1 ), len( 0 )
				{
				}

				int		start;
				size_t	len;
				bool valid() const { return start != -1 && len > 0; }
			};

			/**
			 *	@param fset		the sorted featureset to operate on
			 */
			RowLookupTable( const FeatureSet& fset );

			/**
			 * @brief RowLookupTable
			 * @param constructor for feature descriptorset
			 */
			RowLookupTable( const FeatureDescriptorExtractor& fdset );
			~RowLookupTable();

			const Row& row( size_t r ) const;
			bool  isValidRow( int r ) const;
			
		private:
			int						_maxY, _minY;
			std::vector<Row>		_rowIndex;
			void buildIndex( const FeatureSet& fset );
			void buildIndex( const FeatureDescriptorExtractor& fset );

	};
}

#endif
