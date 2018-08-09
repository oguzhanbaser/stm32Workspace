/* mbed TextLCD Library, for LCDs based on HD44780 controllers
 * Copyright (c) 2014, WH
 *               2014, v01: WH, Extracted from TextLCD.h as of v14
 *               2014, v02: WH, Added AC780 support, added I2C expander modules, fixed setBacklight() for inverted logic modules. Fixed bug in LCD_SPI_N define
 *               2014, v03: WH, Added LCD_SPI_N_3_8 define for ST7070
 *               2015, v04: WH, Added support for alternative fonttables (eg PCF21XX)
 *               2015, v05: WH, Clean up low-level _writeCommand() and _writeData(), Added support for alt fonttables (eg PCF21XX), Added ST7066_ACM for ACM1602 module, fixed contrast for ST7032
 *               2015, v06: WH, Performance improvement I2C portexpander
 *               2015, v07: WH, Fixed Adafruit I2C/SPI portexpander pinmappings, fixed SYDZ Backlight
 *               2015, v08: WH, Added defines to reduce memory footprint (eg LCD_ICON), added some I2C portexpander defines
 *               2015, v09: WH, Added defines to reduce memory footprint (LCD_TWO_CTRL, LCD_CONTRAST, LCD_UTF8_FONT),
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
#ifndef MBED_TEXTLCDCONFIG_H
#define MBED_TEXTLCDCONFIG_H

//Select hardware interface options to reduce memory footprint (multiple options allowed)
#define LCD_I2C        1           /* I2C Expander PCF8574/MCP23008 */
#define LCD_SPI        1           /* SPI Expander SN74595          */
#define LCD_I2C_N      1           /* Native I2C bus     */
#define LCD_SPI_N      1           /* Native SPI bus     */
#define LCD_SPI_N_3_8  1           /* Native SPI bus     */
#define LCD_SPI_N_3_9  1           /* Native SPI bus     */
#define LCD_SPI_N_3_10 1           /* Native SPI bus     */
#define LCD_SPI_N_3_16 1           /* Native SPI bus     */
#define LCD_SPI_N_3_24 1           /* Native SPI bus     */

//Select options to reduce memory footprint (multiple options allowed)
#define LCD_UDC        1           /* Enable predefined UDC example*/
#define LCD_PRINTF     1           /* Enable Stream implementation */
#define LCD_ICON       1           /* Enable Icon implementation -2.0K codesize*/
#define LCD_ORIENT     1           /* Enable Orientation switch implementation -0.9K codesize*/
#define LCD_BIGFONT    1           /* Enable Big Font implementation -0.6K codesize */
#define LCD_INVERT     1           /* Enable display Invert implementation -0.5K codesize*/
#define LCD_POWER      1           /* Enable Power control implementation -0.1K codesize*/
#define LCD_BLINK      1           /* Enable UDC and Icon Blink control implementation -0.8K codesize*/
#define LCD_CONTRAST   1           /* Enable Contrast control implementation -0.9K codesize*/
#define LCD_TWO_CTRL   1           /* Enable LCD40x4 (two controller) implementation -0.1K codesize*/
#define LCD_FONTSEL    0           /* Enable runtime font select implementation using setFont -0.9K codesize*/

//Select option to activate default fonttable or alternatively use conversion for specific controller versions (eg PCF2116C, PCF2119R, SSD1803, US2066)
#define LCD_DEF_FONT   1           //Default HD44780 font
//#define LCD_C_FONT     1           //PCF21xxC font
//#define LCD_R_FONT     1           //PCF21xxR font
//#define LCD_UTF8_FONT  1           /* Enable UTF8 Support (eg Cyrillic tables) -0.4K codesize*/
//#define LCD_UTF8_CYR_B 1           /*  Select specific UTF8 Cyrillic table (SSD1803 ROM_B)              */

//Pin Defines for I2C PCF8574/PCF8574A or MCP23008 and SPI 74595 bus expander interfaces
//Different commercially available LCD portexpanders use different wiring conventions.
//LCD and serial portexpanders should be wired according to the tables below.
//
//Select Serial Port Expander Hardware module (one option only)
#define DEFAULT        0
#define ADAFRUIT       0
#define DFROBOT        0
#define LCM1602        1
#define YWROBOT        0
#define GYLCD          0
#define MJKDZ          0
#define SYDZ           0
#define WIDEHK         0
#define LCDPLUG        0

#if (DEFAULT==1)
//Definitions for default (WH) mapping between serial port expander pins and LCD controller
//This hardware supports the I2C bus expander (PCF8574/PCF8574A or MCP23008) and SPI bus expander (74595) interfaces
//See https://mbed.org/cookbook/Text-LCD-Enhanced
//
//Note: LCD RW pin must be connected to GND
//      E2 is used for LCD40x4 (second controller)
//      BL may be used to control backlight

//I2C bus expander (PCF8574/PCF8574A or MCP23008) interface
#define LCD_BUS_I2C_D4 (1 << 0)
#define LCD_BUS_I2C_D5 (1 << 1)
#define LCD_BUS_I2C_D6 (1 << 2)
#define LCD_BUS_I2C_D7 (1 << 3)
#define LCD_BUS_I2C_RS (1 << 4)
#define LCD_BUS_I2C_E  (1 << 5)
#define LCD_BUS_I2C_E2 (1 << 6)
#define LCD_BUS_I2C_BL (1 << 7)

#define LCD_BUS_I2C_RW (1 << 6)

//SPI bus expander (74595) interface, same as I2C
#define LCD_BUS_SPI_D4 LCD_BUS_I2C_D4
#define LCD_BUS_SPI_D5 LCD_BUS_I2C_D5
#define LCD_BUS_SPI_D6 LCD_BUS_I2C_D6
#define LCD_BUS_SPI_D7 LCD_BUS_I2C_D7
#define LCD_BUS_SPI_RS LCD_BUS_I2C_RS
#define LCD_BUS_SPI_E  LCD_BUS_I2C_E
#define LCD_BUS_SPI_E2 LCD_BUS_I2C_E2
#define LCD_BUS_SPI_BL LCD_BUS_I2C_BL

#define LCD_BUS_SPI_RW LCD_BUS_I2C_RW

//Select I2C Portexpander type (one option only)
#define PCF8574        1
#define MCP23008       0

//Inverted Backlight control
#define BACKLIGHT_INV  0
#endif

#if (ADAFRUIT==1)
//Definitions for Adafruit i2cspilcdbackpack mapping between serial port expander pins and LCD controller
//This hardware supports both an I2C expander (MCP23008) and an SPI expander (74595) selectable by a jumper.
//Slaveaddress may be set by solderbridges (default 0x40). SDA/SCL has pullup Resistors onboard.
//See http://www.ladyada.net/products/i2cspilcdbackpack
//
//Note: LCD RW pin must be kept LOW
//      E2 is not available on this hardware and so it does not support LCD40x4 (second controller)
//      BL is used to control backlight
//Note: The pinmappings are different for the MCP23008 and the 74595!

//I2C bus expander (MCP23008) interface
#define LCD_BUS_I2C_0  (1 << 0)
#define LCD_BUS_I2C_RS (1 << 1)
#define LCD_BUS_I2C_E  (1 << 2)
#define LCD_BUS_I2C_D4 (1 << 3)
#define LCD_BUS_I2C_D5 (1 << 4)
#define LCD_BUS_I2C_D6 (1 << 5)
#define LCD_BUS_I2C_D7 (1 << 6)
#define LCD_BUS_I2C_BL (1 << 7)

#define LCD_BUS_I2C_E2 (1 << 0)
#define LCD_BUS_I2C_RW (1 << 0)

//SPI bus expander (74595) interface
#define LCD_BUS_SPI_0  (1 << 0)
#define LCD_BUS_SPI_RS (1 << 1)
#define LCD_BUS_SPI_E  (1 << 2)
#define LCD_BUS_SPI_D7 (1 << 3)
#define LCD_BUS_SPI_D6 (1 << 4)
#define LCD_BUS_SPI_D5 (1 << 5)
#define LCD_BUS_SPI_D4 (1 << 6)
#define LCD_BUS_SPI_BL (1 << 7)

#define LCD_BUS_SPI_E2 (1 << 0)
#define LCD_BUS_SPI_RW (1 << 0)

//Force I2C portexpander type
#define PCF8574        0
#define MCP23008       1

//Inverted Backlight control
#define BACKLIGHT_INV  0
#endif

#if (DFROBOT==1)
//Definitions for DFROBOT LCD2004 Module mapping between serial port expander pins and LCD controller
//This hardware uses PCF8574 and is different from earlier/different Arduino I2C LCD displays
//Slaveaddress hardwired to 0x4E. SDA/SCL has pullup Resistors onboard.
//See http://arduino-info.wikispaces.com/LCD-Blue-I2C
//
//Definitions for DFROBOT V1.1
//This hardware uses PCF8574. Slaveaddress may be set by jumpers (default 0x40).
//SDA/SCL has pullup Resistors onboard and features a voltage level converter 3V3 <-> 5V.
//See http://www.dfrobot.com/index.php?route=product/product&product_id=135
//
//
//Note: LCD RW pin must be kept LOW
//      E2 is not available on default Arduino hardware and so it does not support LCD40x4 (second controller)
//      BL is used to control backlight

//I2C bus expander PCF8574 interface
#define LCD_BUS_I2C_RS (1 << 0)
#define LCD_BUS_I2C_RW (1 << 1)
#define LCD_BUS_I2C_E  (1 << 2)
#define LCD_BUS_I2C_BL (1 << 3)
#define LCD_BUS_I2C_D4 (1 << 4)
#define LCD_BUS_I2C_D5 (1 << 5)
#define LCD_BUS_I2C_D6 (1 << 6)
#define LCD_BUS_I2C_D7 (1 << 7)

#define LCD_BUS_I2C_E2 (1 << 1)

//SPI bus expander (74595) interface, same as I2C
#define LCD_BUS_SPI_RS LCD_BUS_I2C_RS
#define LCD_BUS_SPI_RW LCD_BUS_I2C_RW
#define LCD_BUS_SPI_E  LCD_BUS_I2C_E
#define LCD_BUS_SPI_BL LCD_BUS_I2C_BL
#define LCD_BUS_SPI_D4 LCD_BUS_I2C_D4
#define LCD_BUS_SPI_D5 LCD_BUS_I2C_D5
#define LCD_BUS_SPI_D6 LCD_BUS_I2C_D6
#define LCD_BUS_SPI_D7 LCD_BUS_I2C_D7

#define LCD_BUS_SPI_E2 LCD_BUS_I2C_E2


//Force I2C portexpander type
#define PCF8574        1
#define MCP23008       0

//Inverted Backlight control
#define BACKLIGHT_INV  0
#endif

#if ((YWROBOT==1) || (LCM1602==1))
//Definitions for YWROBOT LCM1602 V1 Module mapping between serial port expander pins and LCD controller.
//Very similar to DFROBOT. Also marked as 'Funduino'. This hardware uses PCF8574.
//Slaveaddress may be set by solderbridges (default 0x4E). SDA/SCL has no pullup Resistors onboard.
//See http://arduino-info.wikispaces.com/LCD-Blue-I2C
//
//Note: LCD RW pin must be kept LOW
//      E2 is not available on default hardware and so it does not support LCD40x4 (second controller)
//      BL is used to control backlight.

//I2C bus expander PCF8574 interface
#define LCD_BUS_I2C_RS (1 << 0)
#define LCD_BUS_I2C_RW (1 << 1)
#define LCD_BUS_I2C_E  (1 << 2)
#define LCD_BUS_I2C_BL (1 << 3)
#define LCD_BUS_I2C_D4 (1 << 4)
#define LCD_BUS_I2C_D5 (1 << 5)
#define LCD_BUS_I2C_D6 (1 << 6)
#define LCD_BUS_I2C_D7 (1 << 7)

#define LCD_BUS_I2C_E2 (1 << 1)

//SPI bus expander (74595) interface, same as I2C
#define LCD_BUS_SPI_RS LCD_BUS_I2C_RS
#define LCD_BUS_SPI_RW LCD_BUS_I2C_RW
#define LCD_BUS_SPI_E  LCD_BUS_I2C_E
#define LCD_BUS_SPI_BL LCD_BUS_I2C_BL
#define LCD_BUS_SPI_D4 LCD_BUS_I2C_D4
#define LCD_BUS_SPI_D5 LCD_BUS_I2C_D5
#define LCD_BUS_SPI_D6 LCD_BUS_I2C_D6
#define LCD_BUS_SPI_D7 LCD_BUS_I2C_D7

#define LCD_BUS_SPI_E2 LCD_BUS_I2C_E2

//Force I2C portexpander type
#define PCF8574        1
#define MCP23008       0

//Inverted Backlight control
#define BACKLIGHT_INV  0
#endif

#if ((GYLCD==1) || (MJKDZ==1))
//Definitions for Arduino-IIC-LCD GY-LCD-V1, for GY-IICLCD and for MJKDZ Module mapping between serial port expander pins and LCD controller.
//Very similar to DFROBOT. This hardware uses PCF8574.
//Slaveaddress may be set by solderbridges (default 0x4E). SDA/SCL has pullup Resistors onboard.
//See http://arduino-info.wikispaces.com/LCD-Blue-I2C
//
//Note: LCD RW pin must be kept LOW
//      E2 is not available on default hardware and so it does not support LCD40x4 (second controller)
//      BL is used to control backlight, reverse logic: Low turns on Backlight. This is handled in setBacklight()

//I2C bus expander PCF8574 interface
#define LCD_BUS_I2C_D4 (1 << 0)
#define LCD_BUS_I2C_D5 (1 << 1)
#define LCD_BUS_I2C_D6 (1 << 2)
#define LCD_BUS_I2C_D7 (1 << 3)
#define LCD_BUS_I2C_E  (1 << 4)
#define LCD_BUS_I2C_RW (1 << 5)
#define LCD_BUS_I2C_RS (1 << 6)
#define LCD_BUS_I2C_BL (1 << 7)

#define LCD_BUS_I2C_E2 (1 << 5)

//SPI bus expander (74595) interface
#define LCD_BUS_SPI_D4 LCD_BUS_I2C_D4
#define LCD_BUS_SPI_D5 LCD_BUS_I2C_D5
#define LCD_BUS_SPI_D6 LCD_BUS_I2C_D6
#define LCD_BUS_SPI_D7 LCD_BUS_I2C_D7
#define LCD_BUS_SPI_E  LCD_BUS_I2C_E
#define LCD_BUS_SPI_RW LCD_BUS_I2C_RW
#define LCD_BUS_SPI_RS LCD_BUS_I2C_RS
#define LCD_BUS_SPI_BL LCD_BUS_I2C_BL

#define LCD_BUS_SPI_E2 LCD_BUS_I2C_E2

//Force I2C portexpander type
#define PCF8574        1
#define MCP23008       0

//Force Inverted Backlight control
#define BACKLIGHT_INV  1
#endif

#if (SYDZ==1)
//Definitions for SYDZ Module mapping between serial port expander pins and LCD controller.
//Very similar to DFROBOT. This hardware uses PCF8574A.
//Slaveaddress may be set by switches (default 0x70). SDA/SCL has pullup Resistors onboard.
//See ebay
//
//Note: LCD RW pin must be kept LOW
//      E2 is not available on default hardware and so it does not support LCD40x4 (second controller)
//      BL is used to control backlight

//I2C bus expander PCF8574A interface
#define LCD_BUS_I2C_RS (1 << 0)
#define LCD_BUS_I2C_RW (1 << 1)
#define LCD_BUS_I2C_E  (1 << 2)
#define LCD_BUS_I2C_BL (1 << 3)
#define LCD_BUS_I2C_D4 (1 << 4)
#define LCD_BUS_I2C_D5 (1 << 5)
#define LCD_BUS_I2C_D6 (1 << 6)
#define LCD_BUS_I2C_D7 (1 << 7)

#define LCD_BUS_I2C_E2 (1 << 1)

//SPI bus expander (74595) interface, same as I2C
#define LCD_BUS_SPI_RS LCD_BUS_I2C_RS
#define LCD_BUS_SPI_RW LCD_BUS_I2C_RW
#define LCD_BUS_SPI_E  LCD_BUS_I2C_E
#define LCD_BUS_SPI_BL LCD_BUS_I2C_BL
#define LCD_BUS_SPI_D4 LCD_BUS_I2C_D4
#define LCD_BUS_SPI_D5 LCD_BUS_I2C_D5
#define LCD_BUS_SPI_D6 LCD_BUS_I2C_D6
#define LCD_BUS_SPI_D7 LCD_BUS_I2C_D7

#define LCD_BUS_SPI_E2 LCD_BUS_I2C_E2

//Force I2C portexpander type
#define PCF8574        1
#define MCP23008       0

//Force Inverted Backlight control
#define BACKLIGHT_INV  0
#endif

#if (WIDEHK==1)
//Definitions for WIDE.HK I2C backpack mapping between serial port expander pins and LCD controller
//This hardware uses an MCP23008 I2C expander.
//Slaveaddress is hardcoded at 0x4E. SDA/SCL has pullup Resistors onboard (3k3).
//See http://www.wide.hk
//
//Note: LCD RW pin must be kept LOW
//      E2 is not available on this hardware and so it does not support LCD40x4 (second controller)
//      BL is used to control backlight
//

//I2C bus expander (MCP23008) interface
#define LCD_BUS_I2C_D4 (1 << 0)
#define LCD_BUS_I2C_D5 (1 << 1)
#define LCD_BUS_I2C_D6 (1 << 2)
#define LCD_BUS_I2C_D7 (1 << 3)
#define LCD_BUS_I2C_RS (1 << 4)
#define LCD_BUS_I2C_RW (1 << 5)
#define LCD_BUS_I2C_BL (1 << 6)
#define LCD_BUS_I2C_E  (1 << 7)

#define LCD_BUS_I2C_E2 (1 << 5)

//SPI bus expander (74595) interface, same as I2C
#define LCD_BUS_SPI_D4 LCD_BUS_I2C_D4
#define LCD_BUS_SPI_D5 LCD_BUS_I2C_D5
#define LCD_BUS_SPI_D6 LCD_BUS_I2C_D6
#define LCD_BUS_SPI_D7 LCD_BUS_I2C_D7
#define LCD_BUS_SPI_RS LCD_BUS_I2C_RS
#define LCD_BUS_SPI_RW LCD_BUS_I2C_RW
#define LCD_BUS_SPI_BL LCD_BUS_I2C_BL
#define LCD_BUS_SPI_E  LCD_BUS_I2C_E

#define LCD_BUS_SPI_E2 LCD_BUS_I2C_E2

//Force I2C portexpander type
#define PCF8574        0
#define MCP23008       1

//Inverted Backlight control
#define BACKLIGHT_INV  0
#endif

#if (LCDPLUG==1)
//Definitions for Jeelabs LCD_Plug I2C backpack mapping between serial port expander pins and LCD controller
//This hardware uses an MCP23008 I2C expander.
//Slaveaddress is hardcoded at 0x48. SDA/SCL has no pullup Resistors onboard.
//See http://jeelabs.net/projects/hardware/wiki/lcd_plug
//
//Note: LCD RW pin must be kept LOW
//      E2 is available on a plug and so it does support LCD40x4 (second controller)
//      BL is used to control backlight
//

//I2C bus expander (MCP23008) interface
#define LCD_BUS_I2C_D4 (1 << 0)
#define LCD_BUS_I2C_D5 (1 << 1)
#define LCD_BUS_I2C_D6 (1 << 2)
#define LCD_BUS_I2C_D7 (1 << 3)
#define LCD_BUS_I2C_RS (1 << 4)
#define LCD_BUS_I2C_E2 (1 << 5)
#define LCD_BUS_I2C_E  (1 << 6)
#define LCD_BUS_I2C_BL (1 << 7)

#define LCD_BUS_I2C_RW (1 << 5)

//SPI bus expander (74595) interface, same as I2C
#define LCD_BUS_SPI_D4 LCD_BUS_I2C_D4
#define LCD_BUS_SPI_D5 LCD_BUS_I2C_D5
#define LCD_BUS_SPI_D6 LCD_BUS_I2C_D6
#define LCD_BUS_SPI_D7 LCD_BUS_I2C_D7
#define LCD_BUS_SPI_RS LCD_BUS_I2C_RS
#define LCD_BUS_SPI_E2 LCD_BUS_I2C_E2
#define LCD_BUS_SPI_E  LCD_BUS_I2C_E
#define LCD_BUS_SPI_BL LCD_BUS_I2C_BL

#define LCD_BUS_SPI_RW LCD_BUS_I2C_RW

//Force I2C portexpander type
#define PCF8574        0
#define MCP23008       1

//Inverted Backlight control
#define BACKLIGHT_INV  0
#endif


//Bitpattern Defines for I2C PCF8574/PCF8574A, MCP23008 and SPI 74595 Bus expanders
//Don't change!
#define LCD_BUS_I2C_MSK (LCD_BUS_I2C_D4 | LCD_BUS_I2C_D5 | LCD_BUS_I2C_D6 | LCD_BUS_I2C_D7)
#if (BACKLIGHT_INV == 1)
#define LCD_BUS_I2C_DEF (0x00 | LCD_BUS_I2C_BL)
#else
#define LCD_BUS_I2C_DEF  0x00
#endif

#define LCD_BUS_SPI_MSK (LCD_BUS_SPI_D4 | LCD_BUS_SPI_D5 | LCD_BUS_SPI_D6 | LCD_BUS_SPI_D7)
#if (BACKLIGHT_INV == 1)
#define LCD_BUS_SPI_DEF (0x00 | LCD_BUS_SPI_BL)
#else
#define LCD_BUS_SPI_DEF  0x00
#endif


/* PCF8574/PCF8574A I2C portexpander slave address */
#define PCF8574_SA0    0x40
#define PCF8574_SA1    0x42
#define PCF8574_SA2    0x44
#define PCF8574_SA3    0x46
#define PCF8574_SA4    0x48
#define PCF8574_SA5    0x4A
#define PCF8574_SA6    0x4C
#define PCF8574_SA7    0x4E

#define PCF8574A_SA0   0x70
#define PCF8574A_SA1   0x72
#define PCF8574A_SA2   0x74
#define PCF8574A_SA3   0x76
#define PCF8574A_SA4   0x78
#define PCF8574A_SA5   0x7A
#define PCF8574A_SA6   0x7C
#define PCF8574A_SA7   0x7E

/* MCP23008 I2C portexpander slave address */
#define MCP23008_SA0   0x40
#define MCP23008_SA1   0x42
#define MCP23008_SA2   0x44
#define MCP23008_SA3   0x46
#define MCP23008_SA4   0x48
#define MCP23008_SA5   0x4A
#define MCP23008_SA6   0x4C
#define MCP23008_SA7   0x4E

/* MCP23008 I2C portexpander internal registers */
#define IODIR          0x00
#define IPOL           0x01
#define GPINTEN        0x02
#define DEFVAL         0x03
#define INTCON         0x04
#define IOCON          0x05
#define GPPU           0x06
#define INTF           0x07
#define INTCAP         0x08
#define GPIO           0x09
#define OLAT           0x0A

/* ST7032i I2C slave address */
#define ST7032_SA      0x7C

/* ST7036i I2C slave address */
#define ST7036_SA0     0x78
#define ST7036_SA1     0x7A
#define ST7036_SA2     0x7C
#define ST7036_SA3     0x7E

/* ST7066_ACM I2C slave address, Added for ACM1602 module  */
#define ST7066_SA0     0xA0

/* PCF21XX I2C slave address */
#define PCF21XX_SA0    0x74
#define PCF21XX_SA1    0x76

/* AIP31068 I2C slave address */
#define AIP31068_SA    0x7C

/* SSD1803 I2C slave address */
#define SSD1803_SA0    0x78
#define SSD1803_SA1    0x7A

/* US2066/SSD1311 I2C slave address */
#define US2066_SA0     0x78
#define US2066_SA1     0x7A

/* AC780 I2C slave address */
#define AC780_SA0      0x78
#define AC780_SA1      0x7A
#define AC780_SA2      0x7C
#define AC780_SA3      0x7E

/* SPLC792A is clone of ST7032i */
#define SPLC792A_SA0   0x78
#define SPLC792A_SA1   0x7A
#define SPLC792A_SA2   0x7C
#define SPLC792A_SA3   0x7E

//Some native I2C controllers dont support ACK. Set define to '0' to allow code to proceed even without ACK
//#define LCD_I2C_ACK    0
#define LCD_I2C_ACK    1


// Contrast setting, 6 significant bits (only supported for controllers with extended features)
// Voltage Multiplier setting, 2 or 3 significant bits (only supported for controllers with extended features)
#define LCD_DEF_CONTRAST    0x20

//ST7032 EastRising ERC1602FS-4 display
//Contrast setting 6 significant bits (0..63)
//Voltage Multiplier setting 3 significant bits:
// 0: 1.818V
// 1: 2.222V
// 2: 2.667V
// 3: 3.333V
// 4: 3.636V (ST7032 default)
// 5: 4.000V
// 6: 4.444V
// 7: 5.000V
#define LCD_ST7032_CONTRAST 0x28
#define LCD_ST7032_RAB      0x04

//ST7036 EA DOGM1603 display
//Contrast setting 6 significant bits
//Voltage Multiplier setting 3 significant bits
#define LCD_ST7036_CONTRAST 0x28
#define LCD_ST7036_RAB      0x04

//SSD1803 EA DOGM204 display
//Contrast setting 6 significant bits
//Voltage Multiplier setting 3 significant bits
#define LCD_SSD1_CONTRAST   0x28
#define LCD_SSD1_RAB        0x06

//US2066/SSD1311 EastRising ER-OLEDM2002-4 display
//Contrast setting 8 significant bits, use 6 for compatibility
#define LCD_US20_CONTRAST   0x3F
//#define LCD_US20_CONTRAST   0x1F

//PCF2113, PCF2119 display
//Contrast setting 6 significant bits
//Voltage Multiplier setting 2 significant bits
#define LCD_PCF2_CONTRAST   0x20
#define LCD_PCF2_S12        0x02

//PT6314 VFD display
//Contrast setting 2 significant bits, use 6 for compatibility
#define LCD_PT63_CONTRAST   0x3F

//SPLC792A is clone of ST7032i
//Contrast setting 6 significant bits (0..63)
//Voltage Multiplier setting 3 significant bits:
// 0: 1.818V
// 1: 2.222V
// 2: 2.667V
// 3: 3.333V (SPLC792A default)
// 4: 3.636V
// 5: 4.000V
// 6: 4.444V
// 7: 5.000V
#define LCD_SPLC792A_CONTRAST 0x28
#define LCD_SPLC792A_RAB      0x04

#endif //MBED_TEXTLCDCONFIG_H
