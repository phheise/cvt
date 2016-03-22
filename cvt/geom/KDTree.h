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

#ifndef CVT_KDTREE_H
#define CVT_KDTREE_H

#include <vector>

#include <cvt/math/Vector.h>

namespace cvt
{

#define PT( n ) _pts[ _ptidx[ n ] ][ idx ]
#define SWAP( a, b ) do { ssize_t t = _ptidx[ a ]; _ptidx[ a ] = _ptidx[ b ]; _ptidx[ b ] = t; } while( 0 )

    template<class _T=Point2f>
	class KDTree {
		public:
			KDTree( const std::vector<_T> & pts );
			~KDTree();

			// return index of nearest neighbor
			ssize_t locate( const _T & pt, float dist );
            void rangeSearch( std::vector<_T> &output, const _T &pt, float dist );

		private:
            void build( ssize_t l, ssize_t h, ssize_t idx );
            ssize_t partition( ssize_t l, ssize_t h, ssize_t x, ssize_t idx );
			/* select the x-th element according to dimension idx */
            void select(  ssize_t l, ssize_t h, ssize_t x, ssize_t idx );

            ssize_t visit( ssize_t low, ssize_t high, const _T & pt, float distance, ssize_t idx );
            void visit( std::vector<_T> &output, const _T &pt, double *min, double *max, double distance, ssize_t low, ssize_t high, ssize_t idx );

			void print();
            void check( ssize_t l, ssize_t h, ssize_t idx );

			const std::vector<_T>&	_pts;
			ssize_t*				_ptidx;
            size_t					_dim;
	};

    template <class _T>
    inline KDTree<_T>::KDTree( const std::vector<_T> & pts ) : _pts( pts )
    {
        size_t npts = _pts.size();
        if( npts == 0 )
            return;
        _ptidx = new ssize_t[ npts ];
        _dim = pts[0].dimension();

        for( size_t i = 0; i < npts; i++ )
            _ptidx[ i ] = i;

        build( 0, npts - 1, 0 );
    }

    template<class _T>
    inline KDTree<_T>::~KDTree()
    {
        delete[] _ptidx;
    }

    template <class _T>
    inline ssize_t KDTree<_T>::locate( const _T & pt, float dist )
    {
        ssize_t nearestIndex = visit( 0, _pts.size() - 1, pt, dist, 0 );

        return _ptidx[ nearestIndex ];
    }

    template <class _T>
    inline void KDTree<_T>::build( ssize_t l, ssize_t h, ssize_t idx )
    {
        if (l<h)
        {
            ssize_t med = (l+h)>>1;
            select( l, h, med, idx );

            build( l, med-1, (idx+1)%_dim );
            build( med+1, h, (idx+1)%_dim );
        }
    }

    template <class _T>
    inline ssize_t KDTree<_T>::partition( ssize_t l, ssize_t h, ssize_t x, ssize_t idx )
    {
        ssize_t pivot = l;

        SWAP( x, h );

        for ( ssize_t i = l; i < h; ++i )
        {
            if ( PT(i) < PT(h) )
            {
                SWAP( pivot, i );
                pivot++;
            }
        }

        SWAP( pivot, h );
        return pivot;
    }

    template <class _T>
    void KDTree<_T>::select( ssize_t l, ssize_t h, ssize_t x, ssize_t idx )
    {
        if (h>l)
        {
            ssize_t pivot = partition( l, h, x, idx );

            if ( pivot > x )
                select( l, pivot-1, x, idx );
            if ( pivot < x )
                select( pivot+1, h, x, idx );
        }
    }

    template <class _T>
    inline ssize_t KDTree<_T>::visit( ssize_t low, ssize_t high,
                                       const _T & pt, float dist, ssize_t idx )
    {
        if( high - low < 2 ){
            // check low and high
            float nearest = ( _pts[ _ptidx[ low ] ] - pt ).length();
            if( nearest <  ( _pts[ _ptidx[ high ] ] - pt ).length() )
                return low;
            else
                return high;
        }

        ssize_t medianIdx = ( high + low ) >> 1;

        float min = pt[ idx ] - dist;
        float max = pt[ idx ] + dist;

        if( PT( medianIdx ) > max ){
            // search left half
            return visit( low, medianIdx - 1, pt, dist, (idx+1)%_dim );
        }

        if( PT( medianIdx ) < min ){
            // search right half
            return visit( medianIdx + 1, high, pt, dist, (idx+1)%_dim );
        }

        // search both halfes and compare the results and also the median!:
        ssize_t bestLow = visit( low, medianIdx - 1, pt, dist, (idx+1)%_dim );
        ssize_t bestHigh = visit( medianIdx + 1, high, pt, dist, (idx+1)%_dim );

        const _T & other = _pts[ _ptidx[ medianIdx ] ];
        ssize_t bestIdx = medianIdx;
        float nearestDistance = ( other - pt ).length();

        float tmp = ( _pts[ _ptidx[ bestLow ] ] - pt ).length();
        if( tmp < nearestDistance ){
            nearestDistance = tmp;
            bestIdx = bestLow;
        }

        tmp = ( _pts[ _ptidx[ bestHigh ] ] - pt ).length();
        if( tmp < nearestDistance ){
            nearestDistance = tmp;
            bestIdx = bestHigh;
        }

        return bestIdx;
    }

    template<class _T>
    void KDTree<_T>::rangeSearch( std::vector<_T> &output, const _T &pt, float distance )
    {
        double min[ _dim ];
        double max[ _dim ];

        for( size_t i = 0; i < _dim; ++i ){
            min[i] = pt[i] - distance;
            max[i] = pt[i] + distance;
        }

        visit( output, pt, min, max, distance, 0, _pts.size()-1, 0 );
    }

    template<class _T>
    void KDTree<_T>::visit( std::vector<_T> &output, const _T &pt, double *min, double *max, double distance, ssize_t l, ssize_t h, ssize_t idx )
    {
        ssize_t med = (l+h) >> 1;
        const _T &root = _pts[ _ptidx[ med ] ];

        if (h >= l)
        {
            float dist = (root-pt).length();

            if ( dist <= distance )
            {
                output.push_back(root);
            }
        }

        if (h > l)
        {
            if (max[idx] >= root[idx])
                visit( output, pt, min, max, distance, med+1, h, (idx+1)%_dim );

            if (min[idx] <= root[idx])
                visit( output, pt, min, max, distance, l, med-1, (idx+1)%_dim );
        }
    }

    template<class _T>
    inline void KDTree<_T>::check( ssize_t l, ssize_t h, ssize_t idx )
    {
        ssize_t med = ( l + h ) >> 1;

        if( l >= h )
            return;

        for( ssize_t x = l; x < h; x++ ) {
            if( x < med && PT( x ) > PT( med ) )
                std::cout << "lower than median has bigger value" << std::endl;
            if( x > med && PT( x ) < PT( med ) )
                std::cout << "bigger than median has lower value" << std::endl;
        }
        check( l, med - 1, (idx+1)%_dim );
        check( med + 1, h, (idx+1)%_dim );
    }

    template<class _T>
    inline void KDTree<_T>::print()
    {
        for( size_t idx = 0; idx < _dim; ++idx ) {
            printf( "dim %lu : ", idx );
            for( ssize_t i = 0; i < _pts.size(); i++ ){
                std::cout << PT( i ) << " ";
            }
            printf( "\n\n" );
        }
    }
}

#endif
