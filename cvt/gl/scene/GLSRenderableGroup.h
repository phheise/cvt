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

#ifndef CVT_GLSRENDERABLEGROUP_H
#define CVT_GLSRENDERABLEGROUP_H

#include <cvt/gl/scene/GLSRenderable.h>
#include <cvt/container/List.h>

namespace cvt {
	class GLSVisitor;

	class GLSRenderableGroup : public GLSRenderable {
		public:
			GLSRenderableGroup();
			~GLSRenderableGroup();

			void add( GLSRenderable* renderable );
			void remove( GLSRenderable* renderable );

			void accept( GLSVisitor& visitor );
			void visitChildren( GLSVisitor& visitor );
		private:
			List<GLSRenderable*> _renderables;
	};

	inline GLSRenderableGroup::GLSRenderableGroup()
	{
	}


	inline GLSRenderableGroup::~GLSRenderableGroup()
	{
	}

	inline void GLSRenderableGroup::add( GLSRenderable* renderable )
	{
		_renderables.append( renderable );
	}

	inline void GLSRenderableGroup::remove( GLSRenderable* renderable )
	{
		List<GLSRenderable*>::Iterator it;
		it = _renderables.find( renderable );
		_renderables.remove( it );
	}

}

#endif
