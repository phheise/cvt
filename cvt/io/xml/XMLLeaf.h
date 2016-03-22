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

#ifndef CVT_XMLLEAF_H
#define CVT_XMLLEAF_H

#include <cvt/io/xml/XMLNode.h>

namespace cvt {

	/**
	  \ingroup XML
	*/
	class XMLLeaf : public XMLNode
	{
		public:
			virtual ~XMLLeaf() {};

		protected:
			XMLLeaf( XMLNodeType type, const String& name = "", const String value = "" ) : XMLNode( type, name, value )
			{}

			virtual size_t childSize() const;
			virtual XMLNode* child( size_t index );
			virtual const XMLNode* child( size_t index ) const;
			virtual void addChild( XMLNode* node );
			virtual void removeChild( XMLNode* node );
			virtual void removeChild( size_t index );
			virtual XMLNode* childByName( const String& name );
			virtual void xmlString( String& str ) const = 0;

		private:
			XMLLeaf( const XMLLeaf& );
	};

	inline size_t XMLLeaf::childSize() const
	{
		return 0;
	}

	inline XMLNode* XMLLeaf::child( size_t )
	{
		return NULL;
	}

	inline const XMLNode* XMLLeaf::child( size_t ) const
	{
		return NULL;
	}


	inline void XMLLeaf::addChild( XMLNode* )
	{
	}
			
	inline void XMLLeaf::removeChild( XMLNode* )
	{
	}

	inline void XMLLeaf::removeChild( size_t )
	{
	}

	inline XMLNode* XMLLeaf::childByName( const String& )
	{
		return NULL;
	}

}

#endif
