#include <mbed.h>

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(D7);
PwmOut ledPwm(D6);            //led for change brigtness with slider

Serial pc(USBTX, USBRX);      //debug values from pc
Serial nex(D1, D0);           //nextion communicate pins
AnalogIn aIn1(A0), aIn2(A1);  //analog input pins
Ticker sendValTicker;         //ticker need for update values on Nextion screen
Timer t1;                     //timer need for measure pass time in serial read interrupt

int analogVal1 = 0, analogVal2 = 0;
int pageID = 0, widgetID = -1, readVal = -1;
bool isParsed = false, isUARTBusy = false;
bool getCheckBox = false, getSlider = false;
bool checkBoxVal = false;
int sliderVal = 0;
int adcVal1 = 0;



void sendValCallback()
{
  //if uart receive callback is using uart to read data return to main function
  if(isUARTBusy) return;

  //check which nextion page is showing now
  switch(pageID)
  {
    //if page0 showiing send t0 and t1 update value
    case 0:
      nex.printf("t0.txt=\"%d\"%c%c%c", analogVal1, 0xFF, 0xFF, 0xFF);
      nex.printf("t1.txt=\"%d\"%c%c%c", analogVal1, 0xFF, 0xFF, 0xFF);
      break;
    case 1:
    //if page 1 showing sed t0 and j0 update value
      nex.printf("t0.txt=\"%d\"%c%c%c", analogVal1, 0xFF, 0xFF, 0xFF);
      nex.printf("j0.val=%d%c%c%c", analogVal1, 0xFF, 0xFF, 0xFF);
      break;
  }
}

void nexReceiveCallback()
{
  //set uart busy flag
  isUARTBusy = true;
  //set one of the leds to see receive interrupt is working
  led2 = 1;
  
  //set some variables to keep read values
  int buffIndex = 0;
  char nexBuffer[8];
  int EOCcnt = 0;

  //reset timer to see measure pass time 
  t1.reset();
  //we will read max 8 bytes from nextion
  //if data length equal 7, data is touch event
  //if data length equal 8, data is get event response 
  while(buffIndex != 8)
  {
    if(nex.readable())
    {
      nexBuffer[buffIndex] = nex.getc();
      if(nexBuffer[buffIndex] == 0xFF) EOCcnt++;
      buffIndex++;
    }

    //if 0xFF comes 3 times break
    if(EOCcnt == 3)  break;

    //if wait time exceeds 50ms break
    if(t1.read_ms() > 50) break;
  }

  //check first byte and last 3 bytes 
  //if first byte equal to 65 and data length equal 7
  if(nexBuffer[0] == 0x65 && nexBuffer[4] == 0xFF && 
        nexBuffer[5] == 0xFF && nexBuffer[6] == 0xFF)
  {
    isParsed = true;

    //second byte carries widget ID number
    widgetID = nexBuffer[2];
    //third byte carries touch or touch release event
    readVal = nexBuffer[3];

    //debug values from pc serial terminal
    pc.printf("%d %d %d\n\r", pageID, widgetID, readVal);

    //check page change button pushed
    if(widgetID == 6 && readVal == 1 && pageID == 0)
    {
      pageID = 1;
    }else if(widgetID == 2 && readVal == 1 && pageID == 1)
    {
      pageID = 0;
    }
  }//if first byte equal 0x71 and data length equal 8
  else if(nexBuffer[0] == 0x71 && 
      nexBuffer[5] == 0xFF && nexBuffer[6] == 0xFF && nexBuffer[7] == 0xFF)
  {
    //if checkbox pressed
    if(getCheckBox)
    {
      //second byte carries widget value
      sliderVal = nexBuffer[1];
      ledPwm = (float)(sliderVal / 100.0);
      pc.printf("slider %d\n\r", sliderVal);
      getCheckBox = false;      //set checkbox flag false
    } //if slider pressed
    else if(getSlider)
    {
      //second byte carries widget value
      checkBoxVal = nexBuffer[1];
      pc.printf("checkbox %d\n\r", checkBoxVal);
      getSlider = false;      //set slider flag false
    }
  }
  
  //reset uart flag
  isUARTBusy = false;
  //reset led value on interrupt exit
  led2 = 0;
}

int main() {

  // put your setup code here, to run once:

  pc.baud(115200);      //pc uart comm baud rate
  nex.baud(9600);       //nextion uart comm baud rate
  t1.start();           //start timer
  pageID = 0;           //set first page id 
  
  //callback for nextion uart receive interrupt
  //when data comes from uart this callback function will call
  nex.attach(&nexReceiveCallback, Serial::RxIrq); 

  //set ticker callback update time to 0.5 ms
  sendValTicker.attach(&sendValCallback, 0.5);

  while(1) {
    // put your main code here, to run repeatedly:
    analogVal1 = aIn1.read() * 100;
    analogVal2 = aIn2.read() * 100;

    led4 = checkBoxVal;
    
    if(isParsed)
    {
      //button1
      if(pageID == 0 && widgetID == 2 && readVal == 0)
      {
        led1 = !led1;
      } //butonn2
      else if(pageID == 0 && widgetID == 3 && readVal == 0)
      {
        led3 = !led3;
      } //slider
      else if(pageID == 1 && widgetID == 7 && readVal == 0)
      {
        while(isUARTBusy);
        nex.printf("get h0.val%c%c%c", 0xFF, 0xFF, 0xFF);
        getCheckBox = true;
      } //checkbox
      else if(pageID == 1 && widgetID == 4 && readVal == 1)
      {
        while(isUARTBusy);
        nex.printf("get bt0.val%c%c%c", 0xFF, 0xFF, 0xFF);
        getSlider = true;
      }



      isParsed = false;
    }

  }
}