/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "platform/mbed_thread.h"

#include "UipEthernet.h"
#include "TcpServer.h"
#include "TcpClient.h"

// IP Settings
#define IP      "192.168.137.120"
#define GATEWAY "192.168.137.1"
#define NETMASK "255.255.255.0"
#define PORT    61

const uint8_t   MAC[6] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 };
UipEthernet     net(MAC, D11, D12, D13, D10);   // mac, mosi, miso, sck, cs
TcpServer       server;                         // Ethernet server
TcpClient*      client;
uint8_t         recvData[1024];
// const char      sendData[] = "Data received OK";
const char      sendData[] = {0xA5, 0x5A, 0x40, 'O', 'K'};
char            adcArr[] = {0xA5, 0x5A, 0x30, 0x00, 0x00};

// Blinking rate in milliseconds
#define BLINKING_RATE_MS                                                    500

DigitalOut led1(LED1);
DigitalOut led2(LED2);
// DigitalOut led3(LED3);
PwmOut led3(LED3);
AnalogIn pot1(A1), pot2(A2);
Serial pc(USBTX, USBRX);

Timer t1;

int main()
{
    
    pc.baud(115200);
    pc.printf("Trying to connect!\r\n");
    net.set_network(IP, NETMASK, GATEWAY);
    net.connect();

    const char*     ip = net.get_ip_address();
    const char*     netmask = net.get_netmask();
    const char*     gateway = net.get_gateway();

    pc.printf("Connection succesfull\r\n");
    pc.printf("IP address: %s\r\n", ip ? ip : "None");
    pc.printf("Netmask: %s\r\n", netmask ? netmask : "None");
    pc.printf("Gateway: %s\r\n\r\n", gateway ? gateway : "None");

    server.open(&net);

    server.bind(PORT);

    server.listen(4);                   // max client count 
    pc.printf("Start listening!\r\n");

    t1.start();

    while (true) {
        client = server.accept();               // accept client if exist

        uint8_t adcVal = (uint8_t)(pot1.read() * 255);
        uint8_t adcVal2 = (uint8_t)(pot2.read() * 255);
        adcArr[3] = adcVal;
        adcArr[4] = adcVal2;
 
        if (client) {                   // check client is exist
            size_t  recvLen;
 
            // show client' s info
            pc.printf("\r\n----------------------------------\r\n");
            pc.printf("Client with IP address %s connected.\n\r", client->getpeername());

            // read incoming data from client and toggle leds
            if ((recvLen = client->available()) > 0) 
            {
                pc.printf("%d bytes received:\r\n", recvLen);
                client->recv(recvData, recvLen);            // read incoming data from socket
                for (int i = 0; i < recvLen; i++)
                    pc.printf(" 0x%.2X", recvData[i]);

                // check incoming data length and first two bytes
                if(recvLen > 2 && (recvData[0] == 0xA5 && recvData[1] == 0x5A))
                {
                    switch(recvData[2]){
                        case 0x01:
                            led1 = int(recvData[3]);
                            break;
                        case 0x02:
                            led2 = int(recvData[3]);
                            break;
                        case 0x10:
                            client->send((uint8_t*)adcArr, 5);      // send adc data if client wants it
                            break;
                        case 0x30:
                            led3 = (float)(int(recvData[3]) / 100.0);       //read pwm value and write LED
                            break;

                    }
                }

                pc.printf("\r\n");

                // send ok data
                client->send((uint8_t*)sendData, strlen(sendData));
            }
 
            pc.printf("Client with IP address %s disconnected.\r\n", client->getpeername());
            client->close();
        }

        if(t1.read_ms() > 1000)
        {
            //led3 = !led3;
            t1.reset();
        }

    }
}
