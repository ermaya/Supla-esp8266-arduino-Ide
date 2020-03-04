/*
Copyright (C) AC SOFTWARE SP. Z O.O.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <FS.h>       // ---- esp board manager 2.6.3 --- iwip Variant V2 Lower Memory 
#include <SPI.h>
#include <SuplaDevice.h>  // SoftVer, "2.3.1" "custom"
#include <io.h>
#include <supla/sensor/DS18B20.h>
#include <WiFiManager.h>
#include <ArduinoJson.h> //--------- https://github.com/bblanchon/ArduinoJson/tree/v5.13.2 ------
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266TrueRandom.h>
#include <WiFiClientSecure.h>
#include <supla/network/esp_wifi.h>
Supla::ESPWifi wifi("", "");  //------ Do not change----wifimanager takes care------
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

char Supla_server[81]=("Set server address");
char Email[81]=("set email address");
char Supla_name[51]=("Supla_LC4");
char Supla_status[51];
bool shouldSaveConfig = false;
bool initialConfig = false;
bool state10 = true;
bool starting = true;
uint8_t an;
int s;
char GUID[SUPLA_GUID_SIZE];
char AUTHKEY[SUPLA_AUTHKEY_SIZE];
byte uuidNumber[16]; // UUIDs in binary form are 16 bytes long
static const char logo[] PROGMEM = "<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>";
byte relay_1_on[] = {0xA0, 0x01, 0x01, 0xA2};
byte relay_1_off[] = {0xA0, 0x01, 0x00, 0xA1};
byte relay_2_on[] = {0xA0, 0x02, 0x01, 0xA3};
byte relay_2_off[] = {0xA0, 0x02, 0x00, 0xA2};
byte relay_3_on[] = {0xA0, 0x03, 0x01, 0xA4};
byte relay_3_off[] = {0xA0, 0x03, 0x00, 0xA3};
byte relay_4_on[] = {0xA0, 0x04, 0x01, 0xA5};
byte relay_4_off[] = {0xA0, 0x04, 0x00, 0xA4};
unsigned long rele_milis = 0;
ESP8266WebServer httpServer(81);
ESP8266HTTPUpdateServer httpUpdater;
WiFiManager wifiManager;

class LC_4x : public Supla::Io {
  public:
    void customDigitalWrite(int channelNumber, uint8_t pin, uint8_t val) {
      if ((pin == 100)){
        if (val == 1){
          Serial.write(relay_1_on, sizeof(relay_1_on));
          delay(20);
        }else{
          Serial.write(relay_1_off, sizeof(relay_1_off));
          delay(20);
        }
         if (state10 == false){ 
             EEPROM.write(pin, val);
             EEPROM.commit();
         }return;}
      if ((pin == 101)){
        if (val == 1){
          Serial.write(relay_2_on, sizeof(relay_2_on)); 
          delay(20);
        }else{
          Serial.write(relay_2_off, sizeof(relay_2_off));  
          delay(20);
        }
         if (state10 == false){ 
             EEPROM.write(pin, val);
             EEPROM.commit();
         }return;}         
      if ((pin == 102)){
        if (val == 1){
          Serial.write(relay_3_on, sizeof(relay_3_on)); 
          delay(20);
        }else{
          Serial.write(relay_3_off, sizeof(relay_3_off)); 
          delay(20);
        }
         if (state10 == false){ 
             EEPROM.write(pin, val);
             EEPROM.commit();
         }return;}
      if ((pin == 103)){
        if (val == 1){
          Serial.write(relay_4_on, sizeof(relay_4_on));  
          delay(20);
        }else{
          Serial.write(relay_4_off, sizeof(relay_4_off)); 
          delay(20);
        }
         if (state10 == false){ 
             EEPROM.write(pin, val);
             EEPROM.commit();
         }return;}               
      if (pin <= 99) {
        return ::digitalWrite(pin,val);   
      }
   }
   
   int customDigitalRead(int channelNumber, uint8_t pin) {
      if ((pin >= 100)&& (pin <= 103)){
        return EEPROM.read(pin);     
      }     
      if (pin <= 99){
        return ::digitalRead(pin);  
      }
    }   
}LC_4x;

void saveConfigCallback () {          
 // Serial.println("Should save config");
  shouldSaveConfig = true;
}
void ondemandwifiCallback () {
   delay(2000);Serial.println("WIFI DISCONNECT");delay(20);
   WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 81,"required");
   WiFiManagerParameter custom_Email("email", "Email", Email, 81,"required");
   WiFiManagerParameter custom_Supla_name("name", "Supla Device Name", Supla_name, 51,"required");
   WiFiManagerParameter custom_Supla_status("status", "Supla Last State", Supla_status, 51,"readonly");
   
   wifiManager.setBreakAfterConfig(true);
   wifiManager.setSaveConfigCallback(saveConfigCallback);
  
   wifiManager.addParameter(&custom_Supla_server);
   wifiManager.addParameter(&custom_Email);
   wifiManager.addParameter(&custom_Supla_name);
   wifiManager.addParameter(&custom_Supla_status);

   wifiManager.setCustomHeadElement(logo);
   wifiManager.setMinimumSignalQuality(8);
   //wifiManager.setShowStaticFields(true); // force show static ip fields
   //wifiManager.setShowDnsFields(true);    // force show dns field always
   wifiManager.setConfigPortalTimeout(300);
   delay(2000);Serial.println("WIFI DISCONNECT");delay(20);
   if (!wifiManager.startConfigPortal("Supla_LC4")) { Serial.println("Not connected to WiFi but continuing anyway.");delay(20);Serial.println("WIFI GOT IP");delay(20);} else { Serial.println("connected...yeey :)");delay(20);Serial.println("WIFI GOT IP");delay(20);}                
    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(Email, custom_Email.getValue());
    strcpy(Supla_name, custom_Supla_name.getValue());
    if(strcmp(Supla_server, "get_new_guid_and_authkey") == 0){
      //Serial.println("new guid & authkey.");
      EEPROM.write(300, 0);
      EEPROM.commit();
      delay(100);
      ESP.reset(); 
    }  
    WiFi.softAPdisconnect(true);   //  close AP
}
void status_func(int status, const char *msg) {    //    ------------------------ Status --------------------------
  if (s != status){
    s = status; 
      if (s != 10){
        strcpy(Supla_status, msg); 
  }  }            
}
void guid_authkey(void) {
  if (EEPROM.read(300) != 60){
    int eep_gui = 301;
    ESP8266TrueRandom.uuid(uuidNumber);
    String uuidString = "";
    for (int i = 0; i < 16; i++) {
      int topDigit = uuidNumber[i] >> 4;
      int bottomDigit = uuidNumber[i] & 0x0f;
      uuidString += "0123456789abcdef"[topDigit];
      uuidString += "0123456789abcdef"[bottomDigit];
    }
    int length_uuid = uuidString.length();
    for (int i = 0; i < length_uuid; ++i) {
      EEPROM.put(eep_gui + i, uuidString[i]);
    }
    int eep_aut = 341;
    ESP8266TrueRandom.uuid(uuidNumber);
    String uuidString2 = "";
    for (int i = 0; i < 16; i++) {
      int topDigit = uuidNumber[i] >> 4;
      int bottomDigit = uuidNumber[i] & 0x0f;
      uuidString2 += "0123456789abcdef"[topDigit];
      uuidString2 += "0123456789abcdef"[bottomDigit];
    }
    int length_uuid2 = uuidString2.length();
    for (int i = 0; i < length_uuid2; ++i) {
      EEPROM.put(eep_aut + i, uuidString2[i]);
    }
    EEPROM.write(300, 60);
    EEPROM.commit();
    delay(0);
  }
  read_guid();
  read_authkey();
  //Serial.print("GUID : ");Serial.println(read_guid()); 
  //Serial.print("AUTHKEY : ");Serial.println(read_authkey()); 
}
String read_guid(void) {
  String read_eeprom = "";
  int i, ii = 0;
  int eep_star = 301;
  int end_guid = eep_star + SUPLA_GUID_SIZE;
  String temp_read = "0x";
  for (i = eep_star; i < end_guid + 16;  i = i + 1) {
    temp_read += char(EEPROM.read(i));
    read_eeprom += char(EEPROM.read(i));
    if ( (i % 2) == 0) {
      char *_guid = strcpy((char*)malloc(temp_read.length() + 1), temp_read.c_str());
      GUID[ii] = strtoul( _guid, NULL, 16);
      temp_read = "0x";
      ii++;
    }
  }
  return read_eeprom;
}
String read_authkey(void) {
  String read_eeprom = "";
  int i, ii = 0;
  int eep_star = 341;
  int end_authkey = eep_star + SUPLA_AUTHKEY_SIZE;
  String temp_read = "0x";
  for (i = eep_star; i < end_authkey + 16;  i = i + 1) {
    temp_read += char(EEPROM.read(i));
    read_eeprom += char(EEPROM.read(i));
    if ( (i % 2) == 0) {
      char *_authkey = strcpy((char*)malloc(temp_read.length() + 1), temp_read.c_str());
      AUTHKEY[ii] = strtoul( _authkey, NULL, 16);
      temp_read = "0x";
      ii++;
    }
  }
  return read_eeprom;
}

void setup() {
  wifi_set_sleep_type(NONE_SLEEP_T);
   Serial.begin(115200);
   delay(00);
  EEPROM.begin(1024);  
  if (EEPROM.read(300) != 60){initialConfig = true;} 
  guid_authkey();
  delay(20);
  Serial.println("WIFI GOT IP");
  delay(20);
  Serial.println("0,CLOSED");
  delay(20);  
  if (WiFi.SSID()==""){ initialConfig = true;} 

  if (SPIFFS.begin()) {  // ------------------------- wificonfig read -----------------
    //Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
     // Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        //Serial.println("opened config file");
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;         
        JsonObject& json = jsonBuffer.parseObject(buf.get());
       // json.printTo(Serial);   //print config data to serial on startup
        if (json.success()) {//Serial.println("\nparsed json");         
          const char* configSupla_server = json["Supla_server"];if (configSupla_server != nullptr){strcpy(Supla_server, json["Supla_server"]); }else{initialConfig = true;}
          const char* configEmail = json["Email"];if (configEmail != nullptr){strcpy(Email, json["Email"]); }else{initialConfig = true;}
          const char* configSupla_name = json["Supla_name"];if (configSupla_name != nullptr){strcpy(Supla_name, json["Supla_name"]); }else{initialConfig = true;}          
        } else {
          //Serial.println("failed to load json config");
          initialConfig = true;
        }
      }
    }
   } else {
    //Serial.println("failed to mount FS");
  }
  
   wifi_station_set_hostname(Supla_name);   
   WiFi.mode(WIFI_STA); 
   
      SuplaDevice.addRelay(100, false);
      SuplaDevice.addRelay(101, false); 
      SuplaDevice.addRelay(102, false); 
      SuplaDevice.addRelay(103, false); 

      new Supla::Sensor::DS18B20(2);
 

    SuplaDevice.setName(Supla_name);
    SuplaDevice.setStatusFuncImpl(&status_func);
    wifi.enableSSL(false);

    SuplaDevice.begin(GUID,Supla_server,Email,AUTHKEY);
    

          
}

void loop() {
  if (initialConfig == true){ondemandwifiCallback();}

  if (rele_milis < millis()){
    if (EEPROM.read(100) != 0){delay(20);Serial.write(relay_1_on, sizeof(relay_1_on));delay(20);}else{delay(20);Serial.write(relay_1_off, sizeof(relay_1_off));delay(20);}
    if (EEPROM.read(101) != 0){delay(20);Serial.write(relay_2_on, sizeof(relay_2_on));delay(20);}else{delay(20);Serial.write(relay_2_off, sizeof(relay_2_off));delay(20);}
    if (EEPROM.read(102) != 0){delay(20);Serial.write(relay_3_on, sizeof(relay_3_on));delay(20);}else{delay(20);Serial.write(relay_3_off, sizeof(relay_3_off));delay(20);}
    if (EEPROM.read(103) != 0){delay(20);Serial.write(relay_4_on, sizeof(relay_4_on));delay(20);}else{delay(20);Serial.write(relay_4_off, sizeof(relay_4_off));delay(20);}
    if (s == 17) {Serial.println("0,CONNECT");delay(20);}else{;Serial.println("0,CLOSED");delay(20); } 
    rele_milis = millis() +5000;    
  }
      
  if (shouldSaveConfig == true) { // ------------------------ wificonfig save --------------
   // Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Supla_server"] = Supla_server;
    json["Email"] = Email;
    json["Supla_name"] = Supla_name;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) { }   
   // json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
   // Serial.println("config saved");    
    shouldSaveConfig = false;
    initialConfig = false; 
    WiFi.mode(WIFI_STA);   
    delay(5000);
    ESP.restart(); 
  }
  
  SuplaDevice.iterate();
   delay(50);

     if (WiFi.status() == WL_CONNECTED){
    if (starting){
      httpUpdater.setup(&httpServer, "/update", "admin", "pass");
      httpServer.begin(); 
      starting = false;         
     }
     httpServer.handleClient();
   }

  if (Serial.available())
   {
    String S_data = Serial.readString();
    //Serial.print("Serial: ");Serial.println(S_data);
    if (S_data.indexOf("RESTORE") >0) {initialConfig = true;Serial.flush();}  
   }
   
   switch (s) { 
    case 17:      // -----     STATUS_REGISTERED_AND_READY
     if (state10 == true){
      for (int i = 0; i < 4; i++) {
         if (EEPROM.read(i +100) >= 1){
          SuplaDevice.relayOn(i, 0);
          delay(20);
         }
      }            
      state10 = false; 
     }   
     break;     
    case 10:      // --------------------- REGISTER_IN_PROGRESS  ----------------------
     state10 = true;
     break;  
  }

}
