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

#include <FS.h>       // ---- esp board manager 2.5.2 --- iwip Variant V2 higher Bandwidth
#include <SPI.h>
#include <SuplaDevice.h>
#include <SuplaImpulseCounter.h>
#include <io.h>
#include <WiFiManager.h>
#include <ArduinoJson.h> //--------- https://github.com/bblanchon/ArduinoJson/tree/v5.13.2 ------
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <Ticker.h>      //for LED status
#include <ESP8266TrueRandom.h>
#include <HLW8012.h>
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
#define PUYA_SUPPORT //PUYASUPPORT
#define SEL_PIN    12
#define CF1_PIN    5
#define CF_PIN     4
#define status_led 13   // D7
int wificonfig_pin = 3; // RX
int relay_pin = 14;     // D5
int C_W_state = HIGH; 
int last_C_W_state = HIGH;
unsigned long time_last_C_W_change = 0; 
long C_W_delay = 10000;                      // config delay 10 seconds     
int C_W_state2 = HIGH; 
long C_W_delay2 = 100;
unsigned long eep_milis = 10000;
bool recover = true;                       
char Supla_server[81];
char Email[81];
char Supla_name[51];
char Supla_status[51];
char Volt[8];
char Wats[8];
byte mac[6];
bool shouldSaveConfig = false;
bool initialConfig = false;
int s = 0;
bool starting = true;
int lst_energy;
int customFieldLength = 40;
WiFiManagerParameter custom_field; // global param ( for non blocking w params )
char GUID[SUPLA_GUID_SIZE];
char AUTHKEY[SUPLA_AUTHKEY_SIZE];
byte uuidNumber[16]; // UUIDs in binary form are 16 bytes long
String fs_file ="/DGM.json";  // config file name

ESP8266WebServer httpServer(81);
ESP8266HTTPUpdateServer httpUpdater;
WiFiManager wifiManager;
Ticker ticker;
#include <supla/network/esp_wifi.h>
Supla::ESPWifi wifi("", "");  //------ Do not change----wifimanager takes care------

#define CURRENT_MODE                    LOW
#define HJL01_CURRENT_RATIO             25740
#define HJL01_VOLTAGE_RATIO             313400
#define HJL01_POWER_RATIO               3414290
HLW8012 hjl01;

void tick() {
  int state = digitalRead(status_led);  
  digitalWrite(status_led, !state);     
}
void ICACHE_RAM_ATTR hjl01_cf1_interrupt() {
    hjl01.cf1_interrupt();
}
void ICACHE_RAM_ATTR hjl01_cf_interrupt() {
    hjl01.cf_interrupt();
}
void calibrate_hj101() {

   int ex_volt = atoi(Volt); 
   int ex_pow = atoi(Wats);
   double ex_amps = ((float)ex_pow / (float)ex_volt);
   Serial.print("volt:"); Serial.println(ex_volt); 
   Serial.print("wats:"); Serial.println(ex_pow); 
   Serial.print("amps:"); Serial.println(ex_amps); 
   digitalWrite(relay_pin,HIGH); 
   Serial.println("Relay On:");    

    unsigned long timeout1 = millis();
    while ((millis() - timeout1) < 10000) { delay(10);}
          
    hjl01.expectedActivePower(ex_pow);
    hjl01.expectedVoltage(ex_volt);
    hjl01.expectedCurrent(ex_amps);

    unsigned long timeout2 = millis();
    while ((millis() - timeout2) < 2000) {delay(10);}
           
    double current_multi = hjl01.getCurrentMultiplier();
    double voltage_multi = hjl01.getVoltageMultiplier();
    double power_multi = hjl01.getPowerMultiplier();

    Serial.print("[HLW] New current multiplier : "); Serial.println(current_multi);
    Serial.print("[HLW] New voltage multiplier : "); Serial.println(voltage_multi);
    Serial.print("[HLW] New power multiplier   : "); Serial.println(power_multi);

    EEPROM.put(400, current_multi);
    EEPROM.put(410, voltage_multi);
    EEPROM.put(420, power_multi);
    EEPROM.write(430, 60);

  EEPROM.commit();
  yield();
}
void get_hj101_config(){
  if (EEPROM.read(430) == 60){
      Serial.println("get eep multiplier ");
       double p_m;double v_m;double c_m;
        EEPROM.get(400,c_m);
         EEPROM.get(410,v_m);
          EEPROM.get(420,p_m);
           hjl01.setCurrentMultiplier(c_m);
            hjl01.setVoltageMultiplier(v_m);
             hjl01.setPowerMultiplier(p_m);
   }
}
void saveConfigCallback () {          
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void ondemandwifiCallback () {
   ticker.attach(0.8, tick); 
   WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 81,"required");
   WiFiManagerParameter custom_Email("email", "Email", Email, 81,"required");
   WiFiManagerParameter custom_Supla_name("name", "Supla Device Name", Supla_name, 51,"required");
   WiFiManagerParameter custom_Supla_status("status", "Supla Last State", Supla_status, 51,"readonly");
   WiFiManagerParameter custom_Volt("Volt", "expectedVoltage", Volt, 8);
   WiFiManagerParameter custom_Wats("Wats", "expectedActivePower", Wats, 8);
   const char* custom_radio_str = "<br><div style='text-align:center; width:100%; padding:0;'><label for='customfieldid'>Counter mem</label><input type='radio' name='customfieldid' value='0' checked>keep Storage<input type='radio' name='customfieldid' value='1'>clear Storage</div>";
   new (&custom_field) WiFiManagerParameter(custom_radio_str); // custom html input

   wifiManager.setBreakAfterConfig(true);
   wifiManager.setSaveConfigCallback(saveConfigCallback);
   wifiManager.setSaveParamsCallback(saveParamCallback);
  
   wifiManager.addParameter(&custom_Supla_server);
   wifiManager.addParameter(&custom_Email);
   wifiManager.addParameter(&custom_Supla_name);
   wifiManager.addParameter(&custom_Supla_status);
   wifiManager.addParameter(&custom_Volt);
   wifiManager.addParameter(&custom_Wats);
    wifiManager.addParameter(&custom_field);

   wifiManager.setCustomHeadElement("<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>");
   wifiManager.setMinimumSignalQuality(8);
   //wifiManager.setShowStaticFields(true); // force show static ip fields
   //wifiManager.setShowDnsFields(true);    // force show dns field always
   wifiManager.setConfigPortalTimeout(180);

   if (!wifiManager.startConfigPortal("DGM")) { Serial.println("Not connected to WiFi but continuing anyway.");} else { Serial.println("connected...yeey :)");}                
    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(Email, custom_Email.getValue());
    strcpy(Supla_name, custom_Supla_name.getValue()); 
    strcpy(Volt, custom_Volt.getValue());
    strcpy(Wats, custom_Wats.getValue());
    if(strcmp(Supla_server, "get_new_guid_and_authkey") == 0){
      Serial.println("new guid & authkey.");
      EEPROM.write(300, 0);
      EEPROM.commit();
      ESP.reset(); 
    }
    if ((strlen(Volt) != 0) && (strlen(Wats) != 0)){
      calibrate_hj101();
    }
    WiFi.softAPdisconnect(true);   //  close AP
}
void status_func(int status, const char *msg) {    //    ------------------------ Status --------------------------
  //yield();
  if (s != status){
    s = status; 
      if (s != 10){
        strcpy(Supla_status, msg); 
  }  }            
}
String getParam(String name){
  String value;
  if(wifiManager.server->hasArg(name)) {
    value = wifiManager.server->arg(name);
      if (value == "1"){ 
        SuplaImpulseCounter::clearStorage();
         Serial.println("clearStorage");
         }
  }
  return value;
}
void saveParamCallback(){
  Serial.println("[CALLBACK] saveParamCallback fired");
  Serial.println("PARAM customfieldid = " + getParam("customfieldid"));
}
void get_temperature_and_humidity(int channelNumber, double *temp, double *humidity) {

       *temp = hjl01.getVoltage();  // --------------------------------------voltage as temperature ------------
       double Act_Pow = hjl01.getActivePower();
       *humidity = Act_Pow/10;
 
       /* Serial.print("[HLW] Agg. energy (Ws)    : "); Serial.println(hjl01.getEnergy());
        Serial.print("[HLW] Active Power (W)    : "); Serial.println(hjl01.getActivePower());
        Serial.print("[HLW] Voltage (V)         : "); Serial.println(hjl01.getVoltage());
        Serial.print("[HLW] Current (A)         : "); Serial.println(hjl01.getCurrent());
        Serial.print("[HLW] Apparent Power (VA) : "); Serial.println(hjl01.getApparentPower());
        Serial.print("[HLW] Power Factor (%)    : "); Serial.println((int) (100 * hjl01.getPowerFactor()));*/
                    
     int act_energy = (int)hjl01.getEnergy();
     act_energy = (act_energy / 60 / 60);
    if (act_energy > lst_energy){
      int i = (act_energy - lst_energy);
      lst_energy = act_energy;
      if (i == 1){
       SuplaImpulseCounter::getCounterByChannel(0)->incCounter(); 
      }else if (i > 1){
        for(int a=0;a<i;a++){
         SuplaImpulseCounter::getCounterByChannel(0)->incCounter(); 
        }     
      }
     }
}
void Eeprom_save(){
  if (digitalRead(relay_pin)!= EEPROM.read(431)){
    EEPROM.write(431,digitalRead(relay_pin));
    EEPROM.commit();
    Serial.println("mem relay");
  }
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

    int eep_aut = 321;

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
  }
  read_guid();
  read_authkey();
  Serial.print("GUID : ");Serial.println(read_guid()); 
  Serial.print("AUTHKEY : ");Serial.println(read_authkey()); 
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
  int eep_star = 321;
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
  Serial.println();
  EEPROM.begin(1024);
  pinMode(wificonfig_pin, INPUT_PULLUP); 
  pinMode(status_led,OUTPUT); 
  digitalWrite(status_led, HIGH);
  digitalWrite(relay_pin, EEPROM.read(431));
  guid_authkey();
  
  if (WiFi.SSID()==""){ initialConfig = true;} 

  if (SPIFFS.begin()) {  // ------------------------- wificonfig read -----------------
    Serial.println("mounted file system");
    if (SPIFFS.exists(fs_file)) {
      Serial.println("reading config file");
       File configFile = SPIFFS.open(fs_file, "r");
      if (configFile) {
        Serial.println("opened config file");
         size_t size = configFile.size();
         std::unique_ptr<char[]> buf(new char[size]);
         configFile.readBytes(buf.get(), size);
         DynamicJsonBuffer jsonBuffer;         
         JsonObject& json = jsonBuffer.parseObject(buf.get());
         json.printTo(Serial);   //print config data to serial on startup
        if (json.success()) {Serial.println("\nparsed json");         
          strcpy(Supla_server, json["Supla_server"]);
           strcpy(Email, json["Email"]);
           strcpy(Supla_name, json["Supla_name"]);         
        } else {
          Serial.println("failed to load json config");
           initialConfig = true;
        }
        configFile.close(); 
      }
    }
   } else {
    Serial.println("failed to mount FS");
  }
   wifi_station_set_hostname(Supla_name);
  
     hjl01.setCurrentMultiplier(HJL01_CURRENT_RATIO);
     hjl01.setVoltageMultiplier(HJL01_VOLTAGE_RATIO);
     hjl01.setPowerMultiplier(HJL01_POWER_RATIO);
    // void begin(unsigned char cf_pin, unsigned char cf1_pin, unsigned char sel_pin, unsigned char currentWhen = HIGH, bool use_interrupts = false, unsigned long pulse_timeout = PULSE_TIMEOUT);
    hjl01.begin(CF_PIN, CF1_PIN, SEL_PIN, CURRENT_MODE, true);
    get_hj101_config();
     
    attachInterrupt(CF1_PIN, hjl01_cf1_interrupt, FALLING);
    attachInterrupt(CF_PIN, hjl01_cf_interrupt, FALLING);
    lst_energy = (int)hjl01.getEnergy();
    lst_energy = (lst_energy / 60 / 60);
  
     SuplaDevice.setTemperatureHumidityCallback(&get_temperature_and_humidity);
 
   uint8_t mac[WL_MAC_ADDR_LENGTH];
   WiFi.macAddress(mac);
   
    SuplaDevice.addImpulseCounter(200, 0, true, false, 50);  // int _impulsePin, int _statusLedPin, bool _detectLowToHigh, bool _inputPullup, unsigned long _debounceDelay
    SuplaDevice.addDHT22();
    SuplaDevice.addRelay(relay_pin, false);           // ﻿44 - ﻿Pin number where the relay is connected      
                                      // Call SuplaDevice.addRelay(44, true) with an extra "true" parameter 
                                      // to enable "port value inversion"
                                      // where HIGH == LOW, and LOW == HIGH  

    SuplaDevice.setName(Supla_name);
    SuplaDevice.setStatusFuncImpl(&status_func);
  
    SuplaDevice.begin(GUID,Supla_server,Email,AUTHKEY);
                      
}
char buffer[50];
void loop() {
  
  if (initialConfig == true){ondemandwifiCallback();EEPROM.write(431, 0); EEPROM.commit();}

   int C_W_read = digitalRead(wificonfig_pin);{  
   if (C_W_read != last_C_W_state) {            
     time_last_C_W_change = millis();
   }
   if ((millis() - time_last_C_W_change) > C_W_delay2) {     
     if (C_W_read != C_W_state2) {     
       C_W_state2 = C_W_read; 
       if (C_W_state2 == LOW) { 
        Serial.println("short press");     
         if (digitalRead(relay_pin) == LOW) {
          SuplaDevice.relayOn(2,0);
          yield();
          Serial.println("relayOn");
          Eeprom_save() ;
       }else{
         SuplaDevice.relayOff(2);
         yield();
         Serial.println("relayOff");
         Eeprom_save() ;
     }}}       
    }
   if ((millis() - time_last_C_W_change) > C_W_delay) {     
     if (C_W_read != C_W_state) {     
       Serial.println("long press");
       C_W_state = C_W_read;       
       if (C_W_state == LOW) {
        ondemandwifiCallback () ;
       }
     }
    }
   last_C_W_state = C_W_read;            
 }
  
  if (shouldSaveConfig == true) { // ------------------------ wificonfig save --------------
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Supla_server"] = Supla_server;
    json["Email"] = Email;
    json["Supla_name"] = Supla_name;
    File configFile = SPIFFS.open(fs_file, "w");
    if (!configFile) {Serial.println("failed to open config file for writing"); }   
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    Serial.println("config saved");    
    shouldSaveConfig = false;
    initialConfig = false; 
    WiFi.mode(WIFI_STA);   
    delay(5000);
    ESP.restart(); 
  }
  
  if (s == 17){
    if(millis() > eep_milis){
     Eeprom_save() ;
     eep_milis = millis() + 3000 ;  
    }
    if (recover){
      if (EEPROM.read(431) == true){
       SuplaDevice.relayOn(2,0);
        yield();
         Serial.println("relayOn");
          eep_milis = millis() + 3000 ;
       }
       recover = false; 
      }
  }
  if (s == 10){ recover = true; }
     
   SuplaDevice.iterate();
   
   if (digitalRead(relay_pin) == digitalRead(status_led)){
    digitalWrite(status_led,!digitalRead(relay_pin));
   }
   
   if (WiFi.status() == WL_CONNECTED){
    if (starting){
      httpUpdater.setup(&httpServer, "/update", "admin", "pass");
      httpServer.begin(); 
      starting = false;         
     }
    httpServer.handleClient();
   }

}
