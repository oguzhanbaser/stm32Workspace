/* mbed TextLCD Library, for LCDs based on HD44780 controllers
 * Copyright (c) 2007-2010, sford, http://mbed.org
 *               2013, v01: WH, Added LCD types, fixed LCD address issues, added Cursor and UDCs 
 *               2013, v02: WH, Added I2C and SPI bus interfaces  
 *               2013, v03: WH, Added support for LCD40x4 which uses 2 controllers 
 *               2013, v04: WH, Added support for Display On/Off, improved 4bit bootprocess
 *               2013, v05: WH, Added support for 8x2B, added some UDCs   
 *               2013, v06: WH, Added support for devices that use internal DC/DC converters 
 *               2013, v07: WH, Added support for backlight and include portdefinitions for LCD2004 Module from DFROBOT 
 *               2014, v08: WH, Refactored in Base and Derived Classes to deal with mbed lib change regarding 'NC' defined pins 
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
 *               2015, v19: WH, Fixed Adafruit I2C/SPI portexpander pinmappings, fixed SYDZ Backlight 
 *               2015, v20: WH, Fixed occasional Init fail caused by insufficient wait time after ReturnHome command (0x02), Added defines to reduce memory footprint (eg LCD_ICON),
 *                              Fixed and Added more fonttable support for PCF2119R_3V3, Added HD66712 controller.
 *               2015, v21: WH, Added LCD32x2 defines and code, Fixed KS0073 DL=1 init for SPI, Added defines to reduce memory footprint (LCD_TWO_CTRL, LCD_CONTRAST, LCD_UTF8_FONT)
 *                              Added SPLC792A controller, Added UTF8_2_LCD decode for Cyrilic font (By Andriy Ribalko). Added setFont()
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
#include "mbed.h"
#include "TextLCD.h"
#include "TextLCD_UDC.inc"
#include "TextLCD_UTF8.inc"
   
/** Create a TextLCD_Base interface
  *
  * @param type  Sets the panel size/addressing mode (default = LCD16x2)
  * @param ctrl  LCD controller (default = HD44780)           
  */
TextLCD_Base::TextLCD_Base(LCDType type, LCDCtrl ctrl) : _type(type), _ctrl(ctrl) {
    
  // Extract LCDType data  

  // Columns encoded in b15..b8
  _nr_cols = (_type & LCD_T_COL_MSK) >> LCD_T_COL_SHFT;   
  
  // Rows encoded in b23..b16  
  _nr_rows = (_type & LCD_T_ROW_MSK) >> LCD_T_ROW_SHFT;   

  // Addressing mode encoded in b27..b24  
  _addr_mode = _type & LCD_T_ADR_MSK;
  
  // Font table, encoded in LCDCtrl  
  _font = _ctrl & LCD_C_FNT_MSK;
}

/**  Init the LCD Controller(s)
  *  Clear display 
  *  @param _LCDDatalength dl sets the datalength of data/commands
  *  @return none 
  */
void TextLCD_Base::_init(_LCDDatalength dl) {

  wait_ms(100);                  // Wait 100ms to ensure powered up
  
#if (LCD_TWO_CTRL == 1)
  // Select and configure second LCD controller when needed
  if(_type==LCD40x4) {
    _ctrl_idx=_LCDCtrl_1;        // Select 2nd controller   
    _initCtrl(dl);               // Init 2nd controller   
  }
#endif
    
  // Select and configure primary LCD controller
  _ctrl_idx=_LCDCtrl_0;          // Select primary controller  
  _initCtrl(dl);                 // Init primary controller

  // Clear whole display and Reset Cursor location
  // Note: This will make sure that some 3-line displays that skip topline of a 4-line configuration 
  //       are cleared and init cursor correctly.
  cls();   
} 

/**  Init the LCD controller
  *   Set number of lines, fonttype, no cursor etc
  *   The controller is accessed in 4-bit parallel mode either directly via mbed pins or through I2C or SPI expander.
  *   Some controllers also support native I2C or SPI interfaces. 
  *
  *  @param _LCDDatalength dl sets the 4 or 8 bit datalength of data/commands. Required for some native serial modes that dont work when DL=0.
  *  @return none  
  *
  *  Note: some configurations are commented out because they have not yet been tested due to lack of hardware
  */
void TextLCD_Base::_initCtrl(_LCDDatalength dl) {
  int _bias_lines=0; // Set Bias and lines (Instr Set 1), temporary variable.
  int _lines=0;      // Set lines (Ext Instr Set), temporary variable.

    this->_setRS(false); // command mode

    if (dl == _LCD_DL_4) {
      // The Controller could be in 8 bit mode (power-on reset) or in 4 bit mode (warm reboot) at this point.
      // Follow this procedure to make sure the Controller enters the correct state. The hardware interface
      // between the uP and the LCD can only write the 4 most significant bits (Most Significant Nibble, MSN).
      // In 4 bit mode the LCD expects the MSN first, followed by the LSN.
      //
      //    Current state:               8 bit mode                |      4 bit mode, MSN is next        | 4 bit mode, LSN is next          
                           //-------------------------------------------------------------------------------------------------                          
      _writeNibble(0x3);   //  set 8 bit mode (MSN) and dummy LSN, |   set 8 bit mode (MSN),             |    set dummy LSN, 
                           //  remains in 8 bit mode               |    remains in 4 bit mode            |  remains in 4 bit mode
      wait_ms(15);         //                           
     
      _writeNibble(0x3);   //  set 8 bit mode (MSN) and dummy LSN, |      set dummy LSN,                 |    set 8bit mode (MSN), 
                           //  remains in 8 bit mode               |   change to 8 bit mode              |  remains in 4 bit mode
      wait_ms(15);         // 
    
      _writeNibble(0x3);   //  set 8 bit mode (MSN) and dummy LSN, | set 8 bit mode (MSN) and dummy LSN, |    set dummy LSN, 
                           //  remains in 8 bit mode               |   remains in 8 bit mode             |  change to 8 bit mode
      wait_ms(15);         // 

      // Controller is now in 8 bit mode

      _writeNibble(0x2);   // Change to 4-bit mode (MSN), the LSN is undefined dummy
      wait_us(40);         // most instructions take 40us

      // Controller is now in 4-bit mode
      // Note: 4/8 bit mode is ignored for most native SPI and I2C devices. They dont use the parallel bus.
      //       However, _writeNibble() method is void anyway for native SPI and I2C devices.
    }
    else {
      // Reset in 8 bit mode, final Function set will follow 
      _writeCommand(0x30); // Function set 0 0 1 DL=1 N F x x       
      wait_ms(1);          // most instructions take 40us      
    }      
   
    // Device specific initialisations: DC/DC converter to generate VLCD or VLED, number of lines etc
    switch (_ctrl) {

      case KS0073:
          // Initialise Display configuration
          switch (_type) {
//            case LCD6x1:
            case LCD8x1:         //8x1 is a regular 1 line display
//            case LCD8x2B:        //8x1 is a 16x1 line display            
            case LCD12x1:                                
            case LCD16x1:                                            
            case LCD20x1:
            case LCD24x1:
//            case LCD32x1:        // EXT pin is High, extension driver needed
//            case LCD40x1:        // EXT pin is High, extension driver needed 
//            case LCD52x1:        // EXT pin is High, extension driver needed 
              _function = dl | 0x02; // Set function, 0 0 1 DL, N, RE(0), DH, REV                                                                 
                                     // Note: 4 bit mode is NOT ignored for native SPI !                       
                                     //   DL=1 (8 bits bus), DL=0  (4 bits bus)             
                                     //    N=0 (1-line mode), N=1 (2-line mode), dont care for 4 line mode                                     
                                     //   RE=0 (Dis. Extended Regs, special mode for KS0073)
                                     //   DH=1 (Disp shift enable, special mode for KS0073)                                
                                     //   REV=0 (Reverse normal, special mode for KS0073)
                                                 
              _function_1 = dl | 0x04; // Set function, 0 0 1 DL, N, RE(1), BE, LP  (Ext Regs)
                                     // Note: 4 bit mode is NOT ignored for native SPI !                       
                                     //   DL=1 (8 bits bus), DL=0  (4 bits bus)                          
                                     //    N=0 (1-line mode), N=1 (2-line mode), dont care for 4 line mode                                     
                                     //   RE=1 (Ena Extended Regs, special mode for KS0073)
                                     //   BE=0 (Blink Enable, CG/SEG RAM, special mode for KS0073)                                
                                     //   LP=0 (LP=1 Low power mode, LP=0 Normal)

              _function_x = 0x00;   // Ext Function set 0000 1 FW BW NW (Ext Regs)
                                    //    NW=0 (1,2 line), NW=1 (4 Line, special mode for KS0073)                                
              break;                                

//            case LCD12x3D:         // Special mode for KS0073, KS0078 and PCF21XX            
//            case LCD12x3D1:        // Special mode for KS0073, KS0078 and PCF21XX            
            case LCD12x4D:         // Special mode for KS0073, KS0078 and PCF21XX            
//            case LCD16x3D:         // Special mode for KS0073, KS0078             
//            case LCD16x3D1:        // Special mode for KS0073, KS0078             
//            case LCD16x4D:         // Special mode for KS0073, KS0078            
            case LCD20x4D:         // Special mode for KS0073, KS0078            
              _function = dl | 0x02; // Set function, 0 0 1 DL, N, RE(0), DH, REV                                                                 
                                     // Note: 4 bit mode is NOT ignored for native SPI !                       
                                     //   DL=1  (8 bits bus), DL=0 (4 bits bus)             
                                     //    N=0 (1-line mode), N=1 (2-line mode), dont care for 4 line mode                                      
                                     //   RE=0  (Dis. Extended Regs, special mode for KS0073)
                                     //   DH=1  (Disp shift enable, special mode for KS0073)                                
                                     //   REV=0 (Reverse normal, special mode for KS0073)
                                    
              _function_1 = dl | 0x04; // Set function, 0 0 1 DL, N, RE(1), BE, LP  (Ext Regs)
                                     // Note: 4 bit mode is NOT ignored for native SPI !                       
                                     //   DL=1 (8 bits bus), DL=0 (4 bits bus)                          
                                     //    N=0 (1-line mode), N=1 (2-line mode), dont care for 4 line mode              
                                     //   RE=1 (Ena Extended Regs, special mode for KS0073)
                                     //   BE=0 (Blink Enable, CG/SEG RAM, special mode for KS0073)                                
                                     //   LP=0 (LP=1 Low power mode, LP=0 Normal)                                    

              _function_x = 0x01;   // Ext Function set 0000 1 FW BW NW (Ext Regs)
                                    //    NW=0 (1,2 line), NW=1 (4 Line, special mode for KS0073)                                
              break;                                

//            case LCD6x2:
            case LCD8x2:
            case LCD16x2:
//            case LCD16x1C:            
            case LCD20x2:
            case LCD24x2:
            case LCD32x2:            
              // All other LCD types are initialised as 2 Line displays
              _function = dl | 0x0A; // Set function, 0 0 1 DL, N, RE(0), DH, REV                                                                 
                                     // Note: 4 bit mode is NOT ignored for native SPI !                       
                                     //   DL=1  (8 bits bus), DL=0  (4 bits bus)             
                                     //    N=1  (2-line mode), N=0 (1-line mode)
                                     //   RE=0  (Dis. Extended Regs, special mode for KS0073)
                                     //   DH=1  (Disp shift enable, special mode for KS0073)                                
                                     //   REV=0 (Reverse normal, special mode for KS0073)
                                    
              _function_1 = dl | 0x0C; // Set function, 0 0 1 DL, N, RE(1), BE, LP  (Ext Regs)
                                     // Note: 4 bit mode is NOT ignored for native SPI !                       
                                     //   DL=1  (8 bits bus), DL=0  (4 bits bus)                          
                                     //    N=1  (2 line mode), N=0 (1-line mode)
                                     //   RE=1  (Ena Extended Regs, special mode for KS0073)
                                     //   BE=0  (Blink Enable, CG/SEG RAM, special mode for KS0073)                   
                                     //   LP=0  (LP=1 Low power mode, LP=0 Normal)                                                                                     

              _function_x = 0x00;   // Ext Function set 0000 1 FW BW NW (Ext Regs)
                                    //   NW=0  (1,2 line), NW=1 (4 Line, special mode for KS0073)                                
              break;

            default:
              error("Error: LCD Controller type does not support this Display type\n\r"); 
              break;                            
          } // switch type

          // init special features
          _writeCommand(0x20 | _function_1);// Function set 001 DL N RE(1) BE LP (Ext Regs)
                                           //   DL=0 (4 bits bus), DL=1 (8 bits mode)            
                                           //    N=0 (1 line mode), N=1 (2 line mode)
                                           //   RE=1 (Ena Extended Regs, special mode for KS0073)
                                           //   BE=0 (Blink Enable/Disable, CG/SEG RAM, special mode for KS0073)                                
                                           //   LP=0  (LP=1 Low power mode, LP=0 Normal)                                                                                                                                

          _writeCommand(0x08 | _function_x); // Ext Function set 0000 1 FW BW NW (Ext Regs)
                                           //   FW=0  (5-dot font, special mode for KS0073)
                                           //   BW=0  (Cur BW invert disable, special mode for KS0073)
                                           //   NW=0  (1,2 Line), NW=1 (4 line, special mode for KS0073)                                

          _writeCommand(0x10);             // Scroll/Shift set 0001 DS/HS4 DS/HS3 DS/HS2 DS/HS1 (Ext Regs)
                                           //   Dotscroll/Display shift enable (Special mode for KS0073)

          _writeCommand(0x80);             // Scroll Quantity set 1 0 SQ5 SQ4 SQ3 SQ2 SQ1 SQ0 (Ext Regs)
                                           //   Scroll quantity (Special mode for KS0073)

          _writeCommand(0x20 | _function); // Function set 001 DL N RE(0) DH REV (Std Regs)
                                           //   DL=0  (4 bits bus), DL=1 (8 bits mode)             
                                           //    N=0  (1 line mode), N=1 (2 line mode)
                                           //   RE=0  (Dis. Extended Regs, special mode for KS0073)
                                           //   DH=1  (Disp shift enable/disable, special mode for KS0073)                                
                                           //   REV=0 (Reverse/Normal, special mode for KS0073)
          break; // case KS0073 Controller


      case KS0078:
          // Initialise Display configuration
          switch (_type) {
            case LCD8x1:         //8x1 is a regular 1 line display
            case LCD8x2B:        //8x2B is a special case of 16x1
//            case LCD12x1:                                
            case LCD16x1:                                            
//            case LCD20x1:
            case LCD24x1:
              _function = dl | 0x02;    // Function set 001 DL N RE(0) DH REV (Std Regs)
                                    //   DL=0  (4 bits bus)             
                                    //    N=0  (1 line mode), N=1 (2 line mode)
                                    //   RE=0  (Dis. Extended Regs, special mode for KS0078)
                                    //   DH=1  (Disp shift enable, special mode for KS0078)                                
                                    //   REV=0 (Reverse normal, special mode for KS0078)
                                    
              _function_1 = dl | 0x04;   // Function set 001 DL N RE(1) BE 0 (Ext Regs)
                                    //   DL=0  (4 bits bus)             
                                    //    N=0  (1 line mode), N=1 (2 line mode)
                                    //   RE=1  (Ena Extended Regs, special mode for KS0078)
                                    //   BE=0  (Blink Enable, CG/SEG RAM, special mode for KS0078)                                
                                    //      0 

              _function_x = 0x00;   // Ext Function set 0000 1 FW BW NW (Ext Regs)
                                    //    NW=0  (1,2 line), NW=1 (4 Line, special mode for KS0078)                                
              break;                                

//            case LCD12x3D:         // Special mode for KS0073, KS0078 and PCF21XX            
//            case LCD12x3D1:        // Special mode for KS0073, KS0078 and PCF21XX            
//            case LCD12x4D:         // Special mode for KS0073, KS0078 and PCF21XX            
//            case LCD16x3D:         // Special mode for KS0073, KS0078             
//            case LCD16x4D:         // Special mode for KS0073, KS0078            
//            case LCD20x4D:         // Special mode for KS0073, KS0078            
//            case LCD24x3D:         // Special mode for KS0078
//            case LCD24x3D1:        // Special mode for KS0078
            case LCD24x4D:         // Special mode for KS0078
              _function  = dl | 0x02;    // Function set 001 DL N RE(0) DH REV (Std Regs)
                                    //   DL=0  (4 bits bus)             
                                    //    N=0  (dont care for 4 line mode)              
                                    //   RE=0  (Dis. Extended Regs, special mode for KS0078)
                                    //   DH=1  (Disp shift enable, special mode for KS0078)                                
                                    //   REV=0 (Reverse normal, special mode for KS0078)
                                    
              _function_1 = dl | 0x04;   // Function set 001 DL N RE(1) BE 0 (Ext Regs)
                                    //   DL=0  (4 bits bus)             
                                    //    N=0  (1 line mode), N=1 (2 line mode)
                                    //   RE=1  (Ena Extended Regs, special mode for KS0078)
                                    //   BE=0  (Blink Enable, CG/SEG RAM, special mode for KS0078)                                
                                    //      0 

              _function_x = 0x01;   // Ext Function set 0000 1 FW BW NW (Ext Regs)
                                    //    NW=0  (1,2 line), NW=1 (4 Line, special mode for KS0078)                                
              break;                                

//            case LCD6x2:
            case LCD8x2:
            case LCD16x2:
//            case LCD16x1C:            
            case LCD20x2:
            case LCD24x2:
            case LCD32x2:           
            case LCD40x2:                       
              // All other LCD types are initialised as 2 Line displays (including LCD16x1C and LCD40x4)            
              _function  = dl | 0x0A;    // Function set 001 DL N RE(0) DH REV (Std Regs)
                                    //   DL=0  (4 bits bus)             
                                    //    N=1  (1 line mode), N=1 (2 line mode)
                                    //   RE=0  (Dis. Extended Regs, special mode for KS0078)
                                    //   DH=1  (Disp shift enable, special mode for KS0078)                                
                                    //   REV=0 (Reverse normal, special mode for KS0078)
                                    
              _function_1 = dl | 0x0C;   // Function set 001 DL N RE(1) BE 0 (Ext Regs)
                                    //   DL=0  (4 bits bus)             
                                    //    N=1  (1 line mode), N=1 (2 line mode)
                                    //   RE=1  (Ena Extended Regs, special mode for KS0078)
                                    //   BE=0  (Blink Enable, CG/SEG RAM, special mode for KS0078)                                
                                    //      0 

              _function_x = 0x00;   // Ext Function set 0000 1 FW BW NW (Ext Regs)
                                    //   NW=0  (1,2 line), NW=1 (4 Line, special mode for KS0078)                                
              break;

            default:
              error("Error: LCD Controller type does not support this Display type\n\r"); 
              break;                
          } // switch type

          // init special features
          _writeCommand(0x20 | _function_1);// Function set 001 DL N RE(1) BE 0 (Ext Regs)
                                           //   DL=0 (4 bits bus), DL=1 (8 bits mode)            
                                           //    N=0 (1 line mode), N=1 (2 line mode)
                                           //   RE=1 (Ena Extended Regs, special mode for KS0078)
                                           //   BE=0 (Blink Enable/Disable, CG/SEG RAM, special mode for KS0078)                                
                                           //      0 

          _writeCommand(0x08 | _function_x); // Ext Function set 0000 1 FW BW NW (Ext Regs)
                                           //   FW=0  (5-dot font, special mode for KS0078)
                                           //   BW=0  (Cur BW invert disable, special mode for KS0078)
                                           //   NW=0  (1,2 Line), NW=1 (4 line, special mode for KS0078)                                

          _writeCommand(0x10);             // Scroll/Shift set 0001 DS/HS4 DS/HS3 DS/HS2 DS/HS1 (Ext Regs)
                                           //   Dotscroll/Display shift enable (Special mode for KS0078)

          _writeCommand(0x80);             // Scroll Quantity set 1 0 SQ5 SQ4 SQ3 SQ2 SQ1 SQ0 (Ext Regs)
                                           //   Scroll quantity (Special mode for KS0078)

          _writeCommand(0x20 | _function); // Function set 001 DL N RE(0) DH REV (Std Regs)
                                           //   DL=0  (4 bits bus), DL=1 (8 bits mode)             
                                           //    N=0  (1 line mode), N=1 (2 line mode)
                                           //   RE=0  (Dis. Extended Regs, special mode for KS0078)
                                           //   DH=1  (Disp shift enable/disable, special mode for KS0078)                                
                                           //   REV=0 (Reverse/Normal, special mode for KS0078)
          break; // case KS0078 Controller
              
      case ST7032_3V3:
          // ST7032 controller: Initialise Voltage booster for VLCD. VDD=3V3
          // Note: very similar to SPLC792A
      case ST7032_5V:
          // ST7032 controller: Disable Voltage booster for VLCD. VDD=5V
      
          // Initialise Display configuration
          switch (_type) {
            case LCD8x1:         //8x1 is a regular 1 line display
            case LCD8x2B:        //8x2B is a special case of 16x1
//            case LCD12x1:                                
            case LCD16x1:                                            
//            case LCD20x1:                    
            case LCD24x1:
              _function = 0x00;       // FUNCTION SET 0 0 1 DL=0 (4 bit), N=0 (1-line display mode), F=0 (5*7dot), 0, IS
                                      // Note: 4 bit mode is ignored for native SPI and I2C devices
                                      // Saved to allow switch between Instruction sets at later time
              break;  

            case LCD12x3D:            // Special mode for KS0078 and PCF21XX
            case LCD12x3D1:           // Special mode for KS0078 and PCF21XX
            case LCD12x4D:            // Special mode for KS0078 and PCF21XX
            case LCD16x3G:            // Special mode for ST7036                        
            case LCD24x4D:            // Special mode for KS0078
              error("Error: LCD Controller type does not support this Display type\n\r"); 
              break;  

            default:
              // All other LCD types are initialised as 2 Line displays        
              _function = 0x08;       // FUNCTION SET 0 0 1 DL=0 (4 bit), N=1 (2-line display mode), F=0 (5*7dot), 0, IS              
                                      // Note: 4 bit mode is ignored for native SPI and I2C devices
                                      // Saved to allow switch between Instruction sets at later time
              break;                                                                        
          } // switch type    
                                     
          // init special features 
          _writeCommand(0x20 | _function | 0x01);           // Set function,  0 0 1 DL N F 0 IS=1 Select Instr Set = 1              

          _writeCommand(0x1C);                              // Internal OSC frequency adjustment Framefreq=183HZ, Bias will be 1/4 (Instr Set=1)
                                                            // Note: Bias and Osc register not available on SPLC792A 

          _contrast = LCD_ST7032_CONTRAST;              
          _writeCommand(0x70 | (_contrast & 0x0F));         // Set Contrast Low bits, 0 1 1 1 C3 C2 C1 C0 (IS=1)


          if (_ctrl == ST7032_3V3) {
//            _icon_power = 0x04;                             // Icon display off (Bit3=0), Booster circuit is turned on (Bit2=1) (IS=1)
            _icon_power = 0x0C;                             // Icon display on (Bit3=1), Booster circuit is turned on (Bit2=1)  (IS=1)
                                                            // Note: Booster circuit always on for SPLC792A, Bit2 is dont care
                                                            // Saved to allow contrast change at later time
          }
          else { 
//            _icon_power = 0x00;                             // Icon display off, Booster circuit is turned off  (IS=1)
            _icon_power = 0x08;                             // Icon display on, Booster circuit is turned off  (IS=1)            
                                                            // Saved to allow contrast change at later time
          }
          _writeCommand(0x50 | _icon_power | ((_contrast >> 4) & 0x03));  // Set Icon, Booster and Contrast High bits, 0 1 0 1 Ion Bon C5 C4 (IS=1)
          wait_ms(10);            // Wait 10ms to ensure powered up
          
          _writeCommand(0x68 | (LCD_ST7032_RAB & 0x07));      // Voltage follower, 0 1 1 0 FOn=1, Ampl ratio Rab2=1, Rab1=0, Rab0=0  (IS=1)
          wait_ms(10);            // Wait 10ms to ensure powered up
          
          _writeCommand(0x20 | _function);                  // Select Instruction Set = 0

          break; // case ST7032_3V3 Controller
                 // case ST7032_5V Controller

      case ST7036_3V3:
          // ST7036 controller: Initialise Voltage booster for VLCD. VDD=3V3
          // Note: supports 1,2 (LCD_T_A) or 3 lines (LCD_T_G)
      case ST7036_5V:
          // ST7036 controller: Disable Voltage booster for VLCD. VDD=5V
          // Note: supports 1,2 (LCD_T_A) or 3 lines (LCD_T_G)
                    
          // Initialise Display configuration
          switch (_type) {
            case LCD8x1:         //8x1 is a regular 1 line display
            case LCD8x2B:        //8x2D is a special case of 16x1
//            case LCD12x1:                                
            case LCD16x1:   
            case LCD24x1:                                                                         
              _function = 0x00;     // Set function, 0 0 1 DL=0 (4-bit Databus), N=0 (1 Line), DH=0 (5x7font), IS2, IS1 (Select Instruction Set)
                                    // Note: 4 bit mode is ignored for native SPI and I2C devices
                                    // Saved to allow switch between Instruction sets at later time
              
              _bias_lines = 0x04;   // Bias: 1/5, 1 or 2-Lines LCD 
              break;  

//            case LCD12x3G:          // Special mode for ST7036
            case LCD16x3G:          // Special mode for ST7036
              _function = 0x08;     // Set function, 0 0 1 DL=0 (4-bit Databus), N=1 (2 Line), DH=0 (5x7font), IS2,IS1 (Select Instruction Set)              
                                    // Note: 4 bit mode is ignored for native SPI and I2C devices
                                    // Saved to allow switch between Instruction sets at later time
              
              _bias_lines = 0x05;   // Bias: 1/5, 3-Lines LCD           
              break;  

//            case LCD12x3D1:           // Special mode for KS0078 and PCF21XX
//            case LCD16x3D1:           // Special mode for SSD1803
            case LCD12x4D:            // Special mode for PCF2116
            case LCD24x4D:            // Special mode for KS0078
              error("Error: LCD Controller type does not support this Display type\n\r"); 
              break;  

            default:
              // All other LCD types are initialised as 2 Line displays (including LCD16x1C and LCD40x4)       
              _function = 0x08;     // Set function, 0 0 1 DL=0 (4-bit Databus), N=1 (2 Line), DH=0 (5x7font), IS2,IS1 (Select Instruction Set)
                                    // Note: 4 bit mode is ignored for native SPI and I2C devices
                                    // Saved to allow switch between Instruction sets at later time
              
              _bias_lines = 0x04;   // Bias: 1/5, 1 or 2-Lines LCD 
              break;                
          } // switch type


          // init special features 
          _writeCommand(0x20 | _function | 0x01);   // Set function, IS2,IS1 = 01 (Select Instr Set = 1)
          _writeCommand(0x10 | _bias_lines);        // Set Bias and 1,2 or 3 lines (Instr Set 1)

          _contrast = LCD_ST7036_CONTRAST;
          _writeCommand(0x70 | (_contrast & 0x0F)); // Set Contrast, 0 1 1 1 C3 C2 C1 C0 (Instr Set 1)
                           
          if (_ctrl == ST7036_3V3) {
            _icon_power = 0x0C;                       // Set Icon, Booster, Contrast High bits, 0 1 0 1 Ion=1 Bon=1 C5 C4 (Instr Set 1)            
//            _icon_power = 0x04;                       // Set Icon, Booster, Contrast High bits, 0 1 0 1 Ion=0 Bon=1 C5 C4 (Instr Set 1)
                                                      // Saved to allow contrast change at later time
          } 
          else {
            _icon_power = 0x08;                       // Set Icon, Booster, Contrast High bits, 0 1 0 1 Ion=1 Bon=0 C5 C4 (Instr Set 1)             
//            _icon_power = 0x00;                       // Set Icon, Booster, Contrast High bits, 0 1 0 1 Ion=0 Bon=0 C5 C4 (Instr Set 1)                         
          }
          
          _writeCommand(0x50 | _icon_power | ((_contrast >> 4) & 0x03));   // Set Contrast C5, C4 (Instr Set 1)
          wait_ms(10);            // Wait 10ms to ensure powered up

          _writeCommand(0x68 | (LCD_ST7036_RAB & 0x07));  // Voltagefollower On = 1, Ampl ratio Rab2, Rab1, Rab0 = 1 0 1 (Instr Set 1)
          wait_ms(10);            // Wait 10ms to ensure powered up

          _writeCommand(0x20 | _function);          // Set function, IS2,IS1 = 00 (Select Instruction Set = 0)
         
          break; // case ST7036_3V3 Controller
                 // case ST7036_5V Controller

      case ST7070:                   
          // Initialise Display configuration
          switch (_type) {
            case LCD8x1:         //8x1 is a regular 1 line display
            case LCD8x2B:        //8x2D is a special case of 16x1
//            case LCD12x1:                                
            case LCD16x1:   
            case LCD24x1:                                                                         
              _function = dl | 0x00;      // Set function, 0 0 1 DL=0 (4-bit Databus), N=0 (1 Line), EXT=0, x, x 
                                          // Note: 4 bit mode is NOT ignored for native SPI !
                                          // Saved to allow switch between Instruction sets at later time
              break;  

//            case LCD12x3D1:           // Special mode for KS0078 and PCF21XX
//            case LCD16x3D1:           // Special mode for SSD1803
            case LCD12x4D:            // Special mode for PCF2116
            case LCD24x4D:            // Special mode for KS0078
//            case LCD12x3G:          // Special mode for ST7036
            case LCD16x3G:          // Special mode for ST7036           
              error("Error: LCD Controller type does not support this Display type\n\r"); 
              break;  

            default:
              // All other LCD types are initialised as 2 Line displays (including LCD16x1C and LCD40x4)       
              _function = dl | 0x08;   // Set function, 0 0 1 DL, N=1 (2 Line), EXT=0, x, x                                                                 
                                       // Note: 4 bit mode is NOT ignored for native SPI !
                                       // Saved to allow switch between Instruction sets at later time
              break;                
          } // switch type

//          _writeCommand(0x00);                      // NOP, make sure to sync SPI
          
          // init special features                                                    
          _writeCommand(0x20 | _function | 0x04);   // Set function, 0 0 1 DL N EXT=1 x x (Select Instr Set = 1)

          _writeCommand(0x04 | 0x00);               // Set Bias resistors  0 0 0 0 0 1 Rb1,Rb0= 0 0 (Extern Res) (Instr Set 1)

          _writeCommand(0x40 | 0x00);               // COM/SEG directions 0 1 0 0 C1, C2, S1, S2  (Instr Set 1)
                                                    // C1=1: Com1-8 -> Com8-1;   C2=1: Com9-16 -> Com16-9
                                                    // S1=1: Seg1-40 -> Seg40-1; S2=1: Seg41-80 -> Seg80-41                                                    
          
          _writeCommand(0x20 | _function);          // Set function, EXT=0 (Select Instr Set = 0)
         
          break; // case ST7070 Controller
         
      case SSD1803_3V3:
          // SSD1803 controller: Initialise Voltage booster for VLCD. VDD=3V3
          // Note: supports 1,2, 3 or 4 lines
//      case SSD1803_5V:
          // SSD1803 controller: No Voltage booster for VLCD. VDD=5V
                    
          // Initialise Display configuration
          switch (_type) {
            case LCD8x1:         //8x1 is a regular 1 line display
            case LCD8x2B:        //8x2D is a special case of 16x1
//            case LCD12x1:                                
            case LCD16x1:   
            case LCD24x1:                                                                         
              _function = 0x00;     //  Set function 0 0 1 DL N DH RE(0) IS 
                                    //  Saved to allow switch between Instruction sets at later time
                                    //    DL=0 4-bit Databus,
                                    //         Note: 4 bit mode is ignored for native SPI and I2C devices
                                    //     N=0 1 Line / 3 Line
                                    //    DH=0 Double Height disable 
                                    //    IS=0
          
              _function_1 = 0x02;   // Set function, 0 0 1 DL N BE RE(1) REV
                                    //  Saved to allow switch between Instruction sets at later time
                                    //    DL=0 4-bit Databus,
                                    //         Note: 4 bit mode is ignored for native SPI and I2C devices
                                    //     N=0 1 Line / 3 Line
                                    //    BE=0 Blink Enable off, special feature of SSD1803
                                    //   REV=0 Reverse off, special feature of SSD1803            
                        
              _lines = 0x00;        // Ext function set 0 0 0 0 1 FW BW NW 
                                    //    NW=0 1-Line LCD (N=0)
              break;  

            case LCD12x3D:          // Special mode for KS0078 and PCF21XX                                  
//            case LCD12x3D1:           // Special mode for KS0078 and PCF21XX            
            case LCD16x3D:          // Special mode for KS0078
//            case LCD16x3D1:           // Special mode for SSD1803
//            case LCD20x3D:            // Special mode for SSD1803
              _function = 0x00;     //  Set function 0 0 1 DL N DH RE(0) IS 
                                    //  Saved to allow switch between Instruction sets at later time
                                    //    DL=0 4-bit Databus,
                                    //         Note: 4 bit mode is ignored for native SPI and I2C devices
                                    //     N=0 1 Line / 3 Line
                                    //    DH=0 Double Height disable 
                                    //    IS=0
          
              _function_1 = 0x02;   // Set function, 0 0 1 DL N BE RE(1) REV
                                    //  Saved to allow switch between Instruction sets at later time
                                    //    DL=0 4-bit Databus,
                                    //         Note: 4 bit mode is ignored for native SPI and I2C devices
                                    //     N=0 1 Line / 3 Line
                                    //    BE=0 Blink Enable off, special feature of SSD1803
                                    //   REV=0 Reverse off, special feature of SSD1803            
                        
              _lines = 0x00;        // Ext function set 0 0 0 0 1 FW BW NW 
                                    //    NW=1 3-Line LCD (N=0)
              break;  

//            case LCD10x2D:          // Special mode for SSD1803, 4-line mode but switch to double height font
            case LCD10x4D:          // Special mode for SSD1803
            case LCD20x4D:          // Special mode for SSD1803
              _function = 0x08;     //  Set function 0 0 1 DL N DH RE(0) IS 
                                    //  Saved to allow switch between Instruction sets at later time
                                    //    DL=0 4-bit Databus,
                                    //         Note: 4 bit mode is ignored for native SPI and I2C devices
                                    //     N=1 4 Line
                                    //    DH=0 Double Height disable 
                                    //    IS=0
          
              _function_1 = 0x0A;   // Set function, 0 0 1 DL N BE RE(1) REV
                                    //  Saved to allow switch between Instruction sets at later time
                                    //    DL=0 4-bit Databus,
                                    //         Note: 4 bit mode is ignored for native SPI and I2C devices
                                    //     N=1 4 Line
                                    //    BE=0 Blink Enable off, special feature of SSD1803
                                    //   REV=0 Reverse off, special feature of SSD1803            
                        
              _lines = 0x01;        // Ext function set 0 0 0 0 1 FW BW NW 
                                    //    NW=1 4-Line LCD (N=1)
              break;  

            case LCD16x3G:          // Special mode for ST7036            
            case LCD24x4D:          // Special mode for KS0078
              error("Error: LCD Controller type does not support this Display type\n\r"); 
              break;  

            default:
              // All other LCD types are initialised as 2 Line displays (including LCD16x1C and LCD40x4)       
              _function = 0x08;     //  Set function 0 0 1 DL N DH RE(0) IS 
                                    //  Saved to allow switch between Instruction sets at later time
                                    //    DL=0 4-bit Databus,
                                    //         Note: 4 bit mode is ignored for native SPI and I2C devices
                                    //     N=1 2 line / 4 Line
                                    //    DH=0 Double Height disable 
                                    //    RE=0
                                    //    IS=0
          
              _function_1 = 0x0A;   // Set function, 0 0 1 DL N BE RE(1) REV
                                    //  Saved to allow switch between Instruction sets at later time
                                    //    DL=0 4-bit Databus,
                                    //         Note: 4 bit mode is ignored for native SPI and I2C devices
                                    //     N=1 2 line / 4 Line
                                    //    BE=0 Blink Enable off, special feature of SSD1803
                                    //    RE=1
                                    //   REV=0 Reverse off, special feature of SSD1803            
                        
              _lines = 0x00;        // Ext function set 0 0 0 0 1 FW BW NW 
                                    //    NW=0 2-Line LCD (N=1)
              break;                
          } // switch type


          // init special features 
          _writeCommand(0x20 | _function_1);        // Set function, 0 0 1 DL N BE RE(1) REV 
                                                    // Select Extended Instruction Set
          
          _writeCommand(0x06);                      // Set ext entry mode, 0 0 0 0 0 1 BDC=1 COM1-32, BDS=0 SEG100-1    "Bottom View" (Ext Instr Set)
//          _writeCommand(0x05);                      // Set ext entry mode, 0 0 0 0 0 1 BDC=0 COM32-1, BDS=1 SEG1-100    "Top View" (Ext Instr Set)          
          wait_ms(5);                               // Wait to ensure completion or SSD1803 fails to set Top/Bottom after reset..
         
          _writeCommand(0x08 | _lines);             // Set ext function 0 0 0 0 1 FW BW NW 1,2,3 or 4 lines (Ext Instr Set)

          _writeCommand(0x10);                      // Double Height and Bias, 0 0 0 1 UD2=0, UD1=0, BS1=0 Bias 1/5, DH=0 (Ext Instr Set)

//          _writeCommand(0x76);                      // Set TC Control, 0 1 1 1 0 1 1 0 (Ext Instr Set)
//          _writeData(0x02);                         // Set TC data,    0 0 0 0 0 TC2,TC1,TC0 = 0 1 0 (Ext Instr Set)

          _writeCommand(0x20 | _function | 0x01);   // Set function, 0 0 1 DL N DH RE(0) IS=1 Select Instruction Set 1
                                                    // Select Std Instr set, Select IS=1  

          _contrast = LCD_SSD1_CONTRAST;
          _writeCommand(0x70 | (_contrast & 0x0F)); // Set Contrast 0 1 1 1 C3, C2, C1, C0 (Instr Set 1)
                           
//          _icon_power = 0x04;                       // Icon off, Booster on (Instr Set 1)
          _icon_power = 0x0C;                       // Icon on, Booster on (Instr Set 1)          
                                                    // Saved to allow contrast change at later time
          _writeCommand(0x50 | _icon_power | ((_contrast >> 4) & 0x03));   // Set Power, Icon and Contrast, 0 1 0 1 Ion Bon C5 C4 (Instr Set 1)
          wait_ms(10);            // Wait 10ms to ensure powered up

          _writeCommand(0x68 | (LCD_SSD1_RAB & 0x07));  // Set Voltagefollower 0 1 1 0 Don = 1, Ampl ratio Rab2, Rab1, Rab0 = 1 1 0  (Instr Set 1)
          wait_ms(10);            // Wait 10ms to ensure powered up

          _writeCommand(0x20 | _function_1);        // Set function, 0 0 1 DL N BE RE(1) REV 
                                                    // Select Extended Instruction Set 1
          _writeCommand(0x10);                      // Shift/Scroll enable, 0 0 0 1 DS4/HS4 DS3/HS3 DS2/HS2 DS1/HS1  (Ext Instr Set 1)


          _writeCommand(0x20 | _function);          // Set function, 0 0 1 DL N DH RE(0) IS=0 Select Instruction Set 0
                                                    // Select Std Instr set, Select IS=0
         
          break; // case SSD1803 Controller

          
      // Note1: The PCF21XX family of controllers has several types that dont have an onboard voltage generator for V-LCD.
      //        You must supply this LCD voltage externally and not try to enable VGen. 
      // Note2: The early versions of PCF2116 controllers (eg PCF2116C) can not generate sufficiently negative voltage for the LCD at a VDD of 3V3. 
      //        You must supply this voltage externally and not enable VGen or you must use a higher VDD (e.g. 5V) and enable VGen.
      //        More recent versions of the controller (eg PCF2116K) have an improved VGen that will work with 3V3.
      // Note3: See datasheet, PCF2116 and other types provide a V0 pin to control the LCD contrast voltage that is provided by VGen. This pins allows 
      //        contrast control similar to that of pin 3 on the standard 14pin LCD module connector.
      //        You can disable VGen by connecting Vo to VDD. VLCD will then be used directly as LCD voltage.
      // Note4: PCF2113 and PCF2119 are different wrt to VLCD generator! There is no V0 pin. The contrast voltage is software controlled by setting the VA and VB registers.
      //        Vgen is automatically switched off when the contrast voltage VA or VB is set to 0x00. Note that certain limits apply to allowed values for VA and VB.                    
      // Note5: See datasheet, members of the PCF21XX family support different numbers of rows/columns. Not all can support 3 or 4 rows.
      // Note6: See datasheet, the PCF21XX-C and PCF21XX-K use a non-standard character set. This may result is strange looking text when not corrected..

      case PCF2103_3V3:
          // PCF2103 controller: No Voltage generator for VLCD, VDD=3V3..5V, VLCD input controls contrast voltage.                 
          // Initialise Display configuration
          switch (_type) {
            case LCD24x1:                    
              _function = 0x00;       //FUNCTION SET 0 0 1 DL=0 4-bit, 0, M=0 1-line/24 chars display mode, 0, H=0 
                                      //Note: 4 bit mode is ignored for I2C mode
              break;  

//            case LCD12x1D:            //Special mode for PCF21XX, Only top line used
            case LCD12x2:
              _function = 0x04;       //FUNCTION SET 0 0 1 DL=0 4-bit, 0, M=1 2-line/12 chars display mode, 0, H=0
                                      //Note: 4 bit mode is ignored for I2C mode
              break;  
              
            default:
              error("Error: LCD Controller type does not support this Display type\n\r"); 
              break;  
            
          } // switch type    

          _writeCommand(0x20 | _function | 0x01);          // Set function, Select Instr Set = 1              
          wait_ms(10);            // Wait 10ms to ensure powered up                                                    

// Note: Display from GA628 shows 12 chars. This is actually the right half of a 24x1 display. The commons have been connected in reverse order.
          _writeCommand(0x05);                             // Display Conf Set         0000 0, 1, P=0, Q=1               (Instr. Set 1)
                                                        
          _writeCommand(0x02);                             // Screen Config            0000 001, L=0  (Instr. Set 1)
          _writeCommand(0x08);                             // ICON Conf                0000 1, IM=0 (Char mode), IB=0 (no Icon blink), 0 (Instr. Set 1) 

          _writeCommand(0x20 | _function);                 // Set function, Select Instr Set = 0             
          
#if(0)
          // Select CG RAM
          _writeCommand(0x40); //Set CG-RAM address, 8 sequential locations needed per UDC
          // Store UDC/Icon pattern: 
          //   3 x 8 rows x 5 bits = 120 bits for Normal pattern (UDC 0..2) and
          //   3 x 8 rows x 5 bits = 120 bits for Blink pattern (UDC 4..6) 
          for (int i=0; i<(8 * 8); i++) {
//            _writeData(0x1F);  // All On
            _writeData(0x00);  // All Off            
          }
#endif
          break; // case PCF2103_3V3 Controller

      case PCF2113_3V3:
          // PCF2113 controller: Initialise Voltage booster for VLCD. VDD=3V3. VA and VB control contrast.
          // Initialise Display configuration
          switch (_type) {
//            case LCD12x1:                                
//              _function = 0x02;       // FUNCTION SET 0 0 1 DL=0 4 bit, 0, M=0 1-line/12 chars display mode, SL=1, IS=0
                                      // Note: 4 bit mode is ignored for I2C mode
            case LCD24x1:                    
              _function = 0x00;       // FUNCTION SET 0 0 1 DL=0 4 bit, 0, M=0 1-line/24 chars display mode, SL=0, IS=0            
                                      // Note: 4 bit mode is ignored for I2C mode
              break;  

            case LCD12x2:                    
              _function = 0x04;       // FUNCTION SET 0 0 1 DL=0 4 bit, 0, M=1 2-line/12 chars display mode, SL=0, IS=0            
              break;  
             
            default:
              error("Error: LCD Controller type does not support this Display type\n\r"); 
              break;  
                         
          } // switch type    

          // Init special features
          _writeCommand(0x20 | _function | 0x01);          // Set function, Select Instr Set = 1              

          _writeCommand(0x04);                             // Display Conf Set         0000 0, 1, P=0, Q=0               (Instr. Set 1)
          _writeCommand(0x10);                             // Temp Compensation Set    0001 0, 0, TC1=0, TC2=0           (Instr. Set 1)
//          _writeCommand(0x42);                             // HV GEN                   0100 S1=1, S2=0 (2x multiplier)   (Instr. Set 1)
          _writeCommand(0x40 | (LCD_PCF2_S12 & 0x03));     // HV Gen                   0100 S1=1, S2=0 (2x multiplier)   (Instr. Set 1)
          
          _contrast = LCD_PCF2_CONTRAST;              
          _writeCommand(0x80 | 0x00 | (_contrast & 0x3F));      // VLCD_set (Instr. Set 1)  1, V=0, VA=contrast
          _writeCommand(0x80 | 0x40 | (_contrast & 0x3F));      // VLCD_set (Instr. Set 1)  1, V=1, VB=contrast
          wait_ms(10);            // Wait 10ms to ensure powered up
          
          _writeCommand(0x02);                             // Screen Config            0000 001, L=0  (Instr. Set 1)
          _writeCommand(0x08);                             // ICON Conf                0000 1, IM=0 (Char mode), IB=0 (no icon blink) DM=0 (no direct mode) (Instr. Set 1) 

          _writeCommand(0x20 | _function);                 // Set function, Select Instr Set = 0             

          break; // case PCF2113_3V3 Controller


//      case PCF2113_5V:
          // PCF2113 controller: No Voltage generator for VLCD. VDD=5V. Contrast voltage controlled by VA or VB.
//@TODO                            


      case PCF2116_3V3:
          // PCF2116 controller: Voltage generator for VLCD. VDD=5V. V0 controls contrast voltage.                 
          // Initialise Display configuration
          switch (_type) {
//            case LCD12x1:
//            case LCD12x2:                                                                            
            case LCD24x1:                    
              _writeCommand(0x22);    //FUNCTION SET 0 0 1 DL=0 4-bit, N=0/M=0 1-line/24 chars display mode, G=1 Vgen on, 0 
                                      //Note: 4 bit mode is ignored for I2C mode
              wait_ms(10);            // Wait 10ms to ensure powered up                                                    
              break;  

            case LCD12x3D:            // Special mode for KS0078 and PCF21XX                            
            case LCD12x3D1:           // Special mode for PCF21XX                     
            case LCD12x4D:            // Special mode for PCF21XX:
              _writeCommand(0x2E);    //FUNCTION SET 0 0 1 DL=0 4-bit, N=1/M=1 4-line/12 chars display mode, G=1 VGen on, 0                               
                                      //Note: 4 bit mode is ignored for I2C mode              
              wait_ms(10);            // Wait 10ms to ensure powered up                                                    
              break;  

            case LCD24x2:
              _writeCommand(0x2A);    //FUNCTION SET 0 0 1 DL=0 4-bit, N=1/M=0 2-line/24 chars display mode, G=1 VGen on, 0
                                      //Note: 4 bit mode is ignored for I2C mode
              wait_ms(10);            // Wait 10ms to ensure powered up   
              break;  
              
            default:
              error("Error: LCD Controller type does not support this Display type\n\r"); 
              break;  
            
          } // switch type    

          break; // case PCF2116_3V3 Controller


//Experimental for cellphone 3-line display, SA=0x74, No Ack supported, Character set C or K, DL = 8 bit, N=0,M=1 (reserved mode !!), external VLCD -2V5
//@TODO                            
      case PCF2116_5V:
          // PCF2116 controller: No Voltage generator for VLCD. VDD=5V. V0 controls contrast voltage.                           
          // Initialise Display configuration
          switch (_type) {
//            case LCD12x1:
//            case LCD12x2:                                                                            
//            case LCD24x1:                    
//              _writeCommand(0x20);    //FUNCTION SET 0 0 1 DL=0 4-bit, N=0/M=0 1-line/24 chars display mode, G=0 no Vgen, 0 
                                      //Note: 4 bit mode is ignored for I2C mode
//              wait_ms(10);            // Wait 10ms to ensure powered up                                                    
//              break;  

            case LCD12x3D:            // Special mode for KS0078 and PCF21XX                            
            case LCD12x3D1:           // Special mode for PCF21XX                     
            case LCD12x4D:            // Special mode for PCF21XX:
//              _writeCommand(0x34);    //FUNCTION SET 8 bit, N=0/M=1 4-line/12 chars display mode      OK
//              _writeCommand(0x24);    //FUNCTION SET 4 bit, N=0/M=1 4-line/12 chars display mode      OK                                            
              _writeCommand(0x2C);    //FUNCTION SET 0 0 1 DL=0 4-bit, N=1/M=1 4-line/12 chars display mode, G=0 no Vgen, 0  OK       
                                      //Note: 4 bit mode is ignored for I2C mode              
              wait_ms(10);            // Wait 10ms to ensure powered up                                                    
              break;  

//            case LCD24x2:
//              _writeCommand(0x28);    //FUNCTION SET 4 bit, N=1/M=0 2-line/24 chars display mode
                                      //Note: 4 bit mode is ignored for I2C mode
//              wait_ms(10);            // Wait 10ms to ensure powered up   
//              break;  
              
            default:
              error("Error: LCD Controller type does not support this Display type\n\r"); 
              break;  
            
          } // switch type    

          break; // case PCF2116_5V Controller

      case PCF2119_3V3:
      case PCF2119R_3V3:
          // PCF2119 controller: Initialise Voltage booster for VLCD. VDD=3V3. VA and VB control contrast.
          // Note1: See datasheet, the PCF2119 supports icons and provides separate constrast control for Icons and characters.
          // Note2: Vgen is switched off when the contrast voltage VA or VB is set to 0x00.
                  
//POR or Hardware Reset should be applied
          wait_ms(10);            // Wait 10ms to ensure powered up   

          // Initialise Display configuration
          switch (_type) {
            case LCD8x1:
//            case LCD12x1:
            case LCD16x1:           
              _function = 0x02;       // FUNCTION SET 0 0 1 DL=0 4-bit, 0 , M=0 1-line/16 chars display mode, SL=1
                                      // Note: 4 bit mode is ignored for I2C mode
              break;  
            
            case LCD24x1:                    
//            case LCD32x1:                                
              _function = 0x00;       // FUNCTION SET 0 0 1 DL=0 4-bit, 0 , M=0 1-line/32 chars display mode, SL=0
                                      // Note: 4 bit mode is ignored for I2C mode
              break;  

            case LCD8x2:
//            case LCD12x2:            
            case LCD16x2:
              _function = 0x04;       // FUNCTION SET 0 0 1 DL=0 4-bit, 0, M=1 2-line/16 chars display mode, SL=0
                                      // Note: 4 bit mode is ignored for I2C mode
              break;  
             
            default:
              error("Error: LCD Controller type does not support this Display type\n\r"); 
              break;  
            
          } // switch type    

          // Init special features 
          _writeCommand(0x20 | _function | 0x01);           // Set function, Select Instruction Set = 1              

//          _writeCommand(0x04);    // DISP CONF SET (Instr. Set 1)   0000, 0, 1, P=0, Q=0 (IC at Bottom)
//          _writeCommand(0x05);    // Display Conf Set               0000, 0, 1, P=0, Q=1
//          _writeCommand(0x06);    // Display Conf Set               0000, 0, 1, P=1, Q=0
          _writeCommand(0x07);    // Display Conf Set               0000, 0, 1, P=1, Q=1    (IC at Top)

          _writeCommand(0x10);    // TEMP CTRL SET (Instr. Set 1)   0001, 0, 0, TC1=0, TC2=0
//          _writeCommand(0x42);    // HV GEN (Instr. Set 1)          0100, 0, 0, S1=1, S2=0 (2x multiplier)
          _writeCommand(0x40 | (LCD_PCF2_S12 & 0x03));      // HV GEN (Instr. Set 1)          0100, 0, 0, S1=1, S2=0 (2x multiplier)

          _contrast = LCD_PCF2_CONTRAST;              
          _writeCommand(0x80 | 0x00 | (_contrast & 0x3F));      // VLCD_set (Instr. Set 1)    V=0, VA=contrast
          _writeCommand(0x80 | 0x40 | (_contrast & 0x3F));      // VLCD_set (Instr. Set 1)    V=1, VB=contrast
          wait_ms(10);            // Wait 10ms to ensure powered up
          
          _writeCommand(0x02);    // SCRN CONF (Instr. Set 1)    L=0
          _writeCommand(0x08);    // ICON CONF (Instr. Set 1)    IM=0 (Char mode) IB=0 (no icon blink) DM=0 (no direct mode)

          _writeCommand(0x20 | _function);                  // Select Instruction Set = 0

          break; // case PCF2119_3V3 Controller

//      case PCF2119_5V:
          // PCF2119 controller: No Voltage booster for VLCD. VDD=3V3. VA and VB control contrast.
          // Note1: See datasheet, the PCF2119 supports icons and provides separate constrast control for Icons and characters.
          // Note2: Vgen is switched off when the contrast voltage VA or VB is set to 0x00.                     
//@TODO                            

      case WS0010:         
          // WS0010 OLED controller: Initialise DC/DC Voltage converter for LEDs
          // Note1: Identical to RS0010  
          // Note2: supports 1 or 2 lines (and 16x100 graphics)
          //        supports 4 fonts (English/Japanese (default), Western European-I, English/Russian, Western European-II)
                           // Cursor/Disp shift set 0001 SC RL  0 0
                           //
                           // Mode and Power set    0001 GC PWR 1 1                           
                           //  GC  = 0 (Graph Mode=1, Char Mode=0)             
                           //  PWR = 1 (DC/DC On/Off)
   
//@Todo: This may be needed to enable a warm reboot
          //_writeCommand(0x13);   // Char mode, DC/DC off              
          //wait_ms(10);           // Wait 10ms to ensure powered down                  
          _writeCommand(0x17);   // Char mode, DC/DC on        
          wait_ms(10);           // Wait 10ms to ensure powered up        

          // Initialise Display configuration
          switch (_type) {                    
            case LCD8x1:         //8x1 is a regular 1 line display
            case LCD8x2B:        //8x2B is a special case of 16x1
//            case LCD12x1:                                
            case LCD16x1:                                            
            case LCD24x1:
              _writeCommand(0x20); // Function set 001 DL N F FT1 FT0
                                   //  DL=0  (4 bits bus)             
                                   //   N=0  (1 line)
                                   //   F=0  (5x7 dots font)
                                   //  FT=00 (00 = Engl/Jap, 01 = WestEur1, 10 = Engl/Russian, 11 = WestEur2
              break;  

            case LCD12x3D:            // Special mode for KS0078 and PCF21XX                            
            case LCD12x3D1:           // Special mode for PCF21XX                     
            case LCD12x4D:            // Special mode for PCF21XX:
            case LCD16x3G:            // Special mode for ST7036            
            case LCD24x4D:            // Special mode for KS0078
              error("Error: LCD Controller type does not support this Display type\n\r"); 
              break;  

            default:
              // All other LCD types are initialised as 2 Line displays (including LCD16x1C and LCD40x4)       
              _writeCommand(0x28); // Function set 001 DL N F FT1 FT0
                                   //  DL=0  (4 bits bus)
                                   //   N=1  (2 lines)
                                   //   F=0  (5x7 dots font)
                                   //  FT=00 (00 = Engl/Jap, 01 = WestEur1, 10 = Engl/Russian, 11 = WestEur2

              break;
           } // switch type
           
           break; // case WS0010 Controller


      case US2066_3V3:
          // US2066/SSD1311 OLED controller, Initialise for VDD=3V3
          // Note: supports 1,2, 3 or 4 lines
//      case USS2066_5V:
          // US2066 controller, VDD=5V
                    
          // Initialise Display configuration
          switch (_type) {
            case LCD8x1:         //8x1 is a regular 1 line display
            case LCD8x2B:        //8x2D is a special case of 16x1
//            case LCD12x1:                                
            case LCD16x1:   
//            case LCD20x1:                                                                         
              _function = 0x00;     //  Set function 0 0 1 X N DH RE(0) IS 
                                    //  Saved to allow switch between Instruction sets at later time
                                    //    DL=X bit is ignored for US2066. Uses hardwired pins instead
                                    //     N=0 1 Line / 3 Line
                                    //    DH=0 Double Height disable 
                                    //    IS=0
          
              _function_1 = 0x02;   // Set function, 0 0 1 X N BE RE(1) REV
                                    //  Saved to allow switch between Instruction sets at later time
                                    //    DL=X bit is ignored for US2066. Uses hardwired pins instead                                    
                                    //     N=0 1 Line / 3 Line
                                    //    BE=0 Blink Enable off, special feature of SSD1803, US2066
                                    //   REV=0 Reverse off, special feature of SSD1803, US2066            
                        
              _lines = 0x00;        // Ext function set 0 0 0 0 1 FW BW NW 
                                    //    NW=0 1-Line LCD (N=0)
              break;  

            case LCD16x1C:
            case LCD8x2:
            case LCD16x2:
            case LCD20x2:            
              _function = 0x08;     //  Set function 0 0 1 X N DH RE(0) IS 
                                    //  Saved to allow switch between Instruction sets at later time
                                    //    DL=X bit is ignored for US2066. Uses hardwired pins instead                                                                        
                                    //     N=1 2 line / 4 Line
                                    //    DH=0 Double Height disable 
                                    //    IS=0
          
              _function_1 = 0x0A;   // Set function, 0 0 1 X N BE RE(1) REV
                                    //  Saved to allow switch between Instruction sets at later time
                                    //    DL=X bit is ignored for US2066. Uses hardwired pins instead                                                                        
                                    //     N=1 2 line / 4 Line
                                    //    BE=0 Blink Enable off, special feature of SSD1803, US2066
                                    //   REV=0 Reverse off, special feature of SSD1803, US2066            
                        
              _lines = 0x00;        // Ext function set 0 0 0 0 1 FW BW NW 
                                    //    NW=0 2-Line LCD (N=1)
              break;                

            case LCD12x3D:          // Special mode for KS0078 and PCF21XX 
//            case LCD12x3D1:           // Special mode for KS0078 and PCF21XX            
            case LCD16x3D:          // Special mode for KS0078, SSD1803 and US2066
//            case LCD16x3D1:           // Special mode for SSD1803, US2066
//            case LCD20x3D:            // Special mode for SSD1803, US2066
              _function = 0x00;     //  Set function 0 0 1 X N DH RE(0) IS 
                                    //  Saved to allow switch between Instruction sets at later time
                                    //    DL=X bit is ignored for US2066. Uses hardwired pins instead                                    
                                    //     N=0 1 Line / 3 Line
                                    //    DH=0 Double Height disable 
                                    //    IS=0
          
              _function_1 = 0x02;   // Set function, 0 0 1 X N BE RE(1) REV
                                    //  Saved to allow switch between Instruction sets at later time
                                    //    DL=X bit is ignored for US2066. Uses hardwired pins instead                                    
                                    //     N=0 1 Line / 3 Line
                                    //    BE=0 Blink Enable off, special feature of SSD1803, US2066
                                    //   REV=0 Reverse off, special feature of SSD1803, US2066            
                        
              _lines = 0x00;        // Ext function set 0 0 0 0 1 FW BW NW 
                                    //    NW=1 3-Line LCD (N=0)
              break;  

            case LCD20x4D:          // Special mode for SSD1803, US2066
              _function = 0x08;     //  Set function 0 0 1 X N DH RE(0) IS 
                                    //  Saved to allow switch between Instruction sets at later time
                                    //    DL=X bit is ignored for US2066. Uses hardwired pins instead
                                    //     N=1 2 line / 4 Line
                                    //    DH=0 Double Height disable 
                                    //    IS=0
          
              _function_1 = 0x0A;   // Set function, 0 0 1 DL N BE RE(1) REV
                                    //  Saved to allow switch between Instruction sets at later time
                                    //    DL=0 bit is ignored for US2066. Uses hardwired pins instead                                    
                                    //     N=1 2 line / 4 Line
                                    //    BE=0 Blink Enable off, special feature of SSD1803, US2066
                                    //   REV=0 Reverse off, special feature of SSD1803, US2066            
                        
              _lines = 0x01;        // Ext function set 0 0 0 0 1 FW BW NW 
                                    //    NW=1 4-Line LCD (N=1)
              break;  

//            case LCD24x1:                                                                         
//            case LCD16x3G:          // Special mode for ST7036            
//            case LCD24x4D:          // Special mode for KS0078
            default:            
              error("Error: LCD Controller type does not support this Display type\n\r"); 
              break;  

          } // switch type

          _writeCommand(0x00);                      // NOP, make sure to sync SPI

          // init special features 
          _writeCommand(0x20 | _function_1);        // Set function, 0 0 1 X N BE RE(1) REV 
                                                    // Select Extended Instruction Set

          _writeCommand(0x71);                      // Function Select A: 0 1 1 1 0 0 0 1 (Ext Instr Set)
          _writeData(0x00);                         // Disable Internal VDD

          _writeCommand(0x79);                      // Function Select OLED:  0 1 1 1 1 0 0 1 (Ext Instr Set)

          _writeCommand(0xD5);                      // Display Clock Divide Ratio: 1 1 0 1 0 1 0 1 (Ext Instr Set, OLED Instr Set)
          _writeCommand(0x70);                      // Display Clock Divide Ratio value: 0 1 1 1 0 0 0 0 (Ext Instr Set, OLED Instr Set)
                    
          _writeCommand(0x78);                      // Function Disable OLED: 0 1 1 1 1 0 0 0 (Ext Instr Set)
          
//          _writeCommand(0x06);                      // Set ext entry mode, 0 0 0 0 0 1 BDC=1 COM1-32, BDS=0 SEG100-1    "Bottom View" (Ext Instr Set)
          _writeCommand(0x05);                      // Set ext entry mode, 0 0 0 0 0 1 BDC=0 COM32-1, BDS=1 SEG1-100    "Top View" (Ext Instr Set)          
         
          _writeCommand(0x08 | _lines);             // Set ext function 0 0 0 0 1 FW BW NW 1,2,3 or 4 lines (Ext Instr Set)

//          _writeCommand(0x1C);                      // Double Height, 0 0 0 1 UD2=1, UD1=1, X, DH'=0 (Ext Instr Set)
//                                                    // Default

          _writeCommand(0x72);                      // Function Select B: 0 1 1 1 0 0 1 0 (Ext Instr Set)
          _writeData(0x01);                         // Select ROM A (CGRAM 8, CGROM 248)

          _writeCommand(0x79);                      // Function Select OLED:  0 1 1 1 1 0 0 1 (Ext Instr Set)

          _writeCommand(0xDA);                      // Set Segm Pins Config:  1 1 0 1 1 0 1 0 (Ext Instr Set, OLED)
          _writeCommand(0x10);                      // Set Segm Pins Config value: Altern Odd/Even, Disable Remap (Ext Instr Set, OLED)

          _writeCommand(0xDC);                      // Function Select C: 1 1 0 1 1 1 0 0 (Ext Instr Set, OLED)
//          _writeCommand(0x00);                      // Set internal VSL, GPIO pin HiZ (always read low)
          _writeCommand(0x80);                      // Set external VSL, GPIO pin HiZ (always read low)

          _contrast = LCD_US20_CONTRAST;
          _writeCommand(0x81);                      // Set Contrast Control: 1 0 0 0 0 0 0 1 (Ext Instr Set, OLED)
          _writeCommand((_contrast << 2) | 0x03);   // Set Contrast Value: 8 bits, use 6 bits for compatibility 

          _writeCommand(0xD9);                      // Set Phase Length: 1 1 0 1 1 0 0 1 (Ext Instr Set, OLED)
          _writeCommand(0xF1);                      // Set Phase Length Value: 

          _writeCommand(0xDB);                      // Set VCOMH Deselect Lvl: 1 1 0 1 1 0 1 1 (Ext Instr Set, OLED)
          _writeCommand(0x30);                      // Set VCOMH Deselect Value: 0.83 x VCC

          wait_ms(10);            // Wait 10ms to ensure powered up

//Test Fade/Blinking. Hard Blink on/off, No fade in/out ??
//          _writeCommand(0x23);                      // Set (Ext Instr Set, OLED)
//          _writeCommand(0x3F);                      // Set interval 128 frames
//End Test Blinking

          _writeCommand(0x78);                      // Function Disable OLED: 0 1 1 1 1 0 0 0 (Ext Instr Set)
          
          _writeCommand(0x20 | _function | 0x01);   // Set function, 0 0 1 X N DH RE(0) IS=1 Select Instruction Set 1
                                                    // Select Std Instr set, Select IS=1  

          _writeCommand(0x20 | _function_1);        // Set function, 0 0 1 X N BE RE(1) REV 
                                                    // Select Ext Instr Set, IS=1
          _writeCommand(0x10);                      // Shift/Scroll enable, 0 0 0 1 DS4/HS4 DS3/HS3 DS2/HS2 DS1/HS1  (Ext Instr Set, IS=1)

          _writeCommand(0x20 | _function);          // Set function, 0 0 1 DL N DH RE(0) IS=0 Select Instruction Set 0
                                                    // Select Std Instr set, Select IS=0        
          break; // case US2066/SSD1311 Controller

      //not yet tested on hardware
      case PT6314 :
          // Initialise Display configuration
          switch (_type) {
            case LCD8x1:         //8x1 is a regular 1 line display
            case LCD8x2B:        //8x2B is a special case of 16x1
//            case LCD12x1:                                
            case LCD16x1:                                            
            case LCD20x1:                                                        
            case LCD24x1:
              _function = 0x00;    // Function set 001 DL N X BR1 BR0
                                   //  DL=0 (4 bits bus)
                                   //  Note: 4 bit mode is ignored for native SPI and I2C devices                                                                                 
                                   //  N=0 (1 line)
                                   //  X
                                   //  BR1=0 (2 significant bits for brightness
                                   //  BR0=0 
                                   //           0x0 = 100%
                                   //           0x1 =  75%
                                   //           0x2 =  50%
                                   //           0x3 =  25%                

              break;                                
                                                  
            // All other valid LCD types are initialised as 2 Line displays
            case LCD8x2:  
            case LCD16x2:  
            case LCD20x2:
            case LCD24x2:
              _function = 0x08;    // Function set 001 DL N X BR1 BR2
                                   //  DL=0 (4 bits bus)
                                   //  Note: 4 bit mode is ignored for native SPI and I2C devices                                 
                                   //  N=1 (2 lines)
                                   //  X
                                   //  BR1=0 (2 significant bits for brightness
                                   //  BR0=0 
              break;
              
            default:            
              error("Error: LCD Controller type does not support this Display type\n\r"); 
              break;               
          } // switch type
           
          _contrast = LCD_PT63_CONTRAST;
          _writeCommand(0x20 | _function | ((~_contrast) >> 4));        // Invert and shift to use 2 MSBs     
          break; // case PT6314 Controller (VFD)


      case HD66712:
          // Initialise Display configuration
          switch (_type) {
            case LCD8x1:         //8x1 is a regular 1 line display
            case LCD12x1:                                
            case LCD16x1:                                            
            case LCD20x1:
            case LCD24x1:
//            case LCD32x1:        // EXT pin is High, extension driver needed
              _function  = 0x02;    // Function set 001 DL N RE(0) - - (Std Regs)
                                    //   DL=0  (4 bits bus)             
                                    //    N=0  (1-line mode, N=1 2-line mode)
                                    //   RE=0  (Dis. Extended Regs, special mode for HD66712)
                                    //   
                                    
              _function_1 = 0x04;   // Function set 001 DL N RE(1) BE LP (Ext Regs)
                                    //   DL=0  (4 bits bus)             
                                    //    N=0  (1-line mode, N=1 2-line mode)
                                    //   RE=1  (Ena Extended Regs; special mode for HD66712)
                                    //   BE=0  (Blink Enable, CG/SEG RAM; special mode for HD66712)                                
                                    //   LP=0  (LP=1 Low power mode, LP=0 Normal; special mode for HD66712)

              _function_x = 0x00;   // Ext Function set 0000 1 FW BW NW (Ext Regs)
                                    //    NW=0  (1,2 line), NW=1 (4 Line, special mode for HD66712)                                
              break;                                

//            case LCD12x3D:         // Special mode for KS0073, KS0078, PCF21XX and HD66712
//            case LCD12x3D1:        // Special mode for KS0073, KS0078, PCF21XX and HD66712
            case LCD12x4D:         // Special mode for KS0073, KS0078, PCF21XX and HD66712
//            case LCD16x3D:         // Special mode for KS0073, KS0078 and HD66712
//            case LCD16x4D:         // Special mode for KS0073, KS0078 and HD66712            
            case LCD20x4D:         // Special mode for KS0073, KS0078 and HD66712            
              _function  = 0x02;    // Function set 001 DL N RE(0) - - (Std Regs)
                                    //   DL=0  (4 bits bus)             
                                    //    N=0  (1-line mode, N=1 2-line mode)
                                    //   RE=0  (Dis. Extended Regs, special mode for HD66712)
                                    //   
                                    
              _function_1 = 0x04;   // Function set 001 DL N RE(1) BE LP (Ext Regs)
                                    //   DL=0  (4 bits bus)             
                                    //    N=0  (1-line mode, N=1 2-line mode)
                                    //   RE=1  (Ena Extended Regs; special mode for HD66712)
                                    //   BE=0  (Blink Enable, CG/SEG RAM; special mode for HD66712)                                
                                    //   LP=0  (LP=1 Low power mode, LP=0 Normal; special mode for HD66712)

              _function_x = 0x01;   // Ext Function set 0000 1 FW BW NW (Ext Regs)
                                    //    NW=0  (1,2 line), NW=1 (4 Line, special mode for HD66712)                                
              break;                                

            case LCD16x3G:            // Special mode for ST7036                        
//            case LCD24x3D:         // Special mode for KS0078
//            case LCD24x3D1:        // Special mode for KS0078
            case LCD24x4D:         // Special mode for KS0078
              error("Error: LCD Controller type does not support this Display type\n\r"); 
              break;  

            default:
              // All other LCD types are initialised as 2 Line displays (including LCD16x1C and LCD40x4)            
              _function  = 0x0A;    // Function set 001 DL N RE(0) - - (Std Regs)
                                    //   DL=0  (4 bits bus)             
                                    //    N=1  (2-line mode), N=0 (1-line mode)
                                    //   RE=0  (Dis. Extended Regs, special mode for HD66712)
                                    
              _function_1 = 0x0C;   // Function set 001 DL N RE(1) BE LP (Ext Regs)
                                    //   DL=0  (4 bits bus)             
                                    //    N=1  (2 line mode), N=0 (1-line mode)
                                    //   RE=1  (Ena Extended Regs, special mode for HD66712)
                                    //   BE=0  (Blink Enable, CG/SEG RAM, special mode for HD66712)
                                    //   LP=0  (LP=1 Low power mode, LP=0 Normal)

              _function_x = 0x00;   // Ext Function set 0000 1 FW BW NW (Ext Regs)
                                    //   NW=0  (1,2 line), NW=1 (4 Line, special mode for HD66712)
              break;
          } // switch type

          // init special features
          _writeCommand(0x20 | _function_1);// Function set 001 DL N RE(1) BE LP (Ext Regs)
                                           //   DL=0 (4 bits bus), DL=1 (8 bits mode)            
                                           //    N=0 (1 line mode), N=1 (2 line mode)
                                           //   RE=1 (Ena Extended Regs, special mode for HD66712)
                                           //   BE=0 (Blink Enable/Disable, CG/SEG RAM, special mode for HD66712)                                
                                           //   LP=0  (LP=1 Low power mode, LP=0 Normal)                                                                                                                                

          _writeCommand(0x08 | _function_x); // Ext Function set 0000 1 FW BW NW (Ext Regs)
                                           //   FW=0  (5-dot font, special mode for HD66712)
                                           //   BW=0  (Cur BW invert disable, special mode for HD66712)
                                           //   NW=0  (1,2 Line), NW=1 (4 line, special mode for HD66712)                                

          _writeCommand(0x10);             // Scroll/Shift set 0001 HS4 HS3 HS2 HS1 (Ext Regs)
                                           //   Dotscroll/Display shift enable (Special mode for HD66712)

          _writeCommand(0x80);             // Scroll Quantity set 1 0 HDS5 HDS4 HDS3 HDS2 HDS1 HDS0 (Ext Regs)
                                           //   Scroll quantity (Special mode for HD66712)

          _writeCommand(0x20 | _function); // Function set 001 DL N RE(0) DH REV (Std Regs)
                                           //   DL=0  (4 bits bus), DL=1 (8 bits mode)             
                                           //    N=0  (1 line mode), N=1 (2 line mode)
                                           //   RE=0  (Dis. Extended Regs, special mode for HD66712)
                                           //   DH=1  (Disp shift enable/disable, special mode for HD66712)
                                           //   REV=0 (Reverse/Normal, special mode for HD66712)
          break; // case HD66712 Controller

      case SPLC792A_3V3:      
          // SPLC792A controller: Initialise Voltage booster for VLCD. VDD=3V3
          // Note very similar to ST7032
      
          // Initialise Display configuration
          switch (_type) {
            case LCD8x1:         //8x1 is a regular 1 line display
            case LCD8x2B:        //8x2B is a special case of 16x1
//            case LCD12x1:                                
            case LCD16x1:                                            
//            case LCD20x1:                    
            case LCD24x1:
              _function = 0x00;       // FUNCTION SET 0 0 1 DL=0 (4 bit), N=0 (1-line display mode), F=0 (5*7dot), 0, IS
                                      // Note: 4 bit mode is ignored for native SPI and I2C devices
                                      // Saved to allow switch between Instruction sets at later time
              break;  

            case LCD12x3D:            // Special mode for KS0078 and PCF21XX
            case LCD12x3D1:           // Special mode for KS0078 and PCF21XX
            case LCD12x4D:            // Special mode for KS0078 and PCF21XX
            case LCD16x3G:            // Special mode for ST7036                        
            case LCD24x4D:            // Special mode for KS0078
              error("Error: LCD Controller type does not support this Display type\n\r"); 
              break;  

            default:
              // All other LCD types are initialised as 2 Line displays        
              _function = 0x08;       // FUNCTION SET 0 0 1 DL=0 (4 bit), N=1 (2-line display mode), F=0 (5*7dot), 0, IS              
                                      // Note: 4 bit mode is ignored for native SPI and I2C devices
                                      // Saved to allow switch between Instruction sets at later time
              break;                                                                        
          } // switch type    
                                     
          // init special features 
          _writeCommand(0x20 | _function | 0x01);           // Set function,  0 0 1 DL N F 0 IS=1 Select Instr Set = 1              

//SPLC792A Does not support Bias and Internal Osc register
//          _writeCommand(0x1C);                              // Internal OSC frequency adjustment Framefreq=183HZ, Bias will be 1/4 (Instr Set=1)

          _contrast = LCD_SPLC792A_CONTRAST;              
          _writeCommand(0x70 | (_contrast & 0x0F));         // Set Contrast Low bits, 0 1 1 1 C3 C2 C1 C0 (IS=1)


//          _icon_power = 0x04;                               // Icon display off (Bit3=0), Booster circuit is turned on (Bit2=1) (IS=1)
          _icon_power = 0x0C;                               // Icon display on (Bit3=1), Booster circuit is turned on (Bit2=1)  (IS=1)
                                                            // Note: Booster circuit always on for SPLC792A, Bit2 is dont care
                                                            // Saved to allow contrast change at later time

          _writeCommand(0x50 | _icon_power | ((_contrast >> 4) & 0x03));  // Set Icon, Booster and Contrast High bits, 0 1 0 1 Ion Bon C5 C4 (IS=1)
          wait_ms(10);            // Wait 10ms to ensure powered up
          
          _writeCommand(0x68 | (LCD_SPLC792A_RAB & 0x07));  // Voltage follower, 0 1 1 0 FOn=1, Ampl ratio Rab2=1, Rab1=0, Rab0=0  (IS=1)
                                                            // Note: Follower circuit always on for SPLC792A, Bit3 is dont care          
          wait_ms(10);            // Wait 10ms to ensure powered up
          
          _writeCommand(0x20 | _function);                  // Select Instruction Set = 0

          break; // case SPLC792A_3V3 Controller
          
        case ST7066_ACM:                                                // ST7066 4/8 bit, I2C on ACM1602 using a PIC        
        default:
          // Devices fully compatible to HD44780 that do not use any DC/DC Voltage converters but external VLCD, no icons etc

          // Initialise Display configuration
          switch (_type) {
            case LCD8x1:         //8x1 is a regular 1 line display
            case LCD8x2B:        //8x2B is a special case of 16x1
//            case LCD12x1:                                
            case LCD16x1:                                            
//            case LCD20x1:                                                        
            case LCD24x1:
//            case LCD40x1:            
              _function = 0x00;    // Function set 001 DL N F - -
                                   //  DL=0 (4 bits bus)             
                                   //   N=0 (1 line)
                                   //   F=0 (5x7 dots font)
              break;                                
                                                  
            case LCD12x3D:            // Special mode for KS0078 and PCF21XX                            
            case LCD12x3D1:           // Special mode for KS0078 and PCF21XX                     
            case LCD12x4D:            // Special mode for KS0078 and PCF21XX:
            case LCD16x3D:            // Special mode for KS0078
//            case LCD16x3D1:           // Special mode for KS0078
//            case LCD24x3D:            // Special mode for KS0078
//            case LCD24x3D1:           // Special mode for KS0078            
            case LCD24x4D:            // Special mode for KS0078
              error("Error: LCD Controller type does not support this Display type\n\r"); 
              break;  

            // All other LCD types are initialised as 2 Line displays (including LCD16x1C and LCD40x4)
            default:
              _function = 0x08;    // Function set 001 DL N F - -
                                   //  DL=0 (4 bits bus)
                                   //  Note: 4 bit mode is ignored for native SPI and I2C devices                                 
                                   //   N=1 (2 lines)
                                   //   F=0 (5x7 dots font, only option for 2 line display)
                                   //    -  (Don't care)
              break;
          } // switch type

          _writeCommand(0x20 | _function);                         
          break; // case default Controller
          
    } // switch Controller specific initialisations    

    // Controller general initialisations                                          
//    _writeCommand(0x01); // Clear Display and set cursor to 0
//    wait_ms(10);         // The CLS command takes 1.64 ms.
//                         // Since we are not using the Busy flag, Lets be safe and take 10 ms  

    _writeCommand(0x02); // Cursor Home, DDRAM Address to Origin
    wait_ms(10);         // The Return Home command takes 1.64 ms.
                         // Since we are not using the Busy flag, Lets be safe and take 10 ms      

    _writeCommand(0x06); // Entry Mode 0000 0 1 I/D S 
                         //   Cursor Direction and Display Shift
                         //   I/D=1 (Cur incr)
                         //     S=0 (No display shift)                        

    _writeCommand(0x14); // Cursor or Display shift 0001 S/C R/L x x 
                         //   S/C=0 Cursor moves
                         //   R/L=1 Right
                         // 

//    _writeCommand(0x0C); // Display Ctrl 0000 1 D C B
//                         //   Display On, Cursor Off, Blink Off   

//    setCursor(CurOff_BlkOff);     
    setCursor(CurOn_BlkOff);        
    setMode(DispOn);     
}


/** Clear the screen, Cursor home. 
  * Note: The whole display is initialised to charcode 0x20, which may not be a 'space' on some controllers with a
  *       different fontset such as the PCF2116C or PCF2119R. In this case you should fill the display with 'spaces'.
  */
void TextLCD_Base::cls() {

#if (LCD_TWO_CTRL == 1)
  // Select and configure second LCD controller when needed
  if(_type==LCD40x4) {
    _ctrl_idx=_LCDCtrl_1; // Select 2nd controller

    // Second LCD controller Cursor always Off
    _setCursorAndDisplayMode(_currentMode, CurOff_BlkOff);

    // Second LCD controller Clearscreen
    _writeCommand(0x01);  // cls, and set cursor to 0    
    wait_ms(20);          // The CLS command takes 1.64 ms.
                          // Since we are not using the Busy flag, Lets be safe and take 10 ms
  
    _ctrl_idx=_LCDCtrl_0; // Select primary controller
  }

  
  // Primary LCD controller Clearscreen
  _writeCommand(0x01);    // cls, and set cursor to 0
  wait_ms(20);            // The CLS command takes 1.64 ms.
                          // Since we are not using the Busy flag, Lets be safe and take 10 ms

  // Restore cursormode on primary LCD controller when needed
  if(_type==LCD40x4) {
    _setCursorAndDisplayMode(_currentMode,_currentCursor);     
  }

#else
  // Support only one LCD controller
  _writeCommand(0x01);    // cls, and set cursor to 0
  wait_ms(20);            // The CLS command takes 1.64 ms.
                          // Since we are not using the Busy flag, Lets be safe and take 10 ms
#endif
                   
  setAddress(0, 0);  // Reset Cursor location
                     // Note: This is needed because some displays (eg PCF21XX) don't use line 0 in the '3 Line' mode.   
}

/** Locate cursor to a screen column and row
  *
  * @param column  The horizontal position from the left, indexed from 0
  * @param row     The vertical position from the top, indexed from 0
  */ 
void TextLCD_Base::locate(int column, int row) {
    
   // setAddress() does all the heavy lifting:
   //   check column and row sanity, 
   //   switch controllers for LCD40x4 if needed
   //   switch cursor for LCD40x4 if needed
   //   set the new memory address to show cursor at correct location
   setAddress(column, row);      
}
   

/** Write a single character (Stream implementation)
  */
int TextLCD_Base::_putc(int value) {
  int addr;
    
    if (value == '\n') {
      //No character to write
      
      //Update Cursor      
      _column = 0;
      _row++;
      if (_row >= rows()) {
        _row = 0;
      }      
    }
    else {
      //Character to write

#if (LCD_DEF_FONT == 1)   //Default HD44780 font
      _writeData(value);
#elif (LCD_C_FONT == 1) || (LCD_R_FONT == 1) //PCF21xxC or PCF21xxR font
      _writeData(ASCII_2_LCD(value));
#elif (LCD_UTF8_FONT == 1) // UTF8 2 byte font (eg Cyrillic)
//      value = UTF_2_LCD(value, utf_seq_rec_first_cyr, utf_seq_recode_cyr, &utf_rnd_recode_cyr[0][0]);      
      value = UTF_2_LCD(value);            
      if (value >= 0) {
        _writeData(value);
        
        // Only increment cursor when there is something to write
        //   Continue below to closing bracket...
#else
      _writeData('?'); //Oops, no font defined
#endif

      //Update Cursor
      _column++;
      if (_column >= columns()) {
        _column = 0;
        _row++;
        if (_row >= rows()) {
          _row = 0;
        }
      }
      
#if (LCD_DEF_FONT == 1)   //Default HD44780 font

#elif (LCD_C_FONT == 1) || (LCD_R_FONT == 1) //PCF21xxC or PCF21xxR font

#elif (LCD_UTF8_FONT == 1) //UTF8 2 byte font (eg Cyrillic) 
        //   Continue code above to close bracket...  
      } // if (value >= 0) {..
#else             

#endif

    } //else

    //Set next memoryaddress, make sure cursor blinks at next location
    addr = getAddress(_column, _row);
    _writeCommand(0x80 | addr);
            
    return value;
}


// get a single character (Stream implementation)
int TextLCD_Base::_getc() {
    return -1;
}


#if ((LCD_C_FONT == 1) || (LCD_R_FONT == 1)) //PCF21xxC or PCF21xxR font
/** Convert ASCII character code to the LCD fonttable code
  *
  * @param c The character to write to the display
  * @return The character code for the specific fonttable of the controller
  */
int TextLCD_Base::ASCII_2_LCD (int c) {
    
//LCD_C_F0 is default for HD44780 and compatible series
//    if (_font == LCD_C_F0) return c;

//LCD_C_FC for PCF21XXC series    
//LCD_C_FR for PCF21XXR series    
//Used code from Suga koubou library for PCF2119K and PCF2119R
    if (((c >= ' ') && (c <= '?')) || ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'))) {
        c |= 0x80;
    } else if (c >= 0xF0 && c <= 0xFF) {
        c &= 0x0F;
    }
    return c;
}
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
int TextLCD_Base::UTF_2_LCD (int c) {    
    int utf_code;
    int utf_low_byte;            // Low byte UTF8
    static int utf_hi_byte = 0;  // High byte UTF8
 
    if (c < 0x80) { // Regular ASCII code, no need to convert
      return c;
    }  
    else { // UTF8 handling, See wikipedia.org/wiki/UTF-8 and www.utf8-chartable.de
//      printf("0x%X ", c);    

      if (c >= 0xC0) {           // First UTF8 byte should be formatted as 110b bbaa, Do sanity check
        utf_hi_byte = c & 0x1F;  // Mask out significant bits (0x1F) and save high byte
        return -1;               // Nothing to display as yet, wait for second UTF8 byte
      } 
 
      if (c <= 0xBF) {           // Second UTF8 byte should be formatted as 10aa aaaa, Do sanity check
        utf_low_byte = c & 0x3F; // Mask out significant bits (0x3F)
        
        // Compose UTF character code from UTF8 bytes. The UTF codes will be between U+0080 and U+07FF
        utf_code = (utf_hi_byte << 6) | utf_low_byte; // 00000bbb aaaaaaaa  
//        printf("0x%4X ", utf_code);    
      
        // Sanity check on UTF codes
        // For example Cyrillic characters are UTF encoded between 0x0400 and 0x04FF        
        if ((utf_code < UTF_FIRST) || (utf_code > UTF_LAST)) {          
          return -1; // Invalid UTF8 code
        };
        
        //Map some specific UTF codes on a character in LCD fonttable using a special correcting lookup table
        for (char i=0; UTF_RND_RECODE[i][0]; i++) { // Step through table until endvalue 0 is found or until a match is found
          if (utf_code == UTF_RND_RECODE[i][0]) { // UTF8 code match is found
            c = UTF_RND_RECODE[1][1];
            return c; // found match in correcting random table
          }
        }
                 
        //Sanity check on table idx range 
        if ((utf_code < UTF_SEQ_REC_FIRST) || (utf_code > UTF_SEQ_REC_LAST)) {          
          return -1; // Invalid UTF8 code
        };
        
        //Map all other UTF codes on a character in LCD fonttable using a sequential lookup table
        c = UTF_SEQ_RECODE[utf_code - UTF_SEQ_REC_FIRST];       
        return c; // entry in sequential table        
      }
      else {
        return -1; // Invalid UTF8 code for second byte          
      }  
    } // End UTF8 handling
}

#endif
 

#if(LCD_PRINTF != 1)
/** Write a character to the LCD
  *
  * @param c The character to write to the display
  */
int TextLCD_Base::putc(int c){
  return _putc(c);
}  


/** Write a raw string to the LCD
  *
  * @param string text, may be followed by variables to emulate formatting the string.
  *                     However, printf formatting is NOT supported and variables will be ignored! 
  */
int TextLCD_Base::printf(const char* text, ...) {
  
  while (*text !=0) {
    _putc(*text);
    text++;
  }
  return 0;
}
#endif    


// Write a nibble using the 4-bit interface
void TextLCD_Base::_writeNibble(int value) {

// Enable is Low
    this->_setEnable(true);        
    this->_setData(value);        // Low nibble of value on D4..D7
    wait_us(1); // Data setup time        
    this->_setEnable(false);    
    wait_us(1); // Datahold time
// Enable is Low
}

// Write a byte using the 4-bit interface
void TextLCD_Base::_writeByte(int value) {

// Enable is Low
    this->_setEnable(true);          
    this->_setData(value >> 4);   // High nibble
    wait_us(1); // Data setup time    
    this->_setEnable(false);   
    wait_us(1); // Data hold time
    
    this->_setEnable(true);        
    this->_setData(value);        // Low nibble
    wait_us(1); // Data setup time        
    this->_setEnable(false);    
    wait_us(1); // Datahold time

// Enable is Low
}

// Write a command byte to the LCD controller
void TextLCD_Base::_writeCommand(int command) {

    this->_setRS(false);        
    wait_us(1);  // Data setup time for RS       
    
    this->_writeByte(command);   
    wait_us(40); // most instructions take 40us            
}

// Write a data byte to the LCD controller
void TextLCD_Base::_writeData(int data) {

    this->_setRS(true);            
    wait_us(1);  // Data setup time for RS 
        
    this->_writeByte(data);
    wait_us(40); // data writes take 40us                
}


// This replaces the original _address() method.
// It is confusing since it returns the memoryaddress or-ed with the set memorycommand 0x80.
// Left it in here for compatibility with older code. New applications should use getAddress() instead.
int TextLCD_Base::_address(int column, int row) {
  return 0x80 | getAddress(column, row);
}


// This is new method to return the memory address based on row, column and displaytype.
//
/** Return the memoryaddress of screen column and row location
   *
   * @param column  The horizontal position from the left, indexed from 0
   * @param row     The vertical position from the top, indexed from 0
   * @return        The memoryaddress of screen column and row location
   *
   */
int TextLCD_Base::getAddress(int column, int row) {

    switch (_addr_mode) {

        case LCD_T_A:
          //Default addressing mode for 1, 2 and 4 rows (except 40x4)
          //The two available rows are split and stacked on top of eachother. Addressing for 3rd and 4th line continues where lines 1 and 2 were split.          
          //Displays top rows when less than four are used.          
          switch (row) {
            case 0:
              return 0x00 + column;
            case 1:
              return 0x40 + column;
            case 2:
              return 0x00 + _nr_cols + column;
            case 3:
              return 0x40 + _nr_cols + column;
            // Should never get here.
//            default:            
//              return 0x00;                    
            }
          
        case LCD_T_B:
          // LCD8x2B is a special layout of LCD16x1
          if (row==0) 
            return 0x00 + column;                        
          else   
//            return _nr_cols + column;                                    
            return 0x08 + column;                        

        case LCD_T_C:
          // LCD16x1C is a special layout of LCD8x2
          // LCD32x1C is a special layout of LCD16x2                    
          // LCD40x1C is a special layout of LCD20x2          
#if(0)
          if (column < 8) 
            return 0x00 + column;                        
          else   
            return 0x40 + (column - 8);                        
#else
          if (column < (_nr_cols >> 1)) 
            return 0x00 + column;                        
          else   
            return 0x40 + (column - (_nr_cols >> 1));                        
#endif

        case LCD_T_D:
          //Alternate addressing mode for 3 and 4 row displays (except 40x4). Used by PCF21XX, KS0073, KS0078, SSD1803
          //The 4 available rows start at a hardcoded address.                    
          //Displays top rows when less than four are used.
          switch (row) {
            case 0:
              return 0x00 + column;
            case 1:
              return 0x20 + column;
            case 2:
              return 0x40 + column;
            case 3:
              return 0x60 + column;
            // Should never get here.
//            default:            
//              return 0x00;                    
            }

        case LCD_T_D1:
          //Alternate addressing mode for 3 row displays. Used by PCF21XX, KS0073, KS0078, SSD1803
          //The 4 available rows start at a hardcoded address.                              
          //Skips top row of 4 row display and starts display at row 1
          switch (row) {
            case 0:
              return 0x20 + column;
            case 1:
              return 0x40 + column;
            case 2:
              return 0x60 + column;
            // Should never get here.
//            default:            
//              return 0x00;                    
            }
        
        case LCD_T_E:                
          // LCD40x4 is a special case since it has 2 controllers.
          // Each controller is configured as 40x2 (Type A)
          if (row<2) { 
            // Test to see if we need to switch between controllers  
            if (_ctrl_idx != _LCDCtrl_0) {

              // Second LCD controller Cursor Off
              _setCursorAndDisplayMode(_currentMode, CurOff_BlkOff);    

              // Select primary controller
              _ctrl_idx = _LCDCtrl_0;

              // Restore cursormode on primary LCD controller
              _setCursorAndDisplayMode(_currentMode, _currentCursor);    
            }           
            
            return 0x00 + (row * 0x40) + column;          
          }
          else {

            // Test to see if we need to switch between controllers  
            if (_ctrl_idx != _LCDCtrl_1) {
              // Primary LCD controller Cursor Off
              _setCursorAndDisplayMode(_currentMode, CurOff_BlkOff);    

              // Select secondary controller
              _ctrl_idx = _LCDCtrl_1;

              // Restore cursormode on secondary LCD controller
              _setCursorAndDisplayMode(_currentMode, _currentCursor);    
            }           
                                   
            return 0x00 + ((row-2) * 0x40) + column;          
          } 
            
        case LCD_T_F:
          //Alternate addressing mode for 3 row displays.
          //The first half of 3rd row continues from 1st row, the second half continues from 2nd row.                              
          switch (row) {
            case 0:
              return 0x00 + column;
            case 1:
              return 0x40 + column;
            case 2:
              if (column < (_nr_cols >> 1)) // check first or second half of line
                return (0x00 + _nr_cols + column);                        
              else   
                return (0x40 + _nr_cols + (column - (_nr_cols >> 1)));                        
            // Should never get here.
//            default:            
//              return 0x00;                    
          }

        case LCD_T_G:
          //Alternate addressing mode for 3 row displays. Used by ST7036
          switch (row) {
            case 0:
              return 0x00 + column;
            case 1:
              return 0x10 + column;
            case 2:
              return 0x20 + column;
            // Should never get here.
//            default:            
//              return 0x00;                    
            }

        // Should never get here.
        default:            
            return 0x00;        

    } // switch _addr_mode
}


/** Set the memoryaddress of screen column and row location
  *
  * @param column  The horizontal position from the left, indexed from 0
  * @param row     The vertical position from the top, indexed from 0
  */
void TextLCD_Base::setAddress(int column, int row) {
   
// Sanity Check column
    if (column < 0) {
      _column = 0;
    }
    else if (column >= _nr_cols) {
      _column = _nr_cols - 1;
    } else _column = column;
    
// Sanity Check row
    if (row < 0) {
      _row = 0;
    }
    else if (row >= _nr_rows) {
      _row = _nr_rows - 1;
    } else _row = row;
    
    
// Compute the memory address
// For LCD40x4:  switch controllers if needed
//               switch cursor if needed
    int addr = getAddress(_column, _row);
    
    _writeCommand(0x80 | addr);
}


/** Return the number of columns
  *
  * @return  The number of columns
  *
  * Note: some configurations are commented out because they have not yet been tested due to lack of hardware     
  */   
int TextLCD_Base::columns() {
    
  // Columns encoded in b7..b0
  //return (_type & 0xFF);          
  return _nr_cols;           
}

/** Return the number of rows
  *
  * @return  The number of rows
  *
  * Note: some configurations are commented out because they have not yet been tested due to lack of hardware     
  */
int TextLCD_Base::rows() {

  // Rows encoded in b15..b8  
  //return ((_type >> 8) & 0xFF); 
  return _nr_rows;          
}

/** Set the Cursormode
  *
  * @param cursorMode  The Cursor mode (CurOff_BlkOff, CurOn_BlkOff, CurOff_BlkOn, CurOn_BlkOn)
  */
void TextLCD_Base::setCursor(LCDCursor cursorMode) { 

  // Save new cursor mode, needed when 2 controllers are in use or when display is switched off/on
  _currentCursor = cursorMode;
    
  // Configure only current LCD controller
  _setCursorAndDisplayMode(_currentMode, _currentCursor);    
}

/** Set the Displaymode
  *
  * @param displayMode The Display mode (DispOff, DispOn)
  */
void TextLCD_Base::setMode(LCDMode displayMode) { 

  // Save new displayMode, needed when 2 controllers are in use or when cursor is changed
  _currentMode = displayMode;

#if (LCD_TWO_CTRL == 1)    
  // Select and configure second LCD controller when needed
  if(_type==LCD40x4) {
    if (_ctrl_idx==_LCDCtrl_0) {      
      // Configure primary LCD controller
      _setCursorAndDisplayMode(_currentMode, _currentCursor);

      // Select 2nd controller
      _ctrl_idx=_LCDCtrl_1;
  
      // Configure secondary LCD controller    
      _setCursorAndDisplayMode(_currentMode, CurOff_BlkOff);

      // Restore current controller
      _ctrl_idx=_LCDCtrl_0;       
    }
    else {
      // Select primary controller
      _ctrl_idx=_LCDCtrl_0;
    
      // Configure primary LCD controller
      _setCursorAndDisplayMode(_currentMode, CurOff_BlkOff);
       
      // Restore current controller
      _ctrl_idx=_LCDCtrl_1;

      // Configure secondary LCD controller    
      _setCursorAndDisplayMode(_currentMode, _currentCursor);
    }
  }
  else {
    // Configure primary LCD controller
    _setCursorAndDisplayMode(_currentMode, _currentCursor);
  }       
#else
  // Support only one LCD controller
  _setCursorAndDisplayMode(_currentMode, _currentCursor);

#endif  
}

/** Low level method to restore the cursortype and display mode for current controller
  */     
void TextLCD_Base::_setCursorAndDisplayMode(LCDMode displayMode, LCDCursor cursorType) {    

  // Configure current LCD controller   
  switch (_ctrl) {
    case ST7070: 
      //ST7070 does not support Cursorblink. The P bit selects the font instead !   
      _writeCommand(0x08 | displayMode | (cursorType & 0x02));    
      break;
    default:      
      _writeCommand(0x08 | displayMode | cursorType);
      break;
  } //switch      
}

/** Set the Backlight mode
  *
  *  @param backlightMode The Backlight mode (LightOff, LightOn)
  */
void TextLCD_Base::setBacklight(LCDBacklight backlightMode) {

#if (BACKLIGHT_INV==0)      
    // Positive Backlight control pin logic
    if (backlightMode == LightOn) {
      this->_setBL(true);     
    }
    else {
      this->_setBL(false);    
    }
#else
    // Inverted Backlight control pin logic
    if (backlightMode == LightOn) {
      this->_setBL(false);    
    }
    else {
      this->_setBL(true);           
    }
#endif    
} 

/** Set User Defined Characters
  *
  * @param unsigned char c   The Index of the UDC (0..7) for HD44780 or clones and (0..15) for some more advanced controllers 
  * @param char *udc_data    The bitpatterns for the UDC (8 bytes of 5 significant bits for bitpattern and 3 bits for blinkmode (advanced types))     
  */
void TextLCD_Base::setUDC(unsigned char c, char *udc_data) {

#if (LCD_TWO_CTRL == 1)
  // Select and configure second LCD controller when needed
  if(_type==LCD40x4) {
    _LCDCtrl_Idx current_ctrl_idx = _ctrl_idx; // Temp save current controller
   
    // Select primary controller     
    _ctrl_idx=_LCDCtrl_0;
    
    // Configure primary LCD controller
    _setUDC(c, udc_data);

    // Select 2nd controller
    _ctrl_idx=_LCDCtrl_1;
  
    // Configure secondary LCD controller    
    _setUDC(c, udc_data);

    // Restore current controller
    _ctrl_idx=current_ctrl_idx;       
  }
  else {
    // Configure primary LCD controller
    _setUDC(c, udc_data); 
  }   
#else
  // Support only one LCD controller
  _setUDC(c, udc_data); 
#endif  
}

/** Low level method to store user defined characters for current controller
  *
  * @param unsigned char c   The Index of the UDC (0..7) for HD44780 clones and (0..15) for some more advanced controllers 
  * @param char *udc_data    The bitpatterns for the UDC (8 bytes of 5 significant bits for bitpattern and 3 bits for blinkmode (advanced types))       
  */     
void TextLCD_Base::_setUDC(unsigned char c, char *udc_data) {
  
  switch (_ctrl) {
    case PCF2103_3V3 : // Some UDCs may be used for Icons                  
    case PCF2113_3V3 : // Some UDCs may be used for Icons                      
    case PCF2116_3V3 :          
    case PCF2116_5V  :              
    case PCF2119_3V3 : // Some UDCs may be used for Icons
    case PCF2119R_3V3: // Some UDCs may be used for Icons                 
      c = c & 0x0F; // mask down to valid range
      break;

    default:     
      c = c & 0x07; // mask down to valid range    
      break;  
  } //switch _ctrl

  // Select DD RAM for current LCD controller
  // This is needed to correctly set Bit 6 of the addresspointer for controllers that support 16 UDCs
  _writeCommand(0x80 | ((c << 3) & 0x40)) ;  
    
  // Select CG RAM for current LCD controller
  _writeCommand(0x40 | ((c << 3) & 0x3F)); //Set CG-RAM address, (note that Bit 6 is retained and can not be set by this command !)
                                           //8 sequential locations needed per UDC
  // Store UDC pattern 
  for (int i=0; i<8; i++) {
    _writeData(*udc_data++);
  }
   
  //Select DD RAM again for current LCD controller and restore the addresspointer
  int addr = getAddress(_column, _row);
  _writeCommand(0x80 | addr);  
}

#if(LCD_BLINK == 1)
/** Set UDC Blink and Icon blink
  * setUDCBlink method is supported by some compatible devices (eg SSD1803) 
  *
  * @param blinkMode The Blink mode (BlinkOff, BlinkOn)
  */
void TextLCD_Base::setUDCBlink(LCDBlink blinkMode){
  // Blinking UDCs (and icons) are enabled when a specific controlbit (BE) is set.
  // The blinking pixels in the UDC and icons can be controlled by setting additional bits in the UDC or icon bitpattern.
  // UDCs are defined by an 8 byte bitpattern. The P0..P4 form the character pattern.
  //     P7 P6 P5 P4 P3 P2 P1 P0 
  // 0   B1 B0  x  0  1  1  1  0
  // 1   B1 B0  x  1  0  0  0  1
  //        .............
  // 7   B1 B0  x  1  0  0  0  1
  //
  // Bit 6 and Bit 7 in the pattern will control the blinking mode when Blink is enabled through BE. 
  //     B1 B0  Mode
  //      0  0  No Blinking in this row of the UDC
  //      0  1  Enabled pixels in P4 will blink
  //      1  x  Enabled pixels in P0..P4 will blink
  //
  // Note: the PCF2103 and PCF2113 use UDCs to set Icons 
  //   3 x 8 rows x 5 bits = 120 bits Icons for Normal pattern (UDC 0..2) and
  //   3 x 8 rows x 5 bits = 120 bits Icons for Blink pattern (UDC 4..6) 
  // Note: the PCF2119 uses UDCs to set Icons 
  //   4 x 8 rows x 5 bits = 160 bits Icons for Normal pattern (UDC 0..3) and
  //   4 x 8 rows x 5 bits = 160 bits Icons for Blink pattern (UDC 4..7) 
  switch (blinkMode) {
    case BlinkOn: 
      // Controllers that support UDC/Icon Blink  
      switch (_ctrl) {
        case KS0073 :
        case KS0078 :
        case HD66712 :                    
          _function_1 |= 0x02; // Enable UDC/Icon Blink        
          _writeCommand(0x20 | _function_1);        // Function set 0 0 1 DL N RE(1) BE 0/LP (Ext Regs)

          _writeCommand(0x20 | _function);          // Function set 0 0 1 DL N RE(0) DH REV (Std Regs)
          break; // case KS0073, KS0078, HD66712 Controller
    
        case US2066_3V3 :  
        case SSD1803_3V3 :  
          _function_1 |= 0x04; // Enable UDC/Icon Blink
          _writeCommand(0x20 | _function_1);        // Set function, 0 0 1 DL N BE RE(1) REV 
                                                    // Select Ext Instr Set

          _writeCommand(0x20 | _function);          // Set function, 0 0 1 DL N DH RE(0) IS=0 Select Instruction Set 0
                                                    // Select Std Instr set, Select IS=0
          break; // case SSD1803, US2066

        case PCF2103_3V3 :  
        case PCF2113_3V3 :  
        case PCF2119_3V3 :                  
        case PCF2119R_3V3 :                          
          // Enable Icon Blink
          _writeCommand(0x20 | _function | 0x01);   // Set function, Select Instr Set = 1              
          _writeCommand(0x08 | 0x02);               // ICON Conf 0000 1, IM=0 (Char mode), IB=1 (Icon blink), 0 (Instr. Set 1) 
          _writeCommand(0x20 | _function);          // Set function, Select Instr Set = 0             

          break; 
       
        default:
          //Unsupported feature for other controllers        
          break; 
      } //switch _ctrl     
    
      break; // BlinkOn 

    case BlinkOff:
      // Controllers that support UDC Blink  
      switch (_ctrl) {
        case KS0073 :
        case KS0078 :
        case HD66712:
          _function_1 &= ~0x02; // Disable UDC/Icon Blink        
          _writeCommand(0x20 | _function_1);        // Function set 0 0 1 DL N RE(1) BE 0/LP (Ext Regs)

          _writeCommand(0x20 | _function);          // Function set 0 0 1 DL N RE(0) DH REV (Std Regs)
          break; // case KS0073, KS0078, HD66712 Controller
    
        case US2066_3V3 :  
        case SSD1803_3V3 :  
          _function_1 &= ~0x04; // Disable UDC/Icon Blink
          _writeCommand(0x20 | _function_1);        // Set function, 0 0 1 DL N BE RE(1) REV 
                                                    // Select Ext Instr Set

          _writeCommand(0x20 | _function);          // Set function, 0 0 1 DL N DH RE(0) IS=0 Select Instruction Set 0
                                                    // Select Std Instr set, Select IS=0
          break; // case SSD1803, US2066          
 
        case PCF2103_3V3 :
        case PCF2113_3V3 : 
        case PCF2119_3V3 :
        case PCF2119R_3V3 :        
          // Disable Icon Blink
          _writeCommand(0x20 | _function | 0x01);   // Set function, Select Instr Set = 1              
          _writeCommand(0x08);                      // ICON Conf 0000 1, IM=0 (Char mode), IB=1 (Icon blink), 0 (Instr. Set 1) 
          _writeCommand(0x20 | _function);          // Set function, Select Instr Set = 0             

          break; 
       
        default:
          //Unsupported feature for other controllers        
          break; 
      } //switch _ctrl     
    
      break; //BlinkOff
      
    default:
      break;      
  } // blinkMode
  
} // setUDCBlink()
#endif

#if(LCD_CONTRAST == 1)
/** Set Contrast
  * setContrast method is supported by some compatible devices (eg ST7032i) that have onboard LCD voltage generation
  * Initial code for ST70XX imported from fork by JH1PJL
  *
  * @param unsigned char c   contrast data (6 significant bits, valid range 0..63, Value 0 will disable the Vgen)  
  * @return none
  */
//@TODO Add support for 40x4 dual controller
void TextLCD_Base::setContrast(unsigned char c) {

// Function set mode stored during Init. Make sure we dont accidentally switch between 1-line and 2-line mode!
// Icon/Booster mode stored during Init. Make sure we dont accidentally change this!
 
  _contrast = c & 0x3F; // Sanity check
  
  switch (_ctrl) {   
    case PCF2113_3V3 :  
    case PCF2119_3V3 :
    case PCF2119R_3V3 :    
       if (_contrast <  5) _contrast = 0;  // See datasheet. Sanity check for PCF2113/PCF2119
       if (_contrast > 55) _contrast = 55;
      
       _writeCommand(0x20 | _function | 0x01);               // Set function, Select Instruction Set = 1              
       _writeCommand(0x80 | 0x00 | (_contrast & 0x3F));      // VLCD_set (Instr. Set 1)    V=0, VA=contrast
       _writeCommand(0x80 | 0x40 | (_contrast & 0x3F));      // VLCD_set (Instr. Set 1)    V=1, VB=contrast
       _writeCommand(0x20 | _function);                      // Select Instruction Set = 0
       break;
        
    case ST7032_3V3 :  
    case ST7032_5V :      
    case ST7036_3V3 :      
//    case ST7036_5V :          
    case SSD1803_3V3 :
    case SPLC792A_3V3 :  
      _writeCommand(0x20 | _function | 0x01);                        // Select Instruction Set = 1
      _writeCommand(0x70 | (_contrast & 0x0F));                      // Contrast Low bits
      _writeCommand(0x50 | _icon_power | ((_contrast >> 4) & 0x03)); // Contrast High bits 
      _writeCommand(0x20 | _function);                               // Select Instruction Set = 0
      break;

    case US2066_3V3 :      
      _writeCommand(0x20 | _function_1);        // Set function, 0 0 1 DL N BE RE(1) REV 
                                                // Select Extended Instruction Set

      _writeCommand(0x79);                      // Function Select OLED:  0 1 1 1 1 0 0 1 (Ext Instr Set)
         
      _writeCommand(0x81);                      // Set Contrast Control: 1 0 0 0 0 0 0 1 (Ext Instr Set, OLED)
      _writeCommand((_contrast << 2) | 0x03);   // Set Contrast Value: 8 bits. Use 6 bits for compatibility    
      
      _writeCommand(0x78);                      // Function Disable OLED: 0 1 1 1 1 0 0 0 (Ext Instr Set)          

      _writeCommand(0x20 | _function);          // Set function, 0 0 1 DL N DH RE(0) IS=0 Select Instruction Set 0
                                                // Select Std Instr set, Select IS=0
      break;

    //not yet tested on hardware
    case PT6314 :
      // Only 2 significant bits
      //   0x00 = 100%
      //   0x01 =  75%
      //   0x02 =  50%
      //   0x03 =  25%                
      _writeCommand(0x20 | _function | ((~_contrast) >> 4));        // Invert and shift to use 2 MSBs     
      break;
            
    default:  
      //Unsupported feature for other controllers
      break;               
  } // end switch     
} // end setContrast()
#endif

#if(LCD_POWER == 1)
/** Set Power
  * setPower method is supported by some compatible devices (eg SSD1803) that have power down modes
  *
  * @param bool powerOn  Power on/off   
  * @return none
  */
//@TODO Add support for 40x4 dual controller  
void TextLCD_Base::setPower(bool powerOn) {
  
  if (powerOn) {
    // Switch on  
    setMode(DispOn);       

    // Controllers that supports specific Power Down mode
    switch (_ctrl) {
    
//    case PCF2113_3V3 :  
//    case PCF2119_3V3 : 
//    case PCF2119R_3V3 :     
//    case ST7032_3V3 :  
//@todo
//    enable Booster Bon

      case WS0010:      
        _writeCommand(0x17);   // Char mode, DC/DC on        
        wait_ms(10);           // Wait 10ms to ensure powered up             
        break;

      case KS0073:        
      case KS0078:        
      case SSD1803_3V3 :      
//      case SSD1803_5V :            
        _writeCommand(0x20 | _function_1);                             // Select Ext Instr Set
        _writeCommand(0x02);                                           // Power On
        _writeCommand(0x20 | _function);                               // Select Std Instr Set
        break;
                    
      default:  
        //Unsupported feature for other controllers
        break;              
    } // end switch  
  }  
  else {
    // Switch off        
    setMode(DispOff);       

    // Controllers that support specific Power Down mode
    switch (_ctrl) {
    
//    case PCF2113_3V3 :  
//    case PCF2119_3V3 : 
//    case PCF2119R_3V3 :     
//    case ST7032_3V3 :  
//@todo
//    disable Booster Bon

      case WS0010:      
        _writeCommand(0x13);   // Char mode, DC/DC off              
        break;
        
      case KS0073:
      case KS0078:
      case SSD1803_3V3 :      
//      case SSD1803_5V :            
        _writeCommand(0x20 | _function_1);                             // Select Ext Instr Set
        _writeCommand(0x03);                                           // Power Down
        _writeCommand(0x20 | _function);                               // Select Std Instr Set
        break;

      default:  
        //Unsupported feature for other controllers
        break;              
    } // end switch  
  }
} // end setPower()
#endif

#if(LCD_ORIENT == 1)
/** Set Orient
  * setOrient method is supported by some compatible devices (eg SSD1803, US2066) that have top/bottom view modes
  *
  * @param LCDOrient orient Orientation 
  * @return none
  */
void TextLCD_Base::setOrient(LCDOrient orient){

  switch (orient) {
       
    case Top:
      switch (_ctrl) {
        case PCF2103_3V3:              
        case PCF2116_3V3:        
        case PCF2116_5V:                
          _writeCommand(0x20 | _function | 0x01);          // Set function, Select Instr Set = 1              
          _writeCommand(0x05);                             // Display Conf Set         0000 0, 1, P=0, Q=1               (Instr. Set 1)
          _writeCommand(0x20 | _function);                 // Set function, Select Instr Set = 0             
          break;

        case PCF2119_3V3:   
        case PCF2119R_3V3:                         
          _writeCommand(0x20 | _function | 0x01);          // Set function, Select Instr Set = 1              
          _writeCommand(0x07);                             // Display Conf Set         0000 0, 1, P=1, Q=1               (Instr. Set 1)
          _writeCommand(0x20 | _function);                 // Set function, Select Instr Set = 0             
          break;
                               
        case SSD1803_3V3 :      
//      case SSD1803_5V :
        case US2066_3V3 :      
          _writeCommand(0x20 | _function_1);        // Set function, 0 0 1 X N BE RE(1) REV 
                                                    // Select Extended Instruction Set
//          _writeCommand(0x06);                      // Set ext entry mode, 0 0 0 0 0 1 BDC=1 COM1-32, BDS=0 SEG100-1    "Bottom View" (Ext Instr Set)
          _writeCommand(0x05);                      // Set ext entry mode, 0 0 0 0 0 1 BDC=0 COM32-1, BDS=1 SEG1-100    "Top View" (Ext Instr Set)          

          _writeCommand(0x20 | _function);          // Set function, 0 0 1 DL N DH RE(0) IS=0 Select Instruction Set 0
                                                    // Select Std Instr set, Select IS=0       
          break;

        case ST7070:      
          _writeCommand(0x20 | _function | 0x04);   // Set function, 0 0 1 DL, N, EXT=1, x, x (Select Instr Set = 1)

          _writeCommand(0x40 | 0x00);               // COM/SEG directions 0 1 0 0 C1, C2, S1, S2  (Instr Set 1)
                                                    // C1=1: Com1-8 -> Com8-1;   C2=1: Com9-16 -> Com16-9
                                                    // S1=1: Seg1-40 -> Seg40-1; S2=1: Seg41-80 -> Seg80-41                                                    
          wait_ms(5);                               // Wait to ensure completion or ST7070 fails to set Top/Bottom after reset..
          
          _writeCommand(0x20 | _function);          // Set function, EXT=0 (Select Instr Set = 0)
        
          break; // case ST7070 Controller
          
        default:  
          //Unsupported feature for other controllers
          break;              

      } // end switch _ctrl     
      break; // end Top
                
    case Bottom:
      switch (_ctrl) {
        case PCF2103_3V3:              
        case PCF2116_3V3:        
        case PCF2116_5V:                
          _writeCommand(0x20 | _function | 0x01);          // Set function, Select Instr Set = 1              
          _writeCommand(0x06);                             // Display Conf Set         0000 0, 1, P=1, Q=0               (Instr. Set 1)
          _writeCommand(0x20 | _function);                 // Set function, Select Instr Set = 0             
          break;

        case PCF2119_3V3:
        case PCF2119R_3V3 :                                 
          _writeCommand(0x20 | _function | 0x01);          // Set function, Select Instr Set = 1              
          _writeCommand(0x04);                             // Display Conf Set         0000 0, 1, P=0, Q=0               (Instr. Set 1)
          _writeCommand(0x20 | _function);                 // Set function, Select Instr Set = 0             
          break;
        
        case SSD1803_3V3 :      
//      case SSD1803_5V :
        case US2066_3V3 :      
          _writeCommand(0x20 | _function_1);        // Set function, 0 0 1 X N BE RE(1) REV 
                                                    // Select Extended Instruction Set
          _writeCommand(0x06);                      // Set ext entry mode, 0 0 0 0 0 1 BDC=1 COM1-32, BDS=0 SEG100-1    "Bottom View" (Ext Instr Set)
//          _writeCommand(0x05);                      // Set ext entry mode, 0 0 0 0 0 1 BDC=0 COM32-1, BDS=1 SEG1-100    "Top View" (Ext Instr Set)          

          _writeCommand(0x20 | _function);          // Set function, 0 0 1 DL N DH RE(0) IS=0 Select Instruction Set 0
                                                    // Select Std Instr set, Select IS=0       
          break;

        case ST7070:
          //Note: this does not result in correct top/bottom view. 
          //The left and right half of each row are reversed and the addressing of both rows is also incorrect:
          //Top/bottomline when orientation is flipped:
          //  0x48...0x4F  0x40...0x47
          //  0x08...0x0F  0x00...0x07
          _writeCommand(0x20 | _function | 0x04);   // Set function, 0 0 1 DL N EXT=1 x x (Select Instr Set = 1)

          _writeCommand(0x40 | 0x0F);               // COM/SEG directions 0 1 0 0 C1, C2, S1, S2  (Instr Set 1)
                                                    // C1=1: Com1-8 -> Com8-1;   C2=1: Com9-16 -> Com16-9
                                                    // S1=1: Seg1-40 -> Seg40-1; S2=1: Seg41-80 -> Seg80-41                                                    
          wait_ms(5);                               // Wait to ensure completion or ST7070 fails to set Top/Bottom after reset..
          
          _writeCommand(0x20 | _function);          // Set function, EXT=0 (Select Instr Set = 0)
        
          break; // case ST7070 Controller
          
        default:  
          //Unsupported feature for other controllers
          break;              

      } // end switch _ctrl     
    
      break; // end Bottom
  } // end switch orient
} // end setOrient()
#endif

#if(LCD_BIGFONT == 1)
/** Set Big Font
  * setBigFont method is supported by some compatible devices (eg SSD1803, US2066) 
  *
  * @param lines  The selected Big Font lines (None, TopLine, CenterLine, BottomLine, TopBottomLine)
  *                                            Double height characters can be shown on lines 1+2, 2+3, 3+4 or 1+2 and 3+4
  *                                            Valid double height lines depend on the LCDs number of rows.
  */
void TextLCD_Base::setBigFont(LCDBigFont lines) {

  switch (lines) {
    case None:
      switch (_ctrl) {
        case SSD1803_3V3 :      
        case US2066_3V3 :      
          _writeCommand(0x20 | _function_1);        // Set function, 0 0 1 X N BE RE(1) REV 
                                                    // Select Extended Instruction Set
          _writeCommand(0x1C);                      // Double Height, 0 0 0 1 UD2=1, UD1=1, X, DH'=0 (Ext Instr Set)
                                                    // Default
          _function = _function & ~0x04;            // Set function, 0 0 1 DL N DH=0 RE(0) IS=0 Select Instruction Set 0
          _writeCommand(0x20 | _function);          // Set function, 0 0 1 DL N DH RE(0) IS=0 Select Instruction Set 0
                                                    // Select Std Instr set, Select IS=0        
          break; // end US2066      

        default:
          break; // end default      
      } // end switch _ctrl     
      break; // end None      
    
    case TopLine:
      if (_nr_rows < 2) return; //Sanity check  

      switch (_ctrl) {
        case SSD1803_3V3 :              
        case US2066_3V3 :      
          _writeCommand(0x20 | _function_1);        // Set function, 0 0 1 X N BE RE(1) REV 
                                                    // Select Extended Instruction Set
          _writeCommand(0x1C);                      // Double Height, 0 0 0 1 UD2=1, UD1=1, X, DH'=0 (Ext Instr Set)
                                                    // Default
          _function = _function | 0x04;             // Set function, 0 0 1 DL N DH=1 RE(0) IS=0 Select Instruction Set 0
          _writeCommand(0x20 | _function);          // Set function, 0 0 1 DL N DH RE(0) IS=0 Select Instruction Set 0
                                                    // Select Std Instr set, Select IS=0        
          break; // end US2066, SSD1803      

        default:
          break; // end default      
      } // end switch _ctrl     
      break; // end TopLine              

    case CenterLine:
      if (_nr_rows != 4) return; //Sanity check  
          
      switch (_ctrl) {
        case SSD1803_3V3 :              
        case US2066_3V3 :      
          _writeCommand(0x20 | _function_1);        // Set function, 0 0 1 X N BE RE(1) REV 
                                                    // Select Extended Instruction Set
          _writeCommand(0x14);                      // Double Height, 0 0 0 1 UD2=0, UD1=1, X, DH'=0 (Ext Instr Set)
                                                    // Default
          _function = _function | 0x04;             // Set function, 0 0 1 DL N DH=1 RE(0) IS=0 Select Instruction Set 0
          _writeCommand(0x20 | _function);          // Set function, 0 0 1 DL N DH RE(0) IS=0 Select Instruction Set 0
                                                    // Select Std Instr set, Select IS=0        
          break; // end US2066, SSD1803      

        default:
          break; // end default      
      } // end switch _ctrl        
      break; // end CenterLine              

    case BottomLine:
      if (_nr_rows < 3) return; //Sanity check  
          
      switch (_ctrl) {
        case SSD1803_3V3 :              
        case US2066_3V3 :      
          _writeCommand(0x20 | _function_1);        // Set function, 0 0 1 X N BE RE(1) REV 
                                                    // Select Extended Instruction Set
          if (_nr_rows == 3) {
            _writeCommand(0x14);                      // Double Height, 0 0 0 1 UD2=0, UD1=1, X, DH'=0 (Ext Instr Set)
          }  
          else {
            _writeCommand(0x10);                      // Double Height, 0 0 0 1 UD2=0, UD1=0, X, DH'=0 (Ext Instr Set)
          }  
          _function = _function | 0x04;             // Set function, 0 0 1 DL N DH=1 RE(0) IS=0 Select Instruction Set 0
          _writeCommand(0x20 | _function);          // Set function, 0 0 1 DL N DH RE(0) IS=0 Select Instruction Set 0
                                                    // Select Std Instr set, Select IS=0        
          break; // end US2066, SSD1803      

        default:
          break; // end default      
      } // end switch _ctrl           
      break; // end BottomLine          
      
    case TopBottomLine:
      if (_nr_rows != 4) return; //Sanity check  
      
      switch (_ctrl) {
        case SSD1803_3V3 :        
        case US2066_3V3 :      
          _writeCommand(0x20 | _function_1);        // Set function, 0 0 1 X N BE RE(1) REV 
                                                    // Select Extended Instruction Set
          _writeCommand(0x18);                      // Double Height, 0 0 0 1 UD2=1, UD1=0, X, DH'=0 (Ext Instr Set)
                                                    // Default
          _function = _function | 0x04;             // Set function, 0 0 1 DL N DH=1 RE(0) IS=0 Select Instruction Set 0
          _writeCommand(0x20 | _function);          // Set function, 0 0 1 DL N DH RE(0) IS=0 Select Instruction Set 0
                                                    // Select Std Instr set, Select IS=0        
          break; // end US2066, SSD1803      

        default:
          break; // end default      
      } // end switch _ctrl           
      break; // end TopBottomLine          

  } // end switch lines

} // end setBigFont()
#endif


#if (LCD_FONTSEL == 1)
/** Set Font
  * setFont method is supported by some compatible devices (eg SSD1803, US2066, ST7070) 
  *
  * @param LCDFont font  The selected Font 
  * @return none 
  *
  * Note: most controllers support only one font and the hardware specific
  * fonttable is encoded as part of the controller type number (eg PCF21XXC or PCF21XXR).
  * Some controllers support multiple tables that can only be selected by logic levels on a few pins.
  * Some controllers also support runtime fontable switching through a specific instruction     
  */
void TextLCD_Base::setFont(LCDFont font) {
    
  switch (font) {
    case Font_RA:  // UK/EU
      switch (_ctrl) {
        case SSD1803_3V3 :      
        case US2066_3V3 :      
          _writeCommand(0x20 | _function_1);        // Set function, 0 0 1 X N BE RE(1) REV 
                                                    // Select Extended Instruction Set
          _writeCommand(0x72);                      // ROM Select command, 0 1 1 1 0 0 1 0       (Ext Instr Set)
          _writeData(0x00);                         // ROM_0 Select data,  0 0 0 0 ROM2 ROM1 0 0 (Ext Instr Set)
          
          _writeCommand(0x20 | _function);          // Set function, 0 0 1 DL N DH RE(0) IS      (Std Instr Set)

          _font = font; // Save active font
          break; // end SSD1803, US2066      

        case ST7070: 
          //ST7070 does not support Cursorblink. The P bit selects the font instead !   
          _writeCommand(0x08 | _currentMode | (_currentCursor & 0x02));           

          _font = font; // Save active font          
          break; // end ST7070

        default:
          break; // end default      
      } // end switch _ctrl     
      break; // end Font_RA      

    case Font_RB:    // UK/CYR
      switch (_ctrl) {
        case SSD1803_3V3 :      
        case US2066_3V3 :      
          _writeCommand(0x20 | _function_1);        // Set function, 0 0 1 X N BE RE(1) REV 
                                                    // Select Extended Instruction Set
          _writeCommand(0x72);                      // ROM Select command, 0 1 1 1 0 0 1 0       (Ext Instr Set)
          _writeData(0x04);                         // ROM_0 Select data,  0 0 0 0 ROM2 ROM1 0 0 (Ext Instr Set)
          
          _writeCommand(0x20 | _function);          // Set function, 0 0 1 DL N DH RE(0) IS      (Std Instr Set)

          _font = font; // Save active font
          break; // end SSD1803, US2066      

        case ST7070: 
          //ST7070 does not support Cursorblink. The P bit selects the font instead !   
          _writeCommand(0x08 | _currentMode | (_currentCursor & 0x02) | 0x01);           

          _font = font; // Save active font
          break; // end ST7070

        default:
          break; // end default      
      } // end switch _ctrl     
      break; // end Font_RB      

    case Font_0:   //Font_O is pretty similar to ROM_C
    case Font_RC:  // UK/JAP  
      switch (_ctrl) {
        case SSD1803_3V3 :      
        case US2066_3V3 :      
          _writeCommand(0x20 | _function_1);        // Set function, 0 0 1 X N BE RE(1) REV 
                                                    // Select Extended Instruction Set
          _writeCommand(0x72);                      // ROM Select command, 0 1 1 1 0 0 1 0       (Ext Instr Set)
          _writeData(0x08);                         // ROM_0 Select data,  0 0 0 0 ROM2 ROM1 0 0 (Ext Instr Set)
          
          _writeCommand(0x20 | _function);          // Set function, 0 0 1 DL N DH RE(0) IS      (Std Instr Set)

          _font = font; // Save active font
          break; // end SSD1803, US2066  

        default:
          break; // end default      
      } // end switch _ctrl     
      break; // end Font_RC      
  } // end switch font
  
  //SSD1803 seems to screw up cursor position after selecting new font. Restore to make sure...
  //Set next memoryaddress, make sure cursor blinks at next location
  int addr = getAddress(_column, _row);
  _writeCommand(0x80 | addr);
         
}
#endif


#if(LCD_ICON==1)
/** Set Icons
  *
  * @param unsigned char idx   The Index of the icon pattern (0..15) for KS0073 and similar controllers
  *                            and Index (0..31) for PCF2103 and similar controllers  
  * @param unsigned char data  The bitpattern for the icons (6 lsb for KS0073 bitpattern (5 lsb for KS0078) and 2 msb for blinkmode)       
  *                            The bitpattern for the PCF2103 icons is 5 lsb (UDC 0..2) and 5 lsb for blinkmode (UDC 4..6)         
  */
void TextLCD_Base::setIcon(unsigned char idx, unsigned char data) {
  // Blinking icons are enabled when a specific controlbit (BE) is set.
  // The blinking pixels in the icons can be controlled by setting additional bits in the icon bitpattern.
  // Icons are defined by a byte bitpattern. The P0..P5 form the Icon pattern for KS0073, and P0..P4 for KS0078
  //     P7 P6 P5 P4 P3 P2 P1 P0 
  // 0   B1 B0  0  0  1  1  1  0
  // 1   B1 B0  1  1  0  0  0  1
  //        .............
  // 15  B1 B0  1  1  0  0  0  1
  //
  // Bit 6 and Bit 7 in the pattern will control the blinking mode when Blink is enabled through BE. 
  //     B1 B0  Mode
  //      0  0  No Blinking for this icon row
  //      0  1  Enabled pixels in P5 will blink
  //      1  x  Enabled pixels in P0..P5 will blink
  //
  // Note: the PCF2103 and PCF2113 use UDCs to set Icons 
  //   3 x 8 rows x 5 bits = 120 bits Icons for Normal pattern (UDC 0..2) and
  //   3 x 8 rows x 5 bits = 120 bits Icons for Blink pattern (UDC 4..6) 
  // Note: the PCF2119 uses UDCs to set Icons 
  //   4 x 8 rows x 5 bits = 160 bits Icons for Normal pattern (UDC 0..3) and
  //   4 x 8 rows x 5 bits = 160 bits Icons for Blink pattern (UDC 4..7) 
  
  switch (_ctrl) {
    case KS0073:
    case KS0078:    
       _writeCommand(0x20 | _function_1);        // Set function, 0 0 1 DL N RE(1) BE LP 
                                                 // Select Extended Instruction Set
       _writeCommand(0x40 | (idx & 0x0F));       // Set Icon Address, mask Address to valid range (Ext Instr Set)

       _writeData(data);                         // Set Icon pattern (Ext Instr Set)

       _writeCommand(0x20 | _function);          // Set function, 0 0 1 DL N RE(0) DH REV Select Instruction Set 0
                                                 // Select Std Instr set, Select IS=0        
       break; // end KS0073, KS0078

    case ST7032_3V3:
    case ST7032_5V:
    case SPLC792A_3V3:
       _writeCommand(0x20 | _function | 0x01);   // Set function,  0 0 1 DL N F 0 IS=1 Select Instr Set = 1              
       _writeCommand(0x40 | (idx & 0x0F));       // Set Icon Address, mask Address to valid range (Instr Set 1)

       _writeData(data & 0x1F);                  // Set Icon pattern, no blink support (Instr Set 1) 

       _writeCommand(0x20 | _function);          // Set function, 0 0 1 DL N RE(0) DH REV Select Instruction Set 0
                                                 // Select Std Instr set, Select IS=0        
       break; // end ST7032

    case ST7036_3V3:
    case ST7036_5V:
       _writeCommand(0x20 | _function | 0x01);   // Set function, 0 0 1 DL N DH IS2,IS1 = 01 (Select Instr Set = 1)    
       _writeCommand(0x40 | (idx & 0x0F));       // Set Icon Address, mask Address to valid range (Instr Set 1)

       _writeData(data & 0x1F);                  // Set Icon pattern, no blink support (Instr Set 1) 

       _writeCommand(0x20 | _function);          // Set function, IS2,IS1 = 00 (Select Instr Set = 0)    
                                                 // Select Std Instr set, Select IS=0        
       break; // end ST7036

    case SSD1803_3V3:                  
//    case SSD1803_5V:                      
       _writeCommand(0x20 | _function | 0x01);   // Set function, 0 0 1 DL N DH RE(0) IS 
                                                 // Select Instruction Set 1
       _writeCommand(0x40 | (idx & 0x0F));       // Set Icon Address, mask Address to valid range (Instr Set = 1)
       _writeData(data);                         // Set Icon pattern (Instr Set = 1)

       _writeCommand(0x20 | _function);          // Set function, 0 0 1 DL N DH RE(0) IS
                                                 // Select IS=0        
       break; // end SSD1803      

    case PCF2103_3V3:
    case PCF2113_3V3:    
    case PCF2119_3V3:
    case PCF2119R_3V3:                
       // Store UDC/Icon pattern for PCF2103 and PCF2113: 
       //   3 x 8 rows x 5 bits = 120 bits for Normal pattern (UDC 0..2) and
       //   3 x 8 rows x 5 bits = 120 bits for Blink pattern (UDC 4..6) 
       // Store UDC/Icon pattern for PCF2119: 
       //   4 x 8 rows x 5 bits = 160 bits for Normal pattern (UDC 0..3) and
       //   4 x 8 rows x 5 bits = 160 bits for Blink pattern (UDC 4..7)        
       _writeCommand(0x40 | (idx & 0x3F));       //Set CG-RAM address, 8 sequential locations needed per UDC
       _writeData(data);                         // Set Icon pattern (Instr Set = 1)
       break; // case PCF2103_3V3 Controller
                 
    default:
       break; // end default      
  } // end switch _ctrl           
  
  //Select DD RAM again for current LCD controller and restore the addresspointer
  int addr = getAddress(_column, _row);
  _writeCommand(0x80 | addr);  
         
} // end setIcon()

/** Clear Icons
  *
  * @param  none
  * @return none
  */
  //@TODO Add support for 40x4 dual controller    
void TextLCD_Base::clrIcon() {
  // Icons are defined by a byte bitpattern. The P0..P5 form the Icon pattern for KS0073, and P0..P4 for KS0078
  //     P7 P6 P5 P4 P3 P2 P1 P0 
  // 0   B1 B0  0  0  0  0  0  0
  // 1   B1 B0  0  0  0  0  0  0
  //        .............
  // 15  B1 B0  0  0  0  0  0  0
  //
  // Bit 6 and Bit 7 in the pattern will control the blinking mode when Blink is enabled through BE. 
  //     B1 B0  Mode
  //      0  0  No Blinking for this icon row
  //      0  1  Enabled pixels in P5 will blink
  //      1  x  Enabled pixels in P0..P5 will blink
  //
  // Note: the PCF2103 and PCF2113 use UDCs to set Icons 
  //   3 x 8 rows x 5 bits = 120 bits Icons for Normal pattern (UDC 0..2) and
  //   3 x 8 rows x 5 bits = 120 bits Icons for Blink pattern (UDC 4..6)  
  // Note: the PCF2119 uses UDCs to set Icons 
  //   4 x 8 rows x 5 bits = 160 bits Icons for Normal pattern (UDC 0..3) and
  //   4 x 8 rows x 5 bits = 160 bits Icons for Blink pattern (UDC 4..7)  
  int idx;
  
  switch (_ctrl) {
    case KS0073:              
    case KS0078:                  
       _writeCommand(0x20 | _function_1);        // Set function, 0 0 1 DL N RE(1) BE LP 
                                                 // Select Extended Instruction Set
       for (idx=0; idx<16; idx++) { 
         _writeCommand(0x40 | idx);              // Set Icon Address, mask Address to valid range (Ext Instr Set)
         _writeData(0x00);                       // Clear Icon pattern (Ext Instr Set)
       }  
       _writeCommand(0x20 | _function);          // Set function, 0 0 1 DL N RE(0) DH REV Select Std Instruction Set
                                                 // Select Std Instr set        
       break; // end KS0073, KS0078      

    case ST7032_3V3:
    case ST7032_5V:
    case SPLC792A_3V3:
       _writeCommand(0x20 | _function | 0x01);   // Set function,  0 0 1 DL N F 0 IS=1 Select Instr Set = 1              

       for (idx=0; idx<16; idx++) { 
         _writeCommand(0x40 | idx);              // Set Icon Address, mask Address to valid range (Instr Set 1)
         _writeData(0x00);                       // Clear Icon pattern (Instr Set 1)
       }  

       _writeCommand(0x20 | _function);          // Set function, 0 0 1 DL N RE(0) DH REV Select Instruction Set 0
                                                 // Select Std Instr set, Select IS=0        
       break; // end ST7032

    case ST7036_3V3:
    case ST7036_5V:
       _writeCommand(0x20 | _function | 0x01);   // Set function, 0 0 1 DL N DH IS2,IS1 = 01 (Select Instr Set = 1)    

       for (idx=0; idx<16; idx++) { 
         _writeCommand(0x40 | idx);              // Set Icon Address, mask Address to valid range (Instr Set 1)
         _writeData(0x00);                       // Clear Icon pattern (Instr Set 1)
       }  

       _writeCommand(0x20 | _function);          // Set function, IS2,IS1 = 00 (Select Instr Set = 0)    
                                                 // Select Std Instr set, Select IS=0        
       break; // end ST7036

    case SSD1803_3V3:                  
//    case SSD1803_5V:                      
       _writeCommand(0x20 | _function | 0x01);   // Set function, 0 0 1 DL N DH RE(0) IS 
                                                 // Select Instruction Set 1
       for (idx=0; idx<16; idx++) { 
         _writeCommand(0x40 | idx);              // Set Icon Address, mask Address to valid range (Ext Instr Set)
         _writeData(0x00);                       // Clear Icon pattern (Ext Instr Set)
       }  
       _writeCommand(0x20 | _function);          // Set function, 0 0 1 DL N DH RE(0) IS
                                                 // Select IS=0        
       break; // end SSD1803      

     case PCF2103_3V3:
     case PCF2113_3V3:         
       // PCF2103 and PCF2113 use part of the UDC RAM to control Icons   
       // Select CG RAM

       _writeCommand(0x40 | (0 * 8)); //Set CG-RAM address, 8 sequential locations needed per UDC
       // Store UDC/Icon pattern: 
       //   3 x 8 rows x 5 bits = 120 bits for Normal pattern (UDC 0..2) and
       for (int i=0; i<(3 * 8); i++) {
//       _writeData(0x1F);  // All On
         _writeData(0x00);  // All Off            
       }

       _writeCommand(0x40 | (4 * 8)); //Set CG-RAM address, 8 sequential locations needed per UDC
       //   3 x 8 rows x 5 bits = 120 bits for Blink pattern (UDC 4..6) 
       for (int i=0; i<(3 * 8); i++) {
//       _writeData(0x1F);  // All On
         _writeData(0x00);  // All Off            
       }
       break; // case PCF2103_3V3 Controller

     case PCF2119_3V3:  
     case PCF2119R_3V3:
       // PCF2119 uses part of the UDC RAM to control Icons   
       // Select CG RAM

       _writeCommand(0x40 | (0 * 8)); //Set CG-RAM address, 8 sequential locations needed per UDC
       // Store UDC/Icon pattern: 
       //   4 x 8 rows x 5 bits = 160 bits for Normal pattern (UDC 0..3) and
       for (int i=0; i<(4 * 8); i++) {
//       _writeData(0x1F);  // All On
         _writeData(0x00);  // All Off            
       }

       _writeCommand(0x40 | (4 * 8)); //Set CG-RAM address, 8 sequential locations needed per UDC
       //   4 x 8 rows x 5 bits = 160 bits for Blink pattern (UDC 4..7) 
       for (int i=0; i<(4 * 8); i++) {
//       _writeData(0x1F);  // All On
         _writeData(0x00);  // All Off            
       }
       break; // case PCF2119_3V3 Controller

    default:
       break; // end default      
  } // end switch _ctrl           
  
  //Select DD RAM again for current LCD controller and restore the addresspointer
  int addr = getAddress(_column, _row);
  _writeCommand(0x80 | addr);
} //end clrIcon()
#endif

#if(LCD_INVERT == 1)
/** Set Invert
  * setInvert method is supported by some compatible devices (eg KS0073) to swap between black and white 
  *
  * @param bool invertOn  Invert on/off   
  * @return none
  */
//@TODO Add support for 40x4 dual controller  
void TextLCD_Base::setInvert(bool invertOn) {
  
  if (invertOn) {
    // Controllers that support Invert
    switch (_ctrl) {   
      case KS0073:        
      case KS0078:        
        _function = _function | 0x01;                                  // Enable Invert
        _writeCommand(0x20 | _function);                               // Activate Invert (Std Instr Set)
        break;
      case SSD1803_3V3 :      
//      case SSD1803_5V :                 
      case US2066_3V3:
//      case USS2066_5V:
        _function_1 = _function_1 | 0x01;                              // Enable Invert 
                                                                       // Set function, 0 0 1 DL N BE RE(1) REV  (SSD1803)
                                                                       // Set function, 0 0 1 X  N BE RE(1) REV  (US2066)    
        _writeCommand(0x20 | _function_1);                             // Activate Invert (Ext Instr Set)
        _writeCommand(0x20 | _function);                               // Return to Std Instr Set                            
        break;
      default:  
        //Unsupported feature for other controllers
        break;              
    } // end switch  
  }  
  else {
    // Controllers that support Invert
    switch (_ctrl) {    
      case KS0073:
      case KS0078:
        _function = _function & ~0x01;                                 // Disable Invert
        _writeCommand(0x20 | _function);                               // Disable Invert (Std Instr Set)
        break;
      case SSD1803_3V3 :      
//      case SSD1803_5V :                 
      case US2066_3V3:
//      case USS2066_5V:
        _function_1 = _function_1 & ~0x01;                             // Disable Invert 
                                                                       // Set function, 0 0 1 DL N BE RE(1) REV  (SSD1803)
                                                                       // Set function, 0 0 1 X  N BE RE(1) REV  (US2066)                                                                                                                                              
        _writeCommand(0x20 | _function_1);                             // Activate Invert (Ext Instr Set)
        _writeCommand(0x20 | _function);                               // Return to Std Instr Set                            
        break;

      default:  
        //Unsupported feature for other controllers
        break;              
    } // end switch  
  }
} // end setInvert()
#endif

//--------- End TextLCD_Base -----------


//--------- Start TextLCD Bus -----------

/* Create a TextLCD interface for using regular mbed pins
 *
 * @param rs     Instruction/data control line
 * @param e      Enable line (clock)
 * @param d4-d7  Data lines for using as a 4-bit interface
 * @param type   Sets the panel size/addressing mode (default = LCD16x2)
 * @param bl     Backlight control line (optional, default = NC)  
 * @param e2     Enable2 line (clock for second controller, LCD40x4 only) 
 * @param ctrl   LCD controller (default = HD44780)   
 */ 
TextLCD::TextLCD(PinName rs, PinName e,
                 PinName d4, PinName d5, PinName d6, PinName d7,
                 LCDType type, PinName bl, PinName e2, LCDCtrl ctrl) :
                 TextLCD_Base(type, ctrl), 
                 _rs(rs), _e(e), _d(d4, d5, d6, d7) {

  // The hardware Backlight pin is optional. Test and make sure whether it exists or not to prevent illegal access.
  if (bl != NC) {
    _bl = new DigitalOut(bl);   //Construct new pin 
    _bl->write(0);              //Deactivate    
  }
  else {
    // No Hardware Backlight pin       
    _bl = NULL;                 //Construct dummy pin     
  }  

  // The hardware Enable2 pin is only needed for LCD40x4. Test and make sure whether it exists or not to prevent illegal access.
  if (e2 != NC) {
    _e2 = new DigitalOut(e2);   //Construct new pin 
    _e2->write(0);              //Deactivate    
  }
  else {
    // No Hardware Enable pin       
    _e2 = NULL;                 //Construct dummy pin     
  }  
  
   _init(_LCD_DL_4);   // Set Datalength to 4 bit for mbed bus interfaces
}

/** Destruct a TextLCD interface for using regular mbed pins
  *
  * @param  none
  * @return none
  */ 
TextLCD::~TextLCD() {
   if (_bl != NULL) {delete _bl;}  // BL pin
   if (_e2 != NULL) {delete _e2;}  // E2 pin
}

/** Set E pin (or E2 pin)
  * Used for mbed pins, I2C bus expander or SPI shiftregister
  * Default PinName value for E2 is NC, must be used as pointer to avoid issues with mbed lib and DigitalOut pins
  *   @param  value true or false
  *   @return none 
  */
void TextLCD::_setEnable(bool value) {

  if(_ctrl_idx==_LCDCtrl_0) {
    if (value) {
      _e  = 1;    // Set E bit 
    }  
    else { 
      _e  = 0;    // Reset E bit  
    }  
  }    
  else { 
    if (value) {
      if (_e2 != NULL) {_e2->write(1);}  //Set E2 bit
    }  
    else { 
      if (_e2 != NULL) {_e2->write(0);}  //Reset E2 bit     
    }  
  }    
}    

// Set RS pin
// Used for mbed pins, I2C bus expander or SPI shiftregister
void TextLCD::_setRS(bool value) {

  if (value) {
    _rs  = 1;    // Set RS bit 
  }  
  else  {
    _rs  = 0;    // Reset RS bit 
  }  
}    

/** Set BL pin
  * Used for mbed pins, I2C bus expander or SPI shiftregister
  * Default PinName value is NC, must be used as pointer to avoid issues with mbed lib and DigitalOut pins
  *   @param  value true or false
  *   @return none  
  */
void TextLCD::_setBL(bool value) {

  if (value) {
    if (_bl != NULL) {_bl->write(1);}  //Set BL bit
  }  
  else { 
    if (_bl != NULL) {_bl->write(0);}  //Reset BL bit  
  }  
}    

// Place the 4bit data on the databus
// Used for mbed pins, I2C bus expander or SPI shifregister
void TextLCD::_setData(int value) {
  _d = value & 0x0F;   // Write Databits 
}    

//----------- End TextLCD ---------------


//--------- Start TextLCD_I2C -----------
#if(LCD_I2C == 1) /* I2C Expander PCF8574/MCP23008 */
/** Create a TextLCD interface using an I2C PC8574 (or PCF8574A) or MCP23008 portexpander
  *
  * @param i2c             I2C Bus
  * @param deviceAddress   I2C slave address (PCF8574, PCF8574A or MCP23008, default = 0x40)
  * @param type            Sets the panel size/addressing mode (default = LCD16x2)
  * @param ctrl            LCD controller (default = HD44780)    
  */
TextLCD_I2C::TextLCD_I2C(I2C *i2c, char deviceAddress, LCDType type, LCDCtrl ctrl) :
                         TextLCD_Base(type, ctrl), 
                         _i2c(i2c){
                              
  _slaveAddress = deviceAddress & 0xFE;

  // Setup the I2C bus
  // The max bitrate for PCF8574 is 100kbit, the max bitrate for MCP23008 is 400kbit, 
  _i2c->frequency(100000);
  
#if (MCP23008==1)
  // MCP23008 portexpander Init
  _writeRegister(IODIR,   0x00);  // All pins are outputs
  _writeRegister(IPOL,    0x00);  // No reverse polarity on inputs
  _writeRegister(GPINTEN, 0x00);  // No interrupt on change of input pins
  _writeRegister(DEFVAL,  0x00);  // Default value to compare against for interrupts
  _writeRegister(INTCON,  0x00);  // No interrupt on changes, compare against previous pin value 
  _writeRegister(IOCON,   0x20);  // b1=0 - Interrupt polarity active low  
                                  // b2=0 - Interrupt pin active driver output  
                                  // b4=0 - Slew rate enable on SDA
                                  // b5=0 - Auto-increment on registeraddress                                  
                                  // b5=1 - No auto-increment on registeraddress => needed for performance improved I2C expander mode
  _writeRegister(GPPU,    0x00);  // No Pullup 
//               INTF             // Interrupt flags read (Read-Only)
//               INTCAP           // Captured inputpins at time of interrupt (Read-Only)   
//  _writeRegister(GPIO,    0x00);  // Output/Input pins   
//  _writeRegister(OLAT,    0x00);  // Output Latch  
    
  // Init the portexpander bus
  _lcd_bus = LCD_BUS_I2C_DEF;
  
  // write the new data to the portexpander
  _writeRegister(GPIO, _lcd_bus);      
#else
  // PCF8574 of PCF8574A portexpander

  // Init the portexpander bus
  _lcd_bus = LCD_BUS_I2C_DEF;

  // write the new data to the portexpander
  _i2c->write(_slaveAddress, &_lcd_bus, 1);    
#endif

  _init(_LCD_DL_4);   // Set Datalength to 4 bit for all serial expander interfaces
}

// Set E bit (or E2 bit) in the databus shadowvalue
// Used for mbed I2C bus expander
void TextLCD_I2C::_setEnableBit(bool value) {

#if (LCD_TWO_CTRL == 1)
  if(_ctrl_idx==_LCDCtrl_0) {
    if (value) {
      _lcd_bus |= LCD_BUS_I2C_E;     // Set E bit 
    }  
    else {                    
      _lcd_bus &= ~LCD_BUS_I2C_E;    // Reset E bit                     
    }  
  }
  else {
    if (value) {
      _lcd_bus |= LCD_BUS_I2C_E2;    // Set E2 bit 
    }  
    else {
      _lcd_bus &= ~LCD_BUS_I2C_E2;   // Reset E2bit                     
    }  
  }    
#else
// Support only one controller
  if (value) {
    _lcd_bus |= LCD_BUS_I2C_E;     // Set E bit 
  }  
  else {                    
    _lcd_bus &= ~LCD_BUS_I2C_E;    // Reset E bit                     
  }  

#endif  
}    

// Set E pin (or E2 pin)
// Used for mbed pins, I2C bus expander or SPI shiftregister
void TextLCD_I2C::_setEnable(bool value) {

  // Place the E or E2 bit data on the databus shadowvalue
  _setEnableBit(value);

#if (MCP23008==1)
  // MCP23008 portexpander
  
  // write the new data to the portexpander
  _writeRegister(GPIO, _lcd_bus);      
#else
  // PCF8574 of PCF8574A portexpander

  // write the new data to the I2C portexpander
  _i2c->write(_slaveAddress, &_lcd_bus, 1);    
#endif
}    


// Set RS pin
// Used for mbed pins, I2C bus expander or SPI shiftregister
void TextLCD_I2C::_setRS(bool value) {

  if (value) {
    _lcd_bus |= LCD_BUS_I2C_RS;    // Set RS bit 
  }  
  else {                    
    _lcd_bus &= ~LCD_BUS_I2C_RS;   // Reset RS bit                     
  }

#if (MCP23008==1)
  // MCP23008 portexpander
  
  // write the new data to the portexpander
  _writeRegister(GPIO, _lcd_bus);      
#else
  // PCF8574 of PCF8574A portexpander

  // write the new data to the I2C portexpander
  _i2c->write(_slaveAddress, &_lcd_bus, 1);    
#endif                  
}    

// Set BL pin
// Used for mbed pins, I2C bus expander or SPI shiftregister
void TextLCD_I2C::_setBL(bool value) {

  if (value) {
    _lcd_bus |= LCD_BUS_I2C_BL;    // Set BL bit 
  }  
  else {                    
    _lcd_bus &= ~LCD_BUS_I2C_BL;   // Reset BL bit                     
  }
  
#if (MCP23008==1)
  // MCP23008 portexpander
  
  // write the new data to the portexpander
  _writeRegister(GPIO, _lcd_bus);      
#else
  // PCF8574 of PCF8574A portexpander

  // write the new data to the I2C portexpander
  _i2c->write(_slaveAddress, &_lcd_bus, 1);    
#endif                 
}    

#if(0)
// New optimized v018
// Test faster _writeByte 0.11s vs 0.27s for a 20x4 fillscreen (PCF8574), same as v018
// Place the 4bit data in the databus shadowvalue
// Used for mbed I2C bus expander
const char _LCD_DATA_BITS[16] = {
      0x00,
      (                                                   LCD_BUS_I2C_D4),
      (                                  LCD_BUS_I2C_D5                 ),
      (                                  LCD_BUS_I2C_D5 | LCD_BUS_I2C_D4),
      (                 LCD_BUS_I2C_D6                                  ),                  
      (                 LCD_BUS_I2C_D6                  | LCD_BUS_I2C_D4),
      (                 LCD_BUS_I2C_D6 | LCD_BUS_I2C_D5                 ),
      (                 LCD_BUS_I2C_D6 | LCD_BUS_I2C_D5 | LCD_BUS_I2C_D4),
      (LCD_BUS_I2C_D7                                                   ),
      (LCD_BUS_I2C_D7                                   | LCD_BUS_I2C_D4),
      (LCD_BUS_I2C_D7                  | LCD_BUS_I2C_D5                 ),
      (LCD_BUS_I2C_D7                  | LCD_BUS_I2C_D5 | LCD_BUS_I2C_D4),                  
      (LCD_BUS_I2C_D7 | LCD_BUS_I2C_D6                                  ),
      (LCD_BUS_I2C_D7 | LCD_BUS_I2C_D6                  | LCD_BUS_I2C_D4),
      (LCD_BUS_I2C_D7 | LCD_BUS_I2C_D6 | LCD_BUS_I2C_D5                 ),
      (LCD_BUS_I2C_D7 | LCD_BUS_I2C_D6 | LCD_BUS_I2C_D5 | LCD_BUS_I2C_D4)
    };
void TextLCD_I2C::_setDataBits(int value) {

  //Clear all databits
  _lcd_bus &= ~LCD_BUS_I2C_MSK;

  // Set bit by bit to support any mapping of expander portpins to LCD pins 
  _lcd_bus |= _LCD_DATA_BITS[value & 0x0F];
}    
#endif

// Test faster _writeByte 0.11s vs 0.27s for a 20x4 fillscreen (PCF8574)
// Place the 4bit data in the databus shadowvalue
// Used for mbed I2C bus expander
void TextLCD_I2C::_setDataBits(int value) {

  //Clear all databits
  _lcd_bus &= ~LCD_BUS_I2C_MSK;

  // Set bit by bit to support any mapping of expander portpins to LCD pins 
  if (value & 0x01){
    _lcd_bus |= LCD_BUS_I2C_D4;   // Set Databit 
  }  

  if (value & 0x02){
    _lcd_bus |= LCD_BUS_I2C_D5;   // Set Databit 
  }  

  if (value & 0x04) {
    _lcd_bus |= LCD_BUS_I2C_D6;   // Set Databit 
  }  

  if (value & 0x08) {
    _lcd_bus |= LCD_BUS_I2C_D7;   // Set Databit 
  }  
}    


// Place the 4bit data on the databus
// Used for mbed pins, I2C bus expander or SPI shifregister
void TextLCD_I2C::_setData(int value) {

  // Place the 4bit data on the databus shadowvalue
  _setDataBits(value); 
  
  // Place the 4bit data on the databus
#if (MCP23008==1)
  // MCP23008 portexpander
  
  // write the new data to the portexpander
  _writeRegister(GPIO, _lcd_bus);      
#else
  // PCF8574 of PCF8574A portexpander

  // write the new data to the I2C portexpander
  _i2c->write(_slaveAddress, &_lcd_bus, 1);    
#endif                 
}    

// Write data to MCP23008 I2C portexpander
// Used for mbed I2C bus expander
void TextLCD_I2C::_writeRegister (int reg, int value) {
  char data[] = {reg, value};
    
  _i2c->write(_slaveAddress, data, 2); 
}

//New optimized
//Test faster _writeByte 0.11s vs 0.27s for a 20x4 fillscreen (PCF8574)
//Test faster _writeByte 0.14s vs 0.34s for a 20x4 fillscreen (MCP23008)

// Write a byte using I2C
void TextLCD_I2C::_writeByte(int value) {
  char data[6];
  
#if (MCP23008==1)
  // MCP23008 portexpander

  data[0] = GPIO;                 // set registeraddres
                                  // Note: auto-increment is disabled so all data will go to GPIO register
  
  _setEnableBit(true);            // set E 
  _setDataBits(value >> 4);       // set data high  
  data[1] = _lcd_bus;
  
  _setEnableBit(false);           // clear E   
  data[2] = _lcd_bus;
  
  _setEnableBit(true);            // set E   
  _setDataBits(value);            // set data low    
  data[3] = _lcd_bus;
  
  _setEnableBit(false);           // clear E     
  data[4] = _lcd_bus;
  
  // write the packed data to the I2C portexpander
  _i2c->write(_slaveAddress, data, 5);    
#else
  // PCF8574 of PCF8574A portexpander
  
  _setEnableBit(true);            // set E 
  _setDataBits(value >> 4);       // set data high  
  data[0] = _lcd_bus;
  
  _setEnableBit(false);           // clear E   
  data[1] = _lcd_bus;
  
  _setEnableBit(true);            // set E   
  _setDataBits(value);            // set data low    
  data[2] = _lcd_bus;
  
  _setEnableBit(false);           // clear E     
  data[3] = _lcd_bus;
  
  // write the packed data to the I2C portexpander
  _i2c->write(_slaveAddress, data, 4);    
#endif
}

#endif /* I2C Expander PCF8574/MCP23008 */
//---------- End TextLCD_I2C ------------


//--------- Start TextLCD_SPI -----------
#if(LCD_SPI == 1) /* SPI Expander SN74595          */

 /** Create a TextLCD interface using an SPI 74595 portexpander
   *
   * @param spi             SPI Bus
   * @param cs              chip select pin (active low)
   * @param type            Sets the panel size/addressing mode (default = LCD16x2)
   * @param ctrl            LCD controller (default = HD44780)      
   */
TextLCD_SPI::TextLCD_SPI(SPI *spi, PinName cs, LCDType type, LCDCtrl ctrl) :
                         TextLCD_Base(type, ctrl), 
                         _spi(spi),        
                         _cs(cs) {              
  // Init cs
  _cs = 1;  

  // Setup the spi for 8 bit data, low steady state clock,
  // rising edge capture, with a 500KHz or 1MHz clock rate  
  _spi->format(8,0);
  _spi->frequency(500000);    
  //_spi.frequency(1000000);    

  wait_ms(100);                   // Wait 100ms to ensure LCD powered up
  
  // Init the portexpander bus
  _lcd_bus = LCD_BUS_SPI_DEF;
  
  // write the new data to the portexpander
  _cs = 0;  
  _spi->write(_lcd_bus);   
  _cs = 1;  

  _init(_LCD_DL_4);   // Set Datalength to 4 bit for all serial expander interfaces
}

// Set E pin (or E2 pin)
// Used for mbed pins, I2C bus expander or SPI shiftregister
void TextLCD_SPI::_setEnable(bool value) {

  if(_ctrl_idx==_LCDCtrl_0) {
    if (value) {
      _lcd_bus |= LCD_BUS_SPI_E;     // Set E bit 
    }  
    else {                    
      _lcd_bus &= ~LCD_BUS_SPI_E;    // Reset E bit                     
    }  
  }
  else {
    if (value) {
      _lcd_bus |= LCD_BUS_SPI_E2;    // Set E2 bit 
    }  
    else {
      _lcd_bus &= ~LCD_BUS_SPI_E2;   // Reset E2 bit                     
    }  
  }
                  
  // write the new data to the SPI portexpander
  _cs = 0;  
  _spi->write(_lcd_bus);   
  _cs = 1;    
}    

// Set RS pin
// Used for mbed pins, I2C bus expander or SPI shiftregister and SPI_N
void TextLCD_SPI::_setRS(bool value) {

  if (value) {
    _lcd_bus |= LCD_BUS_SPI_RS;    // Set RS bit 
  }  
  else {                    
    _lcd_bus &= ~LCD_BUS_SPI_RS;   // Reset RS bit                     
  }
     
  // write the new data to the SPI portexpander
  _cs = 0;  
  _spi->write(_lcd_bus);   
  _cs = 1;     
}    

// Set BL pin
// Used for mbed pins, I2C bus expander or SPI shiftregister
void TextLCD_SPI::_setBL(bool value) {

  if (value) {
    _lcd_bus |= LCD_BUS_SPI_BL;    // Set BL bit 
  }  
  else {
    _lcd_bus &= ~LCD_BUS_SPI_BL;   // Reset BL bit                     
  }
      
  // write the new data to the SPI portexpander
  _cs = 0;  
  _spi->write(_lcd_bus);   
  _cs = 1;      
}    

// Place the 4bit data on the databus
// Used for mbed pins, I2C bus expander or SPI shiftregister
void TextLCD_SPI::_setData(int value) {

  // Set bit by bit to support any mapping of expander portpins to LCD pins
  if (value & 0x01) {
    _lcd_bus |= LCD_BUS_SPI_D4;   // Set Databit 
  }  
  else {                    
    _lcd_bus &= ~LCD_BUS_SPI_D4;  // Reset Databit                     
  }
  
  if (value & 0x02) {
    _lcd_bus |= LCD_BUS_SPI_D5;   // Set Databit 
  }  
  else {
    _lcd_bus &= ~LCD_BUS_SPI_D5;  // Reset Databit                     
  }
  
  if (value & 0x04) {
    _lcd_bus |= LCD_BUS_SPI_D6;   // Set Databit 
  }  
  else {
    _lcd_bus &= ~LCD_BUS_SPI_D6;  // Reset Databit                     
  }
  
  if (value & 0x08) {
    _lcd_bus |= LCD_BUS_SPI_D7;   // Set Databit 
  }  
  else {
    _lcd_bus &= ~LCD_BUS_SPI_D7;  // Reset Databit
  }  
                    
  // write the new data to the SPI portexpander
  _cs = 0;  
  _spi->write(_lcd_bus);   
  _cs = 1;       
}    

#endif /* SPI Expander SN74595          */
//---------- End TextLCD_SPI ------------


//--------- Start TextLCD_I2C_N ---------
#if(LCD_I2C_N == 1)  /* Native I2C */

 /** Create a TextLCD interface using a controller with native I2C interface
   *
   * @param i2c             I2C Bus
   * @param deviceAddress   I2C slave address (default = 0x7C)  
   * @param type            Sets the panel size/addressing mode (default = LCD16x2)
   * @param bl              Backlight control line (optional, default = NC)     
   * @param ctrl            LCD controller (default = ST7032_3V3)                     
   */
TextLCD_I2C_N::TextLCD_I2C_N(I2C *i2c, char deviceAddress, LCDType type, PinName bl, LCDCtrl ctrl) : 
                               TextLCD_Base(type, ctrl), 

                               _i2c(i2c){
  
  _slaveAddress = deviceAddress & 0xFE;
  
  // Setup the I2C bus
  // The max bitrate for ST7032i is 400kbit, lets stick to default here
  _i2c->frequency(100000);

       
  // The hardware Backlight pin is optional. Test and make sure whether it exists or not to prevent illegal access.
  if (bl != NC) {
    _bl = new DigitalOut(bl);   //Construct new pin 
    _bl->write(0);              //Deactivate    
  }
  else {
    // No Hardware Backlight pin       
    _bl = NULL;                 //Construct dummy pin     
  }  
  
  //Sanity check
  if (_ctrl & LCD_C_I2C) {
    _init(_LCD_DL_8);  // Set Datalength to 8 bit for all native serial interfaces      
  }
  else {
    error("Error: LCD Controller type does not support native I2C interface\n\r");           
  }
}

TextLCD_I2C_N::~TextLCD_I2C_N() {
   if (_bl != NULL) {delete _bl;}  // BL pin
}

// Not used in this mode
void TextLCD_I2C_N::_setEnable(bool value) {
}    

// Set RS pin
// Used for mbed pins, I2C bus expander or SPI shiftregister and native I2C or SPI
void TextLCD_I2C_N::_setRS(bool value) {
// The controlbyte defines the meaning of the next byte. This next byte can either be data or command.
// Start Slaveaddress+RW  b7 b6 b5 b4 b3 b2 b1 b0   b7...........b0  Stop
//                        Co RS RW  0  0  0  0  0   command or data
//
//   C0=1 indicates that another controlbyte will follow after the next data or command byte 
//   RS=1 means that next byte is data, RS=0 means that next byte is command
//   RW=0 means write to controller. RW=1 means that controller will be read from after the next command. 
//        Many native I2C controllers dont support this option and it is not used by this lib. 
//

  if (value) {
    _controlbyte = 0x40; // Next byte is data, No more control bytes will follow
  }
  else {
    _controlbyte = 0x00; // Next byte is command, No more control bytes will follow     
  }
}    

// Set BL pin
void TextLCD_I2C_N::_setBL(bool value) {
    if (_bl) {
        _bl->write(value);   
    }    
}    
    
// Not used in this mode
void TextLCD_I2C_N::_setData(int value) {
}    

// Write a byte using I2C
void TextLCD_I2C_N::_writeByte(int value) {
// The controlbyte defines the meaning of the next byte. This next byte can either be data or command.
// Start Slaveaddress+RW  b7 b6 b5 b4 b3 b2 b1 b0   b7...........b0  Stop
//                        Co RS RW  0  0  0  0  0   command or data
//
//   C0=1 indicates that another controlbyte will follow after the next data or command byte 
//   RS=1 means that next byte is data, RS=0 means that next byte is command
//   RW=0 means write to controller. RW=1 means that controller will be read from after the next command. 
//        Many native I2C controllers dont support this option and it is not used by this lib. 
//
  char data[] = {_controlbyte, value};
    
#if(LCD_I2C_ACK==1)
//Controllers that support ACK
  _i2c->write(_slaveAddress, data, 2); 
#else  
//Controllers that dont support ACK
//Note: This may be issue with some mbed platforms that dont fully/correctly support I2C byte operations.
  _i2c->start(); 
  _i2c->write(_slaveAddress);   
  _i2c->write(data[0]); 
  _i2c->write(data[1]);     
  _i2c->stop();   
#endif  
}
#endif /* Native I2C */
//-------- End TextLCD_I2C_N ------------


//--------- Start TextLCD_SPI_N ---------
#if(LCD_SPI_N == 1) /* Native SPI bus     */
 /** Create a TextLCD interface using a controller with a native SPI4 interface
   *
   * @param spi             SPI Bus
   * @param cs              chip select pin (active low)
   * @param rs              Instruction/data control line
   * @param type            Sets the panel size/addressing mode (default = LCD16x2)
   * @param bl              Backlight control line (optional, default = NC)  
   * @param ctrl            LCD controller (default = ST7032_3V3) 
   */       
TextLCD_SPI_N::TextLCD_SPI_N(SPI *spi, PinName cs, PinName rs, LCDType type, PinName bl, LCDCtrl ctrl) :
                             TextLCD_Base(type, ctrl), 
                             _spi(spi),        
                             _cs(cs),
                             _rs(rs) {      
        
  // Init CS
  _cs = 1;

  // Setup the spi for 8 bit data, high steady state clock,
  // rising edge capture, with a 500KHz or 1MHz clock rate    
//  _spi->format(8,3);  
//  _spi->frequency(500000); 
//  _spi->frequency(1000000);    
  
  // Setup the spi for 8 bit data, low steady state clock,
  // rising edge capture, with a 500KHz or 1MHz clock rate  
  _spi->format(8,0);
//  _spi->frequency(500000); 
  _spi->frequency(1000000);    
    
  // The hardware Backlight pin is optional. Test and make sure whether it exists or not to prevent illegal access.
  if (bl != NC) {
    _bl = new DigitalOut(bl);   //Construct new pin 
    _bl->write(0);              //Deactivate    
  }
  else {
    // No Hardware Backlight pin       
    _bl = NULL;                 //Construct dummy pin     
  }  

  //Sanity check
  if (_ctrl & LCD_C_SPI4) {
    _init(_LCD_DL_8);   // Set Datalength to 8 bit for all native serial interfaces
                        // ST7070 must set datalength to 8 bits!
  }
  else {
    error("Error: LCD Controller type does not support native SPI4 interface\n\r");           
  }
}

TextLCD_SPI_N::~TextLCD_SPI_N() {
   if (_bl != NULL) {delete _bl;}  // BL pin
}

// Not used in this mode
void TextLCD_SPI_N::_setEnable(bool value) {
}    

// Set RS pin
// Used for mbed pins, I2C bus expander or SPI shiftregister, SPI_N
void TextLCD_SPI_N::_setRS(bool value) {
    _rs = value;
}    

// Set BL pin
void TextLCD_SPI_N::_setBL(bool value) {
    if (_bl) {
        _bl->write(value);   
    }    
}    

// Not used in this mode
void TextLCD_SPI_N::_setData(int value) {
}    

// Write a byte using SPI
void TextLCD_SPI_N::_writeByte(int value) {
    _cs = 0;
    wait_us(1);
    _spi->write(value);
    wait_us(1);
    _cs = 1;
}
#endif /* Native SPI bus     */  
//-------- End TextLCD_SPI_N ------------


//-------- Start TextLCD_SPI_N_3_8 --------
#if(LCD_SPI_N_3_8 == 1) /* Native SPI bus     */

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
TextLCD_SPI_N_3_8::TextLCD_SPI_N_3_8(SPI *spi, PinName cs, LCDType type, PinName bl, LCDCtrl ctrl) :
                                     TextLCD_Base(type, ctrl), 
                                     _spi(spi),        
                                     _cs(cs) {      

  // Init CS
  _cs = 1;

  // Setup the spi for 8 bit data, high steady state clock,
  // rising edge capture, with a 500KHz or 1MHz clock rate  
//  _spi->format(8,3);  
//  _spi->frequency(500000);    
//  _spi->frequency(1000000);    

  // Setup the spi for 8 bit data, low steady state clock,
  // rising edge capture, with a 500KHz or 1MHz clock rate  
  _spi->format(8,0);
//  _spi->frequency(500000); 
  _spi->frequency(1000000);    
  
  
  // The hardware Backlight pin is optional. Test and make sure whether it exists or not to prevent illegal access.
  if (bl != NC) {
    _bl = new DigitalOut(bl);   //Construct new pin 
    _bl->write(0);              //Deactivate    
  }
  else {
    // No Hardware Backlight pin       
    _bl = NULL;                 //Construct dummy pin     
  }  

  //Sanity check
  if (_ctrl & LCD_C_SPI3_8) { 
    _init(_LCD_DL_8);  // Set Datalength to 8 bit for all native serial interfaces   
  }
  else {
    error("Error: LCD Controller type does not support native SPI3 8 bits interface\n\r");           
  }
}

TextLCD_SPI_N_3_8::~TextLCD_SPI_N_3_8() {
   if (_bl != NULL) {delete _bl;}  // BL pin
}

// Not used in this mode
void TextLCD_SPI_N_3_8::_setEnable(bool value) {
}    

// Used for mbed pins, I2C bus expander or SPI shiftregister, SPI_N
//   RS=1 means that next byte is data, RS=0 means that next byte is command
void TextLCD_SPI_N_3_8::_setRS(bool value) {
  
  if (value) {
    _controlbyte = 0x01; // Next byte is data, No more control bytes will follow
  }
  else {
    _controlbyte = 0x00; // Next byte is command, No more control bytes will follow     
  }
}      
  
// Set BL pin
void TextLCD_SPI_N_3_8::_setBL(bool value) {
    if (_bl) {
        _bl->write(value);   
    }    
}    

// Not used in this mode
void TextLCD_SPI_N_3_8::_setData(int value) {
}    

// Write a byte using SPI3 8 bits mode (ST7070)
void TextLCD_SPI_N_3_8::_writeByte(int value) {
    
  if (_controlbyte == 0x00) { // Byte is command 
    _cs = 0;
    wait_us(1);
    _spi->write(value);
    wait_us(1);
    _cs = 1;
  }  
  else {                      // Byte is data 
    // Select Extended Instr Set
    _cs = 0;
    wait_us(1);
    _spi->write(0x20 | _function | 0x04);   // Set function, 0 0 1 DL N EXT=1 x x (Select Instr Set = 1));
    wait_us(1);
    _cs = 1;     

    wait_us(40);                            // Wait until command has finished...    
        
    // Set Count to 1 databyte
    _cs = 0;
    wait_us(1);    
    _spi->write(0x80);                      // Set display data length, 1 L6 L5 L4 L3 L2 L1 L0 (Instr Set = 1)
    wait_us(1);
    _cs = 1;

    wait_us(40);    
                
    // Write 1 databyte     
    _cs = 0;
    wait_us(1);    
    _spi->write(value);                     // Write data (Instr Set = 1)
    wait_us(1);
    _cs = 1;         

    wait_us(40);    
        
    // Select Standard Instr Set    
    _cs = 0;
    wait_us(1);    
    _spi->write(0x20 | _function);          // Set function, 0 0 1 DL N EXT=0 x x (Select Instr Set = 0));
    wait_us(1);
    _cs = 1;     
  }  
}
#endif /* Native SPI bus     */  
//------- End TextLCD_SPI_N_3_8 -----------


//-------- Start TextLCD_SPI_N_3_9 --------
#if(LCD_SPI_N_3_9 == 1) /* Native SPI bus     */
//Code checked out on logic analyser. Not yet tested on hardware..

 /** Create a TextLCD interface using a controller with a native SPI3 9 bits interface
   *
   * @param spi             SPI Bus
   * @param cs              chip select pin (active low)
   * @param type            Sets the panel size/addressing mode (default = LCD16x2)
   * @param bl              Backlight control line (optional, default = NC)  
   * @param ctrl            LCD controller (default = AIP31068) 
   */       
TextLCD_SPI_N_3_9::TextLCD_SPI_N_3_9(SPI *spi, PinName cs, LCDType type, PinName bl, LCDCtrl ctrl) :
                                     TextLCD_Base(type, ctrl), 
                                     _spi(spi),        
                                     _cs(cs) {      

  // Init CS
  _cs = 1;

  // Setup the spi for 9 bit data, high steady state clock,
  // rising edge capture, with a 500KHz or 1MHz clock rate  
  _spi->format(9,3);  
  _spi->frequency(1000000);    
  
  // The hardware Backlight pin is optional. Test and make sure whether it exists or not to prevent illegal access.
  if (bl != NC) {
    _bl = new DigitalOut(bl);   //Construct new pin 
    _bl->write(0);              //Deactivate    
  }
  else {
    // No Hardware Backlight pin       
    _bl = NULL;                 //Construct dummy pin     
  }  

  //Sanity check
  if (_ctrl & LCD_C_SPI3_9) { 
    _init(_LCD_DL_8);  // Set Datalength to 8 bit for all native serial interfaces   
  }
  else {
    error("Error: LCD Controller type does not support native SPI3 9 bits interface\n\r");           
  }
}

TextLCD_SPI_N_3_9::~TextLCD_SPI_N_3_9() {
   if (_bl != NULL) {delete _bl;}  // BL pin
}

// Not used in this mode
void TextLCD_SPI_N_3_9::_setEnable(bool value) {
}    

// Set RS pin
// Used for mbed pins, I2C bus expander or SPI shiftregister
void TextLCD_SPI_N_3_9::_setRS(bool value) {
// The controlbits define the meaning of the next byte. This next byte can either be data or command.
//   b8  b7...........b0 
//   RS  command or data
//
//   RS=1 means that next byte is data, RS=0 means that next byte is command
//

  if (value) {
    _controlbyte = 0x01; // Next byte is data
  }
  else {
    _controlbyte = 0x00; // Next byte is command
  }  
}    

// Set BL pin
void TextLCD_SPI_N_3_9::_setBL(bool value) {
    if (_bl) {
        _bl->write(value);   
    }    
}    

// Not used in this mode
void TextLCD_SPI_N_3_9::_setData(int value) {
}    

// Write a byte using SPI3 9 bits mode
void TextLCD_SPI_N_3_9::_writeByte(int value) {
    _cs = 0;
    wait_us(1);
    _spi->write( (_controlbyte << 8) | (value & 0xFF));
    wait_us(1);
    _cs = 1;
}
#endif /* Native SPI bus     */  
//------- End TextLCD_SPI_N_3_9 -----------


//------- Start TextLCD_SPI_N_3_10 --------
#if(LCD_SPI_N_3_10 == 1) /* Native SPI bus     */

 /** Create a TextLCD interface using a controller with a native SPI3 10 bits interface
   *
   * @param spi             SPI Bus
   * @param cs              chip select pin (active low)
   * @param type            Sets the panel size/addressing mode (default = LCD16x2)
   * @param bl              Backlight control line (optional, default = NC)  
   * @param ctrl            LCD controller (default = AIP31068) 
   */       
TextLCD_SPI_N_3_10::TextLCD_SPI_N_3_10(SPI *spi, PinName cs, LCDType type, PinName bl, LCDCtrl ctrl) :
                                       TextLCD_Base(type, ctrl), 
                                       _spi(spi),        
                                       _cs(cs) {      
        
  // Init CS
  _cs = 1;

  // Setup the spi for 10 bit data, low steady state clock,
  // rising edge capture, with a 500KHz or 1MHz clock rate  
  _spi->format(10,0);
  _spi->frequency(1000000);    
  
  // The hardware Backlight pin is optional. Test and make sure whether it exists or not to prevent illegal access.
  if (bl != NC) {
    _bl = new DigitalOut(bl);   //Construct new pin 
    _bl->write(0);              //Deactivate    
  }
  else {
    // No Hardware Backlight pin       
    _bl = NULL;                 //Construct dummy pin     
  }  

  //Sanity check
  if (_ctrl & LCD_C_SPI3_10) {
     _init(_LCD_DL_8);  // Set Datalength to 8 bit for all native serial interfaces            
  }
  else {
    error("Error: LCD Controller type does not support native SPI3 10 bits interface\n\r");           
  }
}

TextLCD_SPI_N_3_10::~TextLCD_SPI_N_3_10() {
   if (_bl != NULL) {delete _bl;}  // BL pin
}

// Not used in this mode
void TextLCD_SPI_N_3_10::_setEnable(bool value) {
}    

// Set RS pin
// Used for mbed pins, I2C bus expander or SPI shiftregister
void TextLCD_SPI_N_3_10::_setRS(bool value) {
// The controlbits define the meaning of the next byte. This next byte can either be data or command.
//   b9 b8  b7...........b0 
//   RS RW  command or data
//
//   RS=1 means that next byte is data, RS=0 means that next byte is command
//   RW=0 means that next byte is writen, RW=1 means that next byte is read (not used in this lib)
//

  if (value) {
    _controlbyte = 0x02; // Next byte is data
  }
  else {
    _controlbyte = 0x00; // Next byte is command
  }  
}    

// Set BL pin
void TextLCD_SPI_N_3_10::_setBL(bool value) {
    if (_bl) {
        _bl->write(value);   
    }    
}    

// Not used in this mode
void TextLCD_SPI_N_3_10::_setData(int value) {
}    

// Write a byte using SPI3 10 bits mode
void TextLCD_SPI_N_3_10::_writeByte(int value) {
    _cs = 0;
    wait_us(1);
    _spi->write( (_controlbyte << 8) | (value & 0xFF));
    wait_us(1);
    _cs = 1;
}
#endif /* Native SPI bus     */  
//------- End TextLCD_SPI_N_3_10 ----------


//------- Start TextLCD_SPI_N_3_16 --------
#if(LCD_SPI_N_3_16 == 1) /* Native SPI bus     */

 /** Create a TextLCD interface using a controller with a native SPI3 16 bits interface
   *
   * @param spi             SPI Bus
   * @param cs              chip select pin (active low)
   * @param type            Sets the panel size/addressing mode (default = LCD16x2)
   * @param bl              Backlight control line (optional, default = NC)  
   * @param ctrl            LCD controller (default = PT6314) 
   */       
TextLCD_SPI_N_3_16::TextLCD_SPI_N_3_16(SPI *spi, PinName cs, LCDType type, PinName bl, LCDCtrl ctrl) :
                                       TextLCD_Base(type, ctrl), 
                                       _spi(spi),        
                                       _cs(cs) {      
        
  // Init CS
  _cs = 1;

  // Setup the spi for 8 bit data, low steady state clock,
  // rising edge capture, with a 500KHz or 1MHz clock rate  
  _spi->format(8,0);
  _spi->frequency(1000000);    
  
  // The hardware Backlight pin is optional. Test and make sure whether it exists or not to prevent illegal access.
  if (bl != NC) {
    _bl = new DigitalOut(bl);   //Construct new pin 
    _bl->write(0);              //Deactivate    
  }
  else {
    // No Hardware Backlight pin       
    _bl = NULL;                 //Construct dummy pin     
  }  

  //Sanity check
  if (_ctrl & LCD_C_SPI3_16) {
     _init(_LCD_DL_8);  // Set Datalength to 8 bit for all native serial interfaces            
  }
  else {
    error("Error: LCD Controller type does not support native SPI3 16 bits interface\n\r");           
  }
}

TextLCD_SPI_N_3_16::~TextLCD_SPI_N_3_16() {
   if (_bl != NULL) {delete _bl;}  // BL pin
}

// Not used in this mode
void TextLCD_SPI_N_3_16::_setEnable(bool value) {
}    

// Set RS pin
// Used for mbed pins, I2C bus expander or SPI shiftregister
void TextLCD_SPI_N_3_16::_setRS(bool value) {
// The 16bit mode is split in 2 bytes. The first byte is for synchronisation and controlbits. The controlbits define the meaning of the next byte.
// The 8 actual bits represent either a data or a command byte.
//   b15 b14 b13 b12 b11 b10  b9  b8 - b7 b6 b5 b4 b3 b2 b1 b0 
//     1   1   1   1   1  RW  RS   0   d7 d6 d5 d4 d3 d2 d1 d0
//
//   RS=1 means that next byte is data, RS=0 means that next byte is command
//   RW=0 means that next byte is writen, RW=1 means that next byte is read (not used in this lib)
//

  if (value) {
    _controlbyte = 0xFA; // Next byte is data
  }
  else {
    _controlbyte = 0xF8; // Next byte is command
  }  
}    

// Set BL pin
void TextLCD_SPI_N_3_16::_setBL(bool value) {
    if (_bl) {
        _bl->write(value);   
    }    
}    

// Not used in this mode
void TextLCD_SPI_N_3_16::_setData(int value) {
}    
   
// Write a byte using SPI3 16 bits mode
void TextLCD_SPI_N_3_16::_writeByte(int value) {
    _cs = 0;
    wait_us(1);

    _spi->write(_controlbyte);

    _spi->write(value);     

    wait_us(1);
    _cs = 1;
}
#endif /* Native SPI bus     */  
//------- End TextLCD_SPI_N_3_16 ----------


//------- Start TextLCD_SPI_N_3_24 --------
#if(LCD_SPI_N_3_24 == 1) /* Native SPI bus     */

 /** Create a TextLCD interface using a controller with a native SPI3 24 bits interface
   *
   * @param spi             SPI Bus
   * @param cs              chip select pin (active low)
   * @param type            Sets the panel size/addressing mode (default = LCD16x2)
   * @param bl              Backlight control line (optional, default = NC)  
   * @param ctrl            LCD controller (default = SSD1803) 
   */       
TextLCD_SPI_N_3_24::TextLCD_SPI_N_3_24(SPI *spi, PinName cs, LCDType type, PinName bl, LCDCtrl ctrl) :
                                       TextLCD_Base(type, ctrl), 
                                       _spi(spi),        
                                       _cs(cs) {      
        
  // Init CS
  _cs = 1;

  // Setup the spi for 8 bit data, high steady state clock,
  // rising edge capture, with a 500KHz or 1MHz clock rate  
  _spi->format(8,3);
  _spi->frequency(1000000);    
  
  // The hardware Backlight pin is optional. Test and make sure whether it exists or not to prevent illegal access.
  if (bl != NC) {
    _bl = new DigitalOut(bl);   //Construct new pin 
    _bl->write(0);              //Deactivate    
  }
  else {
    // No Hardware Backlight pin       
    _bl = NULL;                 //Construct dummy pin     
  }  

  //Sanity check
  if (_ctrl & LCD_C_SPI3_24) {
    _init(_LCD_DL_8);  // Set Datalength to 8 bit for all native serial interfaces      
  }
  else {
    error("Error: LCD Controller type does not support native SPI3 24 bits interface\n\r");           
  }
}

TextLCD_SPI_N_3_24::~TextLCD_SPI_N_3_24() {
   if (_bl != NULL) {delete _bl;}  // BL pin
}

// Not used in this mode
void TextLCD_SPI_N_3_24::_setEnable(bool value) {
}    

// Set RS pin
// Used for mbed pins, I2C bus expander or SPI shiftregister
void TextLCD_SPI_N_3_24::_setRS(bool value) {
// The 24bit mode is split in 3 bytes. The first byte is for synchronisation and controlbits. The controlbits define the meaning of the next two bytes.
// Each byte encodes 4 actual bits. The 8 actual bits represent either a data or a command byte.
//   b23 b22 b21 b20 b19 b18 b17 b16 -  b15 b14 b13 b12 b11 b10 b9 b8 - b7 b6 b5 b4 b3 b2 b1 b0 
//     1   1   1   1   1  RW  RS   0     d0  d1  d2  d3   0   0  0  0   d4 d5 d6 d7  0  0  0  0
//
//   RS=1 means that next byte is data, RS=0 means that next byte is command
//   RW=0 means that next byte is writen, RW=1 means that next byte is read (not used in this lib)
//
// Note: SPI3_24 expects LSB first. This is inconsistent with regular SPI convention (and hardware) that sends MSB first.

  if (value) {
    _controlbyte = 0xFA; // Next byte is data
  }
  else {
    _controlbyte = 0xF8; // Next byte is command
  }   
}    

// Set BL pin
void TextLCD_SPI_N_3_24::_setBL(bool value) {
    if (_bl) {
        _bl->write(value);   
    }    
}    

// Not used in this mode
void TextLCD_SPI_N_3_24::_setData(int value) {
}    

//Mapping table to flip the bits around cause SPI3_24 expects LSB first.
const uint8_t map3_24[16] = {0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0};
    
// Write a byte using SPI3 24 bits mode
void TextLCD_SPI_N_3_24::_writeByte(int value) {
    _cs = 0;
    wait_us(1);
    _spi->write(_controlbyte);

    //Map and send the LSB nibble
    _spi->write( map3_24[value & 0x0F]);     

    //Map and send the MSB nibble
    _spi->write( map3_24[(value >> 4) & 0x0F]);     

    wait_us(1);
    _cs = 1;
}
#endif /* Native SPI bus     */  
//------- End TextLCD_SPI_N_3_24 ----------
