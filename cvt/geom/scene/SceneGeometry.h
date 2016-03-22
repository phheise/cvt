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

#ifndef CVT_SCENEGEOMETRY_H
#define CVT_SCENEGEOMETRY_H


#include <cvt/geom/scene/SceneSpatial.h>
#include <cvt/util/String.h>
#include <cvt/geom/Box.h>

namespace cvt {

	class SceneMesh;

	enum SceneGeometryType {
		SCENEGEOMETRY_MESH,
		SCENEGEOMETRY_POINTS
	};

	class SceneGeometry : public SceneSpatial
	{
		public:
								SceneGeometry( const String& name, SceneGeometryType type );
			virtual				~SceneGeometry();

			SceneGeometryType	type() const;

			virtual Boxf		boundingBox() const = 0;
			const String&		material() const;
			void				setMaterial( const String& name );

		private:
			SceneGeometryType	_type;
			String				_material;
	};

	inline SceneGeometry::SceneGeometry( const String& name, SceneGeometryType type ) :
		SceneSpatial( name ),
		_type( type )
	{
	}

	inline SceneGeometry::~SceneGeometry()
	{
	}

	inline const String& SceneGeometry::material() const
	{
		return _material;
	}

	inline void	SceneGeometry::setMaterial( const String& name )
	{
		_material = name;
	}


	inline SceneGeometryType SceneGeometry::type() const
	{
		return _type;
	}

	std::ostream& operator<<( std::ostream& out, const SceneGeometry& g );
}

#endif
