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


#include "GridFilter.h"
#include <cvt/util/CVTAssert.h>

namespace cvt {

    void GridFilter::filterGrid()
    {
        FeatureSet::const_iterator it;
        for ( it = _featureSet.begin(); it != _featureSet.end(); ++it ) {
            // -> associate each feature to a cell
            const Vector2f& featurePos( it->pt );
            gridCell( static_cast<size_t>( featurePos.x ),
                      static_cast<size_t>( featurePos.y ) ).push_back( &( *it ) );
        }

        getBestFeatures();
    }


    inline GridFilter::GridCell& GridFilter::gridCell( size_t x, size_t y )
    {
        const size_t row = y / _cellHeight; // integer division is what we want here
        const size_t col = x / _cellWidth;

        CVT_ASSERT( _grid.size() >= row + 1, "Grid didn't have enough rows preallocated!" );

        GridCellRow& currentRow = _grid[ row ];

        CVT_ASSERT( currentRow.size() >= col + 1, "Grid didn't have enough columns preallocated!" );

        return currentRow[ col ];
    }


    void GridFilter::getBestFeatures()
    {

        Grid::iterator rowIt;
        GridCellRow::iterator cellIt;
        CmpScore cmp;
        FeatureSet concatenatedFeatures;
        AddOperator addOp( concatenatedFeatures );

        // iterate over all rows
        for ( rowIt = _grid.begin(); rowIt != _grid.end(); ++rowIt ) {
            // iterate over cells:
            GridCellRow& cellRow = *rowIt;
            for ( cellIt = cellRow.begin(); cellIt != cellRow.end(); ++cellIt ) {
                GridCell& cell = *cellIt;

                if ( cell.size() > _featuresPerCell ) {
                    // sort features in every cell to pick the best n
                    std::sort( cell.begin(), cell.end(), cmp ); // ~ O(N*log2(N))

                    // take the x best features per cell
                    cell.resize( _featuresPerCell );
                }

                // concatenate features into temporary featureset, we are still
                // working with pointers into the original feature set until this point!
                std::for_each( cell.begin(), cell.end(), addOp ); // O(N)
            }
        }
        _featureSet = concatenatedFeatures; // O(N), copies all elements
    }
}
