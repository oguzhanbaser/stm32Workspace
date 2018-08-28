import mqtt.*;
import controlP5.*;

ControlP5 cp5;
MQTTClient client;
Chart myChart;
Knob myKnobA;

String username = "my-key-2";
String password = "5db645c0e24f8b38";

Textlabel myTextlabelA;
Textlabel myTextlabelB;

void setup() {
  size(400,600);
  noStroke();

  //MQTT broker connetctşon
  client = new MQTTClient(this);
  client.connect("mqtt://" + username + ":" + password + "@broker.shiftr.io", "processing");    
  client.subscribe("/adcVal");

  cp5 = new ControlP5(this);

  //Add button to Gui
  cp5.addButton("SEND")     //Button name
      .setValue(0)            //Button first value
      .setPosition(275, 150)    //Button Position in gui (X, Y)
      .setSize(75, 200);        //Button Size (X, Y)

  //Add color picker to gui
  cp5.addColorWheel("c")          //color picker name
      .setRGB(color(128,0,255))   //color picker first RGB value
      .setSize(200, 200)          //color picker size
      .setPosition(50, 150);      //color picker position

  //add chart to gui
  myChart = cp5.addChart("dataflow")      //chart name
               .setPosition(50, 400)      //chart position
               .setSize(300, 150)         //chart size
               .setRange(0, 40)           //chart min and max value
               .setView(Chart.LINE)       //chart type, you can use Chart.LINE, Chart.PIE, Chart.AREA, Chart.BAR_CENTERED
               .setStrokeWeight(1.5)      //chart line thickness
               .setColorCaptionLabel(color(40))     //chart line color
               ;

  myChart.addDataSet("tempVal");                  //chart dataset name
  myChart.setData("tempVal", new float[20]);      //set data length

  //add switch to gui
  cp5.addToggle("SW1")            //switch name
     .setPosition(50,40)          //switch position in gui
     .setSize(100,40)             //switch size
     .setValue(false)             //switch first value
     .setMode(ControlP5.SWITCH)   
     ;

  //add switch to gui
  cp5.addToggle("SW2")            //switch name
     .setPosition(250,40)         //switch position in gui
     .setSize(100,40)             //switch size
     .setValue(false)             //switch first value
     .setMode(ControlP5.SWITCH)
     ;

  background(50);               //background color
}

void draw() 
{
  
}

public void controlEvent(ControlEvent theEvent) {
  // println(theEvent.getController().getName());       //if you uncomment this line you can see all events
}

//color valur send button callback
public void SEND(int theValue) {
  //println("a button event from colorA: "+ theValue);
  JSONObject json;                                            
  json = new JSONObject();                                    //create JSON object
  json.setInt("red", cp5.get(ColorWheel.class,"c").r());      //set red value
  json.setInt("green", cp5.get(ColorWheel.class,"c").g());    //set green value
  json.setInt("blue", cp5.get(ColorWheel.class,"c").b());     //set blue value
  //println(json.toString());
  client.publish("/rgbLed", json.toString());                 //send json data as a string
}

//switch1 change callback
void SW1(boolean theFlag) {
  JSONObject json;
  json = new JSONObject();                         //create JSON object
  if(theFlag==true) {                              //if switch ON set led1 value = 1
    json.setInt("led1", 1);
  } else {                                         //if switch ON set led1 value = 0
    json.setInt("led1", 0);
  }
  client.publish("/led1", json.toString());        //send json data as a string
  //println("a toggle1 event.");
}

//switch2 change callback
void SW2(boolean theFlag) {
  JSONObject json;
  json = new JSONObject();                          //create JSON object
  if(theFlag==true) {
    json.setInt("led2", 1);                         //if switch ON set led1 value = 1
  } else {
    json.setInt("led2", 0);                         //if switch ON set led1 value = 0
  }
  client.publish("/led2", json.toString());         //send json data as a string
  //println("a toggle2 event.");
}

//MQTT subscriber callback
void messageReceived(String topic, byte[] payload) {
  println("new message: " + topic + " - " + new String(payload));
  String dataStr = new String(payload);
  JSONObject json = parseJSONObject(dataStr);
  float val = json.getFloat("temp");
  myChart.push("tempVal", val);
}
