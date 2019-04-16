#include <FS.h>       // ---- esp board manager 2.4.2 --- iwip Variant V2 higher Bandwidth
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#define SUPLADEVICE_CPP
#include <SuplaDevice.h>
#include <DNSServer.h>
#include <WiFiManager.h>     //--------- https://github.com/tzapu/WiFiManager/tree/0.14 -------------
#include <ArduinoJson.h>     //--------- https://github.com/bblanchon/ArduinoJson/tree/v5.13.2 ------
#include <Ticker.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESPEFC.h>  // modification of ESP8266HTTPUpdateServer that includes erases flash and wifi credentials 
extern "C"
{
#include "user_interface.h"
}
#define onboard_led 2      //D4           status led
#define TRIGGER_PIN 0      // D3     wifi configuration pin --------pin konfiguracji wifi
#define zeroCal_PIN 14 // D5    MPX zero Calibration pin -------Pin kalibracji zerowej MPX
WiFiClient client;
ESP8266WebServer httpServer(81);
ESPEFC httpUpdater;
const char* host = "supla";
Ticker ticker;
unsigned long wifi_checkDelay = 30000;
unsigned long wifimilis; 
int C_W_state = HIGH; 
int last_C_W_state = HIGH;
unsigned long time_last_C_W_change = 0; 
long C_W_delay = 5000;               // config delay 5 seconds           ----------        opóźnienie konfiguracji 5 sekund
int MPX_mtbs = 15000;                // mean time between MPX update 
double MPX_tipo = 100.0;   
int mpx_zeroCal ;
int mpx_Tipo ;
unsigned long MPX_lasttime;   
double MPX_Value = 0;
double MPV_Send = -1;
double distance = 0;
double rssi = 0;
double rssi_last = 0;
int MPX_zero_offset = 0;
int mpx_Compensation =0;
int s;                   //             ---------------- Status ------------------
int timeout           = 120;
char Supla_server[40];
char Location_id[15];
char Location_Pass[34];
char MPX_Compensation[5];
char MPX_Tipo[5];
char Supla_name[51];
char update_path[21];
char update_username[21];
char update_password[21];
byte mac[6];
bool pr_wifi = true;
bool shouldSaveConfig = false;
bool initialConfig = false;

void get_MPX(){
  int val = 0;
      for(int i = 0; i < 10; i++) {
      val += analogRead(A0);
      delay(1);
      }
      val = val / 10; 
  yield();
  MPX_Value = map(val,MPX_zero_offset,(mpx_Compensation+MPX_zero_offset),0,1000) / MPX_tipo;
  yield(); 
  Serial.print("depth: ");  
  Serial.print(MPX_Value,2);
  Serial.println(" m"); 
  yield();
  if (MPX_Value != MPV_Send){ 
    MPV_Send = MPX_Value;
    SuplaDevice.channelDoubleValueChanged(0, MPV_Send);       
  }
    digitalWrite(onboard_led, LOW);
    delay(100);
    digitalWrite(onboard_led, HIGH);
}
void tick(){
  int state = digitalRead(onboard_led);  
  digitalWrite(onboard_led, !state);  
}
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void ondemandwifiCallback () {
  ticker.attach(0.2, tick);
  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 40);
  WiFiManagerParameter custom_Location_id("ID", "Location_id", Location_id, 15);
  WiFiManagerParameter custom_Location_Pass("Password", "Location_Pass", Location_Pass, 34 );
  WiFiManagerParameter custom_MPX_Compensation("MPX_Compensation", "Compensation 500-1000", MPX_Compensation, 5);
  WiFiManagerParameter custom_MPX_Tipo("MPX_Tipo", "MPX Tipo 5010-5050-5100", MPX_Tipo, 5);
  WiFiManagerParameter custom_Supla_name("name", "Supla Device Name", Supla_name, 51,"required");
  WiFiManagerParameter custom_update_path("updatePath", "/xxxx update path", update_path, 21,"required");
  WiFiManagerParameter custom_update_username("updateUsername", "update username", update_username, 21,"required");
  WiFiManagerParameter custom_update_password("updatePassword", "update password", update_password, 21,"required");


  WiFiManager wifiManager;
  wifiManager.setBreakAfterConfig(true);
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_Location_id);
  wifiManager.addParameter(&custom_Location_Pass);
  wifiManager.addParameter(&custom_MPX_Compensation);
  wifiManager.addParameter(&custom_MPX_Tipo);
  wifiManager.addParameter(&custom_Supla_name);
  wifiManager.addParameter(&custom_update_path);
  wifiManager.addParameter(&custom_update_username);
  wifiManager.addParameter(&custom_update_password);

  wifiManager.setCustomHeadElement("<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>");

  wifiManager.setMinimumSignalQuality();

    wifiManager.setConfigPortalTimeout(timeout);

    if (!wifiManager.startConfigPortal("SuplaMPX")) {   //    ----wifi configuration ap name---
      Serial.println("Not connected to WiFi but continuing anyway.");
    } else {
      
      Serial.println("connected...yeey :)");    //if you get here you have connected to the WiFi
    }
    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(Location_id, custom_Location_id.getValue());
    strcpy(Location_Pass, custom_Location_Pass.getValue());
    strcpy(MPX_Compensation, custom_MPX_Compensation.getValue());
    strcpy(MPX_Tipo, custom_MPX_Tipo.getValue());
    strcpy(Supla_name, custom_Supla_name.getValue());
    strcpy(update_path, custom_update_path.getValue());
    strcpy(update_username, custom_update_username.getValue());
    strcpy(update_password, custom_update_password.getValue());
}
double get_temperature(int channelNumber, double rssi_last) {   //   --------------------------- New -----------------------------
         
         rssi = WiFi.RSSI();  // ---------------------Twój kod daje 
        delay(10);
         if (rssi != rssi_last){ 
          rssi_last = rssi;
          return rssi_last;
         }          
    }
double get_distance(int channelNumber, double distance) {
    delay(10);
    return MPV_Send;
    }
void status_func(int status, const char *msg) {     //    ------------------------ Status --------------------------
 s=status;                                          //    -- to check if we are registered and ready 
}
void setup() {    //    ------------------------ Setup --------------------------

  wifi_set_sleep_type(NONE_SLEEP_T);

  Serial.begin(115200);
  EEPROM.begin(128);  
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  pinMode(zeroCal_PIN, INPUT_PULLUP);
  pinMode(onboard_led, OUTPUT);
  
  if (WiFi.SSID()==""){
    initialConfig = true;
  }
  ticker.attach(0.8, tick);
  
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
          strcpy(MPX_Compensation, json["MPX_Compensation"]);
          strcpy(MPX_Tipo, json["MPX_Tipo"]);
          strcpy(Supla_name, json["Supla_name"]);         
          strcpy(update_path, json["update_path"]);
          strcpy(update_username, json["update_username"]);
          strcpy(update_password, json["update_password"]);
          mpx_Tipo = String(MPX_Tipo).toInt();
          if (mpx_Tipo == 5010){
           MPX_tipo = 1000.0; 
          }
          else if (mpx_Tipo == 5050){
           MPX_tipo = 200.0; 
          }
          else{
           MPX_tipo = 100.0; 
          }
          Serial.print("Mpx Tipo: ");
          Serial.println(MPX_tipo);
          mpx_Compensation = String(MPX_Compensation).toInt();
          Serial.print("Mpx Compensation: ");
          Serial.println(mpx_Compensation);
        } else {
          Serial.println("failed to load json config");          
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  MPX_zero_offset = EEPROM.read(1);
  Serial.print("Read MPX zero offset: ");
  Serial.println(MPX_zero_offset);

  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.macAddress(mac);
  char GUID[SUPLA_GUID_SIZE] = {mac[WL_MAC_ADDR_LENGTH - 6], mac[WL_MAC_ADDR_LENGTH - 5], mac[WL_MAC_ADDR_LENGTH - 4], mac[WL_MAC_ADDR_LENGTH - 3],                                
                                mac[WL_MAC_ADDR_LENGTH - 2], mac[WL_MAC_ADDR_LENGTH - 1], mac[WL_MAC_ADDR_LENGTH - 1], mac[WL_MAC_ADDR_LENGTH - 2], 
                                mac[WL_MAC_ADDR_LENGTH - 3], mac[WL_MAC_ADDR_LENGTH - 4], mac[WL_MAC_ADDR_LENGTH - 5], mac[WL_MAC_ADDR_LENGTH - 6]};
  
  SuplaDevice.addDistanceSensor();
  SuplaDevice.addDS18B20Thermometer();  // wyświetla wifi  
  SuplaDevice.setStatusFuncImpl(&status_func);    //   ----------------------------- Status -----------------------------
 
  SuplaDevice.setName(Supla_name);    // Supla device name

  int LocationID = atoi(Location_id);
  SuplaDevice.begin(GUID,              // Global Unique Identifier 
                    mac,               // Ethernet MAC address
                    Supla_server,      // SUPLA server address
                    LocationID,        // Location ID 
                    Location_Pass);    // Location Password
 WiFi.mode(WIFI_STA);
}

void loop() {
  
  if (WiFi.status() == WL_CONNECTED) 
  {    
    httpServer.handleClient();
  }
  
  
    switch (s) {    //    ------------------------------------------------ Status ------------------------------------
   case 9:      // --------------------- DISCONNECTED  ----------------------      
      break;
        
   case 17:      // -----     STATUS_REGISTERED_AND_READY
         
      if (millis() > MPX_lasttime + MPX_mtbs)  {    //--------------MPX callback--------------------
          get_MPX();
          MPX_lasttime = millis();
          } 
    break;        
  }

  if ( digitalRead(zeroCal_PIN) == LOW )  {
    int val = 0;
      for(int i = 0; i < 10; i++) {
      val += analogRead(A0);
      delay(1);
      }
      val = val / 10;
      MPX_zero_offset = val; 
    Serial.print("Write MPX zero offset: ");
    Serial.println(MPX_zero_offset);
    delay (1000);
    EEPROM.write(1, MPX_zero_offset); 
    EEPROM.commit();
  }

  
  int C_W_read = digitalRead(TRIGGER_PIN);{  
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

  if  (initialConfig)  {     
    ondemandwifiCallback () ;
    initialConfig = false; 
  }

  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Supla_server"] = Supla_server;
    json["Location_id"] = Location_id;
    json["Location_Pass"] = Location_Pass;
    json["MPX_Compensation"] = MPX_Compensation;
    json["MPX_Tipo"] = MPX_Tipo;
    json["Supla_name"] = Supla_name;
    json["update_path"] = update_path;
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
    WiFi.mode(WIFI_STA);
    ticker.detach();
    digitalWrite(onboard_led, HIGH);
    ESP.restart();
      delay(5000);
  }
  
  if (WiFi.status() != WL_CONNECTED) 
  {
    ticker.attach(0.8, tick);
    WiFi_up();
  }
  if (WiFi.status() != WL_CONNECTED) { 
    ticker.attach(0.8, tick);
    WiFi_up();
    pr_wifi = true;
  }
  else if ((WiFi.status() == WL_CONNECTED)  && (pr_wifi)){
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
    ticker.detach();
    digitalWrite(onboard_led, HIGH); 
    pr_wifi = false;
    MDNS.begin(host);
    httpUpdater.setup(&httpServer, update_path, update_username, update_password);
    httpServer.begin();
    MDNS.addService("http", "tcp", 81);
    Serial.printf("HTTPUpdateServer ready! Open http://%s.local:81%s in your browser and login with username '%s' and password '%s'\n", host, update_path, update_username, update_password);      
   }
  SuplaDevice.iterate();
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
          cb.get_temperature = &get_temperature;  // --------------------------- New ------------------------
          cb.get_temperature_and_humidity = NULL;
          cb.get_distance = &get_distance;
          return cb;
}

void WiFi_up(){ // conect to wifi
 
  if (millis() > wifimilis)  {
  WiFi.begin();
  Serial.println("CONNECTING WIFI"); 
  }
  wifimilis = (millis() + wifi_checkDelay) ;
}
