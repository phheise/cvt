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

#ifndef CVT_XMLNODE_H
#define CVT_XMLNODE_H

#include <cvt/util/String.h>
#include <cvt/util/CVTAssert.h>
#include <iostream>

namespace cvt {

	/**
	  \ingroup XML
	*/

	enum XMLNodeType {
        XML_NODE_DECL,
		XML_NODE_ELEMENT,
		XML_NODE_TEXT,
		XML_NODE_CDATA,
		XML_NODE_ATTRIBUTE,
		XML_NODE_COMMENT
	};

	class XMLElement;

	class XMLNode {
		friend class XMLElement;
		friend std::ostream& operator<<( std::ostream &out, const XMLNode& str );

		public:
			virtual ~XMLNode();

//			virtual XMLNode* clone() = 0;

			const String& name() const;
			void setName( const String& name );

			const String& value() const;
			void setValue( const String& value );

			XMLNode* parent() const;

			XMLNodeType type() const;

			virtual size_t childSize() const = 0;
			virtual XMLNode* child( size_t index ) = 0;
			virtual const XMLNode* child( size_t index ) const = 0;
			virtual void addChild( XMLNode* node ) = 0;
			virtual void removeChild( XMLNode* node ) = 0;
			virtual void removeChild( size_t index ) = 0;
			virtual XMLNode* childByName( const String& name ) = 0;
			virtual void xmlString( String& str ) const = 0;

		protected:
			XMLNode( XMLNodeType type, const String& name = "", const String value = "" ) : _name( name ), _value( value ), _type( type ), _parent( NULL )
			{}

			void print( std::ostream &out, size_t d ) const;

			String				 _name;
			String				 _value;
			XMLNodeType			 _type;
			XMLNode*			 _parent;

		private:
			XMLNode( const XMLNode& other );
			XMLNode& operator=( const XMLNode& other );
	};

	inline XMLNode::~XMLNode()
	{
	}

	inline XMLNode* XMLNode::parent() const
	{
		return _parent;
	}

	inline const String& XMLNode::name() const
	{
		return _name;
	}

	inline void XMLNode::setName( const String& name )
	{
		_name = name;
	}

	inline const String& XMLNode::value() const
	{
		return _value;
	}

	inline void XMLNode::setValue( const String& value )
	{
		_value = value;
	}

	inline XMLNodeType XMLNode::type() const
	{
		return _type;
	}

	inline void XMLNode::print( std::ostream &out, size_t d = 0 ) const
	{
		for( size_t i = 0; i < d; i++ )
			out << "\t";
		out << "Name: " << _name << " Value: " << _value << std::endl;
		if( childSize() ) {
			for( size_t i = 0; i < d; i++ )
				out << "\t";
			out << "Children:\n";
			for( size_t i = 0, end = childSize(); i < end; i++ ) {
				child( i )->print( out, d + 1 );
			}
		}
	}

	inline std::ostream& operator<<( std::ostream &out, const XMLNode& xn )
	{
		xn.print( out, 0 );
		return out;
	}

}

#endif
