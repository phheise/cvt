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

#include <cvt/gui/ImageView.h>
#include <cvt/gfx/IConvert.h>

namespace cvt {
	ImageView::ImageView() :
		_alpha( 1.0f ),
		_img( 1, 1, IFormat::BGRA_UINT8, IALLOCATOR_GL ),
		_fixAspect( true )
	{
	}

	ImageView::~ImageView()
	{
	}

	void ImageView::setFixAspect( bool val )
	{
		_fixAspect = val;
	}

	void ImageView::paintEvent( PaintEvent&, GFX& g )
	{
		int w, h;
		size( w, h );
		g.color().setAlpha( _alpha );
		if( _fixAspect ){
			int iw = _img.width();
			int ih = _img.height();
			if( iw > ih ){
				float aspect = ( float )ih / ( float )iw;
				int hnew = aspect * w;
				if( hnew > h ){
					w = h / aspect;
				} else {
					h = hnew;
				}
			}
		}
		g.drawImage( 0, 0, w, h, _img );
	}

	void ImageView::setImage( const Image& img )
	{
		try {
			if( img.format() == IFormat::BAYER_RGGB_UINT8 ||
				img.format() == IFormat::BAYER_GBRG_UINT8 ||
				img.format() == IFormat::BAYER_GRBG_UINT8 ||
				img.format() == IFormat::YUYV_UINT8 ||
				img.format() == IFormat::UYVY_UINT8 ) {
				_img.reallocate( img.width(), img.height(), IFormat::RGBA_UINT8, IALLOCATOR_GL );
				img.convert( _img );
			} else {
				_img.reallocate( img.width(), img.height(), img.format(), IALLOCATOR_GL );
				_img = img;
			}
		} catch( Exception e ) {
			std::cerr << "Copy error: " << e.what() << std::endl;
		}
		update();
	}
}
