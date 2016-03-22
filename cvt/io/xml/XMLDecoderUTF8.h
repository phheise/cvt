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

#ifndef CVT_XMLDECODERUTF8_H
#define CVT_XMLDECODERUTF8_H

#include <cvt/io/xml/XMLDecoder.h>
#include <cvt/util/String.h>
#include <cvt/util/Util.h>
#include <cvt/io/xml/XMLText.h>
#include <iostream>

namespace cvt {

	/**
	  \ingroup XML
	*/
	class XMLDecoderUTF8 : public XMLDecoder
	{
		public:
			XMLDecoderUTF8();
			~XMLDecoderUTF8();

		protected:
			void setData( const void* data, size_t len );
			XMLNode* parseDeclaration();
			XMLNode* parseNode();

		private:
			inline void skipWhitespace();
			inline bool parseName( String& name );
			inline bool parseAttributeValue( String& value );
			XMLNode* parseAttribute();
			XMLNode* parseComment();
			XMLNode* parseText();
			XMLNode* parseCData();
			XMLNode* parsePI();
			XMLNode* parseElement();

			bool match( const char* str );
			bool match( const char* ptr, const char* str );
			bool match( uint8_t val );
			void advance( size_t n );

			const uint8_t* _base;
			size_t   _len;
			const uint8_t* _stream;
			size_t	 _rlen;
			const uint8_t* _end;

			enum State {
				STATE_OK,
				STATE_EOF,
			};
			State _state;
	};

	inline XMLDecoderUTF8::XMLDecoderUTF8()
	{
	}

	inline XMLDecoderUTF8::~XMLDecoderUTF8()
	{
	}

	inline void XMLDecoderUTF8::setData( const void* data, size_t len )
	{
		_base = _stream = ( const uint8_t* ) data;
		_len = _rlen = len;
		_end = _stream + len;
		_state = STATE_OK;
	}

	inline void XMLDecoderUTF8::skipWhitespace()
	{
		while( ( *_stream == 0x20 ||
			   *_stream == 0x09 ||
			   *_stream == 0x0A ||
			   *_stream == 0x0D ) && _rlen ) {
			_stream++;
			_rlen--;
		}
	}

	inline bool XMLDecoderUTF8::match( const char* str )
	{
		const char* ptr = ( const char* ) _stream;
		size_t len = _rlen;

		while( *ptr == *str && len && *str ) {
			len--;
			str++;
			ptr++;
		}
		return *str == '\0';
	}

	inline bool XMLDecoderUTF8::match( const char* ptr, const char* str )
	{
		size_t len = _rlen;

		while( *ptr == *str && len && *str && *ptr ) {
			len--;
			str++;
			ptr++;
		}
		return *str == '\0';
	}

	inline bool XMLDecoderUTF8::match( uint8_t val )
	{
		return  *_stream == val;
	}

	inline void XMLDecoderUTF8::advance( size_t n )
	{
		_stream += n;
		if( _rlen < n )
			throw CVTException( "Premature end of file" );
		_rlen -= n;
	}

	inline bool XMLDecoderUTF8::parseName( String& name )
	{
		size_t n = 0;
		const uint8_t* ptr;
		uint8_t* end = NULL;
		uint32_t u32;

		/* FIXME: this is vulnerable to buffer overrun in case of malformed or malicious UTF8 sequences
		   -> This only works in case of an zero terminated buffer
		 */

		ptr = _stream;
		if( *ptr == ':' || *ptr == '_' ||
			   ( *ptr >= 'A' && *ptr <= 'Z' ) ||
			   ( *ptr >= 'a' && *ptr <= 'z' ) ) {
			ptr++;
			n++;
		} else if( *ptr >= 0x80 ) {
			u32 = Util::UTF8toUTF32( ( char* ) ptr, ( char** ) end );
			if( ( u32 >= 0xC0 && u32 <= 0xD6 ) ||
				( u32 >= 0xD8 && u32 <= 0xF6 ) ||
				( u32 >= 0xF8 && u32 <= 0x2FF ) ||
				( u32 >= 0x370 && u32 <= 0x37D ) ||
				( u32 >= 0x37F && u32 <= 0x1FFF ) ||
				( u32 >= 0x200C && u32 <= 0x200D ) ||
				( u32 >= 0x2070 && u32 <= 0x218F ) ||
				( u32 >= 0x2C00 && u32 <= 0x2FEF ) ||
				( u32 >= 0x3001 && u32 <= 0xD7FF ) ||
				( u32 >= 0xF900 && u32 <= 0xFDCF ) ||
				( u32 >= 0x10000 && u32 <= 0xEFFFF ) ) {
				size_t nn = end - ptr;
				ptr += nn;
				n += nn;
			}
		} else
			return false;

		while( 1 ) {
			if( *ptr == ':' || *ptr == '_' ||
			    *ptr == '-' || *ptr == '.' ||
			   ( *ptr >= '0' && *ptr <= '9' ) ||
			   ( *ptr >= 'A' && *ptr <= 'Z' ) ||
			   ( *ptr >= 'a' && *ptr <= 'z' ) ) {
				n++;
				ptr++;
			} else if( *ptr >= 0x80 ) {
				u32 = Util::UTF8toUTF32( ( char* ) ptr, ( char** ) end );
				if(( u32 == 0xB7 ) ||
				   ( u32 >= 0xC0 && u32 <= 0xD6 ) ||
				   ( u32 >= 0xD8 && u32 <= 0xF6 ) ||
				   ( u32 >= 0xF8 && u32 <= 0x36F ) ||
				   ( u32 >= 0x370 && u32 <= 0x37D ) ||
				   ( u32 >= 0x37F && u32 <= 0x1FFF ) ||
				   ( u32 >= 0x200C && u32 <= 0x200D ) ||
				   ( u32 >= 0x203F && u32 <= 0x2040 ) ||
				   ( u32 >= 0x2070 && u32 <= 0x218F ) ||
				   ( u32 >= 0x2C00 && u32 <= 0x2FEF ) ||
				   ( u32 >= 0x3001 && u32 <= 0xD7FF ) ||
				   ( u32 >= 0xF900 && u32 <= 0xFDCF ) ||
				   ( u32 >= 0x10000 && u32 <= 0xEFFFF ) ) {
					size_t nn = end - ptr;
					ptr += nn;
					n += nn;
				}
			} else
				break;
		}
		name.assign( ( const char* ) _stream, n );
		advance( n );
		return true;
	}

	inline bool XMLDecoderUTF8::parseAttributeValue( String& value )
	{
		size_t n = 0;
		const uint8_t* ptr;

		if( match('"')) {
			advance( 1 );
			ptr = _stream;
			while( *ptr != '"' && *ptr != '<' && *ptr != 0 ) {
				n++;
				ptr++;
			}
			if( *ptr == '<' || *ptr == 0 )
				throw CVTException( "Malformed attribute value" );
			value.assign( ( const char* )  _stream, n );
			advance( n );
			if( !match('"') )
				throw CVTException( "Malformed attribute value, expected '\"'" );
			advance( 1 );
		} else if( match('\'') ) {
			advance( 1 );
			ptr = _stream;
			while( *ptr != '\'' && *ptr != '<' && *ptr != 0 ) {
				n++;
				ptr++;
			}
			if( *ptr == '<' || *ptr == 0 )
				throw CVTException( "Malformed attribute value" );
			value.assign( ( const char* ) _stream, n );
			advance( n );
			if( !match('\'') )
				throw CVTException( "Malformed attribute value, expected \"\'\"" );
			advance( 1 );
		} else
			return false;
		return true;
	}

	inline XMLNode* XMLDecoderUTF8::parseText()
	{
		String text;
		size_t n = 0;
		const uint8_t* ptr;

		ptr = _stream;
		while( *ptr != '<' && *ptr != 0 ) {
			if( *ptr == '[' ) {
				if( match( ( const char* ) ptr, "[[>") )
					throw CVTException( "CDATA-section-close delimiter not allowed in text" );
			}
			n++;
			ptr++;
		}
		if( *ptr == 0 )
			throw CVTException( "Premature end of file" );
		text.assign( ( const char* )  _stream, n );
		advance( n );
		return new XMLText( text );
	}
}

#endif
