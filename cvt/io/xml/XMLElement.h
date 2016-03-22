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

#ifndef CVT_XMLELEMENT_H
#define CVT_XMLELEMENT_H

#include <cvt/io/xml/XMLNode.h>
#include <cvt/util/CVTAssert.h>
#include <vector>

namespace cvt {

	/**
	  \ingroup XML
	*/
	class XMLElement : public XMLNode
	{
		public:
			XMLElement( const String& name );
			~XMLElement();

			size_t childSize() const;
			XMLNode* child( size_t index );
			const XMLNode* child( size_t index ) const;
			void addChild( XMLNode* node );
			XMLNode* childByName( const String& name );
			void xmlString( String& str ) const;

			virtual void removeChild( XMLNode* node );
			virtual void removeChild( size_t index );
		private:
			std::vector<XMLNode*> _children;
	};

	inline XMLElement::XMLElement( const String& name ) : XMLNode( XML_NODE_ELEMENT, name )
	{
	}

	inline XMLElement::~XMLElement()
	{
		for( int i = 0, end = _children.size(); i < end; i++ ) {
			delete _children[ i ];
		}
		_children.clear();
	}

	inline size_t XMLElement::childSize() const
	{
		return _children.size();
	}

	inline XMLNode* XMLElement::child( size_t index )
	{
		CVT_ASSERT( index < _children.size(), "Out of bounds!" );
		return _children[ index ];
	}

	inline const XMLNode* XMLElement::child( size_t index ) const
	{
		CVT_ASSERT( index < _children.size(), "Out of bounds!" );
		return _children[ index ];
	}

	inline void XMLElement::addChild( XMLNode* node )
	{
		if( !node )
			return;
		node->_parent = this;
		_children.push_back( node );
	}

	inline void XMLElement::removeChild( XMLNode* node )
	{
		std::vector<XMLNode*>::iterator it = _children.begin();
		const std::vector<XMLNode*>::const_iterator itEnd = _children.end();

		while( it != itEnd ){

			if( *it == node ){
				_children.erase( it );
				delete node;
				break;		
			}
			++it;
		}
	}
	
	inline void XMLElement::removeChild( size_t index )
	{
		if( index < _children.size() ){
			delete _children[ index ];
			_children.erase( _children.begin() + index );
		}
	}

	inline XMLNode* XMLElement::childByName( const String& name )
	{
		for( int i = 0, end = _children.size(); i < end; i++ ) {
			if( _children[ i ]->name() == name )
				return _children[ i ];
		}
		return NULL;
	}


	inline void XMLElement::xmlString( String& str ) const
	{
		String strchild;
		bool nochild = true;

		str = "<";
		str += _name;
		for( int i = 0, end = _children.size(); i < end; i++ ) {
			if( _children[ i ]->type() == XML_NODE_ATTRIBUTE ) {
				_children[ i ]->xmlString( strchild );
				str += strchild;
			} else
				nochild = false;
		}
		if( nochild ) {
			str +="/>\n";
			return;
		}
		str += ">\n";
		for( int i = 0, end = _children.size(); i < end; i++ ) {
			if( _children[ i ]->type() == XML_NODE_ATTRIBUTE )
				continue;
			_children[ i ]->xmlString( strchild );
			str += strchild;
		}
		str += "</";
		str += _name;
		str += ">\n";
	}
}

#endif
