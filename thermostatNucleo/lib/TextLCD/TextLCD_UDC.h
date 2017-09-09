/* mbed TextLCD Library, for LCDs based on HD44780 controllers
 * Copyright (c) 2014, WH
 *               2014, v01: WH, Extracted from TextLCD.h as of v14
 *               2015, v02: WH, Added some UDC definitions and commented out several UDCs
 *               2015, v03: WH, Added some UDC definitions 
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
#ifndef MBED_TEXTLCDUDC_H
#define MBED_TEXTLCDUDC_H

#include "TextLCD_Config.h"

/** Some sample User Defined Chars 5x7 dots */
#if(LCD_UDC == 1)
//extern const char udc_ae[];      //æ
//extern const char udc_0e[];      //ø
//extern const char udc_ao[];      //å
//extern const char udc_AE[];      //Æ
//extern const char udc_0E[];      //Ø
//extern const char udc_Ao[];      //Å
//extern const char udc_PO[];      //Padlock Open
//extern const char udc_PC[];      //Padlock Closed

//extern const char udc_alpha[];  //alpha
//extern const char udc_ohm[];    //ohm
//extern const char udc_sigma[];  //sigma
//extern const char udc_pi[];     //pi
//extern const char udc_root[];   //root

extern const char udc_0[];       // |>
extern const char udc_1[];       // <|
extern const char udc_2[];       // |
extern const char udc_3[];       // ||
extern const char udc_4[];       // |||
extern const char udc_5[];       // =
extern const char udc_6[];       // checkerboard
extern const char udc_7[];       // \

//extern const char udc_degr[];    // Degree symbol

//extern const char udc_TM_T[];    // Trademark T
//extern const char udc_TM_M[];    // Trademark M

//extern const char udc_Bat_Hi[];  // Battery Full
//extern const char udc_Bat_Ha[];  // Battery Half
//extern const char udc_Bat_Lo[];  // Battery Low
extern const char udc_Bat_Hi[];  // Battery Full
extern const char udc_Bat_Ha[];  // Battery Half
extern const char udc_Bat_Lo[];  // Battery Low
extern const char udc_AC[];      // AC Power

extern const char udc_smiley[];  // Smiley
//extern const char udc_droopy[];   // Droopey
//extern const char udc_note[];     // Note
//extern const char udc_note_off[]; // Note Off

//extern const char udc_bar_1[];   // Bar 1
//extern const char udc_bar_2[];   // Bar 11
//extern const char udc_bar_3[];   // Bar 111
//extern const char udc_bar_4[];   // Bar 1111
//extern const char udc_bar_5[];   // Bar 11111

//extern const char udc_ch_1[];    // Hor bars 4
//extern const char udc_ch_2[];    // Hor bars 4 (inverted)
//extern const char udc_ch_3[];    // Ver bars 3
//extern const char udc_ch_4[];    // Ver bars 3 (inverted)
//extern const char udc_ch_yr[];   // Year   (kana)
//extern const char udc_ch_mo[];   // Month  (kana)
//extern const char udc_ch_dy[];   // Day    (kana)
//extern const char udc_ch_mi[];   // minute (kana)

//extern const char udc_bell[];
//extern const char udc_note[];
//extern const char udc_clock[];
//extern const char udc_heart[];
//extern const char udc_duck[];
//extern const char udc_check[];
//extern const char udc_cross[];
//extern const char udc_retarrow[];
//extern const char udc_OK[];       // Ok
//extern const char udc_1_2[];      // 1/2
//extern const char udc_Euro[];     // Euro symbol
//extern const char udc_key[];      // Key symbol

//extern const char udc_None[]; 
//extern const char udc_All[];
#endif /* LCD_UDC Examples */

#endif // MBED_TEXTLCDUDC_H