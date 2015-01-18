// lm35_iot.ino
//
// Plot LM35 data on thingspeak.com using an Arduino Pro Mini 
// and an ESP8266 WiFi module.
//
// Author: Mahesh Venkitachalam
// Website: electronut.in

#include <SoftwareSerial.h>
#include <stdlib.h>
#include <Narcoleptic.h>

// LED 
int ledPin = 13;
// LM35 analog input
int lm35Pin = 0;
// ESP8266 chip enable
int chipEnablePin = 2;

// replace with your channel's thingspeak API key
String apiKey = "T2RJXWQAVXG4ZV39";

// connect 10 to TX of ESP8266
// connect 11 to RX of ESP8266
SoftwareSerial ser(10, 11); // RX, TX

// this runs once
void setup() {                
  // set outputs
  pinMode(ledPin, OUTPUT);    
  pinMode(chipEnablePin, OUTPUT);    
  
  // enable debug serial
  Serial.begin(9600); 
  // enable software serial
  ser.begin(9600);
  
  // reset ESP8266
  ser.println("AT+RST");
}

// the loop 
void loop() {
  
  // enable ESP8266
  digitalWrite(chipEnablePin, HIGH);   
  
  // reset ESP8266
  ser.println("AT+RST");

  // blink LED on board
  digitalWrite(ledPin, HIGH);   
  delay(200);               
  digitalWrite(ledPin, LOW);

  // read the value from LM35.
  // read 10 values for averaging.
  int val = 0;
  for(int i = 0; i < 10; i++) {
      val += analogRead(lm35Pin);   
      delay(500);
  }

  // convert to temp:
  // temp value is in 0-1023 range
  // LM35 outputs 10mV/degree C. ie, 1 Volt => 100 degrees C
  // So Temp = (avg_val/1023)*3.3 Volts * 100 degrees/Volt
  float temp = val*33.0f/1023.0f;

  // convert to string
  char buf[16];
  String strTemp = dtostrf(temp, 4, 1, buf);
  
  Serial.println(strTemp);
  
  // TCP connection
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com
  cmd += "\",80";
  ser.println(cmd);
   
  if(ser.find("Error")){
    Serial.println("AT+CIPSTART error");
    return;
  }
  
  // prepare GET string
  String getStr = "GET /update?api_key=";
  getStr += apiKey;
  getStr +="&field1=";
  getStr += String(strTemp);
  getStr += "\r\n\r\n";

  // send data length
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  ser.println(cmd);

  if(ser.find(">")){
    ser.print(getStr);
  }
  else{
    ser.println("AT+CIPCLOSE");
    // alert user
    Serial.println("AT+CIPCLOSE");
  }
    
  // this delay is required before disabling the ESP8266 chip
  delay(1000);
  
  // disable ESP8266
  digitalWrite(chipEnablePin, LOW);   

  // deep sleep 
  nsleep(10);
}

// utility method for deep sleep:
// Narcoleptic.delay deosn't seem to work for large values
void nsleep(int nMinutes) {
  for (int i = 0; i < 3*nMinutes; i++) {
    Narcoleptic.delay(20000); 
  }
}
