#include <mbed.h>
#include <Dht11.h>
#include <TextLCD.h>

DigitalOut ledG(D3), ledR(D4);              //durum ledleri
Dht11 sensor(D8);                           //dht 
I2C i2c_lcd(D14, D15);      
TextLCD_I2C lcd(&i2c_lcd,  0x4E, TextLCD::LCD16x2);     //i2c lcd
AnalogIn pot(A0);                           //potansiyometre
InterruptIn btn(USER_BUTTON);

//süre sayaçları
Timer timDHT, timLCD;

//sıcaklık değişkenleri
int dhtTemp = 0, setTemp = 0;

//durum değişkenleri
bool termStat = false, lightStat = false;

//ekran ışığı değişim buton interrupt callback i
void changeBackLight()
{
    if(lightStat)
    {
        lcd.setBacklight(TextLCD_Base::LightOn);
    }else{
        lcd.setBacklight(TextLCD_Base::LightOff);
    }
    lightStat = !lightStat;
}

//ana fonksiyon
int main() {

    //başta lcd ışığını yak ve cursor u kapat
    lcd.setBacklight(TextLCD_Base::LightOn);
    lcd.setCursor(TextLCD::CurOff_BlkOff);

    //lcd i temizle ve başlangıç yazılarını yaz
    lcd.cls();
    lcd.setAddress(0, 0);
    lcd.printf("...Termostat...");
    lcd.setAddress(1, 1);
    lcd.printf("Oguzhan Baser");

    //timer ları başlat
    timDHT.start();
    timLCD.start();

    //buton interruptuna callback i bağla
    btn.fall(changeBackLight);

    wait(2);
    lcd.cls();


    while(1) {

        //pot değerini okuyup değişkene yaz
        setTemp = pot.read() * 50;

        //100ms de bir lcd değerlerini güncelle
        if(timLCD.read_ms() > 100)
        {
            lcd.setAddress(0, 0);
            lcd.printf("Sicaklik: %d", dhtTemp);
            lcd.setAddress(0, 1);
            lcd.printf("Ayar: %d", setTemp);
            timLCD.reset();
            if(termStat)
            {
                lcd.setAddress(10, 1);
                lcd.printf("Acik");
            }else{
                lcd.setAddress(10, 1);
                lcd.printf("Kapali");                
            }
        }

        //2sn de bir dht' yi oku ve lcd yi temizle
        if(timDHT.read_ms() > 2000)
        {
            lcd.cls();
            sensor.read();
            dhtTemp = sensor.getCelsius();
            timDHT.reset();
        }

        //ledlerin durumunu sıcaklığa göre değiştir
        if(dhtTemp > setTemp + 2)
        {
            ledR = 0;
            ledG = 1;
            if(termStat)
            {
                lcd.cls();
                lcd.setAddress(0, 0);
                lcd.printf("Sicaklik: %d", dhtTemp);
                lcd.setAddress(0, 1);
                lcd.printf("Ayar: %d", setTemp);
                lcd.setAddress(10, 1);
                lcd.printf("Kapali");
            }
            termStat = false;
        }else if(dhtTemp < setTemp - 2){
            ledG = 0;
            ledR = 1;
            if(!termStat)
            {
                lcd.cls();
                lcd.setAddress(0, 0);
                lcd.printf("Sicaklik: %d", dhtTemp);
                lcd.setAddress(0, 1);
                lcd.printf("Ayar: %d", setTemp);
                lcd.setAddress(10, 1);
                lcd.printf("Acik");
            }
            termStat = true;
        }
    }
}