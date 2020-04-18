#include <FS.h>       // ---- esp board manager 2.4.2 --- iwip Variant V2 higher Bandwidth
#include <ESP8266WiFi.h>
#define CUSTOMSUPLA_CPP  //  V 1.6.2 C
#include "CustomSupla.h"
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h> 
#include <ArduinoJson.h> //--------- https://github.com/bblanchon/ArduinoJson/tree/v5.13.2 ------
#include <EEPROM.h>
#include <Ticker.h>      //for LED status
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>  // modification of ESP8266HTTPUpdateServer that includes erases flash and wifi credentials
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

Ticker ticker;
Ticker T_btn;
#define BTN_COUNT 2
#define BTN_wificonfig 2
#define relay_1 14
#define relay_2 15
#define button_1 13
#define button_2 12
#define status_led 4 
int btc_1 = 0;
int btc_2 = 0;
int C_W_state = HIGH; 
int last_C_W_state = HIGH;
unsigned long time_last_C_W_change = 0; 
long C_W_delay = 10000;               // config delay 10 seconds 
bool pr_wifi = true;
int s;              
unsigned long wifi_checkDelay = 60000; 
unsigned long wifimilis;
bool tikOn = false;
int inttmp0 = -1;
bool savers0 = false;
int inttmp1 = -1;
bool savers1 = false;
int Sp = -1;
const char* update_path = "/update";
char Supla_server[60];
char Location_id[15];
char Location_Pass[20];
char Supla_name[51];
char update_username[21];
char update_password[21];
char Supla_status[51];
byte mac[6];
bool shouldSaveConfig = false;
bool initialConfig = false;
int timeout  = 300;
WiFiClient client;
ESP8266WebServer httpServer(81);
ESP8266HTTPUpdateServer httpUpdater;

typedef struct {  //------------------------------------------- BTN ----------------------------------------------------
  int pin;
  int relay_pin;
  char last_val;
  char btp;
  unsigned long last_time;
} _btn_t;

_btn_t btn[BTN_COUNT];

ICACHE_RAM_ATTR void sense_1(){
    detachInterrupt(button_1);
  btc_1++;
    attachInterrupt(button_1, sense_1, RISING);
}
ICACHE_RAM_ATTR void sense_2(){
    detachInterrupt(button_2);
  btc_2++;
    attachInterrupt(button_2, sense_2, RISING);
}
void button_timer(){
  detachInterrupt(button_1);
  detachInterrupt(button_2);
  
  if (btc_1 > 8 ){
    btn[0].btp = 1;
  } else {
    btn[0].btp = 0;
  }
  btc_1 = 0;

    if (btc_2 > 8 ){
    btn[1].btp = 1;
  } else {
    btn[1].btp = 0;
  }
  btc_2 = 0;
  
   attachInterrupt(button_1, sense_1, RISING);
   attachInterrupt(button_2, sense_2, RISING);
}

void iterate_botton() {
  unsigned long now = millis();
  {
  for(int a=0;a<BTN_COUNT;a++)
      if (btn[a].pin > 0) {
        if (btn[a].btp != btn[a].last_val && now - btn[a].last_time ) {
           btn[a].last_val = btn[a].btp;
           btn[a].last_time = now + 100;
           if (btn[a].btp == 1)
             {
      if ( a == 0 ) {      
        if ( CustomSupla.rollerShutterMotorIsOn(0) ) {
          Serial.print("0 Stop");
            CustomSupla.rollerShutterStop(0);
        } else {
          Serial.print("0 Reveal");
            CustomSupla.rollerShutterReveal(0);
        }        
     }
       if ( a == 1 ) {      
        if ( CustomSupla.rollerShutterMotorIsOn(0) ) {
          Serial.print("0 Stop");
            CustomSupla.rollerShutterStop(0);
        } else {
          Serial.print("0 Shut");
            CustomSupla.rollerShutterShut(0);
        }        
     }         
         }
        }
      }
    }
}

void supla_btn_init() {
  
  for(int a=0;a<BTN_COUNT;a++)
    if (btn[a].pin > 0) {
        btn[a].last_time = millis();
        pinMode(btn[a].relay_pin-1,OUTPUT);
    }
}


void supla_rs_SavePosition(int channelNumber, int position) {
   
    if ((!digitalRead(relay_1))&&(!digitalRead(relay_2)) || (position <= 100 ) || (position >= 10100)){
      inttmp0 = position;
      savers0 = true;
     }
 
}

void supla_rs_LoadPosition(int channelNumber, int *position) {
  unsigned int R_pos = 0;
                   
      EEPROM.get(1, R_pos);
      *position = R_pos;
      Serial.print("read position R1: ");       
      Serial.println(*position);
   
}

void supla_rs_SendPosition(){
  unsigned int R_pos = 0;
   
         EEPROM.get(1, R_pos);
      CustomSupla.channelRSValueChanged(0, (R_pos-100)/100, 0, 1);
           
}

void supla_rs_SaveSettings(int channelNumber, unsigned int full_opening_time, unsigned int full_closing_time) {

       EEPROM.put(50, full_opening_time);
       Serial.println("R1 full opening time Saved"); 
       EEPROM.put(55, full_closing_time);
       Serial.println("R2 full closing time Saved");
       EEPROM.commit();
  
}

void supla_rs_LoadSettings(int channelNumber, unsigned int *full_opening_time, unsigned int *full_closing_time) {
  unsigned int R_opening_time = 0;
  unsigned int R_closing_time = 0;
 
    EEPROM.get(50, R_opening_time);
    EEPROM.get(55, R_closing_time);
    *full_opening_time = R_opening_time;
    *full_closing_time = R_closing_time;
    Serial.print("read R1--channel: ");Serial.print((channelNumber));Serial.print(" opening_time ");Serial.print((*full_opening_time));Serial.print(" closing_time ");Serial.println((*full_closing_time));   
            
}

void tick(){
  int state = digitalRead(status_led);  // get the current state
  digitalWrite(status_led, !state);     // set pin to the opposite state
}
void saveConfigCallback () {                 //callback notifying us of the need to save config
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void ondemandwifiCallback () {
  ticker.attach(0.8, tick);
  CustomSupla.StopTimer();  
  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 60);
  WiFiManagerParameter custom_Location_id("ID", "Location_id", Location_id, 15);
  WiFiManagerParameter custom_Location_Pass("Password", "Location_Pass", Location_Pass, 20);
  WiFiManagerParameter custom_Supla_name("name", "Supla Device Name", Supla_name, 51,"required");
  WiFiManagerParameter custom_update_username("updateUsername", "update username", update_username, 21,"required");
  WiFiManagerParameter custom_update_password("updatePassword", "update password", update_password, 21,"required");
  WiFiManagerParameter custom_Supla_status("status", "Supla Last State", Supla_status, 51,"readonly");

  WiFiManager wifiManager;
  wifiManager.setBreakAfterConfig(true);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_Location_id);
  wifiManager.addParameter(&custom_Location_Pass);
  wifiManager.addParameter(&custom_Supla_name);
  wifiManager.addParameter(&custom_update_username);
  wifiManager.addParameter(&custom_update_password);
  wifiManager.addParameter(&custom_Supla_status);

  wifiManager.setCustomHeadElement("<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>");

  wifiManager.setMinimumSignalQuality();
  wifiManager.setConfigPortalTimeout(timeout);

    if (!wifiManager.startConfigPortal("Moes_MS-104B_RS")) {
     Serial.println("Not connected to WiFi but continuing anyway.");
    } else {
      
      Serial.println("connected...yeey :)");    //if you get here you have connected to the WiFi
    }   
    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(Location_id, custom_Location_id.getValue());
    strcpy(Location_Pass, custom_Location_Pass.getValue());
    strcpy(Supla_name, custom_Supla_name.getValue());
    strcpy(update_username, custom_update_username.getValue());
    strcpy(update_password, custom_update_password.getValue());

    WiFi.softAPdisconnect(true);   //  close AP  
     
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

    File configFile = SPIFFS.open("/configV2.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    Serial.println("config saved");
    shouldSaveConfig = false;
    WiFi.mode(WIFI_STA);
    initialConfig = false; 
    ticker.detach();
    digitalWrite(status_led, LOW);
    ESP.restart();
    delay(5000); 
  }    
  CustomSupla.StartTimer();
      ticker.detach();
      digitalWrite(status_led, LOW);
      tikOn = false;    
}

void status_func(int status, const char *msg) { 
  if (s != status){
     s=status; 
     Serial.print("status: "); 
     Serial.println(s); 
         if (status != 10){
      strcpy(Supla_status, msg);
        }  
  }                                        
}

void setup() {  //------------------------------------------------ Setup ----------------------------------------------

  wifi_set_sleep_type(NONE_SLEEP_T);
  EEPROM.begin(512);
  Serial.begin(115200);
  SPIFFS.begin();
  pinMode(status_led,OUTPUT); 
  digitalWrite(status_led, LOW);
   
   pinMode(BTN_wificonfig, INPUT_PULLUP);
   pinMode(button_1, INPUT_PULLUP);
   pinMode(button_2, INPUT_PULLUP);
   
   attachInterrupt(button_1, sense_1, RISING);
   attachInterrupt(button_2, sense_2, RISING);
   delay(200);
   T_btn.attach(0.2, button_timer);
   delay(200);

  if (WiFi.SSID()==""){initialConfig = true;}
    
    Serial.println("mounting Fs...");
    if (SPIFFS.exists("/configV2.json")) {
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/configV2.json", "r");
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
          if (json.containsKey("Supla_server")) strcpy(Supla_server, json["Supla_server"]);
          if (json.containsKey("Location_id")) strcpy(Location_id, json["Location_id"]);
          if (json.containsKey("Location_Pass")) strcpy(Location_Pass, json["Location_Pass"]);
          if (json.containsKey("Supla_name")) strcpy(Supla_name, json["Supla_name"]);
          if (json.containsKey("update_username")) strcpy(update_username, json["update_username"]);
          if (json.containsKey("update_password")) strcpy(update_password, json["update_password"]);
        } else {
          Serial.println("failed to load json config");
          initialConfig = true;
        }
      }
    }
  
  button_timer();   
  memset(btn, 0, sizeof(btn));
  btn[0].pin =button_1 +1; 
  btn[0].last_val = btn[0].btp;        
  btn[0].relay_pin =relay_1 +1;  
  btn[1].pin =button_2 +1;
  btn[1].last_val = btn[1].btp;
  btn[1].relay_pin =relay_2 +1;  
  supla_btn_init();
  
  WiFi.mode(WIFI_STA); 

   uint8_t mac[WL_MAC_ADDR_LENGTH];
   WiFi.macAddress(mac);
   char GUID[SUPLA_GUID_SIZE] = {mac[WL_MAC_ADDR_LENGTH - 3], mac[WL_MAC_ADDR_LENGTH - 5], mac[WL_MAC_ADDR_LENGTH - 4], mac[WL_MAC_ADDR_LENGTH - 3],                                
                                 mac[WL_MAC_ADDR_LENGTH - 2], mac[WL_MAC_ADDR_LENGTH - 0], mac[WL_MAC_ADDR_LENGTH - 1], mac[WL_MAC_ADDR_LENGTH - 2], 
                                 mac[WL_MAC_ADDR_LENGTH - 3], mac[WL_MAC_ADDR_LENGTH - 4], mac[WL_MAC_ADDR_LENGTH - 5], mac[WL_MAC_ADDR_LENGTH - 3]};
          
  CustomSupla.addRollerShutterRelays(relay_1, relay_2, false); 
 
  CustomSupla.setStatusFuncImpl(&status_func);   
  CustomSupla.setRollerShutterFuncImpl(&supla_rs_SavePosition, &supla_rs_LoadPosition, &supla_rs_SaveSettings, &supla_rs_LoadSettings);
  CustomSupla.setName(Supla_name);

  int LocationID = atoi(Location_id);
  CustomSupla.begin(GUID,              // Global Unique Identifier 
                    mac,               // Ethernet MAC address
                    Supla_server,      // SUPLA server address
                    LocationID,        // Location ID 
                    Location_Pass);    // Location Password
        
}

void loop() {


  if (initialConfig){
    EEPROM.put(50, 0);EEPROM.put(55, 0);EEPROM.commit();  // CLR EEprom 
    ondemandwifiCallback () ;
  } 

      if (savers0){Serial.print("write position R1:");Serial.println(inttmp0);EEPROM.put(1, inttmp0);EEPROM.commit();savers0 = false;}            
       
  if (WiFi.status() != WL_CONNECTED) { 
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
    pr_wifi = false;   
    httpUpdater.setup(&httpServer, update_path, update_username, update_password);
    httpServer.begin();
  } else { httpServer.handleClient();}

  int C_W_read = digitalRead(BTN_wificonfig);{  
   if (C_W_read != last_C_W_state) {time_last_C_W_change = millis(); }
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
     
  CustomSupla.iterate();
  delay(22);
  iterate_botton(); 
 
  if (s == 17){      // -----     STATUS_REGISTERED_AND_READY
      if (tikOn){            
      ticker.detach();
      digitalWrite(status_led, LOW);
      supla_rs_SendPosition();
      tikOn = false;}
  }else if (!tikOn){
      //ticker.attach(0.8, tick);
      tikOn = true;
    }   
  if (Sp != s){
      Serial.print("Supla State: ");
      Serial.println(s);
      Sp = s;  
      }

  if (s != 17){
    CustomSupla.iterateOfline();
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

CustomSuplaCallbacks supla_arduino_get_callbacks(void) {
          CustomSuplaCallbacks cb;
          
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
 unsigned long ahora = millis();
  if (ahora >= wifimilis)  {
  Serial.println("CONNECTING WIFI"); 
  WiFi.begin();
  yield();
  wifimilis = (millis() + wifi_checkDelay) ;  
  }
}
