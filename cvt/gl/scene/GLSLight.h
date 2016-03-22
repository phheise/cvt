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

#ifndef CVT_GLSLIGHT_H
#define CVT_GLSLIGHT_H

#include <cvt/gl/scene/GLSTransformable.h>
#include <cvt/math/Vector.h>
#include <cvt/gfx/Color.h>

namespace cvt {
	enum GLSLightType {
		GLSLIGHT_POINT,
		GLSLIGHT_SPOT
	};

	class GLSLight : public GLSTransformable
	{
		public:
			GLSLight( GLSLightType ltype = GLSLIGHT_POINT );
			~GLSLight();

			GLSLightType	 lightType() const;

			void			 setColor( const Color& c );

			void			 setAmbientColor( const Color& c );
			void			 setDiffuseColor( const Color& c );
			void			 setSpecularColor( const Color& c );

			const Color&	 ambientColor() const;
			const Color&	 diffuseColor() const;
			const Color&	 specularColor() const;

			bool			 active() const { return _active; }
			void			 setActive( bool b ) { _active = b; }

		private:
			GLSLightType _ltype;
			Color		 _La;
			Color		 _Ld;
			Color		 _Ls;
			bool		 _active;
//			GLTexture*	 _shadowmap;
	};

	inline GLSLight::GLSLight( GLSLightType ltype ) :  _ltype( ltype )
	{
        setColor( Color::WHITE );
	}

	inline GLSLight::~GLSLight()
	{
//		if( _shadowmap )
//			delete _shadowmap;
	}

	inline GLSLightType GLSLight::lightType() const
	{
		return _ltype;
	}


	inline const Color& GLSLight::ambientColor() const
	{
		return _La;
	}

	inline const Color& GLSLight::diffuseColor() const
	{
		return _Ld;
	}

    inline const Color& GLSLight::specularColor() const
	{
		return _Ls;
	}

	inline void GLSLight::setColor( const Color& c )
	{
		setAmbientColor( Color::BLACK );
        setDiffuseColor( c );
        setSpecularColor( c );
	}

	inline void GLSLight::setAmbientColor( const Color& c )
	{
		_La = c;
	}

	inline void GLSLight::setDiffuseColor( const Color& c )
	{
		_Ld = c;
	}

	inline void GLSLight::setSpecularColor( const Color& c )
	{
		_Ls = c;
	}

};

#endif
