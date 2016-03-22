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
 * File:   LSH.h
 * Author: sebi
 *
 * Created on July 22, 2011, 3:56 PM
 */

#ifndef LSH_H
#define	LSH_H

#include <cvt/vision/ORB.h>
#include <algorithm>
#include <list>

namespace cvt {

    template <size_t NumBits = 8, size_t NumTables = 4>
    class LSH {
      public:
        LSH( const std::vector<ORBFeature> * features );
        int find( const ORBFeature& feature, size_t& dist, size_t maxDistance = 30 );

        void updateFeatures( const std::vector<ORBFeature> * features );

      private:
        struct HistPos
        {
            int accum;
            size_t pos;
            static int halfOrbSize;

            bool operator<( const HistPos & other ) const
            {
                return ::abs( accum * 2 - halfOrbSize ) <= ::abs( other.accum * 2 - halfOrbSize );
            }
        };

        void hashORBFeatures();

        void entropy();

        size_t hash( const ORBFeature & ft, size_t table ) const;

        const std::vector<ORBFeature> * _features;

        size_t _bitPos[ NumTables ][ NumBits ];

        std::list<int> _htable[ NumTables ][ ( 1 << NumBits ) ];
    };

    template<size_t NumBits, size_t NumTables>
    int LSH<NumBits, NumTables>::HistPos::halfOrbSize;

    template <size_t NumBits, size_t NumTables>
    inline LSH<NumBits, NumTables>::LSH( const std::vector<ORBFeature>* orb ) : _features( orb )
    {
        entropy();
        hashORBFeatures();
    }

    template <size_t NumBits, size_t NumTables>
    inline void LSH<NumBits, NumTables>::updateFeatures( const std::vector<ORBFeature> * features )
    {
        _features = features;
        for( size_t i = 0; i < NumTables; i++ ){
            for( size_t k = 0; k < ( 1 << NumBits ); k++ ){
                _htable[ i ][ k ].clear();
            }
        }

        entropy();
        hashORBFeatures();
    }

    #define GETBIT( ft, n ) ( ( ft.desc[ n >> 3 ] >> ( n & 0x07 ) ) & 1 )
    template <size_t NumBits, size_t NumTables>
    inline void LSH<NumBits, NumTables>::entropy()
    {
        std::vector<HistPos> hist;
		hist.reserve( 256 );

        HistPos::halfOrbSize = _features->size();

        for( size_t i = 0; i < 256; i++ ){
            hist[ i ].accum = 0;
            hist[ i ].pos = i;
        }

        for ( size_t i = 0, end = _features->size( ); i < end; i++ ) {
            const ORBFeature& feature = ( *_features )[ i ];

            for( size_t b = 0; b < 0xFF; b++ ){
                if( GETBIT( feature, b ) )
                    hist[ b ].accum++;
            }
        }

        std::sort( hist.begin(), hist.end() );

        // save the NumBits best bit positions for hash generation
        for ( size_t n = 0; n < NumTables; n++ ) {
            for ( size_t i = 0; i < NumBits; i++ ) {
                _bitPos[ n ][ i ] = hist[ n * NumBits + i ].pos;
            }
        }

        /*
        for( size_t b = 0; b < 0xFF; b++ ){
            std::cout << ( float )hist[ b ].accum / ( float )_features->size()  << " " << hist[ b ].pos << " acuum: " << hist[ b ].accum << " abs:"<< ::abs( hist[ b ].accum * 2 - HistPos::halfOrbSize ) <<"\n";
        }
        std::cout << std::endl;
         */
    }

    template <size_t NumBits, size_t NumTables>
    inline size_t LSH<NumBits, NumTables>::hash( const ORBFeature & ft, size_t table ) const
    {
        size_t ret = 0;

        for( size_t i = 0; i < NumBits; i++ ){
            ret |= GETBIT( ft, _bitPos[ table ][ i ] ) << i;
        }

        return ret;
    }


    template <size_t NumBits, size_t NumTables>
    inline void LSH<NumBits, NumTables>::hashORBFeatures()
    {
        for ( size_t i = 0, end = _features->size( ); i < end; i++ ) {
            const ORBFeature& feature = ( *_features )[ i ];

            for( size_t n = 0; n < NumTables; n++ ){
                size_t idx = hash( feature, n );
                _htable[ n ][ idx ].push_back( i );
            }
        }

        /*
        for( size_t i = 0; i < ( 1 << NumBits ) - 1; i++ ){
            std::cout << _htable[ i ].size() << std::endl;
        }
         */


        /*		for( size_t k = 0; k < MAXIDX; k++ ) {
                    for( size_t l = 0; l < 256; l++ ) {
                        std::cout << k << " " << l << " " << _htable[ k ][ l ].size() << std::endl;
                    }
                }*/
    }

    template <size_t NumBits, size_t NumTables>
    inline int LSH<NumBits, NumTables>::find( const ORBFeature& feature, size_t& dist, size_t maxDistance )
    {
        int ret = -1;
        size_t retdist = maxDistance;

        for ( size_t n = 0; n < NumTables; n++ ) {
            /* Simple test without any descriptor bit twiddling */
            size_t idx = hash( feature, n );

            for ( std::list<int>::iterator it = _htable[ n ][ idx ].begin( ), end = _htable[ n ][ idx ].end( ); it != end; ++it ) {
                const ORBFeature& current = ( *_features )[ *it ];
                size_t d = feature.distance( current );
                if ( d < retdist ) {
                    retdist = d;
                    ret = *it;
                }
            }

            /* toggle one bit at each position and probe */
            for ( size_t i = 0; i < NumBits; i++ ) {
                size_t index = idx ^ ( 1 << i );
                for ( std::list<int>::iterator it = _htable[ n ][ index ].begin( ), end = _htable[ n ][ index ].end( ); it != end; ++it ) {
                    const ORBFeature& current = ( *_features )[ *it ];
                    size_t d = feature.distance( current );
                    if ( d < retdist ) {
                        retdist = d;
                        ret = *it;
                    }
                }
            }
        }

        dist = retdist;
        return ret;
    }
}


#endif	/* LSH_H */

