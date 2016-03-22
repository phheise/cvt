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

#ifndef CVT_GLSSHADER_H
#define CVT_GLSSHADER_H

#include <cvt/gl/scene/GLSCamera.h>

#include <cvt/util/Flags.h>

namespace cvt {
	enum GLSShaderMode {
		GLSSHADER_DEFAULT	   = 0,
		GLSSHADER_DEPTH        = 1,
		GLSSHADER_NORMAL	   = 2,
		GLSSHADER_DEPTH_NORMAL = 3
	};

    class GLSMaterial;
    class GLSShaderProgram;
    class GLScene;

	class GLSShader {
		friend class GLScene;
		public:
			GLSShader( const GLScene& scene );
			~GLSShader();

			void setCamera( const GLSCamera& camera );
			void setTransformation( const Matrix4f& mat, bool setuniform = false );
			const Matrix4f& transformation() const { return _transformation; }
			void setMaterial( const GLSMaterial* mat );

			void setMode( GLSShaderMode mode );
			GLSShaderMode mode() const;

			void bind();
			void unbind();

		private:
            void generatePrograms();

            const GLScene&      _scene;
			GLSShaderMode       _mode;
			const GLSMaterial*  _mat;
			Matrix4f            _proj;
			Matrix4f            _transformation;
            GLSMaterial*        _defaultmat;
            GLSShaderProgram*   _progs[ 16 ];
	};



	inline void GLSShader::setCamera( const GLSCamera& camera )
	{
		_proj = camera.projection();
		_transformation = camera.transformation().inverse();
	}

	inline void GLSShader::setMode( GLSShaderMode mode )
	{
		_mode = mode;
	}

	inline GLSShaderMode GLSShader::mode() const
	{
		return _mode;
	}

}

#endif
