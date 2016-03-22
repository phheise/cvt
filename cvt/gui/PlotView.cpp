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

#include <cvt/gui/PlotView.h>

namespace cvt {
	PlotView::PlotView() :
		_xmargin( 50 ), _ymargin( 40 ),
		_xticslabelmargin( 5 ), _yticslabelmargin( 5 ),
		_view( -5.0f, -5.0f, 10.0f, 10.0f ),
		_xtics( 1.0f ),
		_ytics( 1.0f ),
		_grid( true ),
		_xticslabel( "%.1f" ),
		_yticslabel( "%.1f" ),
		_xlabel( "x" ),
		_ylabel( "y" )
	{
	}

	PlotView::~PlotView()
	{
		_pdata.clear();
	}


	void PlotView::paintEvent( PaintEvent& , GFX& g )
	{
		int w, h;
		size( w, h );

		// fill background
		g.color() = Color::WHITE;
		g.fillRect( 0, 0, w, h );

		g.setLineWidth( 1.0f );


		if( w <= 2 * _xmargin || h <= 2 * _ymargin )
			return;

		String tmp;
		float sx = ( float ) ( w - 2 * _xmargin ) / _view.width;
		float sy = ( float ) ( h - 2 * _ymargin ) / _view.height;
		float xtic = sx * _xtics;
		float ytic = sy * _ytics;

		// draw grid
		if( _grid ) {
			g.setColor( Color( 0.0f, 0.0f, 0.0f, 0.1f ) );
			// draw vertical lines
			for( float x = _xmargin; x <= w - _xmargin + 1; x += xtic  )
				g.drawLine( x, _ymargin, x, h - _ymargin );
			// draw horizontal lines
			for( float y = _ymargin; y <= h - _ymargin + 1; y += ytic  )
				g.drawLine( _xmargin, y, w - _xmargin, y );
		}

		// draw x/y axes
		g.color() = Color::BLACK;
		g.drawLine( _xmargin, _ymargin, _xmargin, h - _ymargin - 1 );
		g.drawLine( _xmargin, h - _ymargin, w - _xmargin - 1, h - _ymargin );


		// draw x tics and label
		for( float x = _xmargin, xpos = _view.x; x <= w - _xmargin + 1; x += xtic, xpos += _xtics  ) {
			g.drawLine( x, h - _ymargin, x, h - _ymargin - 5 );
			tmp.sprintf( _xticslabel.c_str(), xpos  );
			g.drawText( x - 20.0f, h - _ymargin + _xticslabelmargin, 40, 15, ALIGN_CENTER, tmp.c_str() );
		}

		// draw y tics and label
		for( float y = _ymargin, ypos = _view.y + _view.height; y <= h - _ymargin + 1; y += ytic, ypos -= _ytics  ) {
			g.drawLine( _xmargin, y, _xmargin + 5, y );
			tmp.sprintf( _yticslabel.c_str(), ypos  );
			g.drawText( 0, y - 10, _xmargin - _yticslabelmargin, 20, ALIGN_RIGHT | ALIGN_VCENTER, tmp.c_str() );
		}

		// Draw data
		for( List<PlotDataStyled>::Iterator it = _pdata.begin(); it != _pdata.end(); ++it ) {
			g.setColor( ( *it )._pdata->plotColor() );
			switch(  ( *it )._pdata->plotStyle() ) {
				case PLOT_STYLE_DOTS:
					{
						Matrix3f T( sx, 0, -sx * _view.x + _xmargin + 1 ,
								   0, -sy, sy * _view.y + h - _ymargin + 1,
								   0, 0, 1.0f );
						size_t n = ( *it)._data.size();
						for( size_t i = 0; i < n; i++ ) {
							Vector2f pt = T * ( *it )._data[ i ];
							g.drawIcons( &pt, 1, GFX::ICON_TINY_CROSS  );
						}
					}
					break;
				case PLOT_STYLE_LINES:
					{
						g.setLineWidth( ( *it )._pdata->plotSize() );
						Matrix3f T( sx, 0, -sx * _view.x + _xmargin ,
								   0, -sy, sy * _view.y + h - _ymargin,
								   0, 0, 1.0f );
						size_t n = ( *it)._data.size() >> 1;
						for( size_t i = 0; i < n; i++ ) {
							g.drawLine( T * ( *it )._data[ i << 1 ] , T * ( *it )._data[ ( i << 1 ) + 1 ] );
						}
					}
					break;
			}
		}


	}
}
