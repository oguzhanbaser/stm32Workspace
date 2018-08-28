/*******************************************************************************
 * Copyright (c) 2014, 2015 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Ian Craggs - make sure QoS2 processing works, and add device headers
 *******************************************************************************/

 /**
  This is a sample program to illustrate the use of the MQTT Client library
  on the mbed platform.  The Client class requires two classes which mediate
  access to system interfaces for networking and timing.  As long as these two
  classes provide the required public programming interfaces, it does not matter
  what facilities they use underneath. In this program, they use the mbed
  system libraries.

 */


#include "string.h"
#include "mbed.h"
#include "easy-connect.h"
#include "MQTTmbed.h"
#include "MQTTNetwork.h"
#include "MQTTClient.h"
#include "picojson.h"

//to comminucate with pc
Serial pc(USBTX, USBRX);

//normal leds to use on - off funciton
DigitalOut led1(LED1);
DigitalOut led2(LED2);

//RGB led pins
PwmOut redPin(PB_10);
PwmOut greenPin(PE_12);
PwmOut bluePin(PE_14);

//analog input pin
AnalogIn tempPin(A0);

//MQTT callback for rgb led messages
void rgbLedMessage(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    picojson::value v;
		char *msg = (char*)message.payload;				//get message as a char array
	
		string err = picojson::parse(v, msg, msg + message.payloadlen);			//parse JSON from char array
		
		//get red, green, blue value from JSON with key name
    pc.printf("redVal =%f\r\n" ,  v.get("red").get<double>());
    pc.printf("greenVal =%f\r\n" ,  v.get("green").get<double>());
    pc.printf("blueVal =%f\r\n" ,  v.get("blue").get<double>());
	
		//subtract all values from max value to use with common anode rgb led
		redPin = 1.0 - (v.get("red").get<double>() / 255.0);
		greenPin = 1.0 - (v.get("green").get<double>() / 255.0);
		bluePin = 1.0 - (v.get("blue").get<double>() / 255.0);
}

void led1Message(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    picojson::value v;
		char *msg = (char*)message.payload;
	
		string err = picojson::parse(v, msg, msg + message.payloadlen);
	
		float val = v.get("led1").get<double>();
	
		led1 = val == 1.0 ? 1 : 0;
}

void led2Message(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    picojson::value v;
		char *msg = (char*)message.payload;
	
		string err = picojson::parse(v, msg, msg + message.payloadlen);

		float val = v.get("led2").get<double>();
	
		led2 = val == 1.0 ? 1 : 0;
}


int main(int argc, char* argv[])
{
    float version = 0.6;
		pc.baud(9600);
		redPin = 1.0;
		greenPin = 1.0;
		bluePin = 1.0;

    pc.printf("HelloMQTT: version is %.2f\r\n", version);

		//create network interface with debug on parameter
    NetworkInterface* network = easy_connect(true);
    if (!network) {
        return -1;
    }

    MQTTNetwork mqttNetwork(network);

    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

    const char* hostname = "broker.shiftr.io";								//mqtt broker name
    int port = 1883;																					//mqtt port number
		
    pc.printf("Connecting to %s:%d\r\n", hostname, port);
    
		int rc = mqttNetwork.connect(hostname, port);
    if (rc != 0)																							//if cannot connected return number will not be zero
        pc.printf("rc from TCP connect is %d\r\n", rc);				//print returned error code

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = "mbed";									//our mqtt client ID
    data.username.cstring = "my-key-2";							//our shiftr.io connection ID
    data.password.cstring = "5db645c0e24f8b38";			//our shiftr.io password
		
    if ((rc = client.connect(data)) != 0)
        pc.printf("rc from MQTT connect is %d\r\n", rc);

		//subscribe rgb led messages
    if ((rc = client.subscribe("rgbLed", MQTT::QOS2, rgbLedMessage)) != 0)
        pc.printf("rc from MQTT subscribe is %d\r\n", rc);

		//subscribe led1 messages
		if ((rc = client.subscribe("led1", MQTT::QOS2, led1Message)) != 0)
        pc.printf("rc from MQTT subscribe is %d\r\n", rc);
		
		//subscribe led2 messages
		if ((rc = client.subscribe("led2", MQTT::QOS2, led2Message)) != 0)
        pc.printf("rc from MQTT subscribe is %d\r\n", rc);
		
		//define MQTT Message
    MQTT::Message message;

    // QoS 0 
    char buf[100];
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;

		//json parser 
    picojson::object  adcJson;
		int cnt = 0;
		
		while(true)
		{
			if(cnt == 100)												//if 100 steps passed send message
			{
				double adcVal = tempPin.read() * 40;											//read adc value and scale it between 0 - 40
				adcJson["temp"] = picojson::value(adcVal);                //save value with "temp" key to JSON
				string msgStr = picojson::value(adcJson).serialize();			//convert JSON value to string
				
				sprintf(buf, "%s", msgStr.c_str());                       //convert string value to char array
				message.payload = (void*)buf;															//add converted value to MQTT message
				message.payloadlen = strlen(buf)+1;												//set MQTT message length
				if((rc = client.publish("adcVal", message)) != 0)					//send message to "adcVal" topic
				{
					pc.printf("rc from MQTT subscribe is %d\r\n", rc);			
				}
				
				cnt = 0;
			}

			cnt++;
			Thread::wait(10);															//wait 10 ms between steps
		}
		
		/*if ((rc = client.unsubscribe(topic)) != 0)
		pc.printf("rc from unsubscribe was %d\r\n", rc);

    if ((rc = client.disconnect()) != 0)
        pc.printf("rc from disconnect was %d\r\n", rc);

    mqttNetwork.disconnect();*/

    return 0;
}
