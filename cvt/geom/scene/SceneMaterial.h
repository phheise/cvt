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

#ifndef CVT_SCENEMATERIAL_H
#define CVT_SCENEMATERIAL_H

#include <iostream>
#include <cvt/util/Flags.h>
#include <cvt/gfx/Color.h>
#include <cvt/geom/scene/SceneNode.h>

namespace cvt {
	enum SceneMaterialFeatures {
		SCENEMATERIAL_EMIT = ( 1 << 0 ),
		SCENEMATERIAL_AMBIENT = ( 1 << 1 ),
		SCENEMATERIAL_DIFFUSE = ( 1 << 2 ),
		SCENEMATERIAL_SPECULAR = ( 1 << 3 ),
		SCENEMATERIAL_DIFFUSEMAP = ( 1 << 4 ),
		SCENEMATERIAL_AMBIENTMAP = ( 1 << 5 ),
		SCENEMATERIAL_SPECULARMAP = ( 1 << 6 ),
		SCENEMATERIAL_ALPHAMAP = ( 1 << 7 ), // binary map resp alpha map
		SCENEMATERIAL_NORMALMAP = ( 1 << 8 ),
		SCENEMATERIAL_REFLECTIVE = ( 1 << 9 ),
		SCENEMATERIAL_TRANSMISSON = ( 1 << 10 )
	};

	CVT_ENUM_TO_FLAGS( SceneMaterialFeatures, SceneMaterialFlags )

	class SceneMaterial;
	std::ostream& operator<<( std::ostream& out, const SceneMaterial& mat );

	class SceneMaterial : public SceneNode {
		friend std::ostream& operator<<( std::ostream& out, const SceneMaterial& mat );

		public:
							SceneMaterial( const String& name );
							~SceneMaterial();

			void			setEmitColor( const Color& c );
			void			setAmbientColor( const Color& c );
			void			setDiffuseColor( const Color& c );
			void			setSpecularColor( const Color& c );
			void			setReflectiveColor( const Color& c );
			void			setTransmissonColor( const Color& c );
			void			setShininess( float s );
			void			setReflectivity( float r );
			void			setTransparency( float t );
			void			setRefraction( float r );
			void			setAmbientMap( const String& name );
			void			setDiffuseMap( const String& name );
			void			setSpecularMap( const String& name );
			void			setAlphaMap( const String& name );
			void			setNormalMap( const String& name );

			const Color&	emitColor() const;
			const Color&	ambientColor() const;
			const Color&	diffuseColor() const;
			const Color&	specularColor() const;
			const Color&	reflectiveColor() const;
			const Color&	transmissonColor() const;
			float			shininess() const;
			float			reflectivity() const;
			float			transparency() const;
			float			refraction() const;
			const String&	ambientMap() const;
			const String&	diffuseMap() const;
			const String&   specularMap() const;
			const String&	alphaMap() const;
			const String&	normalMap() const;

			SceneMaterialFlags flags() const;
			SceneMaterialFlags& flags();

		private:
			Color	_emit;
			Color	_ambient;
			Color	_diffuse;
			Color	_specular;
			Color	_reflective;
			Color	_transmisson;
			float	_shininess;
			float	_reflectivity;
			float	_transparency;
			float	_refraction;

			String	_ambientmap;
			String	_diffusemap;
			String	_specularmap;
			String	_alphamap;
			String  _normalmap;

			SceneMaterialFlags _flags;
	};

	inline SceneMaterial::SceneMaterial( const String& name ) : SceneNode( name ),
		_shininess( 1.0f ),
		_reflectivity( 0.0f ),
		_transparency( 1.0f ),
		_refraction( 1.5f )
	{
	}

	inline SceneMaterial::~SceneMaterial()
	{
	}

	inline SceneMaterialFlags SceneMaterial::flags() const
	{
		return _flags;
	}

	inline SceneMaterialFlags& SceneMaterial::flags()
	{
		return _flags;
	}


	inline void SceneMaterial::setEmitColor( const Color& c )
	{
		_emit = c;
		if( _emit != Color::BLACK )
			_flags |= SCENEMATERIAL_EMIT;
	}

	inline void SceneMaterial::setAmbientColor( const Color& c )
	{
		_ambient = c;
		_flags |= SCENEMATERIAL_AMBIENT;
	}

	inline void SceneMaterial::setDiffuseColor( const Color& c )
	{
		_diffuse = c;
		_flags |= SCENEMATERIAL_DIFFUSE;
	}

	inline void SceneMaterial::setSpecularColor( const Color& c )
	{
		_specular = c;
		_flags |= SCENEMATERIAL_SPECULAR;
	}

	inline void SceneMaterial::setReflectiveColor( const Color& c )
	{
		_reflective = c;
		_flags |= SCENEMATERIAL_REFLECTIVE;
	}

	inline void SceneMaterial::setTransmissonColor( const Color& c )
	{
		_transmisson = c;
		_flags |= SCENEMATERIAL_TRANSMISSON;
	}

	inline void SceneMaterial::setShininess( float s )
	{
		_shininess = s;
	}

	inline void SceneMaterial::setReflectivity( float r )
	{
		_reflectivity = r;
	}

	inline void SceneMaterial::setTransparency( float t )
	{
		_transparency = t;
	}

	inline void SceneMaterial::setRefraction( float r )
	{
		_refraction = r;
	}

	inline void SceneMaterial::setAmbientMap( const String& name )
	{
		_ambientmap = name;
		_flags |= SCENEMATERIAL_AMBIENTMAP;
	}

	inline void SceneMaterial::setDiffuseMap( const String& name )
	{
		_diffusemap = name;
		_flags |= SCENEMATERIAL_DIFFUSEMAP;
	}

	inline void SceneMaterial::setSpecularMap( const String& name )
	{
		_specularmap = name;
		_flags |= SCENEMATERIAL_SPECULARMAP;
	}

	inline void SceneMaterial::setAlphaMap( const String& name )
	{
		_alphamap = name;
		_flags |= SCENEMATERIAL_ALPHAMAP;
	}

	inline void SceneMaterial::setNormalMap( const String& name )
	{
		_normalmap = name;
		_flags |= SCENEMATERIAL_NORMALMAP;
	}

	inline const Color&	SceneMaterial::emitColor() const
	{
		return _emit;
	}

	inline const Color&	SceneMaterial::ambientColor() const
	{
		return _ambient;
	}

	inline const Color&	SceneMaterial::diffuseColor() const
	{
		return _diffuse;
	}

	inline const Color&	SceneMaterial::specularColor() const
	{
		return _specular;
	}

	inline const Color&	SceneMaterial::reflectiveColor() const
	{
		return _reflective;
	}

	inline const Color&	SceneMaterial::transmissonColor() const
	{
		return _transmisson;
	}

	inline float SceneMaterial::shininess() const
	{
		return _shininess;
	}

	inline float SceneMaterial::reflectivity() const
	{
		return _reflectivity;
	}

	inline float SceneMaterial::transparency() const
	{
		return _transparency;
	}

	inline float SceneMaterial::refraction() const
	{
		return _refraction;
	}

	inline const String& SceneMaterial::ambientMap() const
	{
		return _ambientmap;
	}

	inline const String& SceneMaterial::diffuseMap() const
	{
		return _diffusemap;
	}

	inline const String& SceneMaterial::specularMap() const
	{
		return _specularmap;
	}

	inline const String& SceneMaterial::alphaMap() const
	{
		return _alphamap;
	}

	inline const String& SceneMaterial::normalMap() const
	{
		return _normalmap;
	}

	inline std::ostream& operator<<( std::ostream& out, const SceneMaterial& mat )
	{
		out << "\nMaterial: " << mat.name() << "\n";
		out	<< "\tTransparency : " << mat._transparency;
		out << "\n\tAmbient: " << mat._ambient;
		out << "\n\tDiffuse: " << mat._diffuse;
		out << "\n\tSpecular: " << mat._specular;
		out << "\n\tShininess: " << mat._shininess;
		out << "\n\tEmit: " << mat._emit;
		out << "\n\tTransmisson: " << mat._transmisson;
		out << "\n\tRefraction: " << mat._refraction;
		out << "\n\tReflectife: " << mat._reflective;
		out << "\n\tReflectifity: " << mat._reflectivity;
		out << "\n\tAmbient-Map: " << mat._ambientmap;
		out << "\n\tDiffuse-Map: " << mat._diffusemap;
		out << "\n\tSpecular-Map: " << mat._specularmap;
		out << "\n\tAlpha-Map: " << mat._alphamap;
		out << "\n\tNormal-Map: " << mat._normalmap;
		return out;
	}
}

#endif
