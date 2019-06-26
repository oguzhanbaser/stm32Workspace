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
*/

#ifdef PINDETECT_EXAMPLE_COMPILE

#include "mbed.h"
#include "PinDetect.h"

PinDetect  pin ( p21  );
DigitalOut led1( LED1 );
DigitalOut led2( LED2 );
DigitalOut led3( LED3 );
DigitalOut led4( LED4 );

/*
 * Note, the PinDetect can be defined thus:-
 *     PinDetect pin( p21, PullDown );
 * This allows you to specify the DigitalIn pinmode
 * when you create the PinDetect object. This means
 * using pin.mode() later is then no longer required.
 */

// C function callbacks follow.

void keyPressed( void ) {
    led2 = 1;
    led3 = 0;
    led4 = 0;
}

void keyReleased( void ) {
    led2 = 0;
    led3 = 0;
    led4 = 0;
}

void keyPressedHeld( void ) {
    led3 = 1;
}

void keyReleasedHeld( void ) {
    led4 = 1;
}

// The main program.

int main() {

    pin.mode( PullDown );
    pin.attach_asserted( &keyPressed );
    pin.attach_deasserted( &keyReleased );
    pin.attach_asserted_held( &keyPressedHeld );
    
    // This callback will often be of little use as it's
    // called after every assertion/deassertion. However,
    // it's provided for completeness. You may find a use
    // for it. If not, just don't attach a callback and it
    // will not activate.
    pin.attach_deasserted_held( &keyReleasedHeld );
    
    // You can define how many continuous samples must be
    // asserted before the attach_asserted() function is called.
    //     pin.setSamplesTillAssert( 10 );
    // This would mean 10 * 20ms debounce time = 200ms.

    // You can define how many continuous samples must be
    // asserted before the attach_asserted_held() function is called.
    //     pin.setSamplesTillHeld( 200 );
    // This would mean 200 * 20ms debounce time = 2seconds.

    // By default, "asserted" assumes the pin going high from 0volts to 5volts
    // and deasserted assumes going from 5volts to 0volts. You can invert this
    // logic so that going to 0volts is asserted and going to 5volts is deasserted
    // using this setup function:-
    //     pin.setAssertValue( 0 );

    // Sampling does NOT begin until you set the frequency. So, until
    // you call this function NO callbacks will be made. With no arguments
    // passed the default is 20000 microseconds (20ms). Specifiy the sampling
    // period in microseconds if you want a different value to 20ms.
    // For example, for a sampling period of 10ms do:-
    //     pin.setSampleFrequency( 10000 );
    // Note, if you change the sampling frequency you will probably also
    // want to change the number of samples till assert and held as show
    // above.
    pin.setSampleFrequency(); // Defaults to 20ms.

    while( 1 ) {
        led1 = !led1;
        wait( 0.2 );
    }
}

#endif
