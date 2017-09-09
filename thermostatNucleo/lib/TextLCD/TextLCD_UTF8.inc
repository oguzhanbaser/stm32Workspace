/* mbed TextLCD Library, for LCDs based on HD44780 controllers
 * Copyright (c) 2015, WH
 *               2015, v01: WH, AR. Added UTF8 decode tables for Cyrilic font (by Andriy Ribalko).
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef MBED_TEXTLCDUTF8_INC
#define MBED_TEXTLCDUTF8_INC

#include "TextLCD_Config.h"

#if(LCD_UTF8_FONT == 1)
// Code by Andriy Ribalko
// UTF8 conversion, please add other tables for your language
// See wikipedia.org/wiki/UTF-8 and www.utf8-chartable.de

//The two tables below are used to map UTF8 codes onto character codes for an LCD controller that
//supports a specific fonttable. The UTF codes for a specific language like Cyrilic are between 0x0400 and 0x04FF.

//Select one table for a specific controller and language

#if(0)
//Table for controller xxxx
//The two tables below are used to map Cyrilic/Russian UTF8 codes onto character codes for an LCD controller that
//supports a Cyrilic fonttable. The UTF codes for Cyrilic are between 0x0400 and 0x04FF

#define UTF_FIRST         0x0400
#define UTF_LAST          0x04FF
#define UTF_SEQ_REC_FIRST utf_seq_rec_first_cyr 
#define UTF_SEQ_REC_LAST  utf_seq_rec_last_cyr 
#define UTF_SEQ_RECODE    utf_seq_recode_cyr
#define UTF_RND_RECODE    utf_rnd_recode_cyr

#define utf_seq_rec_first_cyr 0x0410  //UTF code of first symbol in sequential table UTF_recode
#define utf_seq_rec_last_cyr  0x044F  //UTF code of last symbol in sequential table UTF_recode

const char utf_seq_recode_cyr[] = {
                                0x41,0xa0,0x42,0xa1, 0xe0,0x45,0xa3,0xa4, 0xa5,0xa6,0x4b,0xa7, 0x4d,0x48,0x4f,0xa8,   //Upper case Cyrillic
                                0x50,0x43,0x54,0xa9, 0xaa,0x58,0xe1,0xab, 0xac,0xe2,0xad,0xae, 0x62,0xaf,0xb0,0xb1,
                                0x61,0xb2,0xb3,0xb4, 0xe3,0x65,0xb6,0xb7, 0xb8,0xb9,0xba,0xbb, 0xbc,0xbd,0x6f,0xbe,   //Lower case Cyrillic
                                0x70,0x63,0xbf,0x79, 0xe4,0x78,0xe5,0xc0, 0xc1,0xe6,0xc2,0xc3, 0xc4,0xc5,0xc6,0xc7
                              };

//Two dimensional table for some non-sequential symbol decoding (RUS/UKR)
//U+0401 --> 0xa2 (Ё), U+0451 --> 0xb5 (ё), U+0406 --> 0x49 (І), U+0456 -->  0x69 (і) 
const short int utf_rnd_recode_cyr [5][2]= {
                                                {0x0401, 0xa2},
                                                {0x0451, 0xb5},
                                                {0x0406, 0x49},
                                                {0x0456, 0x69},
                                                {0}                  //Last element table zero
                                           };
#endif

#if(LCD_UTF8_CYR_B == 1)
//ROM_B Table for controller SSD1803 and US2066 
//The two tables below are used to map Cyrilic/Russian UTF8 codes onto character codes for an LCD controller that
//supports a Cyrilic fonttable. The UTF codes for Cyrilic are between 0x0400 and 0x04FF

#define UTF_FIRST         0x0400
#define UTF_LAST          0x04FF
#define UTF_SEQ_REC_FIRST utf_seq_rec_first_cyr 
#define UTF_SEQ_REC_LAST  utf_seq_rec_last_cyr 
#define UTF_SEQ_RECODE    utf_seq_recode_cyr
#define UTF_RND_RECODE    utf_rnd_recode_cyr

#define utf_seq_rec_first_cyr 0x0410  //UTF code of first symbol in sequential table UTF_recode
#define utf_seq_rec_last_cyr  0x044F  //UTF code of last symbol in sequential table UTF_recode
const char utf_seq_recode_cyr[] = {
                                0x80,0x81,0x82,0x83, 0x84,0x85,0x86,0x87, 0x88,0x89,0x8A,0x8B, 0x8C,0x8D,0x8E,0x8F,  //Upper case Cyrillic
                                0x90,0x91,0x92,0x93, 0x94,0x95,0x96,0x97, 0x98,0x99,0x9A,0x9B, 0x9C,0x9D,0x9E,0x9F,
                                0x61,0x81,0x62,0x83, 0x84,0x65,0x86,0x87, 0x88,0x89,0x6B,0x8B, 0x6D,0x69,0x6F,0x8F,  //Lower case Cyrillic (~Upper) 
                                0x70,0x63,0x92,0x79, 0x94,0x95,0x96,0x97, 0x98,0x99,0x9A,0x9B, 0x9C,0x9D,0x9E,0x9F
                              };
 
//Two dimensional table for some non-sequential symbol decoding (RUS/UKR)
//U+0400 --> 0xC8 (E)
//U+0401 --> 0xCB (Ё)
//U+0405 --> 0x53 (S)
//U+0406 --> 0x49 (І)
//U+0407 --> 0xCF (І)
//U+0408 --> 0x4A (J)
//U+0450 --> 0xE8 ( )
//U+0451 --> 0xEB (ё)
//U+0456 --> 0x69 (і) 
//U+0457 --> 0xCF (і) 
//U+0458 --> 0x6A (j) 
const short int utf_rnd_recode_cyr [][2]=  {
                                                {0x0400, 0xC8},
                                                {0x0401, 0xCB},
                                                {0x0405, 0x53},
                                                {0x0406, 0x49},
                                                {0x0407, 0xCF},
                                                {0x0408, 0x4A},
                                                {0x0450, 0xE8},
                                                {0x0451, 0xEB},
                                                {0x0456, 0x69},
                                                {0x0457, 0xCF},
                                                {0x0458, 0x6A},
                                                {0     ,    0}   //Last element table zero
                                           };
#endif


//end UTF conversion
#endif

#endif