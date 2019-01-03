/* Przygotowane przez kolegę elmaya na poniższych bibliotekach
 * Działa z bibliotekami: FS.h            w wersji: 1.0.3
 *                        WiFiManager.h   w wersji: 0.12.0
 *                        ArduinoJson.h   w wersji: 5.13.2
 * zmodyfikowany przez wotas567 dodane kilka praktycznych dodatków :) */

#include <FS.h>

#include <EEPROM.h>      //             ---------- Eeprom ------------

#include <ESP8266WiFi.h>
#define SUPLADEVICE_CPP
#include <SuplaDevice.h>

#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h> 
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
extern "C" {
#include "user_interface.h"
}
#include <DHT.h>
#include <OneWire.h>
#define Sensor_PIN D5     // Rpm sensor pin
#define DHTPIN 3 // GPIO 3-RX
#define DHTTYPE DHT22

bool eep = LOW;          //             ---------------- Eeprom ------------------
bool startEeprom = true; //             ---------------- Eeprom ------------------
int s;             //                   ---------------- Status ------------------
unsigned long wifi_checkDelay = 60000;  // wifi reconnect delay tray to reconnect every 60 seconds ---------new---------- Wi-Fi podłącz tacę opóźniającą, aby ponownie połączyć się co 60 sekund
unsigned long wifimilis;   //                                                                      ---------new----------
int C_W_state = HIGH;      //                                                                      ---------new----------          
int last_C_W_state = HIGH; //                                                                      ---------new----------          
unsigned long time_last_C_W_change = 0;   //                                                       ---------new---------- 
long C_W_delay = 10000;               // config delay 10 seconds                                   ---------new---------- opóźnienie konfiguracji 10 sekund
unsigned long svr_update = 1800000; //mean time between update  30 minutes
unsigned long svr_update_lasttime;   //last time update

int pulseCount;  
double distance;
unsigned long oldTime;

// Setup a DHT instance
DHT dht(DHTPIN, DHTTYPE);

ADC_MODE(ADC_VCC);

#define BTN_COUNT 2

WiFiClient client;
const char* host = "supla";
const char* update_path = "/nowy";
const char* update_username = "admin";
const char* update_password = "supla";

ESP8266WebServer httpServer(81);
ESP8266HTTPUpdateServer httpUpdater;

//define your default values here, if there are different values in config.json, they are overwritten.
//length should be max size + 1 
char Supla_server[40];
char Location_id[15];
char Location_Pass[20];
byte mac[6];

//flag for saving data
bool shouldSaveConfig = false;
bool initialConfig = false;

#define TRIGGER_PIN 0    //      trigger Pin is 0 same as channel 0                      ---------new----------    pin wyzwalacza jest 14 taki sam jak kanał 5  
int timeout           = 120; // seconds to run for

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void ondemandwifiCallback () {
// The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 40);
  WiFiManagerParameter custom_Location_id("ID", "Location_id", Location_id, 15);
  WiFiManagerParameter custom_Location_Pass("Password", "Location_Pass", Location_Pass, 20);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  //add all your parameters here
  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_Location_id);
  wifiManager.addParameter(&custom_Location_Pass);

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  wifiManager.setMinimumSignalQuality();

  // set configportal timeout
    wifiManager.setConfigPortalTimeout(timeout);

    if (!wifiManager.startConfigPortal("SuplaESP_wojtas")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.restart();
      delay(5000);
    }
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    
    //read updated parameters
    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(Location_id, custom_Location_id.getValue());
    strcpy(Location_Pass, custom_Location_Pass.getValue());
}

// obsługa przycisków
typedef struct {
  int pin;
  int relay_pin;
  int channel;
  int ms;
  char last_val;
  unsigned long last_time;
} _btn_t;

_btn_t btn[BTN_COUNT];

void supla_timer() {
  char v;
  unsigned long now = millis();
  
  for(int a=0;a<BTN_COUNT;a++)
    if (btn[a].pin > 0) {
        v = digitalRead(btn[a].pin);
        if (v != btn[a].last_val && now - btn[a].last_time ) {
           btn[a].last_val = v;
           btn[a].last_time = now;
           if (v==0)
             {
             if ( btn[a].ms > 0 ) {
                     SuplaDevice.relayOn(btn[a].channel, btn[a].ms);
                 } else {
                 if ( digitalRead(btn[a].relay_pin) == 1 ) {   //   ----------------- == 1 if channel is false... == 0 if channel is true -----------------------
                  SuplaDevice.relayOff(btn[a].channel);
                  Serial.print("BTN Switsh off relay ");
                  Serial.println(btn[a].relay_pin);
                 } else {
                  SuplaDevice.relayOn(btn[a].channel, 0);
                  Serial.print("BTN Switsh on relay ");
                  Serial.println(btn[a].relay_pin);
                 }        
             }
        }
      }
    }
}
void supla_btn_init() {
  for(int a=0;a<BTN_COUNT;a++)
    if (btn[a].pin > 0) {
        pinMode(btn[a].pin, INPUT_PULLUP);
        btn[a].last_val = digitalRead(btn[a].pin);
        btn[a].last_time = millis();
    }
}

// Obsługa czujnika DHT22 lub BME280 itp
void get_temperature_and_humidity(int channelNumber, double *temp, double *humidity) {

    *temp = dht.readTemperature();
    *humidity = dht.readHumidity();

    if ( isnan(*temp) || isnan(*humidity) ) {
      *temp = -275;
      *humidity = -1;
    }
}

// Obsługa czujnika DS18B20 i odczyt parametrów modułu ESP
void Pcounter ()
{
    pulseCount++;
}
double get_temperature(int channelNumber, double last_val) {
    double t = -275;
          switch(channelNumber)
          {
            case 1:
        t = (ESP.getVcc()/1024.0);        
                    break;
            case 2:
         t = WiFi.RSSI();
                    break;
            case 3:
         t = (ESP.getVcc()/1024.0);
                    break;
            case 4:
         t = (ESP.getFlashChipRealSize()/1024/1024);
                    break;
      };
      Eeprom_save(0) ;   //        ------------------------------Eepron save callback -----------------------------

    return t;  
}
double get_distance(int channelNumber, double distance) {
        
        detachInterrupt(Sensor_PIN);

        yield();
        distance = ((1000.0 / (millis() - oldTime)) * pulseCount)*60;
        Serial.print("Pulse: ");
        Serial.println(distance);
        yield();
        oldTime = millis();
        pulseCount = 0;
        

        attachInterrupt(Sensor_PIN, Pcounter, FALLING);
                                                             
   return  distance; 
}
void Eeprom_save(int offset) {                  //----------EEPROM write  ---------------------- EEprom
    if (startEeprom == true){
      return;
      }                        
      for(int i=offset;i<BTN_COUNT;i++) {
         if ( btn[i].ms > 0 ) {
                     continue;
         } else {
        eep = digitalRead(btn[i].relay_pin);
        if (eep != EEPROM.read(i)){
         EEPROM.write(i,eep);       
         Serial.print("EEPROM.");
         Serial.print(i);
         Serial.print(" write.");
         Serial.println((eep));
         EEPROM.commit();
        }
      }
    }
}
void Eepron_read(int offset) {                  //----------EEPROM read  ---------------------- EEprom
                                
    for(int i=offset;i<BTN_COUNT;i++){
      if ( btn[i].ms > 0 ) {
                     continue;
         } else {
        eep = EEPROM.read(i);
        if (eep > 1) {
          eep = 1;
          Serial.println("correct eep ");
          EEPROM.write(i,eep);
          EEPROM.commit();
        }
         Serial.print("EEPROM.");
         Serial.print(i);
         Serial.print(" read.");
         Serial.println((eep));
        
        if (eep == LOW){
          SuplaDevice.relayOff(btn[i].channel);
    //digitalWrite(btn[i].relay_pin,0);
    Serial.println("out 0 ");
    }
        if (eep == HIGH) {
      SuplaDevice.relayOn(btn[i].channel, 0);
    //digitalWrite(btn[i].relay_pin,1);
    Serial.println("out 1  ");
    }
    }
    }  
    startEeprom = false;
}
void status_func(int status, const char *msg) {     //    ------------------------ Status --------------------------
 s=status;
}
void setup() {

  Serial.begin(115200);
  delay(10);
  EEPROM.begin(512);  //                                           -------------- Start EEprom ------------- EEprom
  pinMode(Sensor_PIN, INPUT_PULLUP);
  attachInterrupt(Sensor_PIN, Pcounter, FALLING);

  wifi_station_set_hostname("E_Supla_12F");  //nazwa w sieci lokalnej

  pinMode(TRIGGER_PIN, INPUT);
  
  if (WiFi.SSID()==""){
    //Serial.println("We haven't got any access point credentials, so get them now");   
    initialConfig = true;
  }
  
  //read configuration from FS json
  Serial.println("mounting FS...");
  
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;         
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        Serial.println(jsonBuffer.size());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(Supla_server, json["Supla_server"]);
          strcpy(Location_id, json["Location_id"]);
          strcpy(Location_Pass, json["Location_Pass"]);

        } else {
          Serial.println("failed to load json config");
          
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.

  // Inicjalizacja DHT
  dht.begin(); 
  SuplaDevice.setTemperatureHumidityCallback(&get_temperature_and_humidity);
  SuplaDevice.setStatusFuncImpl(&status_func);    //   ----------------------------- Status -----------------------------


  SuplaDevice.setTemperatureCallback(&get_temperature);

  // ﻿Replace the falowing GUID
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.macAddress(mac);
  char GUID[SUPLA_GUID_SIZE] = {mac[WL_MAC_ADDR_LENGTH - 6], 
                                mac[WL_MAC_ADDR_LENGTH - 5], 
                                mac[WL_MAC_ADDR_LENGTH - 4], 
                                mac[WL_MAC_ADDR_LENGTH - 3], 
                                mac[WL_MAC_ADDR_LENGTH - 2], 
                                mac[WL_MAC_ADDR_LENGTH - 1]};
  
  // CHANNEL0 - DHT22
  SuplaDevice.addDHT22();               // na GPIO2

  // CHANNEL1,2,3,4 - DS, wifi, zasilanie
  SuplaDevice.addDS18B20Thermometer();  // DS na GPIO03 - RX
  SuplaDevice.addDS18B20Thermometer();  // wyśietla wifi
  SuplaDevice.addDS18B20Thermometer();  // wyśietla zasilanie
  SuplaDevice.addDS18B20Thermometer();  // wyświetla wielkość pamięci
  SuplaDevice.addDistanceSensor();

  // CHANNEL5,6 - RELAY
  SuplaDevice.addRelay(5, false);       // ﻿Line 146 == 1 if false ----  == 0 if true 
                                        //(extra "true" parameter to enable "port value inversion"where HIGH == LOW, and LOW == HIGH) 
  SuplaDevice.addRelay(13, false);      // ﻿Line 146 == 1 if false ----  == 0 if true 
                                        //(extra "true" parameter to enable "port value inversion"where HIGH == LOW, and LOW == HIGH) 

  // CHANNEL5,6 - TWO RELAYS (Roller shutter operation)
  //SuplaDevice.addRollerShutterRelays(5,            // 46 - ﻿﻿Pin number where the 1st relay is connected   
  //                                   13, true);    // 47 - ﻿Pin number where the 2nd relay is connected  

  // CHANNEL7,8 - Opening sensor (Normal Open)
  SuplaDevice.addSensorNO(4);   // A0 - ﻿Pin number where the sensor is connected
  SuplaDevice.addSensorNO(16);  // Call SuplaDevice.addSensorNO(A0, true) with an extra "true" parameter
                                // to enable the internal pull-up resistor

  memset(btn, 0, sizeof(btn));
  btn[0].pin =0;         // pin gpio  button D5
  btn[0].relay_pin =5;    // pin gpio on which is relay  D1
  btn[0].channel =5;      // supla channel
  btn[0].ms =0;           // if = 0 Bistable -- if > 0 Monostable for X ms
  btn[1].pin =12;         // pin gpio  button D6
  btn[1].relay_pin =13;   // pin gpio on which is relay  D7
  btn[1].channel =6;      // supla channel
  btn[1].ms =0;           // if = 0 Bistable -- if > 0 Monostable for X ms
  supla_btn_init();
  SuplaDevice.setTimerFuncImpl(&supla_timer);
  SuplaDevice.setName("supla ESP wojtas");

  int LocationID = atoi(Location_id);
  SuplaDevice.begin(GUID,              // Global Unique Identifier 
                    mac,               // Ethernet MAC address
                    Supla_server,      // SUPLA server address
                    LocationID,        // Location ID 
                    Location_Pass);    // Location Password


  Serial.println();
  Serial.println("Uruchamianie serwera aktualizacji...");
  WiFi.mode(WIFI_STA);

  MDNS.begin(host);
  httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  httpServer.begin();
  MDNS.addService("http", "tcp", 81);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local%s in your browser and login with username '%s' and password '%s'\n", host, update_path, update_username, update_password);
}

void loop() {
  
  int C_W_read = digitalRead(TRIGGER_PIN);{    //   from line 398 to 415                ---------new---------- z linii 398 do 415
   if (C_W_read != last_C_W_state) {       
     
     time_last_C_W_change = millis();
   }
   if ((millis() - time_last_C_W_change) > C_W_delay) {
     
     if (C_W_read != C_W_state) {     
       Serial.println("Triger sate changed");
       C_W_state = C_W_read;
       
       if (C_W_state == LOW) {
        ondemandwifiCallback () ;
       }
     }
    }
   last_C_W_state = C_W_read;            
   }

    // is configuration portal requested?
  if  (initialConfig)  {                       //                                        ---------new----------
    ondemandwifiCallback () ;
    initialConfig = false; 
  }
  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Supla_server"] = Supla_server;
    json["Location_id"] = Location_id;
    json["Location_Pass"] = Location_Pass;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    Serial.println("config saved");
    shouldSaveConfig = false;
    //end save
  }
  
  if (WiFi.status() != WL_CONNECTED) 
  {
    WiFi_up();
  }
switch (s) {    //    ------------------------------------------------ Status ------------------------------------
  case 17:// URZADZENIE JEST ZALOGOWANE I DZIAŁA POPRAWNIE Z SUPLĄ
    if (startEeprom == true){
    Serial.println("Eepron_read...");
    Eepron_read(0) ;                // ------------------------------- Eeprom read callback -------------------------------
  }
    break;        
    case 9:      // --------------------- DISCONNECTED  ----------------------
    startEeprom = true;
    break;
    case 10:      // --------------------- REGISTER_IN_PROGRESS  ----------------------
    startEeprom = true;
    break;
  }
  if (millis() > svr_update_lasttime + svr_update)  {
     startEeprom = true;
            svr_update_lasttime = millis(); 
            }
  SuplaDevice.iterate();
  SuplaDevice.setTemperatureHumidityCallback(&get_temperature_and_humidity);
  SuplaDevice.setTemperatureCallback(&get_temperature);
  SuplaDevice.setDistanceCallback(&get_distance);
  SuplaDevice.addDistanceSensor();
  httpServer.handleClient();
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

       WiFi_up();
    }

SuplaDeviceCallbacks supla_arduino_get_callbacks(void) {
          SuplaDeviceCallbacks cb;
          
          cb.tcp_read = &supla_arduino_tcp_read;
          cb.tcp_write = &supla_arduino_tcp_write;
          cb.eth_setup = &supla_arduino_eth_setup;
          cb.svr_connected = &supla_arduino_svr_connected;
          cb.svr_connect = &supla_arduino_svr_connect;
          cb.svr_disconnect = &supla_arduino_svr_disconnect;
          cb.get_temperature = &get_temperature;
          cb.get_temperature_and_humidity = get_temperature_and_humidity;
          cb.get_distance = &get_distance;
          return cb;
}

void WiFi_up() // conect to wifi
{ 
  if (millis() > wifimilis)  {//                                                        ---------new----------
  WiFi.begin(); 
  for (int x = 20; x > 0; x--) 
  {
    if (x == 1){//                                                                      ---------new----------
    wifimilis = (millis() + wifi_checkDelay) ; //                                       ---------new----------
    }           //                                                                      ---------new----------
    if (WiFi.status() == WL_CONNECTED) 
    {    
     return; // break;                           
    }
    else                              
    {
     yield();
     delay(500);   
      Serial.print(".");                
                         
    }
    
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.println("CONNECTED");
    Serial.print("local IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("subnetMask: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("gatewayIP: ");
    Serial.println(WiFi.gatewayIP());
    long rssi = WiFi.RSSI();
    Serial.print("Signal Strength (RSSI): ");
    Serial.print(rssi);
    Serial.println(" dBm");
  }
  if (WiFi.status() != WL_CONNECTED)          //                                        ---------new----------
  {
    Serial.println("");
    Serial.println("not connected");
  }
  }
}
