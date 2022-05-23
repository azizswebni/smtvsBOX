#include <Arduino.h>
#include <HardwareSerial.h> 
#include "GPRS_Functions.h" 
HardwareSerial mySerial(1); 
#include <ArduinoJson.h>
const int BAUD_RATE = 115200;
const int TIMEOUT_AT = 700;

const int RX_PIN = 16;

const int TX_PIN = 17;
String sendAT(String command)
{
  String response = "";    
  mySerial.println(command); 

  delay(5);
  long int time = millis();   
  while((time+TIMEOUT_AT) > millis())
  {
    while(mySerial.available())
      response += char(mySerial.read());      
      
  }   Serial.println("--------");
  Serial.println(command);
  Serial.println(response);
  
  return response;
}

void serialConfig()
{

  //Set console baud rate
  Serial.begin(BAUD_RATE);

  //Configura baud rate do ESP32 UART
  mySerial.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN); //Esp32 lora 0 = RX, 22 = TX
}

bool gpsConfig()
{
  if(sendAT("AT+CGNSPWR=1").indexOf("OK") >= 0) 
    if(sendAT("AT+CGNSSEQ=RMC").indexOf("OK") >= 0) 
      return true;
  
  return false;
}


bool baudConfig()
{
  if(sendAT("AT+IPR="+String(BAUD_RATE)).indexOf("OK") >= 0)
      return true;

  return false;
}

bool GSMLocationInit(HardwareSerial mySerial)
{  
    String response;    

    //limpa serial (flush)
    response = "";
    while(response == "" || mySerial.available())
        response += char(mySerial.read());   

    //seta GPS
    mySerial.println("AT + CGATT = 1"); 
    delay(1000);  
    response = "";
    while(response == "" || mySerial.available())
        response += char(mySerial.read());   

    if(response.indexOf("OK")<=0)
        return false;

    //ativa perfil do portador
    mySerial.println("AT + SAPBR = 3,1, \"CONTYPE\", \"GPRS\""); 
    delay(1000);  
    response = "";
    while(response == "" || mySerial.available())
        response += char(mySerial.read());   

    if(response.indexOf("OK")<=0)
        return false;

    return true;
}

/*String buildJson(float lat, float lon,String id) {
  String data = "{";
  data += "\n";
  data += "\"ID\":\"";
  data += id;
  data += "\",\n";
  data += "\"Latitude\":\"";
  data += lat;
  data += "\",\n";
  data += "\"Longitude\":\"";
  data += lon;
  data += "\",\n";
  data += "}";
  return data;
}*/

void setup() 
{    

  serialConfig();

  Serial.println("Waiting...");
  delay(3000);

if(!baudConfig())
      ESP.restart();
      

  if(!modemConfig(mySerial))
  {
      Serial.println("Modem init fail");
      delay(5000);
      ESP.restart();
  }
  Serial.println("Modem ok"); 
  if(!GSMLocationInit(mySerial))
  {        
      Serial.println("GPS config failed!");
      delay(3000);
      ESP.restart();
  }

  if(!gpsConfig())
  {        
      Serial.println("GPS config failed!");
      delay(3000);
      ESP.restart();
  }
  Serial.println("GPS config ok");

  // MQTT Broker setup
  mqtt.setServer(broker, 1883);
  mqtt.setCallback(mqttCallback);
}



void loop() 
{ DynamicJsonDocument doc(1024);
  float lat, lon,spd; 
  Serial.println(sendAT("AT+CGPSSTATUS?"));
  
  if (!mqtt.connected()) {
    Serial.println("=== MQTT NOT CONNECTED ===\n");
    // Reconnect every 10 seconds
    uint32_t t = millis();
    if (t - lastReconnectAttempt > 10000L) {
      lastReconnectAttempt = t;
      if (mqttConnect()) { lastReconnectAttempt = 0; }
    }
    delay(100);
    return;
  }
  else{
    Serial.println("=== MQTT CONNECTED ===\n");
  if(modem.getGPS(&lat, &lon,&spd)) 
  {    
    Serial.println("GPS signal ok. Values obtained successfully.");
  }
  else  
  {
    Serial.println("GPS signal is down. Values not obtained.");
  }
 Serial.println(lat);
  Serial.println(lon);
  Serial.println(spd);
   doc["lat"] = lat;
  doc["long"] = lon;
  doc["speed"] = spd;
  serializeJson(doc, Serial);
/*char myspeed[9] ;
char mylong[9] ;
char mylat[9] ;
dtostrf(spd,-10,7,myspeed);
dtostrf(lat,-10,7,mylat);
dtostrf(lon,-10,7,mylong);
Serial.println(mylat);
  Serial.println(mylong);
    Serial.println(myspeed);
    mqtt.publish(topiclat,mylat);
mqtt.publish(topiclon,mylong);
mqtt.publish(topicspeed,myspeed);
*/
/*String d = buildJson(lat,lon,"test1");
char Data[]="";
int len=d.length()+1;
d.toCharArray(Data,len);*/
String j = "{\n lat:"+lat+",\n long:"+lon+",\n speed:"+spd+",\n}";
String a = "{\"lat\":"+ 10 +",\"long\":"+ 10 +",\"speed\":" + 50 + "}";
char Data[200]="";
int len=j.length()+5;
j.toCharArray(Data,len);
mqtt.publish("Azizs/f/box",Data);
  //mqtt.subscribe(topicLed);
  }
  delay(1000);
  mqtt.loop();


}

/*sendAT("AT");
sendAT("AT+CPIN?");
sendAT("AT+CREG?");
sendAT("AT+CGATT?");
sendAT("AT+CIPSHUT");
sendAT("AT+CIPSTATUS");
sendAT("AT+CIPMUX=0");
sendAT("AT+CSTT?");
sendAT("AT+CIPSPRT=0");*/

/*
  Serial.println(sendAT("AT+CIPSTART=\"TCP\",\"azrergdjfkbgkd.herokuapp.com\",\"80\""));
  Serial.println(sendAT("AT+CIPSEND"));
  
  String str = "GET https://azrergdjfkbgkd.herokuapp.com/test/" + String(lat);
   Serial.println(str);
   mySerial.println(str);
   mySerial.println((char)26);
   Serial.println("waiting for response");
*/



/*sendAT("AT+HTTPINIT");

String url = "AT+HTTPPARA=\"URL\",\"azrergdjfkbgkd.herokuapp.com/test/"+String(lat);
String url1 =url+"&"+String(lon)+"\"";
sendAT(url1);
sendAT("AT+HTTPPARA=\"CID\",1");
sendAT("AT+HTTPACTION=0");
sendAT("AT+HTTPREAD");
sendAT("AT+HTTPTERM");*/
