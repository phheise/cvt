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

#ifndef CVT_XMLATTRIBUTE_H
#define CVT_XMLATTRIBUTE_H

#include <cvt/io/xml/XMLLeaf.h>

namespace cvt {
	/**
	  \ingroup XML
	*/
	class XMLAttribute : public XMLLeaf {
		public:
			XMLAttribute( const String& name, const String& value );
			XMLAttribute( const XMLAttribute& other );
			XMLAttribute& operator=( const XMLAttribute& other );
			~XMLAttribute();

			void xmlString( String& str ) const;
	};

	inline XMLAttribute::XMLAttribute( const String& name, const String& value ) : XMLLeaf( XML_NODE_ATTRIBUTE, name, value )
	{
	}

	inline XMLAttribute::XMLAttribute( const XMLAttribute& other ) : XMLLeaf( XML_NODE_ATTRIBUTE, other._name, other._value )
	{
	}

	inline XMLAttribute::~XMLAttribute()
	{
	}

	inline XMLAttribute& XMLAttribute::operator=( const XMLAttribute& other )
	{
		_name = other._name;
		_value = other._value;
		return *this;
	}

	inline void XMLAttribute::xmlString( String& str ) const
	{
		str = " ";
		str += _name;
		str += "=\"";
		str += _value;
		str += "\"";
	}
}

#endif
