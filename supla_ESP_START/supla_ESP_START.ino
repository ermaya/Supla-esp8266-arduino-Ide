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
#include <DallasTemperature.h>

#define DHTPIN 3 // GPIO 3-RX
#define DHTTYPE DHT22

bool eep = LOW;          //             ---------------- Eeprom ------------------
bool startEeprom = true; //             ---------------- Eeprom ------------------
int s;             //                   ---------------- Status ------------------

// Setup a DHT instance
DHT dht(DHTPIN, DHTTYPE);

// Setup a oneWire instance
OneWire oneWire(4); // GPIO 02

// Pass oneWire reference to Dallas Temperature
DallasTemperature sensors(&oneWire);

ADC_MODE(ADC_VCC);

#define BTN_COUNT 3

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

#define TRIGGER_PIN 0
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
double get_temperature(int channelNumber, double last_val) {
    double t = -275;
          switch(channelNumber)
          {
            case 1:
    if ( sensors.getDeviceCount() > 0 )
      {
         sensors.requestTemperatures();
         t = sensors.getTempCByIndex(0);
      }
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
                         //        ---- this happens every few seconds
    return t;  
}
void Eeprom_save(int offset) {                  //----------EEPROM write  ---------------------- EEprom
    if (startEeprom == true){             // ----- don't change memorized state until connected and restored
      return;
      }                        
      for(int i=offset;i<BTN_COUNT;i++) {  //  ---check relay except der have delay (staircase)
         if ( btn[i].ms > 0 ) {
                     continue;
         } else {
        eep = digitalRead(btn[i].relay_pin);   //  --- read relay state
        if (eep != EEPROM.read(i)){            //  --- compare relay state with memorized one
         EEPROM.write(i,eep);                  //  --- if different write memory
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
                                
    for(int i=offset;i<BTN_COUNT;i++){      //  ---check relay except der have delay (staircase)
      if ( btn[i].ms > 0 ) {
                     continue;
         } else {
        eep = EEPROM.read(i);               //  ---read relay state
        if (eep > 1) {                      //  --- check if bigger than 1 and correct (possible on first start)
          eep = 1;
          Serial.println("correct eep ");
          EEPROM.write(i,eep);
          EEPROM.commit();
        }
         Serial.print("EEPROM.");
         Serial.print(i);
         Serial.print(" read.");
         Serial.println((eep));
        
        if (eep == LOW){                    //  --- if 0 send relay off
          SuplaDevice.relayOff(btn[i].channel);
    
    Serial.println("out 0 ");
    }
        if (eep == HIGH) {                  //  --- if 1 send relay on
      SuplaDevice.relayOn(btn[i].channel, 0);
    
    Serial.println("out 1  ");
    }
    }
    }  
    startEeprom = false;       //  --- once finished we do not need more
}
void status_func(int status, const char *msg) {     //    ------------------------ Status --------------------------
 s=status;                                          //    -- to check if we are registered and ready before restore from memory
}
void setup() {

  Serial.begin(115200);
  delay(10);
  EEPROM.begin(512);  //                                           -------------- Start EEprom ------------- EEprom

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

  // Inicjalizacja DS18B20
  sensors.begin();
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

  // CHANNEL5,6 - RELAY
  SuplaDevice.addRelay(2, false);       // ﻿44 - ﻿Pin number where the relay is connected      
  SuplaDevice.addRelay(16, false);      // 45 - ﻿﻿Pin number where the relay is connected  
  SuplaDevice.addRelay(5, false);      // 45 - ﻿﻿Pin number where the relay is connected   

  // CHANNEL5,6 - TWO RELAYS (Roller shutter operation)
  //SuplaDevice.addRollerShutterRelays(5,            // 46 - ﻿﻿Pin number where the 1st relay is connected   
  //                                   13, true);    // 47 - ﻿Pin number where the 2nd relay is connected  

  // CHANNEL7,8 - Opening sensor (Normal Open)
  //SuplaDevice.addSensorNO(4);   // A0 - ﻿Pin number where the sensor is connected
  //SuplaDevice.addSensorNO(16);  // Call SuplaDevice.addSensorNO(A0, true) with an extra "true" parameter
                                // to enable the internal pull-up resistor

  memset(btn, 0, sizeof(btn));
  btn[0].pin =14;         // pin gpio  button D5
  btn[0].relay_pin =2;    // pin gpio on which is relay  D4
  btn[0].channel =5;      // supla channel
  btn[0].ms =0;           // if = 0 Bistable -- if > 0 Monostable for X ms
  btn[1].pin =12;         // pin gpio  button D6
  btn[1].relay_pin =16;   // pin gpio on which is relay  D0
  btn[1].channel =6;      // supla channel
  btn[1].ms =2000;           // if = 0 Bistable -- if > 0 Monostable for X ms
  btn[2].pin =13;         // pin gpio  button D7
  btn[2].relay_pin =5;   // pin gpio on which is relay  D1
  btn[2].channel =7;      // supla channel
  btn[2].ms =0;           // if = 0 Bistable -- if > 0 Monostable for X ms
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
  Serial.printf("HTTPUpdateServer ready! Open http://supla.local:81/nowy in your browser and login with username '%s' and password '%s'\n", host, update_path, update_username, update_password);
}

void loop() {

    // is configuration portal requested?
  if ( digitalRead(TRIGGER_PIN) == LOW|| (initialConfig))  {
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
  case 17:      // -----     STATUS_REGISTERED_AND_READY
  if (startEeprom == true){   // ---- check if first pass
  Serial.println("Eepron_read...");
   Eepron_read(0) ;                // ------------------------------- Eeprom read callback -------------------------------
  }

}
  SuplaDevice.iterate();
  SuplaDevice.setTemperatureHumidityCallback(&get_temperature_and_humidity);
  SuplaDevice.setTemperatureCallback(&get_temperature);
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
          
          return cb;
}

void WiFi_up() // Procedimiento de conexión para redes WiFi
{
  Serial.print("Conexión a la red ");

  WiFi.begin(); // Intentar conectarse a la red

  for (int x = 60; x > 0; x--) 
  {
    if (WiFi.status() == WL_CONNECTED) 

    {
      break;                           
    }
    else                                 
    {
      Serial.print(".");                
      delay(500);                      
    }

  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.println("Conexión hecha");
    Serial.println("Adres IP: ");
    Serial.print(WiFi.localIP());
    Serial.print(" / ");
    Serial.println(WiFi.subnetMask());
    Serial.print("puerta: ");
    Serial.println(WiFi.gatewayIP());
    long rssi = WiFi.RSSI();
    Serial.print("Fuerza de la señal (RSSI): ");
    Serial.print(rssi);
    Serial.println(" dBm");
  }
  else    
  {
    Serial.println("");
    Serial.println("La conexión no pudo hacerse");
  }
}
