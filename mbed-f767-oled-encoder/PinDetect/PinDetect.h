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

#ifndef AJK_PIN_DETECT_H
#define AJK_PIN_DETECT_H

#ifndef MBED_H
#include "mbed.h"
#endif

#ifndef PINDETECT_PIN_ASSTERED
#define PINDETECT_PIN_ASSTERED   1
#endif

#ifndef PINDETECT_SAMPLE_PERIOD
#define PINDETECT_SAMPLE_PERIOD 20000
#endif

#ifndef PINDETECT_ASSERT_COUNT  
#define PINDETECT_ASSERT_COUNT  1
#endif

#ifndef PINDETECT_HOLD_COUNT
#define PINDETECT_HOLD_COUNT    50
#endif

namespace AjK {

/** PinDetect adds mechanical switch debouncing to DigitialIn and interrupt callbacks.
 *
 * This is done by sampling the specified pin at regular intervals and detecting any
 * change of state ( 0 -> 1 or 1 -> 0 ). When a state change is detected the attached
 * callback handler is called. Additionally, if the pin stays in the same state after
 * a state change for a defined period of time, an extra callback is made allowing a
 * program to detect when a "key is pressed and held down" rather than a momentary
 * key/switch press.
 *
 * All parameters are customisable which include:-
 *  <ul>
 *  <li> The sampling frequency. </li>
 *  <li> The number of continuous samples until a state change is detected. </li> 
 *  <li> The number of continuous samples until a key is assumed held after a state change. </li>
 *  <li> The logic level which is assumed to be asserted (0volts or +volts). </li>
 *  </ul>
 *
 * Only callbacks that have been attached will be called by the library.
 *
 * Example:
 * @code
 * #include "mbed.h"
 * #include "PinDetect.h"
 *
 * PinDetect  pin( p30 );
 * DigitialOut led1( LED1 );
 * DigitialOut led2( LED2 );
 * DigitialOut led3( LED3 );
 * DigitialOut led4( LED4 );
 *
 * void keyPressed( void ) {
 *     led2 = 1;
 *     led3 = 0;
 *     led4 = 0;
 * }
 *
 * void keyReleased( void ) {
 *     led2 = 0;
 *     led3 = 0;
 *     led4 = 0;
 * }
 *
 * void keyPressedHeld( void ) {
 *     led3 = 1;
 * }
 *
 * void keyReleasedHeld( void ) {
 *     led4 = 1;
 * }
 *
 * int main() {
 *
 *     pin.mode( PullDown );
 *     pin.attach_asserted( &keyPressed );
 *     pin.attach_deasserted( &keyReleased );
 *     pin.attach_asserted_held( &keyPressedHeld );
 *     pin.attach_deasserted_held( &keyReleasedHeld );
 *
 *     // Sampling does not begin until you set a frequency.
 *     // The default is 20ms. If you want a different frequency
 *     // then pass the period in microseconds for example, for 10ms :-
 *     //     pin.setSampleFrequency( 10000 );
 *     //
 *     pin.setSampleFrequency(); // Defaults to 20ms.
 *
 *     while( 1 ) {
 *         led1 = !led1;
 *         wait( 0.2 );
 *     }
 * }
 * @endcode
 *
 * This example will flash led1 in a similar to a standard starting program.
 *
 * Applying a "1" (switch on) to pin 30 will switch on led2, removing the "1" to "0"
 * (switch off) led2 goes out. Holding the "switch" at one for one second will switch
 * on led3. An unasserted P30 (switched off) will, after one second illuminate led4
 * when the deasserted calledback is called.
 *
 * The above is a very basic introduction. For more details:-
 * @see example.h
 */
class PinDetect {

protected:
    DigitalIn   *_in;
    Ticker      *_ticker;
    int         _prevState;
    int         _currentStateCounter;
    int         _sampleTime;
    int         _assertValue;
    int         _samplesTillAssertReload;
    int         _samplesTillAssert;
    int         _samplesTillHeldReload;
    int         _samplesTillHeld;
    FunctionPointer _callbackAsserted;
    FunctionPointer _callbackDeasserted;
    FunctionPointer _callbackAssertedHeld;
    FunctionPointer _callbackDeassertedHeld;
    
    /** initialise class
     *
     * @param PinName p is a valid pin that supports DigitalIn
     * @param PinMode m The mode the DigitalIn should use.
     */
    void init(PinName p, PinMode m) {
        _sampleTime              = PINDETECT_SAMPLE_PERIOD;
        _samplesTillAssert       = PINDETECT_ASSERT_COUNT;
        _samplesTillHeld         = 0;
        _samplesTillAssertReload = PINDETECT_ASSERT_COUNT;
        _samplesTillHeldReload   = PINDETECT_HOLD_COUNT;
        _assertValue             = PINDETECT_PIN_ASSTERED;
        
        _in = new DigitalIn( p );
        _in->mode( m );        
        _prevState = _in->read();        
        _ticker = new Ticker;
    }
    
public:

    friend class Ticker;
    
    PinDetect() { error("You must supply a PinName"); }

    /** PinDetect constructor
     *
     * By default the PinMode is set to PullDown.
     *
     * @see http://mbed.org/handbook/DigitalIn
     * @param p PinName is a valid pin that supports DigitalIn
     */    
    PinDetect(PinName p) {
        init( p, PullDown );
    }

    /** PinDetect constructor
     *
     * @see http://mbed.org/handbook/DigitalIn
     * @param PinName p is a valid pin that supports DigitalIn
     * @param PinMode m The mode the DigitalIn should use.
     */    
    PinDetect(PinName p, PinMode m) {
        init( p, m );
    }
    
    /** PinDetect destructor
     */    
    ~PinDetect() {
        if ( _ticker )  delete( _ticker );
        if ( _in )      delete( _in );
    }
    
    /** Set the sampling time in microseconds.
     *
     * @param int The time between pin samples in microseconds.
     */
    void setSampleFrequency(int i = PINDETECT_SAMPLE_PERIOD) { 
        _sampleTime = i; 
        _prevState  = _in->read();        
        _ticker->attach_us( this, &PinDetect::isr, _sampleTime );
    }
    
    /** Set the value used as assert.
     *
     * Defaults to 1 (ie if pin == 1 then pin asserted).
     *
     * @param int New assert value (1 or 0)
     */
    void setAssertValue (int i = PINDETECT_PIN_ASSTERED) { _assertValue = i & 1; }
    
    /** Set the number of continuous samples until assert assumed.
     *
     * Defaults to 1 (1 * sample frequency).
     *
     * @param int The number of continuous samples until assert assumed.
     */    
    void setSamplesTillAssert(int i) { _samplesTillAssertReload = i; }
    
    /** Set the number of continuous samples until held assumed.
     *
     * Defaults to 50 * sample frequency.
     *
     * @param int The number of continuous samples until held assumed.
     */    
    void setSamplesTillHeld(int i) { _samplesTillHeldReload = i; }
    
    /** Set the pin mode.
     *
     * @see http://mbed.org/projects/libraries/api/mbed/trunk/DigitalInOut#DigitalInOut.mode
     * @param PinMode m The mode to pass on to the DigitalIn
     */
    void mode(PinMode m) { _in->mode( m ); }
    
    /** Attach a callback function 
     *
     * @code
     *
     * DigitalOut led1( LED1 );
     * PinDetect pin( p30 );
     *
     * void myCallback( void ) {
     *   led1 = 1;
     * };
     * 
     * main() {
     *     pin.attach_asserted( &myCallback );
     * }
     *
     * @endcode
     *
     * Call this function when a pin is asserted.
     * @param function A C function pointer
     */
    void attach_asserted(void (*function)(void)) {
        _callbackAsserted.attach( function );
    }
    
    /** Attach a callback object/method 
     *
     * @code
     *
     * class Bar {
     *   public:
     *     void myCallback( void ) { led1 = 1; }
     * };
     *
     * DigitalOut led1( LED1 );
     * PinDetect pin( p30 );
     * Bar bar;
     *
     * main() {
     *     pin.attach_asserted( &bar, &Bar::myCallback );
     * }
     *
     * @endcode
     *
     * Call this function when a pin is asserted.
     * @param object An object that conatins the callback method.
     * @param method The method within the object to call.
     */
    template<typename T>
    void attach_asserted(T *object, void (T::*member)(void)) {
        _callbackAsserted.attach( object, member );        
    }
    
    /** Attach a callback function 
     *
     * @code
     *
     * DigitalOut led1( LED1 );
     * PinDetect pin( p30 );
     *
     * void myCallback( void ) {
     *   led1 = 0;
     * };
     *
     * main() {
     *     pin.attach_deasserted( &myCallback );
     * }
     *
     * @endcode
     *
     * Call this function when a pin is deasserted.
     * @param function A C function pointer
     */
    void attach_deasserted(void (*function)(void)) {
        _callbackDeasserted.attach( function );
    }
    
    /** Attach a callback object/method
     *
     * @code
     *
     * class Bar {
     *   public:
     *     void myCallback( void ) { led1 = 0; }
     * };
     *
     * DigitalOut led1( LED1 );
     * PinDetect pin( p30 );
     * Bar bar;
     * 
     * main() {
     *     pin.attach_deasserted( &bar, &Bar::myCallback );
     * }
     *
     * @endcode
     *
     * Call this function when a pin is deasserted.
     * @param object An object that conatins the callback method.
     * @param method The method within the object to call.
     */
    template<typename T>
    void attach_deasserted(T *object, void (T::*member)(void)) {
        _callbackDeasserted.attach( object, member );        
    }
    
    /** Attach a callback function 
     *
     * @code
     *
     * DigitalOut led2( LED2 );
     * PinDetect pin( p30 );
     *
     * void myCallback( void ) {
     *   led2 = 1;
     * };
     *
     * main() {
     *     pin.attach_asserted_held( &myCallback );
     * }
     *
     * @endcode
     *
     * Call this function when a pin is asserted and held.
     * @param function A C function pointer
     */
    void attach_asserted_held(void (*function)(void)) {
        _callbackAssertedHeld.attach( function );
    }
    
    /** Attach a callback object/method
     *
     * @code
     *
     * class Bar {
     *   public:
     *     void myCallback( void ) { led2 = 0; }
     * };
     *
     * DigitalOut led2( LED2 );
     * PinDetect pin( p30 );
     * Bar bar;
     * 
     * main() {
     *     pin.attach_asserted_held( &bar, &Bar::myCallback );
     * }
     *
     * @endcode
     *
     * Call this function when a pin is asserted and held.
     * @param object An object that conatins the callback method.
     * @param method The method within the object to call.
     */
    template<typename T>
    void attach_asserted_held(T *object, void (T::*member)(void)) {
        _callbackAssertedHeld.attach( object, member );        
    }
    
    /** Attach a callback function 
     *
     * @code
     *
     * DigitalOut led3( LED3 );
     * PinDetect pin( p30 );
     *
     * void myCallback( void ) {
     *   led3 = 1;
     * };
     *
     * main() {
     *     pin.attach_deasserted_held( &myCallback );
     * }
     *
     * @endcode
     *
     * Call this function when a pin is deasserted and held.
     * @param function A C function pointer
     */
    void attach_deasserted_held(void (*function)(void)) {
        _callbackDeassertedHeld.attach( function );
    }
    
    /** Attach a callback object/method
     *
     * @code
     *
     * class Bar {
     *   public:
     *     void myCallback( void ) { led3 = 0; }
     * };
     *
     * DigitalOut led3( LED3 );
     * PinDetect pin( p30 );
     * Bar bar;
     * 
     * main() {
     *     pin.attach_deasserted_held( &bar, &Bar::myCallback );
     * }
     *
     * @endcode
     *
     * Call this function when a pin is deasserted and held.
     * @param object An object that conatins the callback method.
     * @param method The method within the object to call.
     */
    template<typename T>
    void attach_deasserted_held(T *object, void (T::*member)(void)) {
        _callbackDeassertedHeld.attach( object, member );        
    }
    
    /** operator int()
     *
     * Read the value of the pin being sampled.
     */
    operator int() { return _in->read(); }

protected:    
    /** The Ticker periodic callback function
     */
    void isr(void) {
        int currentState = _in->read();
    
        if ( currentState != _prevState ) {
            if ( _samplesTillAssert == 0 ) {
                _prevState = currentState;
                _samplesTillHeld = _samplesTillHeldReload;
                if ( currentState == _assertValue ) 
                    _callbackAsserted.call();
                else                              
                    _callbackDeasserted.call();
            }
            else {
                _samplesTillAssert--;
            }
        }
        else {
            _samplesTillAssert = _samplesTillAssertReload;
        }
        
        if ( _samplesTillHeld ) {
            if ( _prevState == currentState ) {
                _samplesTillHeld--;
                if ( _samplesTillHeld == 0 ) {
                    if ( currentState == _assertValue ) 
                        _callbackAssertedHeld.call();
                    else                              
                        _callbackDeassertedHeld.call();
                }
            }
            else {
                _samplesTillHeld = 0;
            }
        }
    }
    
};

}; // namespace AjK ends.

using namespace AjK;

#endif
