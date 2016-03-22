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


#include <cvt/gl/scene/GLSVisitor.h>

#include <cvt/gl/scene/GLSRenderableGroup.h>
#include <cvt/gl/scene/GLSBaseModel.h>

namespace cvt {

	void GLSRenderVisitor::visit( GLSRenderableGroup& group )
	{
		Matrix4f told = _shader.transformation();
		Matrix4f tnew = told * group.transformation();

		_shader.setTransformation( tnew );
		group.visitChildren( *this );
		_shader.setTransformation( told );
	}

	void GLSRenderVisitor::visit( GLSBaseModel& bmodel )
	{
		Matrix4f told = _shader.transformation();
		Matrix4f tnew = told * bmodel.transformation();

//		std::cout << _shader.transformation() << std::endl;

		_shader.setMaterial( bmodel.material() );
		_shader.bind();
		_shader.setTransformation( tnew, true );
        if( bmodel.mesh() )
		    bmodel.mesh()->draw();
		_shader.unbind();
		_shader.setTransformation( told );
	}

}
