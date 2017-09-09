/* mbed TextLCD Library, for LCDs based on HD44780 controllers
 * Copyright (c) 2007-2010, sford, http://mbed.org
 *               2013, v01: WH, Added LCD types, fixed LCD address issues, added Cursor and UDCs 
 *               2013, v02: WH, Added I2C and SPI bus interfaces
 *               2013, v03: WH, Added support for LCD40x4 which uses 2 controllers   
 *               2013, v04: WH, Added support for Display On/Off, improved 4bit bootprocess  
 *               2013, v05: WH, Added support for 8x2B, added some UDCs  
 *               2013, v06: WH, Added support for devices that use internal DC/DC converters 
 *               2013, v07: WH, Added support for backlight and include portdefinitions for LCD2004 Module from DFROBOT
 *               2014, v08: WH, Refactored in Base and Derived Classes to deal with mbed lib change regarding 'NC' defined DigitalOut pins
 *               2014, v09: WH/EO, Added Class for Native SPI controllers such as ST7032 
 *               2014, v10: WH, Added Class for Native I2C controllers such as ST7032i, Added support for MCP23008 I2C portexpander, Added support for Adafruit module  
 *               2014, v11: WH, Added support for native I2C controllers such as PCF21XX, Improved the _initCtrl() method to deal with differences between all supported controllers  
 *               2014, v12: WH, Added support for native I2C controller PCF2119 and native I2C/SPI controllers SSD1803, ST7036, added setContrast method (by JH1PJL) for supported devices (eg ST7032i) 
 *               2014, v13: WH, Added support for controllers US2066/SSD1311 (OLED), added setUDCBlink() method for supported devices (eg SSD1803), fixed issue in setPower() 
 *               2014, v14: WH, Added support for PT6314 (VFD), added setOrient() method for supported devices (eg SSD1803, US2066), added Double Height lines for supported devices, 
 *                              added 16 UDCs for supported devices (eg PCF2103), moved UDC defines to TextLCD_UDC file, added TextLCD_Config.h for feature and footprint settings.
 *               2014, v15: WH, Added AC780 support, added I2C expander modules, fixed setBacklight() for inverted logic modules. Fixed bug in LCD_SPI_N define 
 *               2014, v16: WH, Added ST7070 and KS0073 support, added setIcon(), clrIcon() and setInvert() method for supported devices  
 *               2015, v17: WH, Clean up low-level _writeCommand() and _writeData(), Added support for alternative fonttables (eg PCF21XX), Added ST7066_ACM controller for ACM1602 module
 *               2015, v18: WH, Performance improvement I2C portexpander
 *               2015, v19: WH, Added 10x2D and 10x4D type for SSD1803 
 *               2015, v20: WH, Fixed occasional Init fail caused by insufficient wait time after ReturnHome command (0x02), Added defines to reduce memory footprint (eg LCD_ICON),
 *                              Fixed and Added more fonttable support for PCF2119R_3V3, Added HD66712 controller.
 *               2015, v21: WH, Added LCD32x2 defines and code, Fixed LCD12x4D enum, Added font enums, Added SPLC792A controller,
 *                              Added UTF8_2_LCD decode for Cyrilic font (By Andriy Ribalko). Added setFont()
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

#ifndef MBED_TEXTLCD_H
#define MBED_TEXTLCD_H

#include "mbed.h"
#include "TextLCD_Config.h"
#include "TextLCD_UDC.h"

/** A TextLCD interface for driving 4-bit HD44780-based LCDs
 *
 * Currently supports 8x1, 8x2, 12x3, 12x4, 16x1, 16x2, 16x3, 16x4, 20x2, 20x4, 24x1, 24x2, 24x4, 40x2 and 40x4 panels.
 * Interface options include direct mbed pins, I2C portexpander (PCF8474/PCF8574A or MCP23008) or SPI bus shiftregister (74595). 
 * Supports some controllers with native I2C or SPI interface. Supports some controllers that provide internal DC/DC converters for VLCD or VLED. 
 * Supports some controllers that feature programmable contrast control, powerdown, blinking UDCs and/or top/down orientation modes.
 *
 * @code
 * #include "mbed.h"
 * #include "TextLCD.h"
 * 
 * // I2C Communication
 * I2C i2c_lcd(p28,p27); // SDA, SCL
 *
 * // SPI Communication
 * SPI spi_lcd(p5, NC, p7); // MOSI, MISO, SCLK
 *
 * //TextLCD lcd(p15, p16, p17, p18, p19, p20);                          // RS, E, D4-D7, LCDType=LCD16x2, BL=NC, E2=NC, LCDTCtrl=HD44780
 * //TextLCD_SPI lcd(&spi_lcd, p8, TextLCD::LCD40x4);                    // SPI bus, 74595 expander, CS pin, LCD Type  
 * TextLCD_I2C lcd(&i2c_lcd, 0x42, TextLCD::LCD20x4);                    // I2C bus, PCF8574 Slaveaddress, LCD Type
 * //TextLCD_I2C lcd(&i2c_lcd, 0x42, TextLCD::LCD16x2, TextLCD::WS0010); // I2C bus, PCF8574 Slaveaddress, LCD Type, Device Type (OLED)
 * //TextLCD_SPI_N lcd(&spi_lcd, p8, p9);                                // SPI bus, CS pin, RS pin, LCDType=LCD16x2, BL=NC, LCDTCtrl=ST7032_3V3   
 * //TextLCD_I2C_N lcd(&i2c_lcd, ST7032_SA, TextLCD::LCD16x2, NC, TextLCD::ST7032_3V3);   // I2C bus, Slaveaddress, LCD Type, BL=NC, LCDTCtrl=ST7032_3V3  
 * //TextLCD_SPI_N_3_24 lcd(&spi_lcd, p8, TextLCD::LCD20x4D, NC, TextLCD::SSD1803_3V3);   // SPI bus, CS pin, LCDType=LCD20x4D, BL=NC, LCDTCtrl=SSD1803
 * //TextLCD_SPI_N_3_24 lcd(&spi_lcd, p8, TextLCD::LCD20x2, NC, TextLCD::US2066_3V3);     // SPI bus, CS pin, LCDType=LCD20x2, BL=NC, LCDTCtrl=US2066 (OLED)
 *
 * int main() {
 *   lcd.printf("Hello World!\n");
 * }
 * @endcode
 */

//The TextLCD_Config.h file selects hardware interface options to reduce memory footprint
//and provides Pin Defines for I2C PCF8574/PCF8574A or MCP23008 and SPI 74595 bus expander interfaces.
//The LCD and serial portexpanders should be wired accordingly. 

/* LCD Type information on Rows, Columns and Variant. This information is encoded in
 * an int and used for the LCDType enumerators in order to simplify code maintenance */

// Type ID encoded in b7..b0
#define LCD_T_ID_MSK   0x000000FF
#define LCD_T_ID_SHFT           0

// Columns encoded in b15..b8
#define LCD_T_COL_MSK  0x0000FF00
#define LCD_T_COL_SHFT          8
#define LCD_T_C6       0x00000600
#define LCD_T_C8       0x00000800
#define LCD_T_C10      0x00000A00
#define LCD_T_C12      0x00000C00
#define LCD_T_C16      0x00001000
#define LCD_T_C20      0x00001400
#define LCD_T_C24      0x00001800
#define LCD_T_C32      0x00002000
#define LCD_T_C40      0x00002800
//#define LCD_T_C52      0x00003400

// Rows encoded in b23..b16  
#define LCD_T_ROW_MSK  0x00FF0000
#define LCD_T_ROW_SHFT         16
#define LCD_T_R1       0x00010000
#define LCD_T_R2       0x00020000
#define LCD_T_R3       0x00030000
#define LCD_T_R4       0x00040000
#define LCD_T_R6       0x00060000
  
// Addressing mode encoded in b27..b24
#define LCD_T_ADR_MSK  0x0F000000
#define LCD_T_ADR_SHFT         24
#define LCD_T_A        0x00000000  /*Mode A   Default 1, 2 or 4 line display                       */
#define LCD_T_B        0x01000000  /*Mode B,  Alternate 8x2 (actually 16x1 display)                */
#define LCD_T_C        0x02000000  /*Mode C,  Alternate 16x1 (actually 8x2 display)                */
#define LCD_T_D        0x03000000  /*Mode D,  Alternate 3 or 4 line display (12x4, 20x4, 24x4)     */
#define LCD_T_D1       0x04000000  /*Mode D1, Alternate 3 out of 4 line display (12x3, 20x3, 24x3) */
#define LCD_T_E        0x05000000  /*Mode E,  40x4 display (actually two 40x2)                     */
#define LCD_T_F        0x06000000  /*Mode F,  16x3 display (actually 24x2)                         */
#define LCD_T_G        0x07000000  /*Mode G,  16x3 display                                         */

/* LCD Ctrl information on interface support and features. This information is encoded in
 * an int and used for the LCDCtrl enumerators in order to simplify code maintenance */
// Interface encoded in b31..b24
#define LCD_C_BUS_MSK  0xFF000000
#define LCD_C_BUS_SHFT         24
#define LCD_C_PAR      0x01000000  /*Parallel 4 or 8 bit data, E pin, RS pin, RW=GND            */
#define LCD_C_SPI3_8   0x02000000  /*SPI 3 line (MOSI, SCL, CS pins),  8 bits (Count Command initiates Data transfer) */
#define LCD_C_SPI3_9   0x04000000  /*SPI 3 line (MOSI, SCL, CS pins),  9 bits (RS + 8 Data)     */
#define LCD_C_SPI3_10  0x08000000  /*SPI 3 line (MOSI, SCL, CS pins), 10 bits (RS, RW + 8 Data) */
#define LCD_C_SPI3_16  0x10000000  /*SPI 3 line (MOSI, SCL, CS pins), 16 bits (RS, RW + 8 Data) */
#define LCD_C_SPI3_24  0x20000000  /*SPI 3 line (MOSI, SCL, CS pins), 24 bits (RS, RW + 8 Data) */
#define LCD_C_SPI4     0x40000000  /*SPI 4 line (MOSI, SCL, CS, RS pin), RS pin + 8 Data        */
#define LCD_C_I2C      0x80000000  /*I2C (SDA, SCL pin), 8 control bits (Co, RS, RW) + 8 Data   */
// Features encoded in b23..b16
#define LCD_C_FTR_MSK  0x00FF0000 
#define LCD_C_FTR_SHFT         16
#define LCD_C_BST      0x00010000  /*Booster             */
#define LCD_C_CTR      0x00020000  /*Contrast Control    */
#define LCD_C_ICN      0x00040000  /*Icons               */
#define LCD_C_PDN      0x00080000  /*Power Down          */
// Fonttable encoded in b15..b12
#define LCD_C_FNT_MSK  0x0000F000
#define LCD_C_FNT_SHFT         12 
#define LCD_C_F0       0x00000000  /*Default HD44780 UK/JAP    */
#define LCD_C_FC       0x00001000  /*PCF21xxC                  */
#define LCD_C_FR       0x00002000  /*PCF21XXR                  */
#define LCD_C_FK       0x00003000  /*PCF21XXK                  */
#define LCD_C_FRA      0x00004000  /*SSD1803 ROM_A UK/EU       */
#define LCD_C_FRB      0x00005000  /*SSD1803 ROM_B UK/Cyrillic */
#define LCD_C_FRC      0x00006000  /*SSD1803 ROM_C UK/JAP      */
#define LCD_C_FD       0x00007000  /*SPLC780D EU/Cyrillic      */
#define LCD_C_FT       0x00008000  /*ST7066-0T EU/Cyrillic     */

// Type ID encoded in b7..b0
#define LCD_C_ID_MSK   0x000000FF
#define LCD_C_ID_SHFT           0

/** A TextLCD interface for driving 4-bit HD44780-based LCDs
 *
 * @brief Currently supports 8x1, 8x2, 12x2, 12x3, 12x4, 16x1, 16x2, 16x3, 16x4, 20x2, 20x4, 24x2, 24x4, 40x2 and 40x4 panels
 *        Interface options include direct mbed pins, I2C portexpander (PCF8474/PCF8574A or MCP23008) or 
 *        SPI bus shiftregister (74595) or native I2C or SPI interfaces for some supported devices. 
 */
class TextLCD_Base : public Stream {
//class TextLCD_Base {    

//Unfortunately the following #define selection breaks Doxygen !!!
//Add it manually when you want to disable the Stream inheritance
//#if (LCD_PRINTF == 1)
//class TextLCD_Base : public Stream {
//#else    
//class TextLCD_Base {    
//#endif

public:

    /** LCD panel format */
    // The commented out types exist but have not yet been tested with the library
    enum LCDType {
//        LCD6x1     = (LCD_T_A | LCD_T_C6 | LCD_T_R1),     /**<  6x1 LCD panel */
//        LCD6x2     = (LCD_T_A | LCD_T_C6 | LCD_T_R2),     /**<  6x2 LCD panel */
        LCD8x1     = (LCD_T_A | LCD_T_C8 | LCD_T_R1),     /**<  8x1 LCD panel */
        LCD8x2     = (LCD_T_A | LCD_T_C8 | LCD_T_R2),     /**<  8x2 LCD panel */
        LCD8x2B    = (LCD_T_B | LCD_T_C8 | LCD_T_R2),     /**<  8x2 LCD panel (actually 16x1) */
//        LCD10x2D   = (LCD_T_D | LCD_T_C10 | LCD_T_R2),    /**< 10x2 LCD panel, special mode SSD1803, 4-line but double height */        
        LCD10x4D   = (LCD_T_D | LCD_T_C10 | LCD_T_R4),    /**< 10x4 LCD panel, special mode SSD1803 */
        LCD12x1    = (LCD_T_A | LCD_T_C12 | LCD_T_R1),    /**< 12x1 LCD panel */
        LCD12x2    = (LCD_T_A | LCD_T_C12 | LCD_T_R2),    /**< 12x2 LCD panel */
        LCD12x3D   = (LCD_T_D | LCD_T_C12 | LCD_T_R3),    /**< 12x3 LCD panel, special mode PCF21XX, KS0073 */
        LCD12x3D1  = (LCD_T_D1 | LCD_T_C12 | LCD_T_R3),   /**< 12x3 LCD panel, special mode PCF21XX, KS0073 */
//        LCD12x3G   = (LCD_T_G | LCD_T_C12 | LCD_T_R3),    /**< 12x3 LCD panel, special mode ST7036 */
        LCD12x4    = (LCD_T_A | LCD_T_C12 | LCD_T_R4),    /**< 12x4 LCD panel */
        LCD12x4D   = (LCD_T_D | LCD_T_C12 | LCD_T_R4),    /**< 12x4 LCD panel, special mode PCF21XX, KS0073 */
        LCD16x1    = (LCD_T_A | LCD_T_C16 | LCD_T_R1),    /**< 16x1 LCD panel */
        LCD16x1C   = (LCD_T_C | LCD_T_C16 | LCD_T_R1),    /**< 16x1 LCD panel (actually 8x2) */
        LCD16x2    = (LCD_T_A | LCD_T_C16 | LCD_T_R2),    /**< 16x2 LCD panel (default) */
//        LCD16x2B   = (LCD_T_B | LCD_T_C16 | LCD_T_R2),    /**< 16x2 LCD panel, alternate addressing, wrong.. */
        LCD16x3D   = (LCD_T_D | LCD_T_C16 | LCD_T_R3),    /**< 16x3 LCD panel, special mode SSD1803 */           
//        LCD16x3D1  = (LCD_T_D1 | LCD_T_C16 | LCD_T_R3),   /**< 16x3 LCD panel, special mode SSD1803 */
        LCD16x3F   = (LCD_T_F | LCD_T_C16 | LCD_T_R3),    /**< 16x3 LCD panel (actually 24x2) */
        LCD16x3G   = (LCD_T_G | LCD_T_C16 | LCD_T_R3),    /**< 16x3 LCD panel, special mode ST7036 */
        LCD16x4    = (LCD_T_A | LCD_T_C16 | LCD_T_R4),    /**< 16x4 LCD panel */
//        LCD16x4D   = (LCD_T_D | LCD_T_C16 | LCD_T_R4),    /**< 16x4 LCD panel, special mode SSD1803 */
        LCD20x1    = (LCD_T_A | LCD_T_C20 | LCD_T_R1),    /**< 20x1 LCD panel */
        LCD20x2    = (LCD_T_A | LCD_T_C20 | LCD_T_R2),    /**< 20x2 LCD panel */
//        LCD20x3    = (LCD_T_A | LCD_T_C20 | LCD_T_R3),    /**< 20x3 LCD panel */
//        LCD20x3D   = (LCD_T_D | LCD_T_C20 | LCD_T_R3),    /**< 20x3 LCD panel, special mode SSD1803, KS0073 */
//        LCD20x3D1  = (LCD_T_D1 | LCD_T_C20 | LCD_T_R3),   /**< 20x3 LCD panel, special mode SSD1803, KS0073 */
        LCD20x4    = (LCD_T_A | LCD_T_C20 | LCD_T_R4),    /**< 20x4 LCD panel */
        LCD20x4D   = (LCD_T_D | LCD_T_C20 | LCD_T_R4),    /**< 20x4 LCD panel, special mode SSD1803, KS0073 */
//        LCD20x6    = (LCD_T_E | LCD_T_C20 | LCD_T_R6),    /**< 20x6 LCD panel, Two controller version */        
        LCD24x1    = (LCD_T_A | LCD_T_C24 | LCD_T_R1),    /**< 24x1 LCD panel */
        LCD24x2    = (LCD_T_A | LCD_T_C24 | LCD_T_R2),    /**< 24x2 LCD panel */
//        LCD24x3D   = (LCD_T_D | LCD_T_C24 | LCD_T_R3),    /**< 24x3 LCD panel */
//        LCD24x3D1  = (LCD_T_D | LCD_T_C24 | LCD_T_R3),    /**< 24x3 LCD panel */
        LCD24x4D   = (LCD_T_D | LCD_T_C24 | LCD_T_R4),    /**< 24x4 LCD panel, special mode KS0078 */
//        LCD32x1    = (LCD_T_A | LCD_T_C32 | LCD_T_R1),    /**< 32x1 LCD panel */
//        LCD32x1C   = (LCD_T_C | LCD_T_C32 | LCD_T_R1),    /**< 32x1 LCD panel (actually 16x2) */
        LCD32x2    = (LCD_T_A | LCD_T_C32 | LCD_T_R2),    /**< 32x2 LCD panel */
//        LCD32x4    = (LCD_T_A | LCD_T_C32 | LCD_T_R4),    /**< 32x4 LCD panel, Two controller version ? */
//        LCD40x1    = (LCD_T_A | LCD_T_C40 | LCD_T_R1),    /**< 40x1 LCD panel */
//        LCD40x1C   = (LCD_T_C | LCD_T_C40 | LCD_T_R1),    /**< 40x1 LCD panel (actually 20x2) */
        LCD40x2    = (LCD_T_A | LCD_T_C40 | LCD_T_R2),    /**< 40x2 LCD panel */
//        LCD52x1    = (LCD_T_A | LCD_T_C52 | LCD_T_R1),    /**< 52x1 LCD panel, special mode KS0073 */        
#if (LCD_TWO_CTRL == 1)
        LCD40x4    = (LCD_T_E | LCD_T_C40 | LCD_T_R4)     /**< 40x4 LCD panel, Two controller version */
#endif     
    };


    /** LCD Controller Device */
    enum LCDCtrl {
        HD44780         =  0 |  LCD_C_PAR,                                                                    /**<  HD44780 or full equivalent (default)         */    
        AC780           =  1 | (LCD_C_PAR | LCD_C_SPI4 | LCD_C_I2C | LCD_C_PDN),                              /**<  AC780/KS0066i 4/8 bit, SPI, I2C              */
        AIP31068        =  2 | (LCD_C_SPI3_9  | LCD_C_I2C | LCD_C_BST),                                       /**<  AIP31068 I2C, SPI3                           */                           
        KS0073          =  3 | (LCD_C_PAR | LCD_C_SPI3_24 | LCD_C_PDN),                                       /**<  KS0073 4/8 bit, SPI3                         */
        KS0078          =  4 | (LCD_C_PAR | LCD_C_SPI3_24 | LCD_C_PDN),                                       /**<  KS0078  24x4 support, 4/8 bit, SPI3          */                   
        PCF2103_3V3     =  5 | (LCD_C_PAR | LCD_C_I2C),                                                       /**<  PCF2103 3V3 no Booster, 4/8 bit, I2C         */                                   
        PCF2113_3V3     =  6 | (LCD_C_PAR | LCD_C_I2C     | LCD_C_BST | LCD_C_CTR),                           /**<  PCF2113 3V3 with Booster, 4/8 bit, I2C       */                           
        PCF2116_3V3     =  7 | (LCD_C_PAR | LCD_C_I2C     | LCD_C_BST),                                       /**<  PCF2116 3V3 with Booster, 4/8 bit, I2C       */                           
//        PCF2116C_3V3    =    | (LCD_C_PAR | LCD_C_I2C     | LCD_C_BST | LCD_C_FC),                            /**<  PCF2116C 3V3 with Booster, 4/8 bit, I2C       */
//        PCF2116K_3V3    =    | (LCD_C_PAR | LCD_C_I2C     | LCD_C_BST | LCD_C_FK),                            /**<  PCF2116K 3V3 with Booster, 4/8 bit, I2C       */
        PCF2116_5V      =  8 | (LCD_C_PAR | LCD_C_I2C),                                                       /**<  PCF2116 5V no Booster, 4/8 bit, I2C          */
        PCF2116C_5V     =  9 | (LCD_C_PAR | LCD_C_I2C     | LCD_C_BST | LCD_C_FC),                            /**<  PCF2116C 3V3 with Booster, 4/8 bit, I2C       */
        PCF2119_3V3     = 10 | (LCD_C_PAR | LCD_C_I2C     | LCD_C_BST | LCD_C_CTR),                           /**<  PCF2119 3V3 with Booster, 4/8 bit, I2C       */
//        PCF2119C_3V3    = 11 | (LCD_C_PAR | LCD_C_I2C     | LCD_C_BST | LCD_C_CTR | LCD_C_FC),                /**<  PCF2119K 3V3 with Booster, 4/8 bit, I2C       */
        PCF2119R_3V3    = 12 | (LCD_C_PAR | LCD_C_I2C     | LCD_C_BST | LCD_C_CTR | LCD_C_FR),                /**<  PCF2119R 3V3 with Booster, 4/8 bit, I2C       */
//        PCF2119_5V      =    | (LCD_C_PAR | LCD_C_I2C),                                                       /**<  PCF2119 5V no Booster, 4/8 bit, I2C          */
        PT6314          = 13 | (LCD_C_PAR | LCD_C_SPI3_16 | LCD_C_CTR),                                       /**<  PT6314  VFD Controller, 4/8 bit, SPI3        */
//        PT6880          = xx | (LCD_C_PAR | LCD_C_SPI3_16 | LCD_C_CTR),                                       /**<  PT6880  OLED controller, 4/8 bit, SPI3       */        
        SSD1803_3V3     = 14 | (LCD_C_PAR | LCD_C_SPI3_24 | LCD_C_I2C | LCD_C_BST | LCD_C_CTR | LCD_C_PDN),   /**<  SSD1803 3V3 with Booster, 4/8 bit, I2C, SPI3 */
//        SSD1803_5V      = 15 | (LCD_C_PAR | LCD_C_SPI3_24 | LCD_C_I2C | LCD_C_BST | LCD_C_CTR | LCD_C_PDN),   /**<  SSD1803 3V3 with Booster, 4/8 bit, I2C, SPI3 */
        ST7032_3V3      = 16 | (LCD_C_PAR | LCD_C_SPI4    | LCD_C_I2C | LCD_C_BST | LCD_C_CTR),               /**<  ST7032/SPLC792A 3V3 with Booster, 4/8 bit, SPI4, I2C */   
        ST7032_5V       = 17 | (LCD_C_PAR | LCD_C_SPI4    | LCD_C_I2C | LCD_C_CTR),                           /**<  ST7032/SPLC792A 5V no Booster, 4/8 bit, SPI4, I2C    */           
        ST7036_3V3      = 18 | (LCD_C_PAR | LCD_C_SPI4    | LCD_C_I2C | LCD_C_BST | LCD_C_CTR),               /**<  ST7036  3V3 with Booster, 4/8 bit, SPI4, I2C */   
        ST7036_5V       = 19 | (LCD_C_PAR | LCD_C_SPI4    | LCD_C_I2C | LCD_C_BST | LCD_C_CTR),               /**<  ST7036  5V no Booster, 4/8 bit, SPI4, I2C    */   
        ST7066_ACM      = 20 | (LCD_C_PAR | LCD_C_I2C),                                                       /**<  ST7066 4/8 bit, I2C on ACM1602 using a PIC   */        
        ST7070          = 21 | (LCD_C_PAR | LCD_C_SPI3_8 | LCD_C_SPI4),                                       /**<  ST7070 4/8 bit, SPI3                         */
        US2066_3V3      = 22 | (LCD_C_PAR | LCD_C_SPI3_24 | LCD_C_I2C | LCD_C_CTR | LCD_C_PDN),               /**<  US2066/SSD1311 3V3, 4/8 bit, I2C, SPI3 */
        WS0010          = 23 | (LCD_C_PAR | LCD_C_SPI3_10 | LCD_C_PDN),                                       /**<  WS0010/RS0010 OLED Controller, 4/8 bit, SPI3 */    
//        WS0012          = 24 | (LCD_C_PAR | LCD_C_SPI3_10 | LCD_C_I2C | LCD_C_PDN),                           /**<  WS0012 4/8 bit, SPI, I2C                     */
        HD66712         = 25 | (LCD_C_PAR | LCD_C_SPI3_24 | LCD_C_BST | LCD_C_PDN),                           /**<  HD66712 Controller, 4/8 bit, SPI3 */    
        SPLC792A_3V3    = 26 | (LCD_C_PAR | LCD_C_SPI3_9  | LCD_C_SPI4 | LCD_C_I2C | LCD_C_BST | LCD_C_CTR)   /**<  ST7032/SPLC792A 3V3 with Booster, 4/8 bit, SPI3, SPI4, I2C */
    };


    /** LCD Cursor control */
    enum LCDCursor {
        CurOff_BlkOff = 0x00,  /**<  Cursor Off, Blinking Char Off */    
        CurOff_BlkOn  = 0x01,  /**<  Cursor Off, Blinking Char On  */         
        CurOn_BlkOff  = 0x02,  /**<  Cursor On, Blinking Char Off  */   
        CurOn_BlkOn   = 0x03   /**<  Cursor On, Blinking Char On   */
    };

    /** LCD Display control */
    enum LCDMode {
        DispOff = 0x00,  /**<  Display Off */    
        DispOn  = 0x04   /**<  Display On */            
    };

   /** LCD Backlight control */
    enum LCDBacklight {
        LightOff,        /**<  Backlight Off */    
        LightOn          /**<  Backlight On */            
    };

   /** LCD Blink control (UDC), supported for some Controllers */
    enum LCDBlink {
        BlinkOff,        /**<  Blink Off */    
        BlinkOn          /**<  Blink On  */            
    };

   /** LCD Orientation control, supported for some Controllers */
    enum LCDOrient {
        Top,             /**<  Top view */    
        Bottom           /**<  Upside down view */            
    };

#if (LCD_BIGFONT == 1) 
   /** LCD BigFont control, supported for some Controllers */
    enum LCDBigFont {
        None,            /**<  no lines              */    
        TopLine,         /**<  1+2 line              */    
        CenterLine,      /**<  2+3 line              */    
        BottomLine,      /**<  2+3 line or 3+4 line  */
        TopBottomLine    /**<  1+2 line and 3+4 line */
    };
#endif

#if (LCD_FONTSEL == 1) 
   /** LCD setFont control.
       Note: most controllers support only one font and the hardware specific
       fonttable is encoded as part of the controller type number (eg PCF21XXC or PCF21XXR).
       Some controllers support multiple tables that can only be selected by logic levels on a few pins.
       Some controllers also support runtime fontable switching through a specific instruction */
    enum LCDFont {
                               /**< HD44780 fonttable      */
        Font_0  = LCD_C_F0,    /**< default, UK/JAP        */
                               /**< PCF21XX fonttables     */
        Font_C  = LCD_C_FC,    /**< PCF21XXC               */
        Font_R  = LCD_C_FR,    /**< PCF21XXR               */
                               /**< SSD1803 ROM tables     */
        Font_RA = LCD_C_FRA,   /**< UK/EU                  */
        Font_RB = LCD_C_FRB,   /**< UK/Cyrillic            */
        Font_RC = LCD_C_FRC,   /**< default, UK/JAP        */
        Font_D  = LCD_C_FD,    /**< SPLC780D Cyrillic      */
        Font_T  = LCD_C_FT     /**< ST7066-0T Cyrillic     */
    };
#endif

#if ((LCD_C_FONT == 1) || (LCD_R_FONT == 1)) //PCF21xxC or PCF21xxR font
   /** Convert ASCII character code to the LCD fonttable code
     *
     * @param c The character to write to the display
     * @return The character code for the specific fonttable of the controller
     */
   int ASCII_2_LCD (int c);
#endif

#if(LCD_UTF8_FONT == 1)       
    /** Convert UTF8 2-byte character code to the LCD fonttable code
      * @param c The character to write to the display
      * @return character code for the specific fonttable of the controller or -1 if UTF8 code is not yet complete or incorrect
      *
      * Orig by Andriy, Modified by WH
      * 
      * Note: The UTF8 decoding table for a specific controller is defined and selected in file TextLCD_UTF8.inc
      * The table is accessed in this UTF_2_LCD() method through
      *   #define UTF_FIRST, UTF_LAST, UTF_SEQ_REC_FIRST, UTF_SEQ_REC_LAST and 
      *   #define UTF_SEQ_RECODE and UTF_RND_RECODE
      */      
      int UTF_2_LCD (int c);       
#endif
 

#if(LCD_PRINTF != 1)
   /** Write a character to the LCD
     *
     * @param c The character to write to the display
     */
   int putc(int c);

    /** Write a raw string to the LCD
     *
     * @param string text, may be followed by variables to emulate formatting the string.
     *                     However, printf formatting is NOT supported and variables will be ignored! 
     */
    int printf(const char* text, ...);
#else    
#if DOXYGEN_ONLY
    /** Write a character to the LCD
     *
     * @param c The character to write to the display
     */
    int putc(int c);

    /** Write a formatted string to the LCD
     *
     * @param format A printf-style format string, followed by the
     *               variables to use in formatting the string.
     */
    int printf(const char* format, ...);   
#endif
    
#endif    

    /** Locate cursor to a screen column and row
     *
     * @param column  The horizontal position from the left, indexed from 0
     * @param row     The vertical position from the top, indexed from 0
     */
    void locate(int column, int row);

    /** Return the memoryaddress of screen column and row location
     *
     * @param column  The horizontal position from the left, indexed from 0
     * @param row     The vertical position from the top, indexed from 0
     * @return        The memoryaddress of screen column and row location
     */
    int getAddress(int column, int row);     
    
    /** Set the memoryaddress of screen column and row location
     *
     * @param column  The horizontal position from the left, indexed from 0
     * @param row     The vertical position from the top, indexed from 0
     */
    void setAddress(int column, int row);        

    /** Clear the screen and locate to 0,0
     */
    void cls();

    /** Return the number of rows
     *
     * @return  The number of rows
     */
    int rows();
    
    /** Return the number of columns
     *
     * @return  The number of columns
     */  
    int columns();  

    /** Set the Cursormode
     *
     * @param cursorMode  The Cursor mode (CurOff_BlkOff, CurOn_BlkOff, CurOff_BlkOn, CurOn_BlkOn)
     */
    void setCursor(LCDCursor cursorMode);     
    
    /** Set the Displaymode
     *
     * @param displayMode The Display mode (DispOff, DispOn)
     */
    void setMode(LCDMode displayMode);     

    /** Set the Backlight mode
     *
     *  @param backlightMode The Backlight mode (LightOff, LightOn)
     */
    void setBacklight(LCDBacklight backlightMode); 

    /** Set User Defined Characters (UDC)
     *
     * @param unsigned char c   The Index of the UDC (0..7) for HD44780 clones and (0..15) for some more advanced controllers
     * @param char *udc_data    The bitpatterns for the UDC (8 bytes of 5 significant bits for bitpattern and 3 bits for blinkmode (advanced types))       
     */
    void setUDC(unsigned char c, char *udc_data);

#if(LCD_BLINK == 1)
    /** Set UDC Blink and Icon blink
     * setUDCBlink method is supported by some compatible devices (eg SSD1803) 
     *
     * @param blinkMode The Blink mode (BlinkOff, BlinkOn)
     */
    void setUDCBlink(LCDBlink blinkMode);
#endif

#if(LCD_CONTRAST == 1)
    /** Set Contrast
     * setContrast method is supported by some compatible devices (eg ST7032i) that have onboard LCD voltage generation
     * Code imported from fork by JH1PJL
     *
     * @param unsigned char c   contrast data (6 significant bits, valid range 0..63, Value 0 will disable the Vgen)  
     * @return none
     */
    void setContrast(unsigned char c = LCD_DEF_CONTRAST);
#endif

#if(LCD_POWER == 1)
    /** Set Power
     * setPower method is supported by some compatible devices (eg SSD1803) that have power down modes
     *
     * @param bool powerOn  Power on/off   
     * @return none
     */
    void setPower(bool powerOn = true);
#endif

#if(LCD_ORIENT == 1)
    /** Set Orient
     * setOrient method is supported by some compatible devices (eg SSD1803, US2066) that have top/bottom view modes
     *
     * @param LCDOrient orient Orientation 
     * @return none
     */
    void setOrient(LCDOrient orient = Top);
#endif

#if(LCD_BIGFONT == 1)
    /** Set Big Font
     * setBigFont method is supported by some compatible devices (eg SSD1803, US2066) 
     *
     * @param lines  The selected Big Font lines (None, TopLine, CenterLine, BottomLine, TopBottomLine)
     *                                            Double height characters can be shown on lines 1+2, 2+3, 3+4 or 1+2 and 3+4
     *                                            Valid double height lines depend on the LCDs number of rows.
     * @return none     
     */
    void setBigFont(LCDBigFont lines);
#endif

#if(LCD_FONTSEL == 1)
    /** Set Font
     * setFont method is supported by some compatible devices (eg SSD1803, US2066, ST7070) 
     *
     * @param LCDFont font  The selected Font 
     * @return none     
     */
    void setFont(LCDFont font = Font_0);
#endif


#if(LCD_ICON==1)
    /** Set Icons
    *
    * @param unsigned char idx   The Index of the icon pattern (0..15) for KS0073 and similar controllers
    *                            and Index (0..31) for PCF2103 and similar controllers  
    * @param unsigned char data  The bitpattern for the icons (6 lsb for KS0073 bitpattern (5 lsb for KS0078) and 2 msb for blinkmode)       
    *                            The bitpattern for the PCF2103 icons is 5 lsb (UDC 0..2) and 5 lsb for blinkmode (UDC 4..6)         
    */    
    void setIcon(unsigned char idx, unsigned char data);

    /** Clear Icons
     *
     * @param  none
     * @return none
     */
    //@TODO Add support for 40x4 dual controller       
    void clrIcon();
#endif

#if(LCD_INVERT == 1)
   /** Set Invert
     * setInvert method is supported by some compatible devices (eg KS0073) to swap between black and white 
     *
     * @param bool invertOn  Invert on/off   
     * @return none
     */
   //@TODO Add support for 40x4 dual controller  
   void setInvert(bool invertOn);
#endif

protected:

   /** LCD controller select, mainly used for LCD40x4
     */
    enum _LCDCtrl_Idx {
        _LCDCtrl_0,  /*<  Primary */    
        _LCDCtrl_1,  /*<  Secondary */            
    };

    /** LCD Datalength control to select between 4 or 8 bit data/commands, mainly used for native Serial interface */
    enum _LCDDatalength {
        _LCD_DL_4 = 0x00,  /**<  Datalength 4 bit */    
        _LCD_DL_8 = 0x10   /**<  Datalength 8 bit */            
    };

    /** Create a TextLCD_Base interface
      * @brief Base class, can not be instantiated
      *
      * @param type  Sets the panel size/addressing mode (default = LCD16x2)
      * @param ctrl  LCD controller (default = HD44780)           
      */
    TextLCD_Base(LCDType type = LCD16x2, LCDCtrl ctrl = HD44780);
    
    // Stream implementation functions
    virtual int _putc(int value);
    virtual int _getc();

/** Medium level initialisation method for LCD controller
  *  @param _LCDDatalength dl sets the datalength of data/commands
  *  @return none
  */
    void _init(_LCDDatalength dl = _LCD_DL_4);     

/** Low level initialisation method for LCD controller
  *   Set number of lines, fonttype, no cursor etc
  *   The controller is accessed in 4-bit parallel mode either directly via mbed pins or through I2C or SPI expander.
  *   Some controllers also support native I2C or SPI interfaces. 
  *
  *  @param _LCDDatalength dl sets the 4 or 8 bit datalength of data/commands. Required for some native serial modes.
  *  @return none  
  */
    void _initCtrl(_LCDDatalength dl = _LCD_DL_4);    

/** Low level character address set method
  */  
    int  _address(int column, int row);
    
/** Low level cursor enable or disable method
  */  
    void _setCursor(LCDCursor show);

/** Low level method to store user defined characters for current controller
  *
  * @param unsigned char c   The Index of the UDC (0..7) for HD44780 clones and (0..15) for some more advanced controllers 
  * @param char *udc_data    The bitpatterns for the UDC (8 bytes of 5 significant bits)     
  */     
    void _setUDC(unsigned char c, char *udc_data);   

/** Low level method to restore the cursortype and display mode for current controller
  */     
    void _setCursorAndDisplayMode(LCDMode displayMode, LCDCursor cursorType);       
    
/** Low level nibble write operation to LCD controller (serial or parallel)
  */
    void _writeNibble(int value);
   
/** Low level command byte write operation to LCD controller.
  * Methods resets the RS bit and provides the required timing for the command.
  */
    void _writeCommand(int command);
    
/** Low level data byte write operation to LCD controller (serial or parallel).
  * Methods sets the RS bit and provides the required timing for the data.
  */   
    void _writeData(int data);

/** Pure Virtual Low level writes to LCD Bus (serial or parallel)
  * Set the Enable pin.
  */
    virtual void _setEnable(bool value) = 0;

/** Pure Virtual Low level writes to LCD Bus (serial or parallel)
  * Set the RS pin ( 0 = Command, 1 = Data).
  */   
    virtual void _setRS(bool value) = 0;  

/** Pure Virtual Low level writes to LCD Bus (serial or parallel)
  * Set the BL pin (0 = Backlight Off, 1 = Backlight On).
  */   
    virtual void _setBL(bool value) = 0;
    
/** Pure Virtual Low level writes to LCD Bus (serial or parallel)
  * Set the databus value (4 bit).
  */   
    virtual void _setData(int value) = 0;

/** Low level byte write operation to LCD controller (serial or parallel)
  * Depending on the RS pin this byte will be interpreted as data or command
  */
    virtual void _writeByte(int value);

//Display type
    LCDType _type;      // Display type 
    int _nr_cols;       
    int _nr_rows;    
    int _addr_mode;     // Addressing mode of LCDType, defines relation between display row,col and controller memory address
       
//Display mode
    LCDMode _currentMode;

//Controller type 
    LCDCtrl _ctrl;      // Controller type
    int _font;          // ASCII character fonttable
    
//Controller select, mainly used for LCD40x4 
    _LCDCtrl_Idx _ctrl_idx;    

// Cursor
    int _column;
    int _row;
    LCDCursor _currentCursor; 

// Function modes saved to allow switch between Instruction sets after initialisation time 
    int _function, _function_1, _function_x;

// Icon, Booster mode and contrast saved to allow contrast change at later time
// Only available for controllers with added features
    int _icon_power, _contrast;          
};

//--------- End TextLCD_Base -----------


//--------- Start TextLCD Bus -----------

/** Create a TextLCD interface for using regular mbed pins
  *
  */
class TextLCD : public TextLCD_Base {
public:    
    /** Create a TextLCD interface for using regular mbed pins
     *
     * @param rs    Instruction/data control line
     * @param e     Enable line (clock)
     * @param d4-d7 Data lines for using as a 4-bit interface
     * @param type  Sets the panel size/addressing mode (default = LCD16x2)
     * @param bl    Backlight control line (optional, default = NC)      
     * @param e2    Enable2 line (clock for second controller, LCD40x4 only)  
     * @param ctrl  LCD controller (default = HD44780)           
     */
    TextLCD(PinName rs, PinName e, PinName d4, PinName d5, PinName d6, PinName d7, LCDType type = LCD16x2, PinName bl = NC, PinName e2 = NC, LCDCtrl ctrl = HD44780);

   /** Destruct a TextLCD interface for using regular mbed pins
     *
     * @param  none
     * @return none
     */ 
    virtual ~TextLCD();

private:    

/** Implementation of pure Virtual Low level writes to LCD Bus (parallel)
  * Set the Enable pin.
  */
    virtual void _setEnable(bool value);

/** Implementation of pure Virtual Low level writes to LCD Bus (parallel)
  * Set the RS pin (0 = Command, 1 = Data).
  */   
    virtual void _setRS(bool value);  

/** Implementation of pure Virtual Low level writes to LCD Bus (parallel)
  * Set the BL pin (0 = Backlight Off, 1 = Backlight On).
  */   
    virtual void _setBL(bool value);
    
/** Implementation of pure Virtual Low level writes to LCD Bus (parallel)
  * Set the databus value (4 bit).
  */   
    virtual void _setData(int value);

/** Regular mbed pins bus
  */
    DigitalOut _rs, _e;
    BusOut _d;
    
/** Optional Hardware pins for the Backlight and LCD40x4 device
  * Default PinName value is NC, must be used as pointer to avoid issues with mbed lib and DigitalOut pins
  */
    DigitalOut *_bl, *_e2;                                                                                                                                                                                                                                                     
};

//----------- End TextLCD ---------------


//--------- Start TextLCD_I2C -----------
#if(LCD_I2C == 1) /* I2C Expander PCF8574/MCP23008 */

/** Create a TextLCD interface using an I2C PCF8574 (or PCF8574A) or MCP23008 portexpander
  *
  */
class TextLCD_I2C : public TextLCD_Base {    
public:
   /** Create a TextLCD interface using an I2C PCF8574 (or PCF8574A) or MCP23008 portexpander
     *
     * @param i2c             I2C Bus
     * @param deviceAddress   I2C slave address (PCF8574 (or PCF8574A) or MCP23008 portexpander, default = PCF8574_SA0 = 0x40)
     * @param type            Sets the panel size/addressing mode (default = LCD16x2)
     * @param ctrl            LCD controller (default = HD44780)                
     */
    TextLCD_I2C(I2C *i2c, char deviceAddress = PCF8574_SA0, LCDType type = LCD16x2, LCDCtrl ctrl = HD44780);

private:
    
/** Place the Enable bit in the databus shadowvalue
  *  Used for mbed I2C portexpander
  *  @param value data to write
  *  @return none
  */
    void _setEnableBit(bool value);    

/** Implementation of pure Virtual Low level writes to LCD Bus (serial expander)
  * Set the Enable pin.
  */
    virtual void _setEnable(bool value);

/** Implementation of pure Virtual Low level writes to LCD Bus (serial expander)
  * Set the RS pin (0 = Command, 1 = Data).
  */   
    virtual void _setRS(bool value);  

/** Implementation of pure Virtual Low level writes to LCD Bus (serial expander)
  * Set the BL pin (0 = Backlight Off, 1 = Backlight On).
  */   
    virtual void _setBL(bool value);
    
/** Place the 4bit data in the databus shadowvalue
  *  Used for mbed I2C portexpander
  *  @param value data to write
  *  @return none
  */
    void _setDataBits(int value);

/** Implementation of pure Virtual Low level writes to LCD Bus (serial expander)
  * Set the databus value (4 bit).
  */   
    virtual void _setData(int value);
    
//New optimized
//Test faster _writeByte 0.11s vs 0.27s for a 20x4 fillscreen (PCF8574)
//Test faster _writeByte 0.14s vs 0.34s for a 20x4 fillscreen (MCP23008)

/** Low level writes to LCD serial bus expander
  */
    virtual void _writeByte(int value);   

/** Write data to MCP23008 I2C portexpander
  *  @param reg register to write
  *  @param value data to write
  *  @return none     
  */
    void _writeRegister (int reg, int value);     
  
//I2C bus
    I2C *_i2c;
    char _slaveAddress;
    
// Internal bus shadow value for serial bus only
    char _lcd_bus;      
};
#endif /* I2C Expander PCF8574/MCP23008 */

//---------- End TextLCD_I2C ------------


//--------- Start TextLCD_SPI -----------
#if(LCD_SPI == 1) /* SPI Expander SN74595          */

/** Create a TextLCD interface using an SPI 74595 portexpander
  *
  */
class TextLCD_SPI : public TextLCD_Base {    
public:
    /** Create a TextLCD interface using an SPI 74595 portexpander
     *
     * @param spi             SPI Bus
     * @param cs              chip select pin (active low)
     * @param type            Sets the panel size/addressing mode (default = LCD16x2)
     * @param ctrl            LCD controller (default = HD44780)                     
     */
    TextLCD_SPI(SPI *spi, PinName cs, LCDType type = LCD16x2, LCDCtrl ctrl = HD44780);

private:

/** Implementation of pure Virtual Low level writes to LCD Bus (serial expander)
  * Set the Enable pin.
  */
    virtual void _setEnable(bool value);

/** Implementation of pure Virtual Low level writes to LCD Bus (serial expander)
  * Set the RS pin (0 = Command, 1 = Data).
  */   
    virtual void _setRS(bool value);  

/** Implementation of pure Virtual Low level writes to LCD Bus (serial expander)
  * Set the BL pin (0 = Backlight Off, 1 = Backlight On).
  */   
    virtual void _setBL(bool value);
    
/** Implementation of pure Virtual Low level writes to LCD Bus (serial expander)
  * Set the databus value (4 bit).
  */   
    virtual void _setData(int value);     
   
// SPI bus        
    SPI *_spi;
    DigitalOut _cs;    
    
// Internal bus shadow value for serial bus only
    char _lcd_bus;   
};
#endif /* SPI Expander SN74595          */
//---------- End TextLCD_SPI ------------


//--------- Start TextLCD_I2C_N -----------
#if(LCD_I2C_N == 1)  /* Native I2C */

/** Create a TextLCD interface using a controller with native I2C interface
  *
  */
class TextLCD_I2C_N : public TextLCD_Base {    
public:
    /** Create a TextLCD interface using a controller with native I2C interface
     *
     * @param i2c             I2C Bus
     * @param deviceAddress   I2C slave address (default = ST7032_SA = 0x7C)  
     * @param type            Sets the panel size/addressing mode (default = LCD16x2)
     * @param bl              Backlight control line (optional, default = NC)       
     * @param ctrl            LCD controller (default = ST7032_3V3)                     
     */
    TextLCD_I2C_N(I2C *i2c, char deviceAddress = ST7032_SA, LCDType type = LCD16x2, PinName bl = NC, LCDCtrl ctrl = ST7032_3V3);

  /** Destruct a TextLCD interface using a controller with native I2C interface
    */
    virtual ~TextLCD_I2C_N(void);

private:

/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the Enable pin.
  */
    virtual void _setEnable(bool value);

/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the RS pin ( 0 = Command, 1 = Data).
  */   
    virtual void _setRS(bool value);  

/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the BL pin (0 = Backlight Off, 1 = Backlight On).
  */   
    virtual void _setBL(bool value);
    
/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the databus value (4 bit).
  */   
    virtual void _setData(int value);

/** Low level writes to LCD serial bus only (serial native)
  */
    virtual void _writeByte(int value);

//I2C bus
    I2C *_i2c;
    char _slaveAddress;
    
// controlbyte to select between data and command. Internal shadow value for serial bus only
    char _controlbyte;   
    
//Backlight
    DigitalOut *_bl;       
   
};
#endif /* Native I2C */
//---------- End TextLCD_I2C_N ------------


//--------- Start TextLCD_SPI_N -----------
#if(LCD_SPI_N == 1) /* Native SPI bus     */

/** Create a TextLCD interface using a controller with native SPI4 interface
  *
  */
class TextLCD_SPI_N : public TextLCD_Base {    
public:
    /** Create a TextLCD interface using a controller with native SPI4 interface
     *
     * @param spi             SPI Bus
     * @param cs              chip select pin (active low)
     * @param rs              Instruction/data control line
     * @param type            Sets the panel size/addressing mode (default = LCD16x2)
     * @param bl              Backlight control line (optional, default = NC)  
     * @param ctrl            LCD controller (default = ST7032_3V3)                     
     */
    TextLCD_SPI_N(SPI *spi, PinName cs, PinName rs, LCDType type = LCD16x2, PinName bl = NC, LCDCtrl ctrl = ST7032_3V3);

    /** Destruct a TextLCD interface using a controller with native SPI4 interface
      */
    virtual ~TextLCD_SPI_N(void);

private:

/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the Enable pin.
  */
    virtual void _setEnable(bool value);

/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the RS pin (0 = Command, 1 = Data).
  */   
    virtual void _setRS(bool value);  

/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the BL pin (0 = Backlight Off, 1 = Backlight On).
  */   
    virtual void _setBL(bool value);
    
/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the databus value (4 bit).
  */   
    virtual void _setData(int value);

/** Low level writes to LCD serial bus only (serial native)
  */
    virtual void _writeByte(int value);
   
// SPI bus        
    SPI *_spi;
    DigitalOut _cs;    
    DigitalOut _rs;
    
//Backlight    
    DigitalOut *_bl;
};
#endif /* Native SPI bus     */
//---------- End TextLCD_SPI_N ------------


//-------- Start TextLCD_SPI_N_3_8 --------
#if(LCD_SPI_N_3_8 == 1) /* Native SPI bus     */
/** Create a TextLCD interface using a controller with native SPI3 8 bits interface
  * This mode is supported by ST7070. 
  *
  */
class TextLCD_SPI_N_3_8 : public TextLCD_Base {    
public:
 /** Create a TextLCD interface using a controller with a native SPI3 8 bits interface
   * This mode is supported by ST7070. Note that implementation in TexTLCD is not very efficient due to
   * structure of the TextLCD library: each databyte is written separately and requires a separate 'count command' set to 1 byte.
   *
   * @param spi             SPI Bus
   * @param cs              chip select pin (active low)
   * @param type            Sets the panel size/addressing mode (default = LCD16x2)
   * @param bl              Backlight control line (optional, default = NC)  
   * @param ctrl            LCD controller (default = ST7070) 
   */       
  TextLCD_SPI_N_3_8(SPI *spi, PinName cs, LCDType type = LCD16x2, PinName bl = NC, LCDCtrl ctrl = ST7070);

  /** Destruct a TextLCD interface using a controller with native SPI3_8 interface
    */
  virtual ~TextLCD_SPI_N_3_8(void);

private:

/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the Enable pin.
  */
    virtual void _setEnable(bool value);

/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the RS pin (0 = Command, 1 = Data).
  */   
    virtual void _setRS(bool value);  

/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the BL pin (0 = Backlight Off, 1 = Backlight On).
  */   
    virtual void _setBL(bool value);
    
/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the databus value (4 bit).
  */   
    virtual void _setData(int value);

/** Low level writes to LCD serial bus only (serial native)
  */
    virtual void _writeByte(int value);
   
// SPI bus        
    SPI *_spi;
    DigitalOut _cs;    

// controlbyte to select between data and command. Internal shadow value for serial bus only
    char _controlbyte;   
    
//Backlight    
    DigitalOut *_bl;
};
 
#endif /* Native SPI bus     */  
//------- End TextLCD_SPI_N_3_8 -----------


//------- Start TextLCD_SPI_N_3_9 ---------
#if(LCD_SPI_N_3_9 == 1) /* Native SPI bus     */
//Code checked out on logic analyser. Not yet tested on hardware..

/** Create a TextLCD interface using a controller with native SPI3 9 bits interface
  * Note: current mbed libs only support SPI 9 bit mode for NXP platforms
  *
  */
class TextLCD_SPI_N_3_9 : public TextLCD_Base {    
public:
   /** Create a TextLCD interface using a controller with native SPI3 9 bits interface
     * Note: current mbed libs only support SPI 9 bit mode for NXP platforms
     *
     * @param spi             SPI Bus
     * @param cs              chip select pin (active low)
     * @param type            Sets the panel size/addressing mode (default = LCD16x2)
     * @param bl              Backlight control line (optional, default = NC)  
     * @param ctrl            LCD controller (default = AIP31068)                     
     */
    TextLCD_SPI_N_3_9(SPI *spi, PinName cs, LCDType type = LCD16x2, PinName bl = NC, LCDCtrl ctrl = AIP31068);

  /** Destruct a TextLCD interface using a controller with native SPI3_9 interface
    */
    virtual ~TextLCD_SPI_N_3_9(void);

private:

/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the Enable pin.
  */
    virtual void _setEnable(bool value);

/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the RS pin (0 = Command, 1 = Data).
  */   
    virtual void _setRS(bool value);  

/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the BL pin (0 = Backlight Off, 1 = Backlight On).
  */   
    virtual void _setBL(bool value);
    
/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the databus value (4 bit).
  */   
    virtual void _setData(int value);

/** Low level writes to LCD serial bus only (serial native)
  */
    virtual void _writeByte(int value);
   
// SPI bus        
    SPI *_spi;
    DigitalOut _cs;    
   
// controlbyte to select between data and command. Internal shadow value for serial bus only
    char _controlbyte;   

//Backlight
    DigitalOut *_bl;    
};
#endif /* Native SPI bus     */
//-------- End TextLCD_SPI_N_3_9 ----------


//------- Start TextLCD_SPI_N_3_10 ---------
#if(LCD_SPI_N_3_10 == 1) /* Native SPI bus     */

/** Create a TextLCD interface using a controller with native SPI3 10 bits interface
  * Note: current mbed libs only support SPI 10 bit mode for NXP platforms
  *
  */
class TextLCD_SPI_N_3_10 : public TextLCD_Base {    
public:
   /** Create a TextLCD interface using a controller with native SPI3 10 bits interface
     * Note: current mbed libs only support SPI 10 bit mode for NXP platforms
     *
     * @param spi             SPI Bus
     * @param cs              chip select pin (active low)
     * @param type            Sets the panel size/addressing mode (default = LCD16x2)
     * @param bl              Backlight control line (optional, default = NC)  
     * @param ctrl            LCD controller (default = AIP31068)                     
     */
    TextLCD_SPI_N_3_10(SPI *spi, PinName cs, LCDType type = LCD16x2, PinName bl = NC, LCDCtrl ctrl = AIP31068);

  /** Destruct a TextLCD interface using a controller with native SPI3_10 interface
    */
    virtual ~TextLCD_SPI_N_3_10(void);

private:

/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the Enable pin.
  */
    virtual void _setEnable(bool value);

/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the RS pin (0 = Command, 1 = Data).
  */   
    virtual void _setRS(bool value);  

/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the BL pin (0 = Backlight Off, 1 = Backlight On).
  */   
    virtual void _setBL(bool value);
    
/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the databus value (4 bit).
  */   
    virtual void _setData(int value);

/** Low level writes to LCD serial bus only (serial native)
  */
    virtual void _writeByte(int value);
   
// SPI bus        
    SPI *_spi;
    DigitalOut _cs;    
       
// controlbyte to select between data and command. Internal shadow value for serial bus only
    char _controlbyte;   

//Backlight
    DigitalOut *_bl;    
};

#endif /* Native SPI bus     */
//-------- End TextLCD_SPI_N_3_10 ----------


//------- Start TextLCD_SPI_N_3_16 ---------
#if(LCD_SPI_N_3_16 == 1) /* Native SPI bus     */

/** Create a TextLCD interface using a controller with native SPI3 16 bits interface
  *
  */
class TextLCD_SPI_N_3_16 : public TextLCD_Base {    
public:
   /** Create a TextLCD interface using a controller with native SPI3 16 bits interface
     *
     * @param spi             SPI Bus
     * @param cs              chip select pin (active low)
     * @param type            Sets the panel size/addressing mode (default = LCD16x2)
     * @param bl              Backlight control line (optional, default = NC)  
     * @param ctrl            LCD controller (default = PT6314)                     
     */
    TextLCD_SPI_N_3_16(SPI *spi, PinName cs, LCDType type = LCD16x2, PinName bl = NC, LCDCtrl ctrl = PT6314);

  /** Destruct a TextLCD interface using a controller with native SPI3_16 interface
    */
    virtual ~TextLCD_SPI_N_3_16(void);

private:

/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the Enable pin.
  */
    virtual void _setEnable(bool value);

/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the RS pin (0 = Command, 1 = Data).
  */   
    virtual void _setRS(bool value);  

/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the BL pin (0 = Backlight Off, 1 = Backlight On).
  */   
    virtual void _setBL(bool value);
    
/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the databus value (4 bit).
  */   
    virtual void _setData(int value);

/** Low level writes to LCD serial bus only (serial native)
  */
    virtual void _writeByte(int value);
   
// SPI bus        
    SPI *_spi;
    DigitalOut _cs;    
   
// controlbyte to select between data and command. Internal shadow value for serial bus only
    char _controlbyte;   

//Backlight
    DigitalOut *_bl;    
};
#endif /* Native SPI bus     */
//-------- End TextLCD_SPI_N_3_16 ----------


//------- Start TextLCD_SPI_N_3_24 ---------
#if(LCD_SPI_N_3_24 == 1) /* Native SPI bus     */

/** Create a TextLCD interface using a controller with native SPI3 24 bits interface
  * Note: lib uses SPI 8 bit mode
  *
  */
class TextLCD_SPI_N_3_24 : public TextLCD_Base {    
public:
   /** Create a TextLCD interface using a controller with native SPI3 24 bits interface
     * Note: lib uses SPI 8 bit mode
     *
     * @param spi             SPI Bus
     * @param cs              chip select pin (active low)
     * @param type            Sets the panel size/addressing mode (default = LCD16x2)
     * @param bl              Backlight control line (optional, default = NC)  
     * @param ctrl            LCD controller (default = SSD1803)                     
     */
    TextLCD_SPI_N_3_24(SPI *spi, PinName cs, LCDType type = LCD16x2, PinName bl = NC, LCDCtrl ctrl = SSD1803_3V3);

  /** Destruct a TextLCD interface using a controller with native SPI3_24 interface
    */
    virtual ~TextLCD_SPI_N_3_24(void);

private:

/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the Enable pin.
  */
    virtual void _setEnable(bool value);

/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the RS pin (0 = Command, 1 = Data).
  */   
    virtual void _setRS(bool value);  

/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the BL pin (0 = Backlight Off, 1 = Backlight On).
  */   
    virtual void _setBL(bool value);
    
/** Implementation of pure Virtual Low level writes to LCD Bus (serial native)
  * Set the databus value (4 bit).
  */   
    virtual void _setData(int value);

/** Low level writes to LCD serial bus only (serial native)
  */
    virtual void _writeByte(int value);
   
// SPI bus        
    SPI *_spi;
    DigitalOut _cs;    
   
// controlbyte to select between data and command. Internal value for serial bus only
    char _controlbyte;   

//Backlight
    DigitalOut *_bl;    
};
#endif /* Native SPI bus     */
//-------- End TextLCD_SPI_N_3_24 ----------


#endif
