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

#ifndef CVT_LUP_H
#define CVT_LUP_H

#include <cvt/math/Vector.h>

namespace cvt {

    template<typename T>
     class LUDecomposition {
         public:
            typedef typename Vector<T::DIMENSION, typename T::TYPE>::TYPE VECTYPE;

             LUDecomposition( const T& m ) : _lu( m )
             {
                 for( int k = 0; k < T::DIMENSION; k++ ) {
                     for( int i = k + 1; i < T::DIMENSION; i++ ) {
                         _lu[ i ][ k ] /= _lu[ k ][ k ];
                         for( int l = k + 1; l < T::DIMENSION; l++ ) {
                             _lu[ i ][ l ] -= _lu[ i ][ k ] * _lu[ k ][ l ];
                         }
                     }
                 }
             }

             void solve( VECTYPE& x, const VECTYPE& b ) const
             {
                 VECTYPE tmp;
                 fwdSubst( tmp, b );
                 bwdSubst( x, tmp );
             }

             void inverse( T& ret )
             {
                VECTYPE x, b;
                b.setZero();

                for( int i = 0; i < T::DIMENSION; i++ ) {
                    b[ i ] = ( typename T::TYPE ) 1;
                    solve( x, b );
                    for( int k = 0; k < T::DIMENSION; k++ ) {
                        ret( k, i ) = x[ k ];
                    }
                    b[ i ] = ( typename T::TYPE ) 0;
                }
             }

         private:
             void fwdSubst( VECTYPE& x, const VECTYPE& b ) const
             {
                 x[ 0 ] = b[ 0 ];
                 for( int i = 1; i < T::DIMENSION; i++ ) {
                     typename T::TYPE sum = b[ i ];
                     for( int k = 0; k < i; k++ )
                         sum -= _lu[ i ][ k ] * x[ k ];
                     x[ i ] = sum;
                 }
             }


             void bwdSubst( VECTYPE& x, const VECTYPE& b ) const
             {
                 const int LASTENTRY = T::DIMENSION - 1;

                 x[ LASTENTRY ] = b[ LASTENTRY ] / _lu[ LASTENTRY ][ LASTENTRY ];
                 for( int i = 1; i >= 0; i-- ) {
                     typename T::TYPE sum = 0;
                     for( int k = i + 1; k < T::DIMENSION; k++ )
                         sum += _lu[ i ][ k ] * x[ k ];
                     x[ i ] = ( b[ i ] - sum ) / _lu[ i ][ i ];
                 }
             }

             T          _lu;
     };

    template<typename T>
     class LUPDecomposition {
         public:
            typedef typename Vector<T::DIMENSION, typename T::TYPE>::TYPE VECTYPE;

            LUPDecomposition( const T& m ) : _lu( m )
            {


                for( int k = 0; k < T::DIMENSION; k++ ) {
                    // find row with maximum value at k
                    _p[ k ] = k;
                    for( int i = k + 1; i < T::DIMENSION; i++ ) {
                        if( _lu[ _p[ k ] ][ k ] < _lu[ i ][ k ] )
                            _p[ k ] = i;
                    }
                    // swap if necessary
                    if( _p[ k ] != k ) {
                        typename T::TYPE tmp;
                        for( int i = 0; i < T::DIMENSION; i++ ) {
                            tmp = _lu[ k ][ i ];
                            _lu[ k ][ i ] = _lu[ _p[ k ] ][ i ];
                            _lu[ _p[ k ] ][ i ] = tmp;
                        }
                    }
                    // normal LU
                    for( int i = k + 1; i < T::DIMENSION; i++ ) {
                        _lu[ i ][ k ] /= _lu[ k ][ k ];
                        for( int l = k + 1; l < T::DIMENSION; l++ ) {
                            _lu[ i ][ l ] -= _lu[ i ][ k ] * _lu[ k ][ l ];
                        }
                    }
                }
            }

             void solve( VECTYPE& x, const VECTYPE& b ) const
             {
                 VECTYPE tmp;
                 fwdSubst( tmp, b );
                 bwdSubst( x, tmp );
             }

             void inverse( T& ret )
             {
                VECTYPE x, b;
                b.setZero();

                for( int i = 0; i < T::DIMENSION; i++ ) {
                    b[ i ] = ( typename T::TYPE ) 1;
                    solve( x, b );
                    for( int k = 0; k < T::DIMENSION; k++ ) {
                        ret( k, i ) = x[ k ];
                    }
                    b[ i ] = ( typename T::TYPE ) 0;
                }
             }

         private:
             void fwdSubst( VECTYPE& x, const VECTYPE& b ) const
             {
                 x            = b;
                 x[ _p[ 0 ] ] = b[ 0 ];
                 x[ 0 ]       = b[ _p[ 0 ] ];

                 for( int i = 1; i < T::DIMENSION; i++ ) {
                     typename T::TYPE sum = x[ _p[ i ] ];
                    x[ _p[ i ] ] = x[ i ];
                     for( int k = 0; k < i; k++ )
                         sum -= _lu[ i ][ k ] * x[ k ];
                     x[ i ] = sum;
                 }
             }


             void bwdSubst( VECTYPE& x, const VECTYPE& b ) const
             {
                 const int LASTENTRY = T::DIMENSION - 1;

                 x[ LASTENTRY ] = b[ LASTENTRY ] / _lu[ LASTENTRY ][ LASTENTRY ];
                 for( int i = 1; i >= 0; i-- ) {
                     typename T::TYPE sum = 0;
                     for( int k = i + 1; k < T::DIMENSION; k++ )
                         sum += _lu[ i ][ k ] * x[ k ];
                     x[ i ] = ( b[ i ] - sum ) / _lu[ i ][ i ];
                 }
             }

             T          _lu;
             int        _p[ T::DIMENSION ];
     };
}

#endif


