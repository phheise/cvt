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

#ifndef CVT_OSXKEYMAP_H
#define CVT_OSXKEYMAP_H

#import <Cocoa/Cocoa.h>

namespace cvt
{
	class OSXKeyMap
	{
		public:
			static void mapToKeyEvent( KeyEvent& kev, NSEvent* e );
		private:
			static KeyCode	   OSXKeyCodeToKeyCode( NSEvent* e, const String& str );
			static KeyModifier OSXModifierToKeyModifier( NSEvent* e );

			OSXKeyMap();
			~OSXKeyMap();
			OSXKeyMap( const OSXKeyMap& );
	};

	inline void OSXKeyMap::mapToKeyEvent( KeyEvent& kev, NSEvent* e )
	{
		String text( [ [ e characters ] UTF8String ] );
		kev = KeyEvent( OSXKeyCodeToKeyCode( e, text ), OSXModifierToKeyModifier( e ), text );
	}

	inline KeyCode OSXKeyMap::OSXKeyCodeToKeyCode( NSEvent* e, const String& str )
	{
		static const KeyCode keymap[ 0x7f ] = {
			KEY_A,				/* kVK_ANSI_A                    = 0x00 */
			KEY_S,				/* kVK_ANSI_S                    = 0x01 */
			KEY_D,				/* kVK_ANSI_D                    = 0x02 */
			KEY_F,				/* kVK_ANSI_F                    = 0x03 */
			KEY_H,				/* kVK_ANSI_H                    = 0x04 */
			KEY_G,				/* kVK_ANSI_G                    = 0x05 */
			KEY_Z,				/* kVK_ANSI_Z                    = 0x06 */
			KEY_X,				/* kVK_ANSI_X                    = 0x07 */
			KEY_C,				/* kVK_ANSI_C                    = 0x08 */
			KEY_V,				/* kVK_ANSI_V                    = 0x09 */
			KEY_Unknown,		/*							     = 0x0A */
			KEY_B,				/* kVK_ANSI_B                    = 0x0B */
			KEY_Q,				/* kVK_ANSI_Q                    = 0x0C */
			KEY_W,				/* kVK_ANSI_W                    = 0x0D */
			KEY_E,				/* kVK_ANSI_E                    = 0x0E */
			KEY_R,				/* kVK_ANSI_R                    = 0x0F */
			KEY_Y,				/* kVK_ANSI_Y                    = 0x10 */
			KEY_T,				/* kVK_ANSI_T                    = 0x11 */
			KEY_1,				/* kVK_ANSI_1                    = 0x12 */
			KEY_2,				/* kVK_ANSI_2                    = 0x13 */
			KEY_3,				/* kVK_ANSI_3                    = 0x14 */
			KEY_4,				/* kVK_ANSI_4                    = 0x15 */
			KEY_6,				/* kVK_ANSI_6                    = 0x16 */
			KEY_5,				/* kVK_ANSI_5                    = 0x17 */
			KEY_Equal,			/* kVK_ANSI_Equal                = 0x18 */
			KEY_9,				/* kVK_ANSI_9                    = 0x19 */
			KEY_7,				/* kVK_ANSI_7                    = 0x1A */
			KEY_Minus,			/* kVK_ANSI_Minus                = 0x1B */
			KEY_8,				/* kVK_ANSI_8                    = 0x1C */
			KEY_0,				/* kVK_ANSI_0                    = 0x1D */
			KEY_Bracketright,	/* kVK_ANSI_RightBracket         = 0x1E */
			KEY_O,				/* kVK_ANSI_O                    = 0x1F */
			KEY_U,				/* kVK_ANSI_U                    = 0x20 */
			KEY_Braceleft,		/* kVK_ANSI_LeftBracket          = 0x21 */
			KEY_I,				/* kVK_ANSI_I                    = 0x22 */
			KEY_P,				/* kVK_ANSI_P                    = 0x23 */
			KEY_Return,			/* kVK_Return                    = 0x24 */
			KEY_L,				/* kVK_ANSI_L                    = 0x25 */
			KEY_J,				/* kVK_ANSI_J                    = 0x26 */
			KEY_Quotedbl,		/* kVK_ANSI_Quote                = 0x27 */
			KEY_K,				/* kVK_ANSI_K                    = 0x28 */
			KEY_Semicolon,		/* kVK_ANSI_Semicolon            = 0x29 */
			KEY_Backslash,		/* kVK_ANSI_Backslash            = 0x2A */
			KEY_Comma,			/* kVK_ANSI_Comma                = 0x2B */
			KEY_Slash,			/* kVK_ANSI_Slash                = 0x2C */
			KEY_N,				/* kVK_ANSI_N                    = 0x2D */
			KEY_M,				/* kVK_ANSI_M                    = 0x2E */
			KEY_Period,			/* kVK_ANSI_Period               = 0x2F */
			KEY_Tab,			/* kVK_Tab                       = 0x30 */
			KEY_Space,			/* kVK_Space                     = 0x31 */
			KEY_Grave,			/* kVK_ANSI_Grave                = 0x32 */
			KEY_Delete,			/* kVK_Delete                    = 0x33 */
			KEY_Unknown,		/*							     = 0x34 */
			KEY_Escape,			/* kVK_Escape                    = 0x35 */
			KEY_Unknown,		/*							     = 0x36 */
			KEY_Meta,			/* kVK_Command                   = 0x37 */
			KEY_Shift,			/* kVK_Shift                     = 0x38 */
			KEY_Shift,			/* kVK_CapsLock                  = 0x39 */
			KEY_Alt,			/* kVK_Option                    = 0x3A */
			KEY_Control,		/* kVK_Control                   = 0x3B */
			KEY_Shift,			/* kVK_RightShift                = 0x3C */
			KEY_Alt,			/* kVK_RightOption               = 0x3D */
			KEY_Control,		/* kVK_RightControl              = 0x3E */
			KEY_Unknown,		/* kVK_Function                  = 0x3F */
			KEY_F17,			/* kVK_F17                       = 0x40 */
			KEY_Period,			/* kVK_ANSI_KeypadDecimal        = 0x41 */
			KEY_Unknown,		/*							     = 0x42 */
			KEY_Asterisk,		/* kVK_ANSI_KeypadMultiply       = 0x43 */
			KEY_Unknown,		/*							     = 0x44 */
			KEY_Plus,			/* kVK_ANSI_KeypadPlus           = 0x45 */
			KEY_Unknown,		/*							     = 0x46 */
			KEY_Unknown,		/* TODO kVK_ANSI_KeypadClear     = 0x47 */
			KEY_Unknown,		/* kVK_VolumeUp                  = 0x48 */
			KEY_Unknown,		/* kVK_VolumeDown                = 0x49 */
			KEY_Unknown,		/* kVK_Mute                      = 0x4A */
			KEY_Slash,			/* kVK_ANSI_KeypadDivide         = 0x4B */
			KEY_Enter,			/* kVK_ANSI_KeypadEnter          = 0x4C */
			KEY_Unknown,		/*							     = 0x4D */
			KEY_Minus,			/* kVK_ANSI_KeypadMinus          = 0x4E */
			KEY_F18,			/* kVK_F18                       = 0x4F */
			KEY_F19,			/* kVK_F19                       = 0x50 */
			KEY_Equal,			/* kVK_ANSI_KeypadEquals         = 0x51 */
			KEY_0,				/* kVK_ANSI_Keypad0              = 0x52 */
			KEY_1,				/* kVK_ANSI_Keypad1              = 0x53 */
			KEY_2,				/* kVK_ANSI_Keypad2              = 0x54 */
			KEY_3,				/* kVK_ANSI_Keypad3              = 0x55 */
			KEY_4,				/* kVK_ANSI_Keypad4              = 0x56 */
			KEY_5,				/* kVK_ANSI_Keypad5              = 0x57 */
			KEY_6,				/* kVK_ANSI_Keypad6              = 0x58 */
			KEY_7,				/* kVK_ANSI_Keypad7              = 0x59 */
			KEY_F20,			/* kVK_F20                       = 0x5A */
			KEY_8,				/* kVK_ANSI_Keypad8              = 0x5B */
			KEY_9,				/* kVK_ANSI_Keypad9              = 0x5C */
			KEY_Unknown,		/*							     = 0x5D */
			KEY_Unknown,		/*							     = 0x5E */
			KEY_Unknown,		/*							     = 0x5F */
			KEY_F5,				/* kVK_F5                        = 0x60 */
			KEY_F6,				/* kVK_F6                        = 0x61 */
			KEY_F7,				/* kVK_F7                        = 0x62 */
			KEY_F3,				/* kVK_F3                        = 0x63 */
			KEY_F8,				/* kVK_F8                        = 0x64 */
			KEY_F9,				/* kVK_F9                        = 0x65 */
			KEY_Unknown,		/*							     = 0x66 */
			KEY_F11,			/* kVK_F11                       = 0x67 */
			KEY_Unknown,		/*							     = 0x68 */
			KEY_F13,			/* kVK_F13                       = 0x69 */
			KEY_F16,			/* kVK_F16                       = 0x6A */
			KEY_F14,			/* kVK_F14                       = 0x6B */
			KEY_Unknown,		/*							     = 0x6C */
			KEY_F10,			/* kVK_F10                       = 0x6D */
			KEY_Unknown,		/*							     = 0x6E */
			KEY_F12,			/* kVK_F12                       = 0x6F */
			KEY_Unknown,		/*							     = 0x70 */
			KEY_F15,			/* kVK_F15                       = 0x71 */
			KEY_Help,			/* kVK_Help                      = 0x72 */
			KEY_Home,			/* kVK_Home                      = 0x73 */
			KEY_Pageup,			/* kVK_PageUp                    = 0x74 */
			KEY_Delete,			/* TODO: KEY_Clear ? kVK_ForwardDelete             = 0x75 */
			KEY_F4,				/* kVK_F4                        = 0x76 */
			KEY_End,			/* kVK_End                       = 0x77 */
			KEY_F2,				/* kVK_F2                        = 0x78 */
			KEY_Pagedown,		/* kVK_PageDown                  = 0x79 */
			KEY_F1,				/* kVK_F1                        = 0x7A */
			KEY_Left,			/* kVK_LeftArrow                 = 0x7B */
			KEY_Right,			/* kVK_RightArrow                = 0x7C */
			KEY_Down,			/* kVK_DownArrow                 = 0x7D */
			KEY_Up,				/* kVK_UpArrow                   = 0x7E */
		};


		if( str.length() == 1 ) {
            unsigned char key = str[ 0 ];
            if( ( key >= KEY_Space && key <= KEY_Asciitilde ) ||
                ( key >= KEY_Nobreakspace && key <= KEY_ydiaeresis ) )
                return ( KeyCode ) key;
		}
		uint16_t code = [ e keyCode ];
		if( code <= 0x7e )
			return keymap[ code ];

		return KEY_Unknown;
	}

	inline KeyModifier OSXKeyMap::OSXModifierToKeyModifier( NSEvent* e )
	{
		KeyModifier ret = NO_MODIFIER;
		unsigned int flags = [ e modifierFlags ];

		if ( flags & NSAlphaShiftKeyMask )
			ret |= SHIFTLOCK_MODIFIER;
		if ( flags & NSShiftKeyMask )
			ret |= SHIFT_MODIFIER;
		if ( flags & NSControlKeyMask )
			ret |= CTRL_MODIFIER;
		if ( flags & NSAlternateKeyMask )
			ret |= ALT_MODIFIER;
		if( flags & NSCommandKeyMask )
			ret |= META_MODIFIER;
		return ret;
	}
}

#endif
