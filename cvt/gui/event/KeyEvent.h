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

#ifndef CVT_KEYEVENT_H
#define CVT_KEYEVENT_H

#include <cvt/gui/event/Event.h>
#include <cvt/util/Flags.h>
#include <cvt/gui/KeyCodes.h>

namespace cvt {

	enum KeyModifierFlags {
		NO_MODIFIER = ( 0 << 0 ),
		SHIFT_MODIFIER = ( 1 << 0 ),
		SHIFTLOCK_MODIFIER = ( 1 << 1 ),
		CTRL_MODIFIER  = ( 1 << 2 ),
		ALT_MODIFIER   = ( 1 << 3 ),
		META_MODIFIER  = ( 1 << 4 ),
	};

	CVT_ENUM_TO_FLAGS( KeyModifierFlags, KeyModifier)

	class KeyEvent : public Event
	{
		public:
							 KeyEvent( KeyCode keycode = KEY_Unknown, KeyModifier modifier = NO_MODIFIER, const String& text = "" ) : Event( EVENT_KEY ), _key( keycode ), _modifier( modifier ), _text( text ) {}
							 KeyEvent( const KeyEvent& kev ) : Event( EVENT_KEY ), _key( kev._key ), _modifier( kev._modifier), _text( kev._text ) {}

			KeyCode			 key() const { return _key; }
			KeyModifier		 modifiers() const { return _modifier; }
			const String&	 text() const { return _text; }

		private:
			KeyCode		_key;
			KeyModifier	_modifier;
			String		_text;
	};

    static inline std::ostream& operator<<( std::ostream& out, const KeyEvent& k )
    {
		KeyModifier mods = k.modifiers();

		out << "KeyEvent" << std::endl;
		out << "\tModifier: ";
		if( mods & SHIFT_MODIFIER )
			out << " Shift";
		if( mods & SHIFTLOCK_MODIFIER)
			out << " ShiftLock";
		if( mods & CTRL_MODIFIER )
			out << " Control";
		if( mods & ALT_MODIFIER )
			out << " Alt";
		if( mods & META_MODIFIER )
			out << " Meta";
		out << std::endl;

		out << "\tCode: " << std::hex << "0x" << ( unsigned int ) k.key() << std::endl;
		out << "\tText: " << "'" <<k.text() << "'";
        return out;
    }

};

#endif
