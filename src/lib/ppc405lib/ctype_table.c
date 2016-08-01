/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/ctype_table.c $                             */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

/// \file ctype_table.c
/// \brief Character table for <ctype.h> functions. 
///
/// This table is used by the <ctype.h> functions for a quick lookup of
/// character type information. Because the true functional forms of <ctype.h>
/// functions are likely never required, but this file is always required, it
/// is stored separately from ctype.c to (slightly) reduce code/data space
/// requirements.

#include <stdint.h>
#include <ctype.h>

const uint8_t _ctype[256] = {
    _CTYPE_CNTRL,                                /* 0   00 NUL  Null char */
    _CTYPE_CNTRL,                                /* 1   01 SOH  Start of Heading */
    _CTYPE_CNTRL,                                /* 2   02 STX  Start of Text */
    _CTYPE_CNTRL,                                /* 3   03 ETX  End of Text */
    _CTYPE_CNTRL,                                /* 4   04 EOT  End of Transmission */
    _CTYPE_CNTRL,                                /* 5   05 ENQ  Enquiry */
    _CTYPE_CNTRL,                                /* 6   06 ACK  Acknowledgment */
    _CTYPE_CNTRL,                                /* 7   07 BEL  Bell */
    _CTYPE_CNTRL,                                /* 8   08  BS  Back Space */
    _CTYPE_CNTRL | _CTYPE_SPACE,                 /* 9   09  HT  Horizontal Tab */
    _CTYPE_CNTRL | _CTYPE_SPACE,                 /* 10  0A  LF  Line Feed */
    _CTYPE_CNTRL | _CTYPE_SPACE,                 /* 11  0B  VT  Vertical Tab */
    _CTYPE_CNTRL | _CTYPE_SPACE,                 /* 12  0C  FF  Form Feed */
    _CTYPE_CNTRL | _CTYPE_SPACE,                 /* 13  0D  CR  Carriage Return */
    _CTYPE_CNTRL,                                /* 14  0E  SO  Shift Out / X-On */
    _CTYPE_CNTRL,                                /* 15  0F  SI  Shift In / X-Off */
    0,                                           /* 16  10 DLE  Data Line Escape */
    0,                                           /* 17  11 DC1  Device Control 1 (oft. XON) */
    0,                                           /* 18  12 DC2  Device Control 2 */
    0,                                           /* 19  13 DC3  Device Control 3 (oft. XOFF) */
    0,                                           /* 20  14 DC4  Device Control 4 */
    0,                                           /* 21  15 NAK  Negative Acknowledgement */
    0,                                           /* 22  16 SYN  Synchronous Idle */
    0,                                           /* 23  17 ETB  End of Transmit Block */
    0,                                           /* 24  18 CAN  Cancel */
    0,                                           /* 25  19  EM  End of Medium */
    0,                                           /* 26  1A SUB  Substitute */
    0,                                           /* 27  1B ESC  Escape */
    0,                                           /* 28  1C  FS  File Separator */
    0,                                           /* 29  1D  GS  Group Separator */
    0,                                           /* 30  1E  RS  Record Separator */
    0,                                           /* 31  1F  US  Unit Separator */
    _CTYPE_PRINT | _CTYPE_SPACE,                 /* 32  20      Space */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 33  21 !    Exclamation mark */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 34  22 "    Double quotes (or speech marks) */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 35  23 #    Number */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 36  24 $    Dollar */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 37  25 %    Procenttecken */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 38  26 &    Ampersand */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 39  27 '    Single quote */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 40  28 (    Open parenthesis (or open bracket) */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 41  29 )    Close parenthesis (or close bracket) */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 42  2A *    Asterisk */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 43  2B +    Plus */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 44  2C ,    Comma */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 45  2D -    Hyphen */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 46  2E .    Period, dot or full stop */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 47  2F /    Slash or divide */
    _CTYPE_PRINT | _CTYPE_DIGIT,                 /* 48  30 0    Zero */
    _CTYPE_PRINT | _CTYPE_DIGIT,                 /* 49  31 1    One */
    _CTYPE_PRINT | _CTYPE_DIGIT,                 /* 50  32 2    Two */
    _CTYPE_PRINT | _CTYPE_DIGIT,                 /* 51  33 3    Three */
    _CTYPE_PRINT | _CTYPE_DIGIT,                 /* 52  34 4    Four */
    _CTYPE_PRINT | _CTYPE_DIGIT,                 /* 53  35 5    Five */
    _CTYPE_PRINT | _CTYPE_DIGIT,                 /* 54  36 6    Six */
    _CTYPE_PRINT | _CTYPE_DIGIT,                 /* 55  37 7    Seven */
    _CTYPE_PRINT | _CTYPE_DIGIT,                 /* 56  38 8    Eight */
    _CTYPE_PRINT | _CTYPE_DIGIT,                 /* 57  39 9    Nine */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 58  3A :    Colon */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 59  3B ;    Semicolon */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 60  3C <    Less than (or open angled bracket) */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 61  3D =    Equals */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 62  3E >    Greater than (or close angled bracket) */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 63  3F ?    Question mark */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 64  40 @    At symbol */
    _CTYPE_PRINT | _CTYPE_UPPER | _CTYPE_XDIGIT, /* 65  41 A    Uppercase A */
    _CTYPE_PRINT | _CTYPE_UPPER | _CTYPE_XDIGIT, /* 66  42 B    Uppercase B */
    _CTYPE_PRINT | _CTYPE_UPPER | _CTYPE_XDIGIT, /* 67  43 C    Uppercase C */
    _CTYPE_PRINT | _CTYPE_UPPER | _CTYPE_XDIGIT, /* 68  44 D    Uppercase D */
    _CTYPE_PRINT | _CTYPE_UPPER | _CTYPE_XDIGIT, /* 69  45 E    Uppercase E */
    _CTYPE_PRINT | _CTYPE_UPPER | _CTYPE_XDIGIT, /* 70  46 F    Uppercase F */
    _CTYPE_PRINT | _CTYPE_UPPER,                 /* 71  47 G    Uppercase G */
    _CTYPE_PRINT | _CTYPE_UPPER,                 /* 72  48 H    Uppercase H */
    _CTYPE_PRINT | _CTYPE_UPPER,                 /* 73  49 I    Uppercase I */
    _CTYPE_PRINT | _CTYPE_UPPER,                 /* 74  4A J    Uppercase J */
    _CTYPE_PRINT | _CTYPE_UPPER,                 /* 75  4B K    Uppercase K */
    _CTYPE_PRINT | _CTYPE_UPPER,                 /* 76  4C L    Uppercase L */
    _CTYPE_PRINT | _CTYPE_UPPER,                 /* 77  4D M    Uppercase M */
    _CTYPE_PRINT | _CTYPE_UPPER,                 /* 78  4E N    Uppercase N */
    _CTYPE_PRINT | _CTYPE_UPPER,                 /* 79  4F O    Uppercase O */
    _CTYPE_PRINT | _CTYPE_UPPER,                 /* 80  50 P    Uppercase P */
    _CTYPE_PRINT | _CTYPE_UPPER,                 /* 81  51 Q    Uppercase Q */
    _CTYPE_PRINT | _CTYPE_UPPER,                 /* 82  52 R    Uppercase R */
    _CTYPE_PRINT | _CTYPE_UPPER,                 /* 83  53 S    Uppercase S */
    _CTYPE_PRINT | _CTYPE_UPPER,                 /* 84  54 T    Uppercase T */
    _CTYPE_PRINT | _CTYPE_UPPER,                 /* 85  55 U    Uppercase U */
    _CTYPE_PRINT | _CTYPE_UPPER,                 /* 86  56 V    Uppercase V */
    _CTYPE_PRINT | _CTYPE_UPPER,                 /* 87  57 W    Uppercase W */
    _CTYPE_PRINT | _CTYPE_UPPER,                 /* 88  58 X    Uppercase X */
    _CTYPE_PRINT | _CTYPE_UPPER,                 /* 89  59 Y    Uppercase Y */
    _CTYPE_PRINT | _CTYPE_UPPER,                 /* 90  5A Z    Uppercase Z */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 91  5B [    Opening bracket */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 92  5C \    Backslash */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 93  5D ]    Closing bracket */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 94  5E ^    Caret - circumflex */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 95  5F _    Underscore */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 96  60 `    Grave accent */
    _CTYPE_PRINT | _CTYPE_LOWER | _CTYPE_XDIGIT, /* 97  61 a    Lowercase a */
    _CTYPE_PRINT | _CTYPE_LOWER | _CTYPE_XDIGIT, /* 98  62 b    Lowercase b */
    _CTYPE_PRINT | _CTYPE_LOWER | _CTYPE_XDIGIT, /* 99  63 c    Lowercase c */
    _CTYPE_PRINT | _CTYPE_LOWER | _CTYPE_XDIGIT, /* 100 64 d    Lowercase d */
    _CTYPE_PRINT | _CTYPE_LOWER | _CTYPE_XDIGIT, /* 101 65 e    Lowercase e */
    _CTYPE_PRINT | _CTYPE_LOWER | _CTYPE_XDIGIT, /* 102 66 f    Lowercase f */
    _CTYPE_PRINT | _CTYPE_LOWER,                 /* 103 67 g    Lowercase g */
    _CTYPE_PRINT | _CTYPE_LOWER,                 /* 104 68 h    Lowercase h */
    _CTYPE_PRINT | _CTYPE_LOWER,                 /* 105 69 i    Lowercase i */
    _CTYPE_PRINT | _CTYPE_LOWER,                 /* 106 6A j    Lowercase j */
    _CTYPE_PRINT | _CTYPE_LOWER,                 /* 107 6B k    Lowercase k */
    _CTYPE_PRINT | _CTYPE_LOWER,                 /* 108 6C l    Lowercase l */
    _CTYPE_PRINT | _CTYPE_LOWER,                 /* 109 6D m    Lowercase m */
    _CTYPE_PRINT | _CTYPE_LOWER,                 /* 110 6E n    Lowercase n */
    _CTYPE_PRINT | _CTYPE_LOWER,                 /* 111 6F o    Lowercase o */
    _CTYPE_PRINT | _CTYPE_LOWER,                 /* 112 70 p    Lowercase p */
    _CTYPE_PRINT | _CTYPE_LOWER,                 /* 113 71 q    Lowercase q */
    _CTYPE_PRINT | _CTYPE_LOWER,                 /* 114 72 r    Lowercase r */
    _CTYPE_PRINT | _CTYPE_LOWER,                 /* 115 73 s    Lowercase s */
    _CTYPE_PRINT | _CTYPE_LOWER,                 /* 116 74 t    Lowercase t */
    _CTYPE_PRINT | _CTYPE_LOWER,                 /* 117 75 u    Lowercase u */
    _CTYPE_PRINT | _CTYPE_LOWER,                 /* 118 76 v    Lowercase v */
    _CTYPE_PRINT | _CTYPE_LOWER,                 /* 119 77 w    Lowercase w */
    _CTYPE_PRINT | _CTYPE_LOWER,                 /* 120 78 x    Lowercase x */
    _CTYPE_PRINT | _CTYPE_LOWER,                 /* 121 79 y    Lowercase y */
    _CTYPE_PRINT | _CTYPE_LOWER,                 /* 122 7A z    Lowercase z */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 123 7B {    Opening brace */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 124 7C |    Vertical bar */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 125 7D }    Closing brace */
    _CTYPE_PRINT | _CTYPE_PUNCT,                 /* 126 7E ~    Equivalency sign - tilde */
    0,                                           /* 127 7F      Delete */
    0,                                           /* 128 80 Å Euro sign */
    0,                                           /* 129 81        */
    0,                                           /* 130 82 '    Single low-9 quotation mark */
    0,                                           /* 131 83 Å Latin small letter f with hook */
    0,                                           /* 132 84 "    Double low-9 quotation mark */
    0,                                           /* 133 85 Å Horizontal ellipsis */
    0,                                           /* 134 86 Å Dagger */
    0,                                           /* 135 87 Å Double dagger */
    0,                                           /* 136 88 Å Modifier letter circumflex accent */
    0,                                           /* 137 89 Å Per mille sign */
    0,                                           /* 138 8A Å Latin capital letter S with caron */
    0,                                           /* 139 8B Å Single left-pointing angle quotation */
    0,                                           /* 140 8C Å Latin capital ligature OE */
    0,                                           /* 141 8D        */
    0,                                           /* 142 8E Å Latin captial letter Z with caron */
    0,                                           /* 143 8F        */
    0,                                           /* 144 90        */
    0,                                           /* 145 91 '    Left single quotation mark */
    0,                                           /* 146 92 '    Right single quotation mark */
    0,                                           /* 147 93 "    Left double quotation mark */
    0,                                           /* 148 94 "    Right double quotation mark */
    0,                                           /* 149 95 Å Bullet */
    0,                                           /* 150 96 Å En dash */
    0,                                           /* 151 97 Å Em dash */
    0,                                           /* 152 98 Å Small tilde */
    0,                                           /* 153 99 Å Trade mark sign */
    0,                                           /* 154 9A Å Latin small letter S with caron */
    0,                                           /* 155 9B Å Single right-pointing angle quotation mark */
    0,                                           /* 156 9C Å Latin small ligature oe */
    0,                                           /* 157 9D        */
    0,                                           /* 158 9E Å Latin small letter z with caron */
    0,                                           /* 159 9F Å Latin capital letter Y with diaeresis */
    0,                                           /* 160 A0      Non-breaking space */
    0,                                           /* 161 A1 °    Inverted exclamation mark */
    0,                                           /* 162 A2 ¢    Cent sign */
    0,                                           /* 163 A3 £    Pound sign */
    0,                                           /* 164 A4 §    Currency sign */
    0,                                           /* 165 A5 •    Yen sign */
    0,                                           /* 166 A6 ¶    Pipe, Broken vertical bar */
    0,                                           /* 167 A7 ß    Section sign */
    0,                                           /* 168 A8 ®    Spacing diaeresis - umlaut */
    0,                                           /* 169 A9 ©    Copyright sign */
    0,                                           /* 170 AA ™    Feminine ordinal indicator */
    0,                                           /* 171 AB ´    Left double angle quotes */
    0,                                           /* 172 AC ¨    Not sign */
    0,                                           /* 173 AD      Soft hyphen */
    0,                                           /* 174 AE Æ    Registered trade mark sign */
    0,                                           /* 175 AF Ø    Spacing macron - overline */
    0,                                           /* 176 B0 ∞    Degree sign */
    0,                                           /* 177 B1 ±    Plus-or-minus sign */
    0,                                           /* 178 B2 ≤    Superscript two - squared */
    0,                                           /* 179 B3 ≥    Superscript three - cubed */
    0,                                           /* 180 B4 ¥    Acute accent - spacing acute */
    0,                                           /* 181 B5 µ    Micro sign */
    0,                                           /* 182 B6 ∂    Pilcrow sign - paragraph sign */
    0,                                           /* 183 B7 ∑    Middle dot - Georgian comma */
    0,                                           /* 184 B8 ∏    Spacing cedilla */
    0,                                           /* 185 B9 π    Superscript one */
    0,                                           /* 186 BA ∫    Masculine ordinal indicator */
    0,                                           /* 187 BB ª    Right double angle quotes */
    0,                                           /* 188 BC º     one quarter */
    0,                                           /* 189 BD Ω    Fraction one half */
    0,                                           /* 190 BE æ    Fraction three quarters */
    0,                                           /* 191 BF ø    Inverted question mark */
    0,                                           /* 192 C0 ¿    Latin capital letter A with grave */
    0,                                           /* 193 C1 ¡    Latin capital letter A with acute */
    0,                                           /* 194 C2 ¬    Latin capital letter A with circumflex */
    0,                                           /* 195 C3 √    Latin capital letter A with tilde */
    0,                                           /* 196 C4 ƒ    Latin capital letter A with diaeresis */
    0,                                           /* 197 C5 ≈    Latin capital letter A with ring above */
    0,                                           /* 198 C6 ∆    Latin capital letter AE */
    0,                                           /* 199 C7 «    Latin capital letter C with cedilla */
    0,                                           /* 200 C8 »    Latin capital letter E with grave */
    0,                                           /* 201 C9 …    Latin capital letter E with acute */
    0,                                           /* 202 CA      Latin capital letter E with circumflex */
    0,                                           /* 203 CB À    Latin capital letter E with diaeresis */
    0,                                           /* 204 CC Ã    Latin capital letter I with grave */
    0,                                           /* 205 CD Õ    Latin capital letter I with acute */
    0,                                           /* 206 CE Œ    Latin capital letter I with circumflex */
    0,                                           /* 207 CF œ    Latin capital letter I with diaeresis */
    0,                                           /* 208 D0 –    Latin capital letter ETH */
    0,                                           /* 209 D1 —    Latin capital letter N with tilde */
    0,                                           /* 210 D2 “    Latin capital letter O with grave */
    0,                                           /* 211 D3 ”    Latin capital letter O with acute */
    0,                                           /* 212 D4 ‘    Latin capital letter O with circumflex */
    0,                                           /* 213 D5 ’    Latin capital letter O with tilde */
    0,                                           /* 214 D6 ÷    Latin capital letter O with diaeresis */
    0,                                           /* 215 D7 ◊    Multiplication sign */
    0,                                           /* 216 D8 ÿ    Latin capital letter O with slash */
    0,                                           /* 217 D9 Ÿ    Latin capital letter U with grave */
    0,                                           /* 218 DA ⁄    Latin capital letter U with acute */
    0,                                           /* 219 DB €    Latin capital letter U with circumflex */
    0,                                           /* 220 DC ‹    Latin capital letter U with diaeresis */
    0,                                           /* 221 DD ›    Latin capital letter Y with acute */
    0,                                           /* 222 DE ﬁ    Latin capital letter THORN */
    0,                                           /* 223 DF ﬂ    Latin small letter sharp s - ess-zed */
    0,                                           /* 224 E0 ‡    Latin small letter a with grave */
    0,                                           /* 225 E1 ·    Latin small letter a with acute */
    0,                                           /* 226 E2 ‚    Latin small letter a with circumflex */
    0,                                           /* 227 E3 „    Latin small letter a with tilde */
    0,                                           /* 228 E4 ‰    Latin small letter a with diaeresis */
    0,                                           /* 229 E5 Â    Latin small letter a with ring above */
    0,                                           /* 230 E6 Ê    Latin small letter ae */
    0,                                           /* 231 E7 Á    Latin small letter c with cedilla */
    0,                                           /* 232 E8 Ë    Latin small letter e with grave */
    0,                                           /* 233 E9 È    Latin small letter e with acute */
    0,                                           /* 234 EA Í    Latin small letter e with circumflex */
    0,                                           /* 235 EB Î    Latin small letter e with diaeresis */
    0,                                           /* 236 EC Ï    Latin small letter i with grave */
    0,                                           /* 237 ED Ì    Latin small letter i with acute */
    0,                                           /* 238 EE Ó    Latin small letter i with circumflex */
    0,                                           /* 239 EF Ô    Latin small letter i with diaeresis */
    0,                                           /* 240 F0     Latin small letter eth */
    0,                                           /* 241 F1 Ò    Latin small letter n with tilde */
    0,                                           /* 242 F2 Ú    Latin small letter o with grave */
    0,                                           /* 243 F3 Û    Latin small letter o with acute */
    0,                                           /* 244 F4 Ù    Latin small letter o with circumflex */
    0,                                           /* 245 F5 ı    Latin small letter o with tilde */
    0,                                           /* 246 F6 ˆ    Latin small letter o with diaeresis */
    0,                                           /* 247 F7 ˜    Division sign */
    0,                                           /* 248 F8 ¯    Latin small letter o with slash */
    0,                                           /* 249 F9 ˘    Latin small letter u with grave */
    0,                                           /* 250 FA ˙    Latin small letter u with acute */
    0,                                           /* 251 FB ˚    Latin small letter u with circumflex */
    0,                                           /* 252 FC ¸    Latin small letter u with diaeresis */
    0,                                           /* 253 FD ˝    Latin small letter y with acute */
    0,                                           /* 254 FE ˛    Latin small letter thorn */
    0,                                           /* 255 FF ˇ    Latin small letter y with diaeresis */
};
