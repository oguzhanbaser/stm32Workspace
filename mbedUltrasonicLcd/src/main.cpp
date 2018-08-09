#include <mbed.h>
#include <string>
#include "TextLCD.h"

using namespace std;

I2C i2c_lcd(D14, D15);
TextLCD_I2C lcd(&i2c_lcd, 0x4E, TextLCD::LCD16x2);

//led pins on nucleo-f767zi
DigitalOut led1(PB_7);
DigitalOut led2(PB_0);
DigitalOut led3(PB_14);

InterruptIn echo(D5);           //echo pin defines as a interuupt in
DigitalOut trig(D6);            //trig pin defines as a digital out

Timer t1, t2;   

Serial pc(USBTX, USBRX, 115200);

uint8_t cnt = 0;
float u_distance = 0.0;
bool echoStat = false;
Thread u_thread, l_thread;

//lcd update thread
void lcd_thread()
{
    while(true)
    {
        lcd.cls();              //clear lcd
        lcd.setAddress(0, 0);
        lcd.printf("Oguzhan BASER");
        lcd.setAddress(0, 1);                           //set cursor to second row
        lcd.printf("Distance %3.2f", u_distance);           //print distance on second row
        led1 = !led1;
        Thread::wait(500);
    }
}

//ultrasonic sensor thread
void ultasonic_thread()
{

    t1.start();         //timer 1 starts counting
    while(true)
    {
        led2 = !led2;           //change led status to see thread working

        t1.reset();             //set timer 1 counter to zero
        trig = 1;
        while(t1.read_us() < 10);       //wait until timer 1 reach 10 us
        //wait_us(10);              // not used wait_us, because that function can break threads
        trig = 0;
        
        //this loop used for make timeout
        //wait three seconds until echo pin rises 
        //if passed time reach 3 seconds do not wait anymore
        t2.start();
        while(t2.read_ms() < 3000)      
        {
            if(echoStat == true)  break;  //if echo pin rises echoStat become true and breaks loop
        }

        //if echo stat true calculate passed time until echo pin falls
        if(echoStat)
        {
            t2.reset();
            while(echo.read() == true);
            t2.stop();
        }

        //if echo stat true calculate distance
        if(echoStat)
        {
            long duration = t2.read_us();
            u_distance = duration / 58.0;
            pc.printf("Value %ld %.2f\n\r", duration, u_distance);
            echoStat = false;
        }else{
            u_distance = 0;
        }

        t2.reset();
        Thread::wait(1000);
    }
}

void ultasonic_rise()
{
    echoStat = true;
}

int main() {

    // put your setup code here, to run once:
    trig = 0;           //set trig pin low on start
    
    led1 = led2 = led3 = 0;

    pc.printf("Initializing LCD...\n\r");

    //clear lcd make first settings
    lcd.cls();
    lcd.setBacklight(TextLCD_Base::LightOn);
    lcd.setCursor(TextLCD::CurOff_BlkOff);
    lcd.setAddress(0, 0);
    lcd.printf("Hello World");
    wait_ms(1500);
    
    //start threads
    u_thread.start(&ultasonic_thread);
    l_thread.start(&lcd_thread);
    echo.rise(&ultasonic_rise);     //interrupt callback function triggers on rising edge

    return 0;
}
