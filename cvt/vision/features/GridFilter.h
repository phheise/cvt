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

#ifndef CVT_GRIDFILTER_H
#define CVT_GRIDFILTER_H

#include <algorithm>
#include <vector>
#include <math.h>
#include <cvt/vision/features/Feature.h>
#include <cvt/vision/features/FeatureSet.h>

namespace cvt {
    /**
     * @brief The GridFilter class
     *  Span a grid with user-defined cell-size over a @see FeatureSet and instead of
     *  filtering the best features image-wide, filter the best n features for every
     *  given cell, enforcing a more regular spread of features over the image.
     *  This better coverage might produce better results even if some of the features
     *  are lower-scored opposed to an image-global feature ranking.
     *  Example of usage:
     *
     *  cvt::FeatureSet myFeatures;
     *  // ... fill myFeatures somehow ...
     *  cvt::GridFilter gridFilter( myFeatures, 1024, 768, 10, 10, 100 );
     *  gridFilter.filterGrid(); // this will modify myFeatures!
     */
    class GridFilter {
        public:
            typedef std::vector<const Feature*>		GridCell;
            typedef std::vector<GridCell>           GridCellRow;
            typedef std::vector<GridCellRow>        Grid;

            /**
             * @brief GridFilter - Constructor
             * @param featureSet        A given FeatureSet that will be modified
             * @param featuresPerCell   Keep the best [featuresPerCell] features
             * @param imageWidth        Width of the image containing the features
             * @param imageHeight       Height of the image containing the features
             * @param xSections         Subdivide image's width  into xSections sections
             * @param ySections         Subdivide image's height into ySections sections
             */
            GridFilter( FeatureSet& featureSet, size_t featuresPerCell, size_t imageWidth,
                       size_t imageHeight, size_t xSections, size_t ySections ):
                _featureSet( featureSet ),
                _featuresPerCell( featuresPerCell ),
                _cellWidth( ceil( static_cast<float>( imageWidth ) / xSections ) ),
                _cellHeight( ceil ( static_cast<float>( imageHeight ) / ySections ) )
            {
                // the dimensions are known here, so we can preallocate the vectors
                GridCell cell;
                cell.reserve( _featuresPerCell );
                GridCellRow cellRow( xSections, cell );
                _grid = Grid( ySections, cellRow );
            }


            /**
             * @brief filterGrid
             *	Start the filtering operation. Results are saved in the initially
             *	passed in featureSet, @see GridFilter::GridFilter
             */
            void filterGrid();

        private:
            FeatureSet& _featureSet;
            size_t		_featuresPerCell;
            size_t		_cellWidth;
            size_t		_cellHeight;
            Grid        _grid;

            struct AddOperator {
                AddOperator( FeatureSet& fs ) : featureSet( fs ) {}
                FeatureSet& featureSet;
                inline void operator() ( const Feature* f ) { featureSet.add( *f ); }
            };

            struct CmpScore {
                bool operator()( const Feature* f1, const Feature* f2 ) {
                    return f1->score < f2->score;
                }
            };

            GridCell& gridCell( size_t x, size_t y );
            void getBestFeatures();
    };
}

#endif
