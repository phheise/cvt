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

#ifndef CVT_SCENETEXTURE_H
#define CVT_SCENETEXTURE_H

#include <cvt/geom/scene/SceneNode.h>
#include <cvt/gfx/Image.h>

namespace cvt {
	class SceneTexture : public SceneNode {
		public:
							SceneTexture( const String& name );
							~SceneTexture();

			void			load( const String& path );
			const Image&	image() const;

		private:
			Image _image;
	};

	inline SceneTexture::SceneTexture( const String& name ) : SceneNode( name )
	{
	}

	inline SceneTexture::~SceneTexture()
	{
	}

	inline void SceneTexture::load( const String& path )
	{
		try {
			_image.load( path );
		} catch( Exception& e ) {
			// mark missing texture with PINK
			_image.reallocate( 16, 16, IFormat::RGBA_UINT8 );
			_image.fill( Color::PINK );
		}
	}

	inline const Image& SceneTexture::image() const
	{
		return _image;
	}


}

#endif
