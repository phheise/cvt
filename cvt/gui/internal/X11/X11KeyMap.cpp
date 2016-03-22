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

#include <cvt/gui/internal/X11/X11KeyMap.h>

namespace cvt {



	void X11KeyMap::mapToKeyEvent( KeyEvent& kev, XKeyEvent* xev )
	{
		char buf[ 10 ];
		KeySym keysym;

		int count = XLookupString( xev, buf, 10, &keysym, NULL );
		kev = KeyEvent( XKeySymToKeycode( keysym ), XModifierToKeyModifier( xev ), String( buf, count ) );
	}


	KeyCode X11KeyMap::XKeySymToKeycode( KeySym sym )
	{
		static const KeyCode _xk_ttykeys1[ 0x06 ] = {
			KEY_Backspace, /* XK_BackSpace   0xff08 */
			KEY_Tab      , /* XK_Tab         0xff09 */
		    KEY_Unknown  , /* XK_Linefeed    0xff0a */
			KEY_Delete	 , /* XK_Clear       0xff0b */
			KEY_Unknown  , /*				 0xff0c */
			KEY_Return	   /* XK_Return      0xff0d */
		};
		static const KeyCode _xk_ttykeys2[ 0x09 ] = {
			KEY_Pause	 , /* XK_Pause         0xff13 */
			KEY_Scrolllock , /* XK_Scroll_Lock 0xff14 */
			KEY_Sysreq	 , /* XK_Sys_Req       0xff15 */
			KEY_Unknown  , /*				   0xff16 */
			KEY_Unknown  , /*				   0xff17 */
			KEY_Unknown  , /*				   0xff18 */
			KEY_Unknown  , /*				   0xff19 */
			KEY_Unknown  , /*				   0xff1a */
			KEY_Escape	   /* XK_Escape        0xff1b */
		};
		static const KeyCode _xk_other[ 0xbd ] = {
			KEY_Home,		/*  XK_Home                          0xff50  */
			KEY_Left,		/*  XK_Left                          0xff51  */
			KEY_Up,			/*  XK_Up                            0xff52  */
			KEY_Right,		/*  XK_Right                         0xff53  */
			KEY_Down,		/*  XK_Down                          0xff54  */
			KEY_Pageup,		/*  XK_Page_Up                       0xff55  */
			KEY_Pagedown,	/*  XK_Page_Down                     0xff56  */
			KEY_End,		/*  XK_End                           0xff57  */
			KEY_Unknown,	/*  XK_Begin                         0xff58  */
			KEY_Unknown,	/*									 0xff59  */
			KEY_Unknown,	/*									 0xff5a  */
			KEY_Unknown,	/*									 0xff5b  */
			KEY_Unknown,	/*									 0xff5c  */
			KEY_Unknown,	/*									 0xff5d  */
			KEY_Unknown,	/*									 0xff5e  */
			KEY_Unknown,	/*									 0xff5f  */
			KEY_Unknown,	/*  XK_Select                        0xff60  */
			KEY_Print,		/*  XK_Print                         0xff61  */
			KEY_Unknown,	/*  XK_Execute                       0xff62  */
			KEY_Insert,		/*  XK_Insert                        0xff63  */
			KEY_Unknown,	/*									 0xff64  */
			KEY_Unknown,	/*  XK_Undo                          0xff65  */
			KEY_Unknown,	/*  XK_Redo                          0xff66  */
			KEY_Menu,		/*  XK_Menu                          0xff67  */
			KEY_Unknown,	/*  XK_Find                          0xff68  */
			KEY_Unknown,	/*  XK_Cancel                        0xff69  */
			KEY_Help,		/*  XK_Help                          0xff6a  */
			KEY_Unknown,	/*  XK_Break                         0xff6b  */
			KEY_Unknown,	/*									 0xff6c  */
			KEY_Unknown,	/*									 0xff6d  */
			KEY_Unknown,	/*									 0xff6e  */
			KEY_Unknown,	/*									 0xff6f  */
			KEY_Unknown,	/*									 0xff70  */
			KEY_Unknown,	/*									 0xff71  */
			KEY_Unknown,	/*									 0xff72  */
			KEY_Unknown,	/*									 0xff73  */
			KEY_Unknown,	/*									 0xff74  */
			KEY_Unknown,	/*									 0xff75  */
			KEY_Unknown,	/*									 0xff76  */
			KEY_Unknown,	/*									 0xff77  */
			KEY_Unknown,	/*									 0xff78  */
			KEY_Unknown,	/*									 0xff79  */
			KEY_Unknown,	/*									 0xff7a  */
			KEY_Unknown,	/*									 0xff7b  */
			KEY_Unknown,	/*									 0xff7c  */
			KEY_Unknown,	/*									 0xff7d  */
			KEY_Unknown,	/*  XK_Mode_switch                   0xff7e , XK_script_switch                 0xff7e  */
			KEY_Numlock,	/*  XK_Num_Lock                      0xff7f  */
			KEY_Space,		/*  XK_KP_Space                      0xff80  */
			KEY_Unknown,	/*									 0xff81  */
			KEY_Unknown,	/*									 0xff82  */
			KEY_Unknown,	/*									 0xff83  */
			KEY_Unknown,	/*									 0xff84  */
			KEY_Unknown,	/*									 0xff85  */
			KEY_Unknown,	/*									 0xff86  */
			KEY_Unknown,	/*									 0xff87  */
			KEY_Unknown,	/*									 0xff88  */
			KEY_Tab,		/*  XK_KP_Tab                        0xff89  */
			KEY_Unknown,	/*									 0xff8a  */
			KEY_Unknown,	/*									 0xff8b  */
			KEY_Unknown,	/*									 0xff8c  */
			KEY_Enter,		/*  XK_KP_Enter                      0xff8d  */
			KEY_Unknown,	/*									 0xff8e  */
			KEY_Unknown,	/*									 0xff8f  */
			KEY_Unknown,	/*									 0xff90  */
			KEY_F1,			/*  XK_KP_F1                         0xff91  */
			KEY_F2,			/*  XK_KP_F2                         0xff92  */
			KEY_F3,			/*  XK_KP_F3                         0xff93  */
			KEY_F4,			/*  XK_KP_F4                         0xff94  */
			KEY_Home,		/*  XK_KP_Home                       0xff95  */
			KEY_Left,		/*  XK_KP_Left                       0xff96  */
			KEY_Up,			/*  XK_KP_Up                         0xff97  */
			KEY_Right,		/*  XK_KP_Right                      0xff98  */
			KEY_Down,		/*  XK_KP_Down                       0xff99  */
			KEY_Pageup,		/*  XK_KP_Page_Up                    0xff9a  */
			KEY_Pagedown,	/*  XK_KP_Page_Down                  0xff9b  */
			KEY_End,		/*  XK_KP_End                        0xff9c  */
			KEY_Unknown,	/*  XK_KP_Begin                      0xff9d  */
			KEY_Insert,		/*  XK_KP_Insert                     0xff9e  */
			KEY_Delete,		/*  XK_KP_Delete                     0xff9f  */
			KEY_Unknown,	/*									 0xffa0  */
			KEY_Unknown,	/*									 0xffa1  */
			KEY_Unknown,	/*									 0xffa2  */
			KEY_Unknown,	/*									 0xffa3  */
			KEY_Unknown,	/*									 0xffa4  */
			KEY_Unknown,	/*									 0xffa5  */
			KEY_Unknown,	/*									 0xffa6  */
			KEY_Unknown,	/*									 0xffa7  */
			KEY_Unknown,	/*									 0xffa8  */
			KEY_Unknown,	/*									 0xffa9  */
			KEY_Asterisk,	/*  XK_KP_Multiply                   0xffaa  */
			KEY_Plus,		/*  XK_KP_Add                        0xffab  */
			KEY_Comma,		/*  XK_KP_Separator                  0xffac  */
			KEY_Minus,		/*  XK_KP_Subtract                   0xffad  */
			KEY_Period,		/*  XK_KP_Decimal                    0xffae  */
			KEY_Slash,		/*  XK_KP_Divide                     0xffaf  */
			KEY_0,			/*  XK_KP_0                          0xffb0  */
			KEY_1,			/*  XK_KP_1                          0xffb1  */
			KEY_2,			/*  XK_KP_2                          0xffb2  */
			KEY_3,			/*  XK_KP_3                          0xffb3  */
			KEY_4,			/*  XK_KP_4                          0xffb4  */
			KEY_5,			/*  XK_KP_5                          0xffb5  */
			KEY_6,			/*  XK_KP_6                          0xffb6  */
			KEY_7,			/*  XK_KP_7                          0xffb7  */
			KEY_8,			/*  XK_KP_8                          0xffb8  */
			KEY_9,			/*  XK_KP_9                          0xffb9  */
			KEY_Unknown,	/*									 0xffba  */
			KEY_Unknown,	/*									 0xffbb  */
			KEY_Unknown,	/*									 0xffbc  */
			KEY_Equal,		/*  XK_KP_Equal                      0xffbd  */
		};
		static const KeyCode _xk_mods[ 0xe ] = {
			KEY_Shift,		/* XK_Shift_L                       0xffe1 */
			KEY_Shift,		/* XK_Shift_R                       0xffe2 */
			KEY_Control,    /* XK_Control_L                     0xffe3 */
			KEY_Control,	/* XK_Control_R                     0xffe4 */
			KEY_Shift,		/* XK_Caps_Lock                     0xffe5 */
			KEY_Shift,		/* XK_Shift_Lock                    0xffe6 */
			KEY_Meta,		/* XK_Meta_L                        0xffe7 */
			KEY_Meta,		/* XK_Meta_R                        0xffe8 */
			KEY_Alt,		/* XK_Alt_L                         0xffe9 */
			KEY_Alt,		/* XK_Alt_R                         0xffea */
			KEY_Super_L,	/* XK_Super_L                       0xffeb */
			KEY_Super_R,	/* XK_Super_R                       0xffec */
			KEY_Hyper_L,	/* XK_Hyper_L                       0xffed */
			KEY_Hyper_R,	/* XK_Hyper_R                       0xffee */
		};

		if( ( sym >= 0x20 && sym <= 0x7e ) ||
		    ( sym >= 0xa0 && sym <= 0xff ) ) {
			return ( KeyCode ) sym;
		} else if( sym >= XK_BackSpace && sym <= XK_Return ) {
			return _xk_ttykeys1[ ( sym & 0xff ) - 0x08 ];
		} else if( sym >= XK_Pause && sym <= XK_Escape ) {
			return _xk_ttykeys2[ ( sym & 0xff ) - 0x13 ];
		} else if( sym >= XK_Home && sym <= XK_KP_Equal ) {
			return _xk_other[ ( sym & 0xff ) - 0x50 ];
		} else if( sym >= XK_Shift_L && sym <= XK_Hyper_R ) {
			return _xk_mods[ ( sym & 0xff ) - 0xe1 ];
		} else if( sym >= XK_F1 && sym <= XK_F35 ) {
			return ( KeyCode ) ( KEY_F1 + ( sym - XK_F1 ) );
		} else if( sym == XK_Delete ) {
			return KEY_Delete;
		}

		return KEY_Unknown;
	}


	KeyModifier X11KeyMap::XModifierToKeyModifier( XKeyEvent* xev )
	{
		static const KeyModifier modtab[ 32 ] = {
			NO_MODIFIER,
			SHIFT_MODIFIER,
			SHIFTLOCK_MODIFIER,
			SHIFTLOCK_MODIFIER | SHIFT_MODIFIER,
			CTRL_MODIFIER,
			CTRL_MODIFIER | SHIFT_MODIFIER,
			CTRL_MODIFIER | SHIFTLOCK_MODIFIER,
			CTRL_MODIFIER | SHIFTLOCK_MODIFIER | SHIFT_MODIFIER,
			META_MODIFIER,
			META_MODIFIER | SHIFT_MODIFIER,
			META_MODIFIER | SHIFTLOCK_MODIFIER,
			META_MODIFIER | SHIFTLOCK_MODIFIER | SHIFT_MODIFIER,
			META_MODIFIER | CTRL_MODIFIER,
			META_MODIFIER | CTRL_MODIFIER | SHIFT_MODIFIER,
			META_MODIFIER | CTRL_MODIFIER | SHIFTLOCK_MODIFIER,
			META_MODIFIER | CTRL_MODIFIER | SHIFTLOCK_MODIFIER | SHIFT_MODIFIER,
			ALT_MODIFIER,
			ALT_MODIFIER | SHIFT_MODIFIER,
			ALT_MODIFIER | SHIFTLOCK_MODIFIER,
			ALT_MODIFIER | SHIFTLOCK_MODIFIER | SHIFT_MODIFIER,
			ALT_MODIFIER | CTRL_MODIFIER,
			ALT_MODIFIER | CTRL_MODIFIER | SHIFT_MODIFIER,
			ALT_MODIFIER | CTRL_MODIFIER | SHIFTLOCK_MODIFIER,
			ALT_MODIFIER | CTRL_MODIFIER | SHIFTLOCK_MODIFIER | SHIFT_MODIFIER,
			ALT_MODIFIER | META_MODIFIER,
			ALT_MODIFIER | META_MODIFIER | SHIFT_MODIFIER,
			ALT_MODIFIER | META_MODIFIER | SHIFTLOCK_MODIFIER,
			ALT_MODIFIER | META_MODIFIER | SHIFTLOCK_MODIFIER | SHIFT_MODIFIER,
			ALT_MODIFIER | META_MODIFIER | CTRL_MODIFIER,
			ALT_MODIFIER | META_MODIFIER | CTRL_MODIFIER | SHIFT_MODIFIER,
			ALT_MODIFIER | META_MODIFIER | CTRL_MODIFIER | SHIFTLOCK_MODIFIER,
			ALT_MODIFIER | META_MODIFIER | CTRL_MODIFIER | SHIFTLOCK_MODIFIER | SHIFT_MODIFIER,
		};
		return modtab[ xev->state & 0x1f ];
	}
}
