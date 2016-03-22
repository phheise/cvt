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

#ifndef CVT_PLOTDATA_H
#define CVT_PLOTDATA_H

#include <cvt/util/Signal.h>
#include <cvt/math/Vector.h>
#include <cvt/geom/Rect.h>
#include <cvt/gfx/Clipping.h>

namespace cvt {
	enum PlotStyle {
		PLOT_STYLE_DOTS,
		PLOT_STYLE_LINES
	};

	class PlotData {
		public:
			PlotData();
			virtual ~PlotData();

			PlotStyle plotStyle() const;
			void	  setPlotStyle( PlotStyle style );
			void	  setPlotColor( const Color& c );
			const	  Color& plotColor() const;
			void	  setPlotSize( float s );
			float	  plotSize() const;
			void      setName( const String& title );
			const     String& name() const;

			virtual void dataInRectWithStyle( std::vector<Point2f>& data, const Rectf& rect, PlotStyle style ) const = 0;

			Signal<PlotData*> changed;

		protected:
			static void pointsInRectToLines( std::vector<Point2f>& lines, const Rectf& rect, const Point2f* inpts, size_t n );
			static void pointsInRect( std::vector<Point2f>& lines, const Rectf& rect, const Point2f* inpts, size_t n );

		private:
			PlotStyle _style;
			Color	  _color;
			float	  _size;
			String    _name;
	};

	inline PlotData::PlotData() :
		_style( PLOT_STYLE_LINES ),
		_color( Color::RED ),
		_size( 1.0f ),
		_name( "unnamed" )
	{
	}

	inline PlotData::~PlotData()
	{
	}

	inline PlotStyle PlotData::plotStyle() const
	{
		return _style;
	}

	inline void PlotData::setPlotStyle( PlotStyle style )
	{
		_style = style;
		changed.notify( this );
	}

	inline void PlotData::setPlotColor( const Color& c )
	{
		_color = c;
		changed.notify( this );
	}

	inline const Color& PlotData::plotColor() const
	{
		return _color;
	}

	inline void  PlotData::setPlotSize( float s )
	{
		_size = s;
		changed.notify( this );
	}

	inline float PlotData::plotSize() const
	{
		return _size;
	}

	inline void PlotData::setName( const String& name )
	{
		_name = name;
		changed.notify( this );
	}

	inline const String& PlotData::name() const
	{
		return _name;
	}

	inline void PlotData::pointsInRectToLines( std::vector<Point2f>& lines, const Rectf& rect, const Point2f* inpts, size_t n )
	{
		Vector2f pt1, pt2;
		lines.clear();
		n--;
		while( n-- ) {
			pt1 = *inpts;
			pt2 = *( inpts + 1 );
			if( Clipping::clip( rect, pt1, pt2 ) ) {
				lines.push_back( pt1 );
				lines.push_back( pt2 );
			}
			inpts++;
		}
	}

	inline void PlotData::pointsInRect( std::vector<Point2f>& pts, const Rectf& rect, const Point2f* inpts, size_t n )
	{
		pts.clear();
		while( n-- ) {
			if( rect.contains( *inpts ) ) {
				pts.push_back( *inpts );
			}
			inpts++;
		}
	}
}

#endif
