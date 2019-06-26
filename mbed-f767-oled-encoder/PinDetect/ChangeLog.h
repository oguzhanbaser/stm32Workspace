/*
    Copyright (c) 2010 Andy Kirkham
 
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

    1.5     13 Jan 2011 - Made the isr() protected and made the class a friend of Ticker.
                          Read the previous state and store when the sample frequency is
                          set so that initial assert_deassert_held() isn't called at startup.
                              
    1.4     13 Jan 2011 - Added an extra overloaded constructor that allows
                          the DigitalIn PinMode to be passed when the the class
                          is declared. So mode() isn't really needed but is left
                          in for completeness.
    
    1.3     13 Jan 2011 - Updated some doxygen comments.

    1.2     13 Jan 2011 - Added ChangeLog.h file.

    1.1     13 Jan 2011 - Fixed some typos.

    1.0     13 Jan 2011 - Initial release.

*/
