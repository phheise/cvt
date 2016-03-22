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

#ifndef CVT_PLOTVIEW_H
#define CVT_PLOTVIEW_H

#include <cvt/gui/Widget.h>
#include <cvt/container/List.h>

#include <cvt/gui/PlotData.h>

namespace cvt {
	class PlotView : public Widget {
		public:
			PlotView();
			~PlotView();

			void			setView( const Rectf& r );
			const Rectf&	view() const;

			void			setXMargin( int value );
			int				xMargin() const;
			void			setYMargin( int value );
			int				yMargin() const;
			void			setXTicsLabelMargin( int value );
			int				xTicsLabelMargin() const;
			void			setYTicsLabelMargin( int value );
			int				yTicsLabelMargin() const;

			void			setXTics( float xtics );
			float			xTics() const;
			void			setYTics( float xtics );
			float			yTics() const;
			void			setGrid( bool b );
			bool			grid() const;
			void			setXTicsLabel( const String& xticslabel );
			const String&	xTicsLabel() const;
			void			setYTicsLabel( const String& yticslabel );
			const String&	yTicsLabel() const;
			void			setXLabel( const String& xlabel );
			const String&	xLabel() const;
			void			setYLabel( const String& xlabel );
			const String&	yLabel() const;

			void			addPlotData( PlotData* pdata );
			void			removePlotData( PlotData* pdata );
			size_t			plotDataSize() const;

			void paintEvent( PaintEvent& pe, GFX& g);

		private:
			int				_xmargin, _ymargin;
			int				_xticslabelmargin, _yticslabelmargin;
			Rectf			_view;
			float			_xtics, _ytics;
			bool			_grid;
			String			_xticslabel, _yticslabel;
			String			_xlabel, _ylabel;


			inline void onChange( PlotData* pdata );

			struct PlotDataStyled{
				PlotDataStyled( PlotData* pdata, const Rectf& rect ) : _pdata( pdata ) { update( rect ); }
				void update( const Rectf& rect ) {
					_pdata->dataInRectWithStyle( _data, rect, _pdata->plotStyle() );
				}

				bool operator==( const PlotData* pdata ) { return _pdata == pdata; }

				PlotData* _pdata;
				std::vector<Point2f> _data;
			};

			void updateData( PlotDataStyled& pds );

			List<PlotDataStyled> _pdata;
	};

	inline void PlotView::setXMargin( int value )
	{
		if( _xmargin == value )
			return;
		_xmargin = value;
		update();
	}

	inline int PlotView::xMargin() const
	{
		return _xmargin;
	}

	inline void PlotView::setYMargin( int value )
	{
		if( _ymargin == value )
			return;
		_ymargin = value;
		update();
	}

	inline int PlotView::yMargin() const
	{
		return _ymargin;
	}

	inline void PlotView::setXTicsLabelMargin( int value )
	{
		if( _xticslabelmargin == value )
			return;
		_xticslabelmargin = value;
		update();
	}

	inline int PlotView::xTicsLabelMargin() const
	{
		return _xticslabelmargin;
	}

	inline void PlotView::setYTicsLabelMargin( int value )
	{
		if( _yticslabelmargin == value )
			return;
		_yticslabelmargin = value;
		update();
	}

	inline int PlotView::yTicsLabelMargin() const
	{
		return _yticslabelmargin;
	}

	inline void PlotView::setView( const Rectf& r )
	{
		if( _view == r )
			return;
		_view = r;

		List<PlotDataStyled>::Iterator it = _pdata.begin();
		List<PlotDataStyled>::Iterator end = _pdata.end();
		while( it != end ) {
			( *it ).update( _view );
			it++;
		}

		update();
	}

	inline const Rectf& PlotView::view() const
	{
		return _view;
	}

	inline void PlotView::setXTics( float xtics )
	{
		if( _xtics == xtics )
			return;
		_xtics = xtics;
		update();
	}

	inline float PlotView::xTics() const
	{
		return _xtics;
	}

	inline void PlotView::setYTics( float ytics )
	{
		if( _ytics == ytics )
			return;
		_ytics = ytics;
		update();
	}

	inline float PlotView::yTics() const
	{
		return _ytics;
	}

	inline void PlotView::setGrid( bool b )
	{
		if( _grid == b)
			return;
		_grid = b;
		update();
	}

	inline bool PlotView::grid() const
	{
		return _grid;
	}

	inline void PlotView::setXTicsLabel( const String& label )
	{
		if( _xticslabel == label )
			return;
		_xticslabel = label;
		update();
	}

	inline const String& PlotView::xTicsLabel() const
	{
		return _xticslabel;
	}

	inline void PlotView::setYTicsLabel( const String& label )
	{
		if( _yticslabel == label )
			return;
		_yticslabel = label;
		update();
	}

	inline const String& PlotView::yTicsLabel() const
	{
		return _yticslabel;
	}

	inline void PlotView::setXLabel( const String& label )
	{
		if( _xlabel == label )
			return;
		_xlabel = label;
		update();
	}

	inline const String& PlotView::xLabel() const
	{
		return _xlabel;
	}

	inline void PlotView::setYLabel( const String& label )
	{
		if( _ylabel == label )
			return;
		_ylabel = label;
		update();
	}

	inline const String& PlotView::yLabel() const
	{
		return _ylabel;
	}

	inline void PlotView::onChange( PlotData* pdata )
	{
		List<PlotDataStyled>::Iterator it = _pdata.find( pdata );
		if( it != _pdata.end() ) {
			( *it ).update( _view );
		}
	}

	inline void PlotView::addPlotData( PlotData* pdata )
	{
		pdata->changed.add( Delegate<void ( PlotData* )>( this, &PlotView::onChange ) );
		_pdata.append( PlotDataStyled( pdata, _view ) );
		update();
	}

	inline void PlotView::removePlotData( PlotData* pdata )
	{
		pdata->changed.remove( Delegate<void ( PlotData* )>( this, &PlotView::onChange ) );
		List<PlotDataStyled>::Iterator it = _pdata.find( pdata );
		if( it != _pdata.end() ) {
			_pdata.remove( it );
			update();
		}
	}

	inline size_t PlotView::plotDataSize() const
	{
		return _pdata.size();
	}

}

#endif
