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

#include <cvt/gfx/GFX.h>
#include <cvt/util/CVTAssert.h>
#include <cvt/gfx/GFXEngine.h>

namespace cvt {

	GFX::GFX() : _engine( NULL ), _status( 0 ), _translationGlobal( 0, 0 )
	{
		setDefault();
	}

	GFX::GFX( const GFX& g ) :  _engine( NULL ), _status( 0 ), _color( g._color ), _linewidth( g._linewidth ), _translation( g._translation), _translationGlobal( 0, 0 )
	{
	}

	GFX::GFX( GFXEngine* engine ) : _engine( engine ), _status( 0 ), _translationGlobal( 0, 0 )
	{
		setDefault();
		begin();
	}

	GFX::~GFX()
    {
        if( _status & GFX_ACTIVE )
            end();
        if( _status & GFX_DELETE_ENGINE )
            delete _engine;
    }

	GFX::GFX( Drawable* drawable ) : _engine( NULL ), _status( 0 ), _translationGlobal( 0, 0 )
	{
		_engine = drawable->gfxEngine();
        _status |= GFX_DELETE_ENGINE;
		setDefault();
		begin();
	}

	GFX::GFX( Drawable& drawable ) : _engine( NULL ), _status( 0 ), _translationGlobal( 0, 0 )
	{
		_engine = drawable.gfxEngine();
        _status |= GFX_DELETE_ENGINE;
		setDefault();
		begin();
	}

	void GFX::begin()
	{
		CVT_ASSERT( !( _status & GFX_ACTIVE ), "GFX engine already active" );
		_status |= GFX_ACTIVE;
		_engine->begin();
	}

	void GFX::begin( GFXEngine* engine )
	{
		CVT_ASSERT( ( !( _status & GFX_ACTIVE ) ) && _engine == NULL && engine, "GFX engine already active or NULL"  );
		_engine = engine;
		begin();
	}

	void GFX::begin( Drawable* draw )
	{
		CVT_ASSERT( ( !( _status & GFX_ACTIVE ) ) && _engine == NULL && draw, "GFX engine already active or NULL"   );
		_engine = draw->gfxEngine();
		CVT_ASSERT( _engine, "GFX engine is NULL" );
		begin();
	}

	void GFX::end()
	{
		CVT_ASSERT( _status & GFX_ACTIVE, "GFX engine is not active"  );
		_status &= ~GFX_ACTIVE;
		_engine->end();
	}

	void GFX::setDefault()
	{
		_color.set( 1.0f, 1.0f, 1.0f, 1.0f );
		_linewidth = 1.0f;
		_translation.setZero();
		_engine->setTranslation( _translationGlobal );
	}

	void GFX::drawLine( int x1, int y1, int x2, int y2 )
	{
		Vector2f pts[ 2 ];
		pts[ 0 ].set( ( float ) x1 + 0.5f, ( float ) y1 + 0.5f );
		pts[ 1 ].set( ( float ) x2 + 0.5f, ( float ) y2 + 0.5f );
		_engine->drawLines( pts, 2, _linewidth, _color );
	}


	void GFX::drawLine( const Vector2f& pt1, const Vector2f& pt2 )
	{
		Vector2f pts[ 2 ];
		pts[ 0 ] = pt1;
		pts[ 1 ] = pt2;
		_engine->drawLines( pts, 2, _linewidth, _color );
	}

	void GFX::drawLines( const Vector2f* pts, size_t n )
	{
		_engine->drawLines( pts, n, _linewidth, _color );
	}

	void GFX::drawRect( const Recti& rect )
	{
		_engine->drawRect( rect, _linewidth, _color );
	}
	
	void GFX::fillRect( const Recti& rect )
	{
		_engine->fillRect( rect, _color );
	}

	void GFX::fillRect( int x, int y, int width, int height )
	{
		Recti rect( x, y, width, height );
		_engine->fillRect( rect, _color );
	}

	void GFX::fillRoundRect( const Recti& rect, float radius )
	{
		_engine->fillRoundRect( rect, radius, _color );
	}

	void GFX::fillRoundRect( int x, int y, int width, int height, float radius )
	{
		Recti rect( x, y, width, height );
		_engine->fillRoundRect( rect, radius, _color );
	}

	void GFX::fillPath( const Pathf& path, GFX::PolygonFillRule rule )
	{
		_engine->fillPath( path, _color, rule );
	}

	void GFX::strokePath( const Pathf& path )
	{
		_engine->strokePath( path, _linewidth, _color, GFX::STROKE_MITER );
	}

	const Font& GFX::font( ) const
	{
		return _engine->font();
	}

	void GFX::drawText( int x, int y, const char* text )
	{
		_engine->drawText( x, y, text, _color );
	}


	void GFX::drawText( const Recti& rect, Alignment alignment, const char* text )
	{
		Recti bbox;
		int x , y;

		bbox = _engine->font().stringBounds( text );

		if( alignment & ALIGN_CENTER )
			x = rect.x + bbox.x + ( rect.width - bbox.width ) / 2;
		else if( alignment & ALIGN_RIGHT )
			x = rect.x + bbox.x + ( rect.width - bbox.width );
		else //if( alignment & ALIGN_LEFT )
			x = rect.x + bbox.x;

		if( alignment & ALIGN_VCENTER )
			y = rect.y + ( rect.height - bbox.height ) / 2;
		else if( alignment & ALIGN_BOTTOM )
			y = rect.y + ( rect.height - bbox.height );
		else //if( alignment & ALIGN_TOP )
			y = rect.y;

		y += _engine->font().ascent();
		drawText( x, y, text );
	}

	void GFX::drawImage( int x, int y, const Image& img )
	{
		_engine->drawImage( x, y, img, _color.alpha() );
	}

	void GFX::drawImage( int x, int y, int width, int height, const Image& img )
	{
		Recti rect( x, y, width, height );
		_engine->drawImage( rect , img, _color.alpha() );
	}

	void GFX::drawIcon( int x, int y, Icon i )
	{
		_engine->drawIcon( x, y, i, _color );
	}

	void GFX::drawIcons( const Vector2i* pts, size_t npts, Icon i )
	{
		_engine->drawIcons( pts, npts, i, _color );
	}

	void GFX::drawIcons( const Vector2f* pts, size_t npts, Icon i )
	{
		_engine->drawIcons( pts, npts, i, _color );
	}


	void GFX::setClipRect( const Recti& r )
	{
		_engine->setClipRect( r );
	}

	const Recti& GFX::clipRect() const
	{
		return _engine->clipRect();
	}

	bool GFX::supportsGL() const
	{
		return _engine->supportsGL();
	}

	void GFX::beginGL()
	{
		_engine->beginGL();
	}

	void GFX::endGL()
	{
		_engine->endGL();
	}

	void GFX::setTranslation( const Vector2i& translation )
	{
		_translation = translation;
		_engine->setTranslation( _translationGlobal + _translation );
	}

	void GFX::setTranslation( int tx, int ty )
	{
		_engine->setTranslation( _translation + Vector2i( tx, ty ) );
	}

	const Vector2i& GFX::translation() const
	{
		return _translation;
	}

	void GFX::setTranslationGlobal( const Vector2i& translation )
	{
		_translationGlobal = translation;
	}

	void GFX::getTranslationGlobal( Vector2i& translation ) const
	{
		translation = _translationGlobal;
	}

}
