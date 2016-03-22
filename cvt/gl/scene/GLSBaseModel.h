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

#ifndef CVT_GLSBASEMODEL_H
#define CVT_GLSBASEMODEL_H

#include <cvt/gl/scene/GLSRenderable.h>
#include <cvt/gl/GLMesh.h>

namespace cvt {
	class GLSVistor;
	class GLSMaterial;

	class GLSBaseModel : public GLSRenderable {
		public:
			GLSBaseModel( GLMesh* mesh, const GLSMaterial* material = NULL );
			~GLSBaseModel();

			GLMesh*		       mesh() { return _mesh; }
			const GLSMaterial* material() { return _material; }
			bool		       visible() const { return _visible; }
			void		       setVisible( bool b ) { _visible = b; }

			void		       accept( GLSVisitor& visitor );
		private:
			bool		       _visible;
			GLMesh*		       _mesh;
			const GLSMaterial* _material;
	};


	inline GLSBaseModel::GLSBaseModel( GLMesh* mesh, const GLSMaterial* mat ) :
		_mesh( mesh ),
		_material( mat )
	{
	}

	inline GLSBaseModel::~GLSBaseModel()
	{
	}

}

#endif
