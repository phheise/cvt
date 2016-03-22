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

#ifndef CVT_LABEL_H
#define CVT_LABEL_H

#include <cvt/gui/Widget.h>
#include <cvt/util/Signal.h>
#include <cvt/util/String.h>
#include <cvt/gfx/Alignment.h>

namespace cvt {
	class Label : public Widget
	{
		public:
			Label( const String& label, Alignment align = ALIGN_LEFT | ALIGN_VCENTER );
			~Label();
			const String& label() const;
			void setLabel( const String& label );
			Alignment labelAlignment() const;
			void setLabelAlignment( Alignment a );
			const Color& color() const;
			Color& color();

		private:
			void paintEvent( PaintEvent& e, GFX& g );
			Label( const Label& b );

			String _label;
			Alignment _aligment;
			Color _color;
	};

	inline const String& Label::label() const
	{
		return _label;
	}

	inline void Label::setLabel( const String& label )
	{
		_label = label;
		update();
	}

	inline Alignment Label::labelAlignment() const
	{
		return _aligment;
	}

	inline void Label::setLabelAlignment( Alignment a )
	{
		_aligment = a;
		update();
	}

	inline const Color& Label::color() const
	{
		return _color;
	}

	inline Color& Label::color()
	{
		return _color;
	}

}

#endif
