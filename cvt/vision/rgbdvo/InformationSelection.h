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


#ifndef CVT_INFORMATIONSELECTION_H
#define CVT_INFORMATIONSELECTION_H

#include <set>
#include <algorithm>
#include <vector>
#include <cvt/math/Math.h>

namespace cvt {

    template <class JType>
    class InformationSelection
    {
        public:
            InformationSelection( size_t numpixels );
            ~InformationSelection();

            const std::set<size_t>& selectInformation( JType* j, size_t n );

            void setNumPixels( size_t n ){ _numPixels = n; }

        private:
            /* number of pixels to select */
            size_t              _numPixels;

            /* ids of all selected pixels */
            std::set<size_t>    _ids;

            /* sort input descending by parameter */
            std::vector<std::vector<size_t> > _sortedIds;

            class IndexComparator {
                public:
                    IndexComparator( const JType* data, size_t idx ) :
                        _data( data ), _idx( idx )
                    {}

                    bool operator()( size_t id0, size_t id1 )
                    {
                        return Math::abs( _data[ id0 ][ _idx ] ) > Math::abs( _data[ id1 ][ _idx ] );
                    }

                private:
                    const JType* _data;
                    size_t       _idx;
            };
    };

    template <class JType>
    inline InformationSelection<JType>::InformationSelection( size_t numPixels ) :
        _numPixels( numPixels )
    {
        // array for each degree of freedom
        _sortedIds.resize( JType::ColsAtCompileTime );
    }

    template <class JType>
    inline InformationSelection<JType>::~InformationSelection()
    {
    }

    template <class JType>
    inline const std::set<size_t>& InformationSelection<JType>::selectInformation( JType* j, size_t n )
    {
        _ids.clear();

        // sort the first _numpixel parts for each dimension
        _sortedIds[ 0 ].clear();

        // initialize the indices for the first dof
        _sortedIds[ 0 ].resize( n );
        for( size_t i = 0; i < n; i++ ){
            _sortedIds[ 0 ][ i ] = i;
        }

        {
            IndexComparator cmp( j, 0 );
            std::partial_sort( _sortedIds[ 0 ].begin(),
                               _sortedIds[ 0 ].begin() + _numPixels,
                               _sortedIds[ 0 ].end(), cmp );
        }

        for( size_t i = 1; i < _sortedIds.size(); i++ ) {
            _sortedIds[ i ] = _sortedIds[ 0 ];
            IndexComparator cmp( j, i );
            std::partial_sort( _sortedIds[ i ].begin(),
                               _sortedIds[ i ].begin() + _numPixels,
                               _sortedIds[ i ].end(), cmp );
        }

        // ids for each DOF
        std::vector<size_t> idForDim( _sortedIds.size(), 0 );

        size_t currDim = 0;// alternate dimensions while selecting the jacobians
        size_t idx = 0; // idx into id vector
        while( _ids.size() < _numPixels ){
            idx = idForDim[ currDim ];
            const std::vector<size_t>& currVec = _sortedIds[ currDim ];            
            while( _ids.find( currVec[ idx ] ) != _ids.end() ){
                idx++;                
            }

            _ids.insert( currVec[ idx ] );

            idForDim[ currDim ] = idx + 1;
            currDim++;

            if( currDim >= _sortedIds.size() )
                currDim = 0;
        }

        return _ids;
    }

}

#endif // INFORMATIONSELECTION_H
