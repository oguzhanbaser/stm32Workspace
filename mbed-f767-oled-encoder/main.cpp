/* mbed Microcontroller Library
 * Copyright (c) 2018 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include <mbed.h>
#include "mRotaryEncoder.h"
#include "Adafruit_SSD1306.h"
#include "DHT.h"

// Used pins

// OLED Pins
#define I2C_SCL PB_8
#define I2C_SDA PB_9

// Led ve pot pins
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
AnalogIn pot(A0);

Timer t1;     // timer

DHT sensor(PF_14, DHT11);   // Used class 

I2C i2c(I2C_SDA, I2C_SCL);
Adafruit_SSD1306_I2c display(i2c, NC);
mRotaryEncoder encoder(PF_13, PE_9, PE_11);
int pulseCount, lastPulseCount;
int temp, hum;
int screenCount = 0;

// This function will run every encoder turns
void encoderCallback()
{
    pulseCount = encoder.Get();                 // get counter value
    if(pulseCount > lastPulseCount + 3)         // check is counter value increased
    {
        led1 = !led1;                           // toggle led1
        screenCount++;                          // increase screen counter 
        if(screenCount > 2) screenCount = 2;
        lastPulseCount = pulseCount;            // save encoder counter value 
    }else if(pulseCount < lastPulseCount - 3){  // check is counter value decreased
        led3 = !led3;                           // toggle led3
        screenCount--;                          // decrease screen counter
        if(screenCount < 0) screenCount = 0;
        lastPulseCount = pulseCount;            // save encoder counter value
    }
}

// main() runs in its own thread in the OS
int main()
{
    pc.baud(9600);
    encoder.attachROT(&encoderCallback);
    display.clearDisplay();

    display.setTextSize(2);

    t1.start();
    while (true) {
        // Blink LED and wait 0.5 seconds
        led2 = !led2;
        display.setTextCursor(5,10);

        if(t1.read_ms () > 1000)        // check is 1 secod passed
        {
          sensor.readData();            // read DHT sensor value
          temp = sensor.ReadTemperature(CELCIUS);     
          hum = sensor.ReadHumidity();
          t1.reset();                   // reset timer value
        }

        display.clearDisplay();           // clear oled display
        switch (screenCount)              // switch screen using counter value
        {
        case 0:     
          display.printf("ADC:  %4d", (int)(pot.read() * 1023));      // print analog value
          break;
        case 1:
          display.printf("Temp: %4d", (int)(temp));           // print temperature value
          break;
        case 2:
          display.printf("Hum:  %4d", (int)(hum));            // print humidity value  
          break;
        }
        display.display();                  // show screen

        wait_ms(30);
    }
}
