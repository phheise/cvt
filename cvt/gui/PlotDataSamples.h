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

#ifndef CVT_PLOTDATASAMPLES_H
#define CVT_PLOTDATASAMPLES_H

#include <cvt/gui/PlotData.h>

namespace cvt {
	class PlotDataSamples : public PlotData 
	{
	 	public:
			PlotDataSamples();
			~PlotDataSamples();
	
			void addSample( const Point2f& pt );
			void clear();

			void dataInRectWithStyle( std::vector<Point2f>& data, const Rectf& rect, PlotStyle style ) const;

		private:
			std::vector<Point2f> _samples;
	};

	inline PlotDataSamples::PlotDataSamples()
	{
	}

	
	inline PlotDataSamples::~PlotDataSamples()
	{
	}

	inline void PlotDataSamples::addSample( const Point2f& pt )
	{
		_samples.push_back( pt );
		changed.notify( this );
	}

	inline void PlotDataSamples::clear()
	{
		_samples.clear();
		changed.notify( this );
	}

	inline void PlotDataSamples::dataInRectWithStyle( std::vector<Point2f>& data, const Rectf& rect, PlotStyle style ) const
	{
		if( !_samples.size() )
			return;
		switch( style ) {
			case PLOT_STYLE_LINES:
				pointsInRectToLines( data, rect, &_samples[ 0 ], _samples.size() );
				return;
			case PLOT_STYLE_DOTS:
				pointsInRect( data, rect, &_samples[ 0 ], _samples.size() );
				return;
		}
	}

};

#endif
