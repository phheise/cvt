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

/*
 * File:   XMLDecl.h
 * Author: sebi
 *
 * Created on August 8, 2011, 2:50 PM
 */

#ifndef CVT_XMLDECL_H
#define	CVT_XMLDECL_H

#include <cvt/io/xml/XMLEncoding.h>
#include <cvt/io/xml/XMLLeaf.h>

namespace cvt
{

	/**
	  \ingroup XML
	*/
    class XMLDecl : public XMLLeaf
    {
      public:
        XMLDecl( XMLEncodingType encoding );

        void xmlString( String& str ) const;

      private:
        XMLEncodingType     _encoding;

    };

    inline XMLDecl::XMLDecl( XMLEncodingType encoding ) :
        XMLLeaf( XML_NODE_DECL, "xml" ),
        _encoding( encoding )
    {
    }

    inline void XMLDecl::xmlString( String& str ) const
    {
        str = "<?xml version=\"1.0\" encoding=\"";
        switch( _encoding ){
            case XMLENCODING_UTF8:
                str += "UTF-8";
            break;
        }
        str += "\"?>\n";
    }
}

#endif

