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

#ifndef CVT_APPROXMEDIAN_H
#define CVT_APPROXMEDIAN_H

#include <vector>

namespace cvt {

    class ApproxMedian
    {
        public:
            ApproxMedian( float min, float max, float resolution ) :
                _min( min ),
                _max( max ),
                _range( _max - _min ),
                _resolution( resolution ),
                _samples( 0 )
            {
                size_t nBins = _range / resolution;
                _hist.resize( nBins, 0 );
            }

            void add( float value )
            {
                value = Math::clamp( value, _min, _max );
                float fidx =  value / ( _range ) * ( float ) ( _hist.size() - 1 ) + 0.5f;
                int idx = ( int ) fidx;

//                if( idx < 0 || idx >= _hist.size() )
//                    std::cout << "value: " << value << " idx = " << idx << std::endl;
                _hist[ idx ] += 1;
                _samples++;
            }

            // approximate the nth value
            float approximateNth( size_t nth ) const
            {
                size_t bin = 0;
                size_t num = _hist[ bin++ ];

                while( num < nth ){
                    num += _hist[ bin++ ];
                }
                bin--;
                size_t nPrev = num - _hist[ bin ];

                if( bin )
                    bin--;

                // previous is smaller:
                float frac = ( nth - nPrev ) / ( float )( num - nPrev );

                return ( bin + frac ) * _resolution;
            }

            size_t numSamples() const { return _samples; }

            void clearHistogram()
            {
                for( size_t i = 0; i < _hist.size(); i++ ){
                    _hist[ i ] = 0;
                }
                _samples = 0;
            }

        private:
            float               _min;
            float               _max;
            float               _range;
            float               _resolution;
            size_t              _samples;
            std::vector<size_t> _hist;
    };

}

#endif // APPROXMEDIAN_H
