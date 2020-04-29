/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "platform/mbed_thread.h"
#include <string>

Serial pcSerial(USBTX, USBRX);       // pc serial port connection over usb
PwmOut led1(D9);                     // pwm1 pin
PwmOut led2(D10);                    // pwm2 pin
PwmOut led3(D11);                    // pwm3 pin
DigitalOut led4(LED3);               // internal LED pin

Timer t1;                           // timer for create async delay

AnalogIn pot1(A0);                  // pot1 pin 
AnalogIn pot2(A1);                  // pot2 pin

int main()
{
    // Initialise the digital pin LED1 as an output

    pcSerial.baud(115200);          // set uart baud rate 115200
    t1.start();                     // start timer

    while (true) {

        if(pcSerial.readable())     // check data received from uart
        {
            if(pcSerial.getc() == 0xA5)     // check first byte of receivded data
            {
                if(pcSerial.getc() == 0x5A)     // check second byte of received data
                {
                    // parse received data to Id and value

                    uint8_t mLen = pcSerial.getc();
                    uint8_t mId = pcSerial.getc();
                    uint8_t mVal = pcSerial.getc();
                    
                    // switch according to received ID
                    switch(mId){
                        case 0x80:
                            led1 = mVal / 100.0;
                            break;
                        case 0x81:
                            led2 = mVal / 100.0;
                            break;
                        case 0x82:
                            led3 = mVal / 100.0;
                            break;
                        case 0x84:
                            led4 = mVal;        // write received data to LED value
                            break;
                    } 
                }
            }
        }

        // send potantiometer values over uart to pc with 100ms time interval
        // sending data format is #|val1|val2\n
        if(t1.read_ms() > 100)
        {
            uint8_t pot1Val = pot1.read() * 100;
            uint8_t pot2Val = pot2.read() * 100;
            pcSerial.printf("#|%d|%d|\n", pot1Val, pot2Val);
            t1.reset();
        }

    }
}
