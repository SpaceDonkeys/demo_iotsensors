#include<stdlib.h>
#include "WaveshareSharpDustSensor.h"
#include <SoftwareSerial.h>
#include <SimpleDHT.h>
int pinDHT11 = 2;
SimpleDHT11 dht11;
/* Waveshare/Sharp Dust Sensor config */
const int iled = 4;                                            //drive the led of sensor
const int vout = 0;  
int   adcvalue;
WaveshareSharpDustSensor ds; //sensor init
/* ********************************* */
/* NETWORK SETTINGS */
#define SSID "NETIASPOT-CE3880"
#define PASS "wot637a6zbd6"
#define IP "184.106.153.149" // thingspeak.com
String GET = "GET /update?key=M0SQK6K276Z4G2MF&field1=0";
/* **************** */
SoftwareSerial monitor(10, 11); // RX, TX

void setup()
{
  pinMode(iled, OUTPUT);
  digitalWrite(iled, LOW);
  
  monitor.begin(9600);
  Serial.begin(115200);
  sendDebug("AT");
  delay(5000);
  if(Serial.find("OK")){
    monitor.println("RECEIVED: OK");
    connectWiFi();
  }
}

void loop(){
  float density=readDust();
  byte temperature = 0;
  byte humidity = 0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(pinDHT11, &temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    monitor.print("Read DHT11 failed, err="); monitor.println(err);delay(1000);
    return;
  }
  char buffer[10];
  char buf_hum[10];
  char buf_dust[17];
  String str_temp = dtostrf((int)temperature, 4, 1, buffer);
  String str_hum = dtostrf((int)humidity,4,1,buf_hum);
  String str_dust=dtostrf(density,6,2,buf_dust);
  updateTemp(str_temp,str_hum,str_dust);
  delay(60000);
}

void updateTemp(String s_temp,String s_hum,String s_dust){
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  sendDebug(cmd);
  delay(2000);
  if(Serial.find("Error")){
    monitor.print("RECEIVED: Error");
    return;
  }
  cmd = GET;
  cmd += s_temp;
  cmd+="&field2="+s_hum+"&field3="+s_dust;
  cmd += "\r\n";
  Serial.print("AT+CIPSEND=");
  Serial.println(cmd.length());
  if(Serial.find(">")){
    monitor.print(">");
    monitor.print(cmd);
    Serial.print(cmd);
  }else{
    sendDebug("AT+CIPCLOSE");
  }
  if(Serial.find("OK")){
    monitor.println("RECEIVED: OK");
  }else{
    monitor.println("RECEIVED: Error");
  }
}
void sendDebug(String cmd){
  monitor.print("SEND: ");
  monitor.println(cmd);
  Serial.println(cmd);
} 
float readDust()
{
digitalWrite(iled, HIGH);
  delayMicroseconds(280);
  adcvalue = analogRead(vout);
  digitalWrite(iled, LOW);
  adcvalue = ds.Filter(adcvalue);
  return ds.Conversion(adcvalue);
   
}
boolean connectWiFi(){
  Serial.println("AT+CWMODE=1");
  delay(2000);
  String cmd="AT+CWJAP=\"";
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASS;
  cmd+="\"";
  sendDebug(cmd);
  delay(5000);
  if(Serial.find("OK")){
    monitor.println("RECEIVED: OK");
    return true;
  }else{
    monitor.println("RECEIVED: Error");
    return false;
  }
}
