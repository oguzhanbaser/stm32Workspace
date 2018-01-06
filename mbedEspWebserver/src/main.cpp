#include <mbed.h>
#include <string> 
#include <sstream>
//#include "ESP8266.h"

#define DEBUG 
#define BUFFER_SIZE 1024
#define OK "OK"

#define MY_WIFI_SSID "YOUR_SSID"
#define MY_WIFI_PASS "YOUR_PSWD"

//variables
char rxBuff[BUFFER_SIZE], oldBUffer[BUFFER_SIZE];//array to store data received from esp
unsigned int bufferWritingIndex=0;//write pointer
char serverIp[16];

//constructors
DigitalOut led1(D13);
AnalogOut led2(A2);

Serial pc(USBTX, USBRX);
Serial esp(D8,D2);//TX,RX TO BE CONNECTED TO ESP

//function prototype
void espRxInterrupt();
void debugPrintf(const char *msg);
bool isWifiConnected();
bool readEspResponse(const char *key);
void sendHomepage(char ch_id);
void hadleHttpRequests();
void flush_fifo(void);
const std::string intToString(int p_val);
void printIpAdress();


int main() {
    // put your setup code here, to run once:

    led1 = 0;

    pc.baud(115200);                            //bilgisayar ile haberleşme hıız
    esp.baud(115200);                           //esp ile haberleşme hızı
    
    esp.attach(&espRxInterrupt, Serial::RxIrq);

    debugPrintf("Application starting...");

    esp.printf("AT+RST\r\n");        //wait after this command because firmares respons can be diferent
    wait(3);        //wait for 3 seconds
    debugPrintf("RESET");    

    esp.printf("ATE0\r\n");         //disable ECHO
    while(readEspResponse("OK"));

    esp.printf("AT+CWMODE_CUR=1\r\n");  //only softAp mode
    while(readEspResponse("OK"));
    
    esp.printf("AT+CWLAP\r\n");     //find wifi networks
    while(readEspResponse("OK"));   

    esp.printf("AT+CWJAP_CUR=\"%s\",\"%s\"\r\n", MY_WIFI_SSID, MY_WIFI_PASS);
    while(readEspResponse("OK")); 

    esp.printf("AT+CIPMUX=1\r\n");      //enable multiple connections
    while(readEspResponse("OK"));

    esp.printf("AT+CIPSERVER=1,80\r\n");        //start server at port 80
    while(readEspResponse("OK"));
    
    esp.printf("AT+CIPSTO=10\r\n");     //Server timeout=10 seconds
    while(readEspResponse("OK"));

    printIpAdress();                //print Ip adress to serial terminal

    debugPrintf("Server READY");

    while(1) {
        // put your main code here, to run repeatedly:
        
        hadleHttpRequests();
        wait(0.1);
    }
}

//esp8266 rx interrupt funciton
void espRxInterrupt(){
    if(esp.readable())
    {
        rxBuff[bufferWritingIndex++]=esp.getc();            //save values to arr
        //pc.printf("%c", rxBuff[bufferWritingIndex - 1]);
        if(bufferWritingIndex>=BUFFER_SIZE)                 //if buffer fill overwrite array from start index
            bufferWritingIndex=0;
    }
}

void debugPrintf(const char *msg)
{
    pc.printf("%s\n\r", msg);
}

//returns false when found key
bool readEspResponse(const char *key)
{
    char *ret;

    ret = strstr(rxBuff, key);

    //pc.printf("rxbuff: %s %s\n",key, rxBuff);

    if(ret)
    {
        pc.printf("%s\n", rxBuff);
        memcpy(oldBUffer, rxBuff, BUFFER_SIZE);
        memset(&rxBuff[0], 0, BUFFER_SIZE);
        bufferWritingIndex = 0;
        return false;
    }else{
        return true;
    }
}

unsigned int bufferReadingIndex_IpdSearch = 0;

void hadleHttpRequests()
{
    char ipdBuff[256];

    //get ipd string from last buffer
    while(bufferReadingIndex_IpdSearch != bufferWritingIndex)
    {
        if(rxBuff[bufferReadingIndex_IpdSearch] == '+')
        {
            for(int i = 0; i < 256; i++)
            {
                if(rxBuff[bufferReadingIndex_IpdSearch + i] == 0x0D)
                    break;
                ipdBuff[i] = rxBuff[bufferReadingIndex_IpdSearch + i];
            }
            break;
        }
        bufferReadingIndex_IpdSearch++;
    }
    
    if(strncmp(ipdBuff, "+IPD,", 5) == 0)       //if +IPD, is in start of the array
    {
        pc.printf("buff: %s\n", ipdBuff);

        char requestType[20];
        char request[50];
        char linkId;
        int ipdLen;

        //get connection ID, len, request type and request from buffer array
        sscanf(ipdBuff + 5, "%c,%d:%s %s", &linkId, &ipdLen, requestType, request);

        //after parsing value clear buffer
        flush_fifo();

        //check pwm request
        if(strncmp(request, "/?pwmVal", 8) == 0)
        {
            int pwmVal = 0;
            char *ptr;

            //get integer value from request
            ptr = std::strtok(request, "=");
            ptr = std::strtok(NULL, "=");

            pwmVal = atoi(ptr);                 //convert string value to integer

            led2.write((double)pwmVal / 255.0);         //write led pin to pwm value
            
            pc.printf("\nPWM val: %d %f\n", pwmVal, ((double)pwmVal / 255.0));
            
            sendHomepage(linkId);           //refresh homepage
        
        }else if(strcmp(request, "/?led") == 0) //check led request
        {
            led1 = !led1;               //toggle led status
            sendHomepage(linkId);       //refresh homepage
        }else{                          //else send homepage again
            sendHomepage(linkId);
        }

    }
}

void sendHomepage(char ch_id)           //homepage
{
    std::string Header;        //http header

    Header =  "HTTP/1.1 200 OK\r\n";
    Header += "Content-Type: text/html\r\n";
    Header += "Connection: close\r\n";

    std::string Content;     //Web page content
    Content = "<center><h1>Hello World!</h1></center><hr><center><h2>Oguzhan Baser</h2></center><hr>Led Durumu: ";
    //Content += intToString(led1.read());
    Content += (led1.read() == 1) ? "Yandi" : "Sondu";
    Content += "<br><a href=\"/?led\"><input type=\"button\" value=\"Led Durum\"></a>";
    Content += "<br><input type=\"text\" name=\"pwmVal\" id=\"pwmVal\">";
    Content += "<input type=\"submit\" name=\"btnPWM\" value=\"PWM Ayarla\" onclick=\"location.href='?pwmVal='+document.getElementById('pwmVal').value;\" id=\"btnPWM\" class=\"btnPWM\"/>";

    Header += "Content-Length: ";
    Header += (int)(Content.length());
    Header += "\r\n\r\n";

    esp.printf("AT+CIPSEND=%c,%d\r\n", ch_id, Header.length() + Content.length());      //send web page length
    wait(0.5);
    int cc = 0;
    
    //while(readEspResponse(">")) wait_ms(10);
    while(rxBuff[cc++] != '>') wait_ms(10);     // wait until '>' character

    //send web page header and content
    for(int i = 0; i < Header.length(); i++)
    {
        esp.putc(Header.at(i));
    }
    for(int i = 0; i < Content.length(); i++)
    {
        esp.putc(Content.at(i));
    }

    while(readEspResponse("OK"));       //wait until OK response

}


//not used
bool isWifiConnected()
{
    bool retVal = 0;

    esp.printf("AT+CIFSR\r\n");

    std::string ss(rxBuff);

    int val1 = ss.find(":STAIP,");
    int val2 = ss.find("\"", val1 + 1);

    //pc.printf("vals: %s %d %d\n", ss, val1, val2);

    return retVal;
}

//clear rx buffer
void flush_fifo(void) 
{
    while (esp.readable()) {
        (void)esp.getc();
    }
    
    memset(&rxBuff[0], 0, BUFFER_SIZE);
    bufferWritingIndex = 0;
    bufferReadingIndex_IpdSearch = 0;
}

const std::string intToString(int p_val)
{
    std::stringstream ssc;
    ssc << p_val;
    return ssc.str();
}

void printIpAdress()
{
    esp.printf("AT+CIFSR\r\n");
    while(readEspResponse("OK"));

    bool validIp = false;
    while(!validIp)
    {
        if(strstr(oldBUffer, "0.0.0.0") == NULL)
        {
            validIp = true;
        }
    }
    int index1, index2;

    for(int i = 0; i < BUFFER_SIZE; i++)
    {
        if(oldBUffer[i] == ',')
        {
            index1 = i;
        }

        if(oldBUffer[i] == '\r')
        {
            index2 = i;
            break;
        }
    }

    char IpArr[20];
    int cnt = 0;
    for(int i = index1 + 1; i < index2 ; i++)
    {
        IpArr[cnt++] = oldBUffer[i];
    }

    IpArr[cnt] = '\0';

    pc.printf("Ip Adress: %s\r\n", IpArr);
}