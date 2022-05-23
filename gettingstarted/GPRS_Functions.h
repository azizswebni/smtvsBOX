//#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_MODEM_SIM808
// #define TINY_GSM_MODEM_SIM900
// #define TINY_GSM_MODEM_UBLOX
// #define TINY_GSM_MODEM_BG96
// #define TINY_GSM_MODEM_A6
// #define TINY_GSM_MODEM_A7
// #define TINY_GSM_MODEM_M590
// #define TINY_GSM_MODEM_ESP8266
// #define TINY_GSM_MODEM_XBEE

#include <TinyGsmClient.h> //Biblioteca que configura o modem GSM

const char apn[]  = "APN"; //YourAPN
const char user[] = "";
const char pass[] = "";
#define GSM_PIN ""
// Your WiFi connection credentials, if applicable
const char wifiSSID[] = "YourSSID";
const char wifiPass[] = "YourWiFiPass";

// MQTT details
const char* broker = "io.adafruit.com";

const char* topiclat   = "Azizs/feeds/lat";
const char* topiclon   = "Azizs/feeds/long";
const char* topicspeed = "Azizs/feeds/speed";



#include <PubSubClient.h>
HardwareSerial SerialAT(1);
TinyGsm        modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient  mqtt(client);
uint32_t lastReconnectAttempt = 0;

bool networkConnect()

{
  Serial.println("Waiting for network...");
  if (!modem.waitForNetwork()) 
  {
    Serial.println(" fail");
    return false;
  }

  Serial.println(" OK");
  Serial.print("Connecting to ");
  Serial.print(apn);
  if (!modem.gprsConnect(apn, user, pass)) 
  {
    Serial.println(" fail");
    return false;
  }
 Serial.println(" OK");
  return true;

}

//Configura o modem GPRS
bool modemConfig(HardwareSerial mySerial)
{
  SerialAT = mySerial;
  //Inicia modem
  Serial.println("Setting modem...");  
  
  if(!modem.restart()) 
{    return false;
}
 else{ return networkConnect();}
}

void mqttCallback(char* topic, byte* payload, unsigned int len) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.write(payload, len);
  Serial.println();
}

boolean mqttConnect() {
  Serial.print("Connecting to ");
  Serial.print(broker);
 boolean status = mqtt.connect("GsmClientName", "Azizs", "aio_XNhN10SVMiE8Ybqaiiue1YrCYws9");

  if (status == false) {
    Serial.println("============>fail");
    return false;
  }
  Serial.println("============> success");
  return mqtt.connected();
}
