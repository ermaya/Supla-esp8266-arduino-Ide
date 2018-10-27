/**
 * Supla.org NodeMCU WiFi minimal example
 * Author: Programistyk - Kamil Kaminski <kamil@programistyk.pl>
 * 
 * This example shows how to configure SuplaDevice for building for NodeMCU within Arduino IDE
 */
 
int button1;  //wartość dla przycisku1
int button2;  //wartość dla przycisku2
int button3;  //wartość dla przycisku3
int button4;  //wartość dla przycisku4
 
#include <srpc.h>
#include <log.h>
#include <eh.h>
#include <proto.h>
#include <IEEE754tools.h>
// We define our own ethernet layer
#define SUPLADEVICE_CPP
 
#include <DHT.h>
#define DHTPIN D9 //pin pod którym jest nasz czujnik temperatury
#define DHTTYPE DHT22 //Zmieniamy na DHT11 jeśli mamy taki czujnik. DHT11 jest tańszy, ale także mniej dokładny
 
#include <SuplaDevice.h>
#include <lck.h>
DHT dht(DHTPIN, DHTTYPE);
 
#include <WiFiClient.h>
#include <ESP8266WiFiType.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiServer.h>
#include <ESP8266WiFiGeneric.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiSTA.h>
#include <WiFiUdp.h>
#include <UniversalTelegramBot.h>          //------------------------------- New ------------------------ 
                                          //https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
WiFiClient client;
 
// Setup Supla connection
const char* ssid     = "XXXXXXX"; //np. const char* ssid     = "to wcale nie jest CBA";
const char* password = "XXXXXXX"; //np. const char* password = "1jednaktak1";

byte mac[6];

char BotToken[46] = "XXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"  //------------------------------- New -------------------
                                                  // your Bot Token (Get from Botfather)  https://core.telegram.org/bots#botfather
char Chat_Id[12] = "XXXXXXXX";                  //------------------------------- New ------------------------------------      
WiFiClientSecure client2;                             //------------------------------- New ------------------------------------
UniversalTelegramBot bot(BotToken, client2);          //------------------------------- New ------------------------------------

 
void get_temperature_and_humidity(int channelNumber, double *temp, double *humidity) { 
    *temp = dht.readTemperature();
    *humidity = dht.readHumidity();
    if ( isnan(*temp) || isnan(*humidity) ) {
      *temp = -275;
      *humidity = -1;
    }
}
 
void setup() {
  Serial.begin(115200);
  dht.begin();
  delay(10);
  pinMode(D3,INPUT_PULLUP);
  pinMode(D4,INPUT_PULLUP);
  pinMode(D5,INPUT_PULLUP);
  pinMode(D6,INPUT_PULLUP);
  SuplaDevice.setTemperatureHumidityCallback(&get_temperature_and_humidity);
 
  // Replace the falowing GUID
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.macAddress(mac);
  char GUID[SUPLA_GUID_SIZE] = {mac[WL_MAC_ADDR_LENGTH - 6], 
                                mac[WL_MAC_ADDR_LENGTH - 5], 
                                mac[WL_MAC_ADDR_LENGTH - 4], 
                                mac[WL_MAC_ADDR_LENGTH - 3], 
                                mac[WL_MAC_ADDR_LENGTH - 2], 
                                mac[WL_MAC_ADDR_LENGTH - 1]};

SuplaDevice.setName("test tele");
 
//Nasze przekaźniki
 SuplaDevice.addRelay(D1);
 SuplaDevice.addRelay(D2); 
 SuplaDevice.addRelay(D7); 
 SuplaDevice.addRelay(D0);
 
//Nasz czujnik, można zmienić na DHT11 
 SuplaDevice.addDHT22();
 
 
 
  SuplaDevice.begin(GUID,              
                    mac,               
                    "svrX.supla.org",  // adres serweru SUPLI
                    XXXX,                 // ID lokacji
                    "XXXX");               // Hasło lokacji
 
}
 
void loop() {
  SuplaDevice.iterate();
  
TSD_SuplaChannelNewValue value1; 
value1.SenderID = 0; 
value1.ChannelNumber = 0; // Kanał naszego przekaźnika
value1.DurationMS = 0; 
 
button1 = digitalRead(D3); //zczytanie stanu przycisku podłączonego pod D3 do funkcji button1
if(digitalRead(D3)==LOW){ //jeśli stan jest niski
value1.value[0] = !value1.value[0]; //sprawdzenie stanu przekaźnika
SuplaDevice.channelSetValue(&value1); //zmiana stanu
while(digitalRead(D3)==LOW); //przycisk długo wciśnięty
delay(20);
bot.sendMessage(Chat_Id, "pin D3 LOW");                   //------------------------------- New ------------------------------------
}
//Teraz pozostałe przyciski
TSD_SuplaChannelNewValue value2; //Set value name  OKKKK
value2.SenderID = 0; // Notify other users that your smartphone opens a blind. In case of actuators is to be 0
value2.ChannelNumber = 1; // Need enter the relay channel number
value2.DurationMS = 0; //turn on time
button2 = digitalRead(D4); 
if(digitalRead(D4)==LOW){
value2.value[0] = !value2.value[0]; 
SuplaDevice.channelSetValue(&value2);
while(digitalRead(D4)==LOW);
delay(20);
bot.sendMessage(Chat_Id, "pin D4 LOW");                   //------------------------------- New ------------------------------------
}
TSD_SuplaChannelNewValue value3; //Set value name OKKKKKKK
value3.SenderID = 0; // Notify other users that your smartphone opens a blind. In case of actuators is to be 0
value3.ChannelNumber = 2; // Need enter the relay channel number
value3.DurationMS = 0; //turn on time
 
button3 = digitalRead(D5); 
if(digitalRead(D5)==LOW){
value3.value[0] = !value3.value[0]; 
SuplaDevice.channelSetValue(&value3);
while(digitalRead(D5)==LOW);
delay(20);
bot.sendMessage(Chat_Id, "pin D5 LOW");                   //------------------------------- New ------------------------------------
}
TSD_SuplaChannelNewValue value4; //Set value name OKKKKKKKK
value4.SenderID = 0; // Notify other users that your smartphone opens a blind. In case of actuators is to be 0
value4.ChannelNumber = 3; // Need enter the relay channel number
value4.DurationMS = 0; //turn on time
 
button4 = digitalRead(D6); 
if(digitalRead(D6)==LOW){
value4.value[0] = !value4.value[0]; 
SuplaDevice.channelSetValue(&value4);
while(digitalRead(D6)==LOW);
delay(20);
bot.sendMessage(Chat_Id, "pin D6 LOW");                   //------------------------------- New ------------------------------------
}
}
 
// Supla.org ethernet layer
    int supla_arduino_tcp_read(void *buf, int count) {
        _supla_int_t size = client.available();
       
        if ( size > 0 ) {
            if ( size > count ) size = count;
            return client.read((uint8_t *)buf, size);
        };
    
        return -1;
    };
    
    int supla_arduino_tcp_write(void *buf, int count) {
        return client.write((const uint8_t *)buf, count);
    };
    
    bool supla_arduino_svr_connect(const char *server, int port) {
          return client.connect(server, 2015);
    }
    
    bool supla_arduino_svr_connected(void) {
          return client.connected();
    }
    
    void supla_arduino_svr_disconnect(void) {
         client.stop();
    }
    
    void supla_arduino_eth_setup(uint8_t mac[6], IPAddress *ip) {
 
       // Serial.println("WiFi init");
        WiFi.begin(ssid, password);
 
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
        //    Serial.print(".");
        }

        bot.sendMessage(Chat_Id, "połączone");                   //------------------------------- New ------------------------------------
 
        //Serial.print("\nlocalIP: ");
        //Serial.println(WiFi.localIP());
        //Serial.print("subnetMask: ");
        //Serial.println(WiFi.subnetMask());
        //Serial.print("gatewayIP: ");
        //Serial.println(WiFi.gatewayIP());
    }
 
SuplaDeviceCallbacks supla_arduino_get_callbacks(void) {
          SuplaDeviceCallbacks cb;
          
          cb.tcp_read = &supla_arduino_tcp_read;
          cb.tcp_write = &supla_arduino_tcp_write;
          cb.eth_setup = &supla_arduino_eth_setup;
          cb.svr_connected = &supla_arduino_svr_connected;
          cb.svr_connect = &supla_arduino_svr_connect;
          cb.svr_disconnect = &supla_arduino_svr_disconnect;
          cb.get_temperature = NULL;
          cb.get_temperature_and_humidity = NULL;
          cb.get_rgbw_value = NULL;
          cb.set_rgbw_value = NULL;
          
          return cb;
}
