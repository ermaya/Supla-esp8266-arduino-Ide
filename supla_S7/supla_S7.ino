#include <FS.h>       // ---- esp board manager 2.4.2 --- iwip Variant V2 higher Bandwidth
#include <ESP8266WiFi.h>
#define SUPLADEVICE_CPP
#include <SuplaDevice.h>
#include <math.h>
#include <DNSServer.h>
#include <WiFiManager.h> 
#include <ArduinoJson.h> //--------- https://github.com/bblanchon/ArduinoJson/tree/v5.13.2 ------
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESPEFC.h>  // modification of ESP8266HTTPUpdateServer that includes erases flash and wifi credentials

extern "C"
{
#include "user_interface.h"
}
//#define D0 16  //no internal pullup resistor
//#define D1  5
//#define D2  4
//#define D3  0  //must not be pulled low during power on/reset, toggles value during boot
//#define D4  2  //must not be pulled low during power on/reset, toggles value during boot
//#define D5 14
//#define D6 12
//#define D7 13
//#define D8 15  //must not be pulled high during power on/reset

#define CD4051_a 14     // D5  CD4051 Pin 11 Data A
#define CD4051_b 12     // D6  CD4051 Pin 10 Data B
#define CD4051_c 13     // D7  CD4051 Pin  9 Data C
#define config_pin 0    // D3  wifi config
#define status_led 2    // D4  status Led 
bool pr_wifi = true;
bool start = true;           
int s;             
unsigned long wifi_checkDelay = 20000;
unsigned long wifimilis;
int C_W_state = HIGH; 
int last_C_W_state = HIGH;
unsigned long time_last_C_W_change = 0; 
long C_W_delay = 10000;               // config delay 10 seconds           ----------        opóźnienie konfiguracji 10 sekund
WiFiClient client;
ESP8266WebServer httpServer(81);
ESPEFC httpUpdater;
const char* update_path = "/";
char Supla_server[80];
char Location_id[15];
char Location_Pass[34];
char Supla_name[51];
char update_username[21];
char update_password[21];
byte mac[6];
bool shouldSaveConfig = false;
bool initialConfig = false;
int timeout = 180;          // seconds to run the wifi config
unsigned long voltage_lasttime; 
int voltage_mtbs = 1000;                // mean time between get_voltage
int epr = 0;  
int val = 0;
double Voltage_0_Send = -1;
double Voltage_1_Send = -1;
double Voltage_2_Send = -1;
double Voltage_3_Send = -1;
double Voltage_4_Send = -1;
double Voltage_5_Send = -1;
double Voltage_6_Send = -1;
double Voltage_7_Send = -1;
double Voltage_Value = 0;

void saveConfigCallback () {                 //callback notifying us of the need to save config
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void ondemandwifiCallback () {
  digitalWrite(status_led, LOW);
  httpServer.stop();
  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 80);
  WiFiManagerParameter custom_Location_id("ID", "Location id", Location_id, 15);
  WiFiManagerParameter custom_Location_Pass("Password", "Location Pass", Location_Pass, 34);
  WiFiManagerParameter custom_Supla_name("name", "Supla Device Name", Supla_name, 51,"required");
  WiFiManagerParameter custom_update_username("updateUsername", "update username", update_username, 21,"required");
  WiFiManagerParameter custom_update_password("updatePassword", "update password", update_password, 21,"required");

  WiFiManager wifiManager;
  wifiManager.setBreakAfterConfig(true);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_Location_id);
  wifiManager.addParameter(&custom_Location_Pass);
  wifiManager.addParameter(&custom_Supla_name);
  wifiManager.addParameter(&custom_update_username);
  wifiManager.addParameter(&custom_update_password);

  wifiManager.setCustomHeadElement("<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>");
  wifiManager.setMinimumSignalQuality(8);
  wifiManager.setConfigPortalTimeout(timeout);

    if (!wifiManager.startConfigPortal("Supla_8_X_V")) {
      Serial.println("Not connected to WiFi but continuing anyway.");
    } else {
      
      Serial.println("connected...yeey :)");    //if you get here you have connected to the WiFi
    }
    //read updated parameters
    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(Location_id, custom_Location_id.getValue());
    strcpy(Location_Pass, custom_Location_Pass.getValue());
    strcpy(Supla_name, custom_Supla_name.getValue());
    strcpy(update_username, custom_update_username.getValue());
    strcpy(update_password, custom_update_password.getValue());
   
  WiFi.softAPdisconnect(true);   //  close AP
}
void get_voltage(int cvs) {
         if (cvs == 0){  
            digitalWrite(CD4051_c, LOW);digitalWrite(CD4051_b, LOW);digitalWrite(CD4051_a, LOW);
            delay(2);
            val = 0;
            for(int i = 0; i < 5; i++) {
            val += analogRead(A0);
            delay(1);
            }
            val = val / 5;
            Voltage_Value = val  * (30.0 / 1023.0);
            Voltage_Value = round(Voltage_Value * 100.0) / 100.0;
            if (Voltage_Value != Voltage_0_Send){ 
            Voltage_0_Send = Voltage_Value;
            Serial.print("send voltage 0 = ");
            Serial.println(Voltage_0_Send,2);
            SuplaDevice.channelDoubleValueChanged(0, Voltage_0_Send);       
            }yield();}
            
        if (cvs == 1){
            digitalWrite(CD4051_c, LOW);digitalWrite(CD4051_b, LOW);digitalWrite(CD4051_a, HIGH);
            delay(2);
            val = 0;
            for(int i = 0; i < 5; i++) {
            val += analogRead(A0);
            delay(1);
            }
            val = val / 5;
            Voltage_Value = val  * (30.0 / 1023.0);
            Voltage_Value = round(Voltage_Value * 100.0) / 100.0;
            Voltage_Value = Voltage_Value - Voltage_0_Send;
            if (Voltage_Value != Voltage_1_Send){ 
            Voltage_1_Send = Voltage_Value;
            Serial.print("send voltage 1 = ");
            Serial.println(Voltage_1_Send,2);
            SuplaDevice.channelDoubleValueChanged(1, Voltage_1_Send);    
            }yield();}
            
        if (cvs == 2){
            digitalWrite(CD4051_c, LOW);digitalWrite(CD4051_b, HIGH);digitalWrite(CD4051_a, LOW);
            delay(2);
            val = 0;
            for(int i = 0; i < 5; i++) {
            val += analogRead(A0);
            delay(1);
            }
            val = val / 5;
            Voltage_Value = val  * (30.0 / 1023.0);
            Voltage_Value = round(Voltage_Value * 100.0) / 100.0;
            Voltage_Value = Voltage_Value - Voltage_0_Send - Voltage_1_Send;
            if (Voltage_Value != Voltage_2_Send){ 
            Voltage_2_Send = Voltage_Value;
            Serial.print("send voltage 2 = ");
            Serial.println(Voltage_2_Send,2);
            SuplaDevice.channelDoubleValueChanged(2, Voltage_2_Send); 
            }yield();}
            
        if (cvs == 3){
            digitalWrite(CD4051_c, LOW);digitalWrite(CD4051_b, HIGH);digitalWrite(CD4051_a, HIGH);
            delay(2);
            val = 0;
            for(int i = 0; i < 5; i++) {
            val += analogRead(A0);
            delay(1);
            }
            val = val / 5;
            Voltage_Value = val  * (30.0 / 1023.0);
            Voltage_Value = round(Voltage_Value * 100.0) / 100.0;
            Voltage_Value = Voltage_Value - Voltage_0_Send - Voltage_1_Send - Voltage_2_Send;
            if (Voltage_Value != Voltage_3_Send){ 
            Voltage_3_Send = Voltage_Value;
            Serial.print("send voltage 3 = ");
            Serial.println(Voltage_3_Send,2);
            SuplaDevice.channelDoubleValueChanged(3, Voltage_3_Send); 
           }yield();}
            
        if (cvs == 4){
            digitalWrite(CD4051_c, HIGH);digitalWrite(CD4051_b, LOW);digitalWrite(CD4051_a, LOW);
            delay(2);
            val = 0;
            for(int i = 0; i < 5; i++) {
            val += analogRead(A0);
            delay(1);
            }
            val = val / 5;
            Voltage_Value = val  * (30.0 / 1023.0);
            Voltage_Value = round(Voltage_Value * 100.0) / 100.0;
            Voltage_Value = Voltage_Value - Voltage_0_Send - Voltage_1_Send - Voltage_2_Send - Voltage_3_Send;
            if (Voltage_Value != Voltage_4_Send){ 
            Voltage_4_Send = Voltage_Value;
            Serial.print("send voltage 4 = ");
            Serial.println(Voltage_4_Send,2);
            SuplaDevice.channelDoubleValueChanged(4, Voltage_4_Send); 
            }yield();}
            
        if (cvs == 5){
            digitalWrite(CD4051_c, HIGH);digitalWrite(CD4051_b, LOW);digitalWrite(CD4051_a, HIGH);
            delay(2);
            val = 0;
            for(int i = 0; i < 5; i++) {
            val += analogRead(A0);
            delay(1);
            }
            val = val / 5;
            Voltage_Value = val  * (30.0 / 1023.0);
            Voltage_Value = round(Voltage_Value * 100.0) / 100.0;
            Voltage_Value = Voltage_Value - Voltage_0_Send - Voltage_1_Send - Voltage_2_Send - Voltage_3_Send - Voltage_4_Send;
            if (Voltage_Value != Voltage_5_Send){ 
            Voltage_5_Send = Voltage_Value;
            Serial.print("send voltage 5 = ");
            Serial.println(Voltage_5_Send,2);
            SuplaDevice.channelDoubleValueChanged(5, Voltage_5_Send); 
            }yield();}
            
        if (cvs == 6){
            digitalWrite(CD4051_c, HIGH);digitalWrite(CD4051_b, HIGH);digitalWrite(CD4051_a, LOW);
            delay(2);
            val = 0;
            for(int i = 0; i < 5; i++) {
            val += analogRead(A0);
            delay(1);
            }
            val = val / 5;
            Voltage_Value = val  * (30.0 / 1023.0);
            Voltage_Value = round(Voltage_Value * 100.0) / 100.0;
            Voltage_Value = Voltage_Value - Voltage_0_Send - Voltage_1_Send - Voltage_2_Send - Voltage_3_Send - Voltage_4_Send - Voltage_5_Send;
            if (Voltage_Value != Voltage_6_Send){ 
            Voltage_6_Send = Voltage_Value;
            Serial.print("send voltage 6 = ");
            Serial.println(Voltage_6_Send,2);
            SuplaDevice.channelDoubleValueChanged(6, Voltage_6_Send);
            }yield();}
            
        if (cvs == 7){
            digitalWrite(CD4051_c, HIGH);digitalWrite(CD4051_b, HIGH);digitalWrite(CD4051_a, LOW);
            delay(2);
            val = 0;
            for(int i = 0; i < 5; i++) {
            val += analogRead(A0);
            delay(1);
            }
            val = val / 5;
            Voltage_Value = val  * (30.0 / 1023.0);
            Voltage_Value = round(Voltage_Value * 100.0) / 100.0;
            if (Voltage_Value != Voltage_7_Send){ 
            Voltage_7_Send = Voltage_Value;
            Serial.print("send voltage 7 = ");
            Serial.println(Voltage_7_Send,2);
            SuplaDevice.channelDoubleValueChanged(7, Voltage_7_Send); 
            }yield();}
            
}
void status_func(int status, const char *msg) {     //    ------------------------ Status --------------------------
 s=status;                                          //    -- to check if we are registered and ready before restore from memory
}
void setup() {  //------------------------------------------------ Setup ----------------------------------------------

  wifi_set_sleep_type(NONE_SLEEP_T);

  Serial.begin(115200);
  delay(200);
  pinMode(status_led,OUTPUT); 
  pinMode(CD4051_a,OUTPUT); 
  pinMode(CD4051_b,OUTPUT); 
  pinMode(CD4051_c,OUTPUT); 
  digitalWrite(status_led, HIGH);

  if (WiFi.SSID()==""){
    initialConfig = true;
  } 
  
  WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.

   uint8_t mac[WL_MAC_ADDR_LENGTH];
   WiFi.macAddress(mac);
   char GUID[SUPLA_GUID_SIZE] = {mac[WL_MAC_ADDR_LENGTH - 6], mac[WL_MAC_ADDR_LENGTH - 5], mac[WL_MAC_ADDR_LENGTH - 4], mac[WL_MAC_ADDR_LENGTH - 3],                                
                                 mac[WL_MAC_ADDR_LENGTH - 2], mac[WL_MAC_ADDR_LENGTH - 1], mac[WL_MAC_ADDR_LENGTH - 1], mac[WL_MAC_ADDR_LENGTH - 2], 
                                 mac[WL_MAC_ADDR_LENGTH - 3], mac[WL_MAC_ADDR_LENGTH - 4], mac[WL_MAC_ADDR_LENGTH - 5], mac[WL_MAC_ADDR_LENGTH - 6]};
          
  SuplaDevice.addDS18B20Thermometer(); 
  SuplaDevice.addDS18B20Thermometer(); 
  SuplaDevice.addDS18B20Thermometer(); 
  SuplaDevice.addDS18B20Thermometer();
  SuplaDevice.addDS18B20Thermometer(); 
  SuplaDevice.addDS18B20Thermometer(); 
  SuplaDevice.addDS18B20Thermometer(); 
  SuplaDevice.addDS18B20Thermometer();     

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;         
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);   //print config data to serial on startup
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(Supla_server, json["Supla_server"]);
          strcpy(Location_id, json["Location_id"]);
          strcpy(Location_Pass, json["Location_Pass"]);
          strcpy(Supla_name, json["Supla_name"]);         
          strcpy(update_username, json["update_username"]);
          strcpy(update_password, json["update_password"]);
        } else {
          Serial.println("failed to load json config");
          initialConfig = true;
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  wifi_station_set_hostname(Supla_name);  //nazwa w sieci lokalnej  @cino111

  SuplaDevice.setStatusFuncImpl(&status_func);    //   ----------------------------- Status -----------------------------
  SuplaDevice.setName(Supla_name);

  int LocationID = atoi(Location_id);
  SuplaDevice.begin(GUID,              // Global Unique Identifier 
                    mac,               // Ethernet MAC address
                    Supla_server,      // SUPLA server address
                    LocationID,        // Location ID 
                    Location_Pass);    // Location Password                           
}

void loop() {
 
  if (initialConfig){
    ondemandwifiCallback () ;
  }
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Supla_server"] = Supla_server;
    json["Location_id"] = Location_id;
    json["Location_Pass"] = Location_Pass;
    json["Supla_name"] = Supla_name;
    json["update_username"] = update_username;
    json["update_password"] = update_password;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    Serial.println("config saved");
    shouldSaveConfig = false;
    initialConfig = false; 
    WiFi.mode(WIFI_STA);
    ESP.restart();
    delay(5000); 
  }
     
  int C_W_read = digitalRead(config_pin);{  
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
   
   if (WiFi.status() == WL_CONNECTED){
    SuplaDevice.iterate();      
    if (pr_wifi == true){
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
     pr_wifi = false;       
     httpUpdater.setup(&httpServer, update_path, update_username, update_password);
     httpServer.begin();
     delay(50);
    }
    if (pr_wifi == false){
     httpServer.handleClient(); 
     yield();
     if (s == 17){
     if (millis() > voltage_lasttime + voltage_mtbs)  {    //--------------Voltage callback--------------------
          if (epr>7){epr = 0;}
          get_voltage(epr);
          epr = epr+1;
          voltage_lasttime = millis();
          }
     }else {
       Voltage_0_Send = -1;Voltage_1_Send = -1;Voltage_2_Send = -1;Voltage_3_Send = -1;Voltage_4_Send = -1;Voltage_5_Send = -1;Voltage_6_Send = -1;Voltage_7_Send = -1; 
     }
    }
   }else{
    WiFi_up();
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

void WiFi_up(){ // conect to wifi
 
  if (millis() > wifimilis)  {
  WiFi.begin();
  delay(500);
  pr_wifi = true;
  Serial.println("CONNECTING WIFI"); 
  wifimilis = (millis() + wifi_checkDelay) ;
  }
  
}

