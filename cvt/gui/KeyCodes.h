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

#ifndef CVT_KEYCODES_H
#define CVT_KEYCODES_H


namespace cvt {
	enum KeyCode {
		KEY_Unknown = 0x0,
		KEY_Escape = 0x01000000,
		KEY_Tab = 0x01000001,
		KEY_Backtab = 0x01000002,
		KEY_Backspace = 0x01000003,
		KEY_Return = 0x01000004,
		KEY_Enter = 0x01000005,
		KEY_Insert = 0x01000006,
		KEY_Delete = 0x01000007,
		KEY_Pause = 0x01000008,
		KEY_Print = 0x01000009,
		KEY_Sysreq = 0x0100000A,
		KEY_Clear = 0x0100000B,
		KEY_Home = 0x01000010,
		KEY_End = 0x01000011,
		KEY_Left = 0x01000012,
		KEY_Up = 0x01000013,
		KEY_Right = 0x01000014,
		KEY_Down = 0x01000015,
		KEY_Pageup = 0x01000016,
		KEY_Pagedown = 0x01000017,
		KEY_Shift = 0x01000020,
		KEY_Control = 0x01000021,
		KEY_Meta = 0x01000022,
		KEY_Alt = 0x01000023,
		KEY_Altgr = 0x01001103,
		KEY_Capslock = 0x01000024,
		KEY_Numlock = 0x01000025,
		KEY_Scrolllock = 0x01000026,
		KEY_F1 = 0x01000030,
		KEY_F2 = 0x01000031,
		KEY_F3 = 0x01000032,
		KEY_F4 = 0x01000033,
		KEY_F5 = 0x01000034,
		KEY_F6 = 0x01000035,
		KEY_F7 = 0x01000036,
		KEY_F8 = 0x01000037,
		KEY_F9 = 0x01000038,
		KEY_F10 = 0x01000039,
		KEY_F11 = 0x0100003A,
		KEY_F12 = 0x0100003B,
		KEY_F13 = 0x0100003C,
		KEY_F14 = 0x0100003D,
		KEY_F15 = 0x0100003E,
		KEY_F16 = 0x0100003F,
		KEY_F17 = 0x01000040,
		KEY_F18 = 0x01000041,
		KEY_F19 = 0x01000042,
		KEY_F20 = 0x01000043,
		KEY_F21 = 0x01000044,
		KEY_F22 = 0x01000045,
		KEY_F23 = 0x01000046,
		KEY_F24 = 0x01000047,
		KEY_F25 = 0x01000048,
		KEY_F26 = 0x01000049,
		KEY_F27 = 0x0100004A,
		KEY_F28 = 0x0100004B,
		KEY_F29 = 0x0100004C,
		KEY_F30 = 0x0100004D,
		KEY_F31 = 0x0100004E,
		KEY_F32 = 0x0100004F,
		KEY_F33 = 0x01000050,
		KEY_F34 = 0x01000051,
		KEY_F35 = 0x01000052,
		KEY_Super_L = 0x01000053,
		KEY_Super_R = 0x01000054,
		KEY_Menu = 0x01000055,
		KEY_Hyper_L = 0x01000056,
		KEY_Hyper_R = 0x01000057,
		KEY_Help = 0x01000058,
		KEY_Direction_L = 0x01000059,
		KEY_Direction_R = 0x01000060,

		/* Latin 1*/
		KEY_Space                         = 0x0020,  /* U+0020 SPACE */
		KEY_Exclam                        = 0x0021,  /* U+0021 EXCLAMATION MARK */
		KEY_Quotedbl                      = 0x0022,  /* U+0022 QUOTATION MARK */
		KEY_Numbersign                    = 0x0023,  /* U+0023 NUMBER SIGN */
		KEY_Dollar                        = 0x0024,  /* U+0024 DOLLAR SIGN */
		KEY_Percent                       = 0x0025,  /* U+0025 PERCENT SIGN */
		KEY_Ampersand                     = 0x0026,  /* U+0026 AMPERSAND */
		KEY_Apostrophe                    = 0x0027,  /* U+0027 APOSTROPHE */
		KEY_Quoteright                    = 0x0027,  /* deprecated */
		KEY_Parenleft                     = 0x0028,  /* U+0028 LEFT PARENTHESIS */
		KEY_Parenright                    = 0x0029,  /* U+0029 RIGHT PARENTHESIS */
		KEY_Asterisk                      = 0x002a,  /* U+002A ASTERISK */
		KEY_Plus                          = 0x002b,  /* U+002B PLUS SIGN */
		KEY_Comma                         = 0x002c,  /* U+002C COMMA */
		KEY_Minus                         = 0x002d,  /* U+002D HYPHEN-MINUS */
		KEY_Period                        = 0x002e,  /* U+002E FULL STOP */
		KEY_Slash                         = 0x002f,  /* U+002F SOLIDUS */
		KEY_0                             = 0x0030,  /* U+0030 DIGIT ZERO */
		KEY_1                             = 0x0031,  /* U+0031 DIGIT ONE */
		KEY_2                             = 0x0032,  /* U+0032 DIGIT TWO */
		KEY_3                             = 0x0033,  /* U+0033 DIGIT THREE */
		KEY_4                             = 0x0034,  /* U+0034 DIGIT FOUR */
		KEY_5                             = 0x0035,  /* U+0035 DIGIT FIVE */
		KEY_6                             = 0x0036,  /* U+0036 DIGIT SIX */
		KEY_7                             = 0x0037,  /* U+0037 DIGIT SEVEN */
		KEY_8                             = 0x0038,  /* U+0038 DIGIT EIGHT */
		KEY_9                             = 0x0039,  /* U+0039 DIGIT NINE */
		KEY_Colon                         = 0x003a,  /* U+003A COLON */
		KEY_Semicolon                     = 0x003b,  /* U+003B SEMICOLON */
		KEY_Less                          = 0x003c,  /* U+003C LESS-THAN SIGN */
		KEY_Equal                         = 0x003d,  /* U+003D EQUALS SIGN */
		KEY_Greater                       = 0x003e,  /* U+003E GREATER-THAN SIGN */
		KEY_Question                      = 0x003f,  /* U+003F QUESTION MARK */
		KEY_At                            = 0x0040,  /* U+0040 COMMERCIAL AT */
		KEY_A                             = 0x0041,  /* U+0041 LATIN CAPITAL LETTER A */
		KEY_B                             = 0x0042,  /* U+0042 LATIN CAPITAL LETTER B */
		KEY_C                             = 0x0043,  /* U+0043 LATIN CAPITAL LETTER C */
		KEY_D                             = 0x0044,  /* U+0044 LATIN CAPITAL LETTER D */
		KEY_E                             = 0x0045,  /* U+0045 LATIN CAPITAL LETTER E */
		KEY_F                             = 0x0046,  /* U+0046 LATIN CAPITAL LETTER F */
		KEY_G                             = 0x0047,  /* U+0047 LATIN CAPITAL LETTER G */
		KEY_H                             = 0x0048,  /* U+0048 LATIN CAPITAL LETTER H */
		KEY_I                             = 0x0049,  /* U+0049 LATIN CAPITAL LETTER I */
		KEY_J                             = 0x004a,  /* U+004A LATIN CAPITAL LETTER J */
		KEY_K                             = 0x004b,  /* U+004B LATIN CAPITAL LETTER K */
		KEY_L                             = 0x004c,  /* U+004C LATIN CAPITAL LETTER L */
		KEY_M                             = 0x004d,  /* U+004D LATIN CAPITAL LETTER M */
		KEY_N                             = 0x004e,  /* U+004E LATIN CAPITAL LETTER N */
		KEY_O                             = 0x004f,  /* U+004F LATIN CAPITAL LETTER O */
		KEY_P                             = 0x0050,  /* U+0050 LATIN CAPITAL LETTER P */
		KEY_Q                             = 0x0051,  /* U+0051 LATIN CAPITAL LETTER Q */
		KEY_R                             = 0x0052,  /* U+0052 LATIN CAPITAL LETTER R */
		KEY_S                             = 0x0053,  /* U+0053 LATIN CAPITAL LETTER S */
		KEY_T                             = 0x0054,  /* U+0054 LATIN CAPITAL LETTER T */
		KEY_U                             = 0x0055,  /* U+0055 LATIN CAPITAL LETTER U */
		KEY_V                             = 0x0056,  /* U+0056 LATIN CAPITAL LETTER V */
		KEY_W                             = 0x0057,  /* U+0057 LATIN CAPITAL LETTER W */
		KEY_X                             = 0x0058,  /* U+0058 LATIN CAPITAL LETTER X */
		KEY_Y                             = 0x0059,  /* U+0059 LATIN CAPITAL LETTER Y */
		KEY_Z                             = 0x005a,  /* U+005A LATIN CAPITAL LETTER Z */
		KEY_Bracketleft                   = 0x005b,  /* U+005B LEFT SQUARE BRACKET */
		KEY_Backslash                     = 0x005c,  /* U+005C REVERSE SOLIDUS */
		KEY_Bracketright                  = 0x005d,  /* U+005D RIGHT SQUARE BRACKET */
		KEY_Asciicircum                   = 0x005e,  /* U+005E CIRCUMFLEX ACCENT */
		KEY_Underscore                    = 0x005f,  /* U+005F LOW LINE */
		KEY_Grave                         = 0x0060,  /* U+0060 GRAVE ACCENT */
		KEY_Quoteleft                     = 0x0060,  /* deprecated */
		KEY_a                             = 0x0061,  /* U+0061 LATIN SMALL LETTER A */
		KEY_b                             = 0x0062,  /* U+0062 LATIN SMALL LETTER B */
		KEY_c                             = 0x0063,  /* U+0063 LATIN SMALL LETTER C */
		KEY_d                             = 0x0064,  /* U+0064 LATIN SMALL LETTER D */
		KEY_e                             = 0x0065,  /* U+0065 LATIN SMALL LETTER E */
		KEY_f                             = 0x0066,  /* U+0066 LATIN SMALL LETTER F */
		KEY_g                             = 0x0067,  /* U+0067 LATIN SMALL LETTER G */
		KEY_h                             = 0x0068,  /* U+0068 LATIN SMALL LETTER H */
		KEY_i                             = 0x0069,  /* U+0069 LATIN SMALL LETTER I */
		KEY_j                             = 0x006a,  /* U+006A LATIN SMALL LETTER J */
		KEY_k                             = 0x006b,  /* U+006B LATIN SMALL LETTER K */
		KEY_l                             = 0x006c,  /* U+006C LATIN SMALL LETTER L */
		KEY_m                             = 0x006d,  /* U+006D LATIN SMALL LETTER M */
		KEY_n                             = 0x006e,  /* U+006E LATIN SMALL LETTER N */
		KEY_o                             = 0x006f,  /* U+006F LATIN SMALL LETTER O */
		KEY_p                             = 0x0070,  /* U+0070 LATIN SMALL LETTER P */
		KEY_q                             = 0x0071,  /* U+0071 LATIN SMALL LETTER Q */
		KEY_r                             = 0x0072,  /* U+0072 LATIN SMALL LETTER R */
		KEY_s                             = 0x0073,  /* U+0073 LATIN SMALL LETTER S */
		KEY_t                             = 0x0074,  /* U+0074 LATIN SMALL LETTER T */
		KEY_u                             = 0x0075,  /* U+0075 LATIN SMALL LETTER U */
		KEY_v                             = 0x0076,  /* U+0076 LATIN SMALL LETTER V */
		KEY_w                             = 0x0077,  /* U+0077 LATIN SMALL LETTER W */
		KEY_x                             = 0x0078,  /* U+0078 LATIN SMALL LETTER X */
		KEY_y                             = 0x0079,  /* U+0079 LATIN SMALL LETTER Y */
		KEY_z                             = 0x007a,  /* U+007A LATIN SMALL LETTER Z */
		KEY_Braceleft                     = 0x007b,  /* U+007B LEFT CURLY BRACKET */
		KEY_Bar                           = 0x007c,  /* U+007C VERTICAL LINE */
		KEY_Braceright                    = 0x007d,  /* U+007D RIGHT CURLY BRACKET */
		KEY_Asciitilde                    = 0x007e,  /* U+007E TILDE */

		KEY_Nobreakspace                  = 0x00a0,  /* U+00A0 NO-BREAK SPACE */
		KEY_Exclamdown                    = 0x00a1,  /* U+00A1 INVERTED EXCLAMATION MARK */
		KEY_Cent                          = 0x00a2,  /* U+00A2 CENT SIGN */
		KEY_Sterling                      = 0x00a3,  /* U+00A3 POUND SIGN */
		KEY_Currency                      = 0x00a4,  /* U+00A4 CURRENCY SIGN */
		KEY_Yen                           = 0x00a5,  /* U+00A5 YEN SIGN */
		KEY_Brokenbar                     = 0x00a6,  /* U+00A6 BROKEN BAR */
		KEY_Section                       = 0x00a7,  /* U+00A7 SECTION SIGN */
		KEY_Diaeresis                     = 0x00a8,  /* U+00A8 DIAERESIS */
		KEY_Copyright                     = 0x00a9,  /* U+00A9 COPYRIGHT SIGN */
		KEY_Ordfeminine                   = 0x00aa,  /* U+00AA FEMININE ORDINAL INDICATOR */
		KEY_Guillemotleft                 = 0x00ab,  /* U+00AB LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */
		KEY_Notsign                       = 0x00ac,  /* U+00AC NOT SIGN */
		KEY_Hyphen                        = 0x00ad,  /* U+00AD SOFT HYPHEN */
		KEY_Registered                    = 0x00ae,  /* U+00AE REGISTERED SIGN */
		KEY_Macron                        = 0x00af,  /* U+00AF MACRON */
		KEY_Degree                        = 0x00b0,  /* U+00B0 DEGREE SIGN */
		KEY_Plusminus                     = 0x00b1,  /* U+00B1 PLUS-MINUS SIGN */
		KEY_Twosuperior                   = 0x00b2,  /* U+00B2 SUPERSCRIPT TWO */
		KEY_Threesuperior                 = 0x00b3,  /* U+00B3 SUPERSCRIPT THREE */
		KEY_Acute                         = 0x00b4,  /* U+00B4 ACUTE ACCENT */
		KEY_Mu                            = 0x00b5,  /* U+00B5 MICRO SIGN */
		KEY_Paragraph                     = 0x00b6,  /* U+00B6 PILCROW SIGN */
		KEY_Periodcentered                = 0x00b7,  /* U+00B7 MIDDLE DOT */
		KEY_Cedilla                       = 0x00b8,  /* U+00B8 CEDILLA */
		KEY_Onesuperior                   = 0x00b9,  /* U+00B9 SUPERSCRIPT ONE */
		KEY_Masculine                     = 0x00ba,  /* U+00BA MASCULINE ORDINAL INDICATOR */
		KEY_Guillemotright                = 0x00bb,  /* U+00BB RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */
		KEY_Onequarter                    = 0x00bc,  /* U+00BC VULGAR FRACTION ONE QUARTER */
		KEY_Onehalf                       = 0x00bd,  /* U+00BD VULGAR FRACTION ONE HALF */
		KEY_Threequarters                 = 0x00be,  /* U+00BE VULGAR FRACTION THREE QUARTERS */
		KEY_Questiondown                  = 0x00bf,  /* U+00BF INVERTED QUESTION MARK */
		KEY_Agrave                        = 0x00c0,  /* U+00C0 LATIN CAPITAL LETTER A WITH GRAVE */
		KEY_Aacute                        = 0x00c1,  /* U+00C1 LATIN CAPITAL LETTER A WITH ACUTE */
		KEY_Acircumflex                   = 0x00c2,  /* U+00C2 LATIN CAPITAL LETTER A WITH CIRCUMFLEX */
		KEY_Atilde                        = 0x00c3,  /* U+00C3 LATIN CAPITAL LETTER A WITH TILDE */
		KEY_Adiaeresis                    = 0x00c4,  /* U+00C4 LATIN CAPITAL LETTER A WITH DIAERESIS */
		KEY_Aring                         = 0x00c5,  /* U+00C5 LATIN CAPITAL LETTER A WITH RING ABOVE */
		KEY_AE                            = 0x00c6,  /* U+00C6 LATIN CAPITAL LETTER AE */
		KEY_Ccedilla                      = 0x00c7,  /* U+00C7 LATIN CAPITAL LETTER C WITH CEDILLA */
		KEY_Egrave                        = 0x00c8,  /* U+00C8 LATIN CAPITAL LETTER E WITH GRAVE */
		KEY_Eacute                        = 0x00c9,  /* U+00C9 LATIN CAPITAL LETTER E WITH ACUTE */
		KEY_Ecircumflex                   = 0x00ca,  /* U+00CA LATIN CAPITAL LETTER E WITH CIRCUMFLEX */
		KEY_Ediaeresis                    = 0x00cb,  /* U+00CB LATIN CAPITAL LETTER E WITH DIAERESIS */
		KEY_Igrave                        = 0x00cc,  /* U+00CC LATIN CAPITAL LETTER I WITH GRAVE */
		KEY_Iacute                        = 0x00cd,  /* U+00CD LATIN CAPITAL LETTER I WITH ACUTE */
		KEY_Icircumflex                   = 0x00ce,  /* U+00CE LATIN CAPITAL LETTER I WITH CIRCUMFLEX */
		KEY_Idiaeresis                    = 0x00cf,  /* U+00CF LATIN CAPITAL LETTER I WITH DIAERESIS */
		KEY_ETH                           = 0x00d0,  /* U+00D0 LATIN CAPITAL LETTER ETH */
		KEY_Eth                           = 0x00d0,  /* deprecated */
		KEY_Ntilde                        = 0x00d1,  /* U+00D1 LATIN CAPITAL LETTER N WITH TILDE */
		KEY_Ograve                        = 0x00d2,  /* U+00D2 LATIN CAPITAL LETTER O WITH GRAVE */
		KEY_Oacute                        = 0x00d3,  /* U+00D3 LATIN CAPITAL LETTER O WITH ACUTE */
		KEY_Ocircumflex                   = 0x00d4,  /* U+00D4 LATIN CAPITAL LETTER O WITH CIRCUMFLEX */
		KEY_Otilde                        = 0x00d5,  /* U+00D5 LATIN CAPITAL LETTER O WITH TILDE */
		KEY_Odiaeresis                    = 0x00d6,  /* U+00D6 LATIN CAPITAL LETTER O WITH DIAERESIS */
		KEY_multiply                      = 0x00d7,  /* U+00D7 MULTIPLICATION SIGN */
		KEY_Oslash                        = 0x00d8,  /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
		KEY_Ooblique                      = 0x00d8,  /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
		KEY_Ugrave                        = 0x00d9,  /* U+00D9 LATIN CAPITAL LETTER U WITH GRAVE */
		KEY_Uacute                        = 0x00da,  /* U+00DA LATIN CAPITAL LETTER U WITH ACUTE */
		KEY_Ucircumflex                   = 0x00db,  /* U+00DB LATIN CAPITAL LETTER U WITH CIRCUMFLEX */
		KEY_Udiaeresis                    = 0x00dc,  /* U+00DC LATIN CAPITAL LETTER U WITH DIAERESIS */
		KEY_Yacute                        = 0x00dd,  /* U+00DD LATIN CAPITAL LETTER Y WITH ACUTE */
		KEY_THORN                         = 0x00de,  /* U+00DE LATIN CAPITAL LETTER THORN */
		KEY_Thorn                         = 0x00de,  /* deprecated */
		KEY_ssharp                        = 0x00df,  /* U+00DF LATIN SMALL LETTER SHARP S */
		KEY_agrave                        = 0x00e0,  /* U+00E0 LATIN SMALL LETTER A WITH GRAVE */
		KEY_aacute                        = 0x00e1,  /* U+00E1 LATIN SMALL LETTER A WITH ACUTE */
		KEY_acircumflex                   = 0x00e2,  /* U+00E2 LATIN SMALL LETTER A WITH CIRCUMFLEX */
		KEY_atilde                        = 0x00e3,  /* U+00E3 LATIN SMALL LETTER A WITH TILDE */
		KEY_adiaeresis                    = 0x00e4,  /* U+00E4 LATIN SMALL LETTER A WITH DIAERESIS */
		KEY_aring                         = 0x00e5,  /* U+00E5 LATIN SMALL LETTER A WITH RING ABOVE */
		KEY_ae                            = 0x00e6,  /* U+00E6 LATIN SMALL LETTER AE */
		KEY_ccedilla                      = 0x00e7,  /* U+00E7 LATIN SMALL LETTER C WITH CEDILLA */
		KEY_egrave                        = 0x00e8,  /* U+00E8 LATIN SMALL LETTER E WITH GRAVE */
		KEY_eacute                        = 0x00e9,  /* U+00E9 LATIN SMALL LETTER E WITH ACUTE */
		KEY_ecircumflex                   = 0x00ea,  /* U+00EA LATIN SMALL LETTER E WITH CIRCUMFLEX */
		KEY_ediaeresis                    = 0x00eb,  /* U+00EB LATIN SMALL LETTER E WITH DIAERESIS */
		KEY_igrave                        = 0x00ec,  /* U+00EC LATIN SMALL LETTER I WITH GRAVE */
		KEY_iacute                        = 0x00ed,  /* U+00ED LATIN SMALL LETTER I WITH ACUTE */
		KEY_icircumflex                   = 0x00ee,  /* U+00EE LATIN SMALL LETTER I WITH CIRCUMFLEX */
		KEY_idiaeresis                    = 0x00ef,  /* U+00EF LATIN SMALL LETTER I WITH DIAERESIS */
		KEY_eth                           = 0x00f0,  /* U+00F0 LATIN SMALL LETTER ETH */
		KEY_ntilde                        = 0x00f1,  /* U+00F1 LATIN SMALL LETTER N WITH TILDE */
		KEY_ograve                        = 0x00f2,  /* U+00F2 LATIN SMALL LETTER O WITH GRAVE */
		KEY_oacute                        = 0x00f3,  /* U+00F3 LATIN SMALL LETTER O WITH ACUTE */
		KEY_ocircumflex                   = 0x00f4,  /* U+00F4 LATIN SMALL LETTER O WITH CIRCUMFLEX */
		KEY_otilde                        = 0x00f5,  /* U+00F5 LATIN SMALL LETTER O WITH TILDE */
		KEY_odiaeresis                    = 0x00f6,  /* U+00F6 LATIN SMALL LETTER O WITH DIAERESIS */
		KEY_division                      = 0x00f7,  /* U+00F7 DIVISION SIGN */
		KEY_oslash                        = 0x00f8,  /* U+00F8 LATIN SMALL LETTER O WITH STROKE */
		KEY_ooblique                      = 0x00f8,  /* U+00F8 LATIN SMALL LETTER O WITH STROKE */
		KEY_ugrave                        = 0x00f9,  /* U+00F9 LATIN SMALL LETTER U WITH GRAVE */
		KEY_uacute                        = 0x00fa,  /* U+00FA LATIN SMALL LETTER U WITH ACUTE */
		KEY_ucircumflex                   = 0x00fb,  /* U+00FB LATIN SMALL LETTER U WITH CIRCUMFLEX */
		KEY_udiaeresis                    = 0x00fc,  /* U+00FC LATIN SMALL LETTER U WITH DIAERESIS */
		KEY_yacute                        = 0x00fd,  /* U+00FD LATIN SMALL LETTER Y WITH ACUTE */
		KEY_thorn                         = 0x00fe,  /* U+00FE LATIN SMALL LETTER THORN */
		KEY_ydiaeresis                    = 0x00ff,  /* U+00FF LATIN SMALL LETTER Y WITH DIAERESIS */

	};
}

#endif
