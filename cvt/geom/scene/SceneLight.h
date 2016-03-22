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

#ifndef CVT_SCENELIGHT_H
#define CVT_SCENELIGHT_H

namespace cvt {
	enum SceneLightType {
		SCENELIGHT_AMBIENT,
		SCENELIGHT_DIRECTIONAL,
		SCENELIGHT_POINT,
		SCENELIGHT_SPOT
	};

	class SceneLight : public SceneSpatial {
		public:
			SceneLight( const String& name, SceneLightType type );
			~SceneLight();

			SceneLightType type() const;

			const Color& ambientColor() const;
			const Color& diffuseColor() const;
			const Color& specularColor() const;

		private:
			SceneLightType _type;
			Color		   _ambient;
			Color		   _diffuse;
			Color		   _specular;
			float		   _intensity;
			float		   _constant;
			float		   _linear;
			float		   _quadratic;
			bool		   _attenuate;
			bool		   _active;

			// for spotlights
			float		   _exponent;
			float		   _angle;
	};

	inline SceneLight::SceneLight( const String& name, SceneLightType type ) : SceneSpatial( name ), _type( type )
	{
	}

	inline SceneLight::~SceneLight()
	{
	}
}

#endif
