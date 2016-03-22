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

#ifndef CVT_SCENECAMERA_H
#define CVT_SCENECAMERA_H

#include <cvt/geom/scene/SceneSpatial.h>

namespace cvt {
	class SceneCamera : public SceneSpatial {
		public:
			SceneCamera( const String& name );
			~SceneCamera();

			void lookAt( const Vector3f& eye, const Vector3f& center, const Vector3f& up );
			void setFrustum( float left, float right, float top, float bottom, float near, float far );
			void setPerspective( float fovy, float aspect, float near, float far );

			void getCamera( const Matrix4& mvp ) const;

		private:
			float	 _left, _right;
			float	 _top, _bottom;
			float	 _near, _far;
	};

	inline SceneCamera::SceneCamera( const String& name ) : SceneSpatial( name )
	{
	}

	inline SceneCamera::~SceneCamera()
	{
	}

	inline void SceneCamera::setFrustum( float left, float right, float top, float bottom, float near, float far )
	{
		_left = left;
		_right = right;
		_top = top;
		_bottom = bottom;
		_near = near;
		_far = far;
	}

	inline void SceneCamera::setPerspective( float fovy, float aspect, float near, float far )
	{
		float range = Math::tan( Math::deg2Rad( fovy / 2.0f ) ) * near;
		_left = -range * aspect;
		_right = range * aspect;
		_top = range;
		_bottom = -range;
		_near = near;
		_far = far;
	}

	inline void SceneCamera::lookAt( const Vector3f& eye, const Vector3f& center, const Vector3f& _up )
	{
		Vector3f f = center - eye;
		f.normalize();

		Vector3f up = _up;
		up.normalize();

		Vector3f s = f.cross( up );
		Vector3f up = s.cross( f );

		_transform[ 0 ].set(  s.x,  s.y,  s.z, -eye.x );
		_transform[ 1 ].set(  u.x,  u.y,  u.z, -eye.y );
		_transform[ 2 ].set( -f.x, -f.y, -f.z, -eye.z );
		_transform[ 3 ].set( 0.0f, 0.0f, 0.0f,  1.0f );
	}

	inline void SceneCamera::getCamera( Matrix4f& mat )
	{
		mat.setZero();
		mat[ 0 ][ 0 ] = ( 2.0f * _near ) / ( _right - _left );
		mat[ 0 ][ 2 ] = ( _right + _left ) / ( right - _left );
		mat[ 1 ][ 1 ] = 2.0f * _near / ( _top - _bottom );
		mat[ 1 ][ 2 ] = ( _top + _bottom ) / ( _top - _bottom );
		mat[ 2 ][ 2 ] = - ( _far + _near )  / ( _far - _near );
		mat[ 2 ][ 3 ] = - ( 2.0f * _far * _near ) / ( _far - _near );
		mat[ 3 ][ 2 ] = - 1.0f;

		mat = mat * _transform.inverse();
	}
}

#endif
