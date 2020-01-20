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
#include <SuplaDevice.h>  //------- https://github.com/SUPLA/arduino/tree/develop
#include <WiFiManager.h>
#include <ArduinoJson.h> //--------- https://github.com/bblanchon/ArduinoJson/tree/v5.13.2 ------
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266TrueRandom.h>
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

#define PUYA_SUPPORT 1//PUYASUPPORT
#define CONFIG_PIN 0  // WiFiconfig 
#define RED_PIN 15
#define GREEN_PIN 13
#define BLUE_PIN 12
#define BRIGHTNESS_PIN1 14
#define BRIGHTNESS_PIN2  4
#define POWER_ENABLE_LED 15
#define LEDPIN  5  // Green Led
#define LED2PIN 1  // Red Led
char Supla_server[81]=("Set server address");
char Email[81]=("set email address");                        
char Supla_name[51];
char Supla_status[51];
int C_W_state = HIGH; 
int last_C_W_state = HIGH;
unsigned long time_last_C_W_change = 0; 
long C_W_delay = 10000;                      // config delay 10 seconds        
int C_W_state2 = HIGH; 
long C_W_delay2 = 50;                      // button debounce
bool shouldSaveConfig = false;
bool initialConfig = false;
bool pr_wifi = true;
bool wificonfig = false;
bool state10 = true;
int s;
bool starting = true; 
unsigned long wifi_checkDelay = 20000;
unsigned long wifimilis;
unsigned long mem_milis;
unsigned char _red = 0;
unsigned char _green = 255;
unsigned char _blue = 0;
unsigned char _color_brightness = 0;
unsigned char _brightness = 0;
unsigned char _brightness1 = 0;
unsigned char _brightness2 = 0;
int _col_bright = 0;
int _bright = 0;
int bright = 0 ;
int _bright1 = 0;
int bright1 = 0 ;
int _bright2 = 0;
int bright2 = 0 ;
int col_bright = 0 ;
int redin = 0;
int redout = 0; 
int greenin = 0;
int greenout = 0; 
int bluein = 0;
int blueout = 0; 
int memBr;
int memBr1;
int memBr2;
int memCBr;
int memRed;
int memGreen;
int memBlue;
int Power_Off;

char GUID[SUPLA_GUID_SIZE];
char AUTHKEY[SUPLA_AUTHKEY_SIZE];
byte uuidNumber[16]; // UUIDs in binary form are 16 bytes long
ESP8266WebServer httpServer(81);
ESP8266HTTPUpdateServer httpUpdater;
WiFiManager wifiManager;

void saveConfigCallback () {          
  Serial1.println("Should save config");
  shouldSaveConfig = true;
}
void ondemandwifiCallback () {
  digitalWrite(LEDPIN,LOW);
  digitalWrite(LED2PIN,HIGH);
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

   wifiManager.setCustomHeadElement("<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>");
   wifiManager.setMinimumSignalQuality(8);
   //wifiManager.setShowStaticFields(true); // force show static ip fields
   //wifiManager.setShowDnsFields(true);    // force show dns field always
   wifiManager.setConfigPortalTimeout(180);

   if (!wifiManager.startConfigPortal("H801_RGB_W_W")) { Serial1.println("Not connected to WiFi but continuing anyway.");} else { Serial1.println("connected...yeey :)");}                
    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(Email, custom_Email.getValue());
    strcpy(Supla_name, custom_Supla_name.getValue()); 
    if(strcmp(Supla_server, "get_new_guid_and_authkey") == 0){
      Serial1.println("new guid & authkey.");
      EEPROM.write(300, 0);
      EEPROM.commit();
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
  Serial1.print("GUID : ");Serial1.println(read_guid()); 
  Serial1.print("AUTHKEY : ");Serial1.println(read_authkey()); 
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
void supla_timer() {

  if (bright > _bright){
    _bright = _bright + 1;
    set_rgbw(0);
  }else if (bright < _bright){
    _bright = _bright - 1;
    set_rgbw(0);
  }
  if (bright1 > _bright1){
    _bright1 = _bright1 + 1;
    set_rgbw(1);
  }else if (bright1 < _bright1){
    _bright1 = _bright1 - 1;
    set_rgbw(1);
  }
  if (bright2 > _bright2){
    _bright2 = _bright2 + 1;
    set_rgbw(2);
  }else if (bright2 < _bright2){
    _bright2 = _bright2 - 1;
    set_rgbw(2);
  }
  if (col_bright > _col_bright){
    _col_bright = _col_bright + 1;
    set_rgbw(0);
  }else if (col_bright < _col_bright){
    _col_bright = _col_bright - 1;
    set_rgbw(0);
  } 
  if (greenin > greenout){
    greenout = greenout + 1;
    set_rgbw(0);
  }else if (greenin < greenout){
    greenout = greenout - 1;
    set_rgbw(0);
  }
  if (redin > redout){
    redout = redout + 1;
    set_rgbw(0);
  }else if (redin < redout){
    redout = redout - 1;
    set_rgbw(0);
  }
  if (bluein > blueout){
    blueout = blueout + 1;
    set_rgbw(0);
  }else if (bluein < blueout){
    blueout = blueout - 1;
    set_rgbw(0);
  } 
}
void get_rgbw_value(int channelNumber, unsigned char *red, unsigned char *green, unsigned char *blue, unsigned char *color_brightness, unsigned char *brightness) {
   
  if ( channelNumber == 0){
   *brightness = _brightness;
    *color_brightness= _color_brightness;
    *red = _red;
    *green = _green;
    *blue = _blue;
   }
  if ( channelNumber == 1){
    *brightness = _brightness1;
   }
  if ( channelNumber == 2){
    *brightness = _brightness2;
   }

}
void set_rgbw(int channelNumber) {
   
  if ( channelNumber == 0){
    int out_br =((_col_bright * 1023) / 100);
    int out_red = map(redout, 0, 255, 0,out_br);
    analogWrite(RED_PIN, (out_red));
    int out_green = map(greenout, 0, 255, 0,out_br);
    analogWrite(GREEN_PIN, (out_green));
    int out_blue = map(blueout, 0, 255, 0,out_br);
    analogWrite(BLUE_PIN, (out_blue));
  } 
  if ( channelNumber == 1){
    analogWrite(BRIGHTNESS_PIN1, (_brightness1 * 1023) / 100);
  }
  if ( channelNumber == 2){
    analogWrite(BRIGHTNESS_PIN2, (_brightness2 * 1023) / 100);
  }  

    
  
    mem_milis = (millis()+3000);      
}
void set_rgbw_value(int channelNumber, unsigned char red, unsigned char green, unsigned char blue, unsigned char color_brightness, unsigned char brightness) {
  
   if ( channelNumber == 0){
    if(((String(brightness).toInt()) == 0) && ((String(color_brightness).toInt()) == 0)){Power_Off = 1;}else{Power_Off = 0;}
    _brightness = brightness;
    bright = (String(_brightness).toInt()); 
    _color_brightness= color_brightness;
    col_bright = (String(_color_brightness).toInt());  
    _red = red;
    redin = (String(_red).toInt()); 
    _green = green;
    greenin = (String(green).toInt()); 
    _blue = blue;
    bluein = (String(blue).toInt());    
   }
   if ( channelNumber == 1){
    _brightness1 = brightness;
    bright1 = (String(_brightness1).toInt()); 
    //set_rgbw(channelNumber);  
   }
   if ( channelNumber == 2){
    _brightness2 = brightness; 
    bright2 = (String(_brightness2).toInt());
    //set_rgbw(channelNumber);  
   }   
}
void save_epp(){
  EEPROM.write(10,memRed);EEPROM.write(11,memGreen);EEPROM.write(12,memBlue);EEPROM.write(13,memBr);EEPROM.write(14,memCBr);EEPROM.write(15,Power_Off);EEPROM.write(16,memBr1);EEPROM.write(17,memBr2);
  Serial1.println("Epp Write ");Serial1.println(memRed);Serial1.println(memGreen);Serial1.println(memBlue);Serial1.println(memBr);Serial1.println(memCBr);Serial1.println(Power_Off);
  EEPROM.commit();
}
void read_epp(){
  memRed = EEPROM.read(10);memGreen = EEPROM.read(11);memBlue = EEPROM.read(12);memBr = EEPROM.read(13);memCBr = EEPROM.read(14);Power_Off = EEPROM.read(15);memBr1 = EEPROM.read(16);memBr2 = EEPROM.read(17);
  Serial1.println("Epp Read ");Serial1.println(memRed);Serial1.println(memGreen);Serial1.println(memBlue);Serial1.println(memBr);Serial1.println(memCBr);Serial1.println(Power_Off);
  if (Power_Off == 0){
   bright = memBr;
   col_bright = memCBr;
   redin = memRed;
   greenin = memGreen;
   bluein = memBlue;
  }
}

void setup() {

   wifi_set_sleep_type(NONE_SLEEP_T);  
    
  digitalWrite(12, 0);
  digitalWrite(13, 0);
  pinMode(CONFIG_PIN, INPUT_PULLUP);  
  analogWriteFreq(100);
  //Serial.begin(115200);
  Serial1.begin(115200,SERIAL_8N1,SERIAL_TX_ONLY);
  delay(20);
  Serial1.println();
  Serial1.println();

  digitalWrite(LEDPIN, 0);
  digitalWrite(LED2PIN, 0);
  digitalWrite(RED_PIN, 0);
  digitalWrite(GREEN_PIN, 0);
  digitalWrite(BLUE_PIN, 0);
  digitalWrite(BRIGHTNESS_PIN1, 0);
  digitalWrite(BRIGHTNESS_PIN2, 0);
  digitalWrite(POWER_ENABLE_LED, 1);

  pinMode(LEDPIN, OUTPUT);
  pinMode(LED2PIN, OUTPUT);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(BRIGHTNESS_PIN1, OUTPUT);
  pinMode(BRIGHTNESS_PIN2, OUTPUT);
  pinMode(POWER_ENABLE_LED, OUTPUT);
  digitalWrite(LEDPIN,HIGH);
  digitalWrite(LED2PIN,LOW);

  analogWrite(RED_PIN, 0);
  analogWrite(GREEN_PIN, 0);
  analogWrite(BLUE_PIN, 0);
  analogWrite(BRIGHTNESS_PIN1, 0);
  analogWrite(BRIGHTNESS_PIN2, 0);  

  EEPROM.begin(1024);
  if (EEPROM.read(300) != 60){initialConfig = true;} 
  guid_authkey();

  if (WiFi.SSID()==""){ initialConfig = true;} 

  if (SPIFFS.begin()) {  // ------------------------- wificonfig read -----------------
    Serial1.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      Serial1.println("reading config file");
       File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial1.println("opened config file");
         size_t size = configFile.size();
         std::unique_ptr<char[]> buf(new char[size]);
         configFile.readBytes(buf.get(), size);
         DynamicJsonBuffer jsonBuffer;         
         JsonObject& json = jsonBuffer.parseObject(buf.get());
         json.printTo(Serial);   //print config data to serial on startup
        if (json.success()) {Serial1.println("\nparsed json");         
          strcpy(Supla_server, json["Supla_server"]);
           strcpy(Email, json["Email"]);
           strcpy(Supla_name, json["Supla_name"]);         
        } else {
          Serial1.println("failed to load json config");
           initialConfig = true;
        }
        configFile.close(); 
      }
    }
   } else {
    Serial1.println("failed to mount FS");
  }
   wifi_station_set_hostname(Supla_name);
   
  WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.


  read_epp();
  if (Power_Off == 0){_brightness = memBr;_color_brightness= memCBr; _red = memRed;_green = memGreen;_blue = memBlue;_brightness1 = memBr1;_brightness2 = memBr2;} 
  else{_brightness = 0;_color_brightness= 0; _red = memRed;_green = memGreen;_blue = memBlue;_brightness1 = 0;_brightness2 = 0;}
  set_rgbw(0);
  SuplaDevice.setTimerFuncImpl(&supla_timer);  
  SuplaDevice.setRGBWCallbacks(&get_rgbw_value, &set_rgbw_value);
  SuplaDevice.setStatusFuncImpl(&status_func);
  SuplaDevice.setName(Supla_name);

     SuplaDevice.addRgbController();
     SuplaDevice.addDimmer();
     SuplaDevice.addDimmer();

     SuplaDevice.begin(GUID,Supla_server,Email,AUTHKEY);
  
}

void loop() {
   if (initialConfig == true){ondemandwifiCallback();}

   if (shouldSaveConfig == true) { // ------------------------ wificonfig save --------------
    Serial1.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Supla_server"] = Supla_server;
    json["Email"] = Email;
    json["Supla_name"] = Supla_name;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {Serial1.println("failed to open config file for writing"); }   
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    Serial1.println("config saved");    
    shouldSaveConfig = false;
    initialConfig = false; 
    WiFi.mode(WIFI_STA);   
    delay(5000);
    ESP.restart(); 
  }
     
   SuplaDevice.iterate();
   delay(75);

   if (WiFi.status() == WL_CONNECTED){
    if (starting){
      httpUpdater.setup(&httpServer, "/update", "admin", "pass");
      httpServer.begin(); 
      starting = false;         
     }
    httpServer.handleClient();
   }
   
  if (millis() > mem_milis){
    if (EEPROM.read(15) != Power_Off){
      save_epp();
    }
    if (Power_Off == 0){
     if (((String(_brightness).toInt()) != EEPROM.read(13)) || ((String(_color_brightness).toInt()) != EEPROM.read(14)) || ((String(_red).toInt()) !=EEPROM.read(10)) || ((String(_green).toInt()) != EEPROM.read(11)) || ((String(_blue).toInt()) != EEPROM.read(12)) || ((String(_brightness1).toInt()) != EEPROM.read(16)) || ((String(_brightness2).toInt()) != EEPROM.read(17))){      
        memBr = (String(_brightness).toInt());memCBr =(String(_color_brightness).toInt());memRed =(String(_red).toInt());memGreen = (String(_green).toInt());memBlue = (String(_blue).toInt());memBr1 = (String(_brightness1).toInt());memBr2 = (String(_brightness2).toInt());
        save_epp();
     }}
     mem_milis = (millis()+3000); 
   }
  
  int C_W_read = digitalRead(CONFIG_PIN);{  
   if (C_W_read != last_C_W_state) {            
     time_last_C_W_change = millis();
   }
   if ((millis() - time_last_C_W_change) > C_W_delay2) {     
     if (C_W_read != C_W_state2) {     
       Serial1.println("short press");
       C_W_state2 = C_W_read;       
       if (C_W_state2 == LOW) {
        if (Power_Off == 0){
         Power_Off = 1;
         _brightness = 0;
         _color_brightness = 0;
         bright = (String(_brightness).toInt()); 
         col_bright = (String(_color_brightness).toInt());
         SuplaDevice.channelSetRGBWvalues(0,memRed,memGreen,memBlue,0,0); // off _brightness
         SuplaDevice.channelSetRGBWvalues(1,0,0,0,0,0); // off _brightness
         SuplaDevice.channelSetRGBWvalues(2,0,0,0,0,0); // off _brightness 
         //SuplaDevice.channelValueChanged(0, 0); // off _brightness
     }else{
        _brightness = memBr;
        _color_brightness = memCBr;
        bright = (String(_brightness).toInt());
        col_bright = (String(_color_brightness).toInt()); 
        Power_Off = 0;
        SuplaDevice.channelSetRGBWvalues(0,memRed,memGreen,memBlue,memCBr,memBr); // restore _color_brightness & _brightness
        SuplaDevice.channelSetRGBWvalues(1,0,0,0,0,memBr1); //  _brightness 1
        SuplaDevice.channelSetRGBWvalues(2,0,0,0,0,memBr2); //  _brightness 2
     }
     if (Power_Off){
      Serial1.println("Power sate Off");
     }else{
      Serial1.println("Power sate On");
     }
       }
     }
    }
   if ((millis() - time_last_C_W_change) > C_W_delay) {     
     if (C_W_read != C_W_state) {     
       Serial1.println("long press");
       C_W_state = C_W_read;       
       if (C_W_state == LOW) {
        ondemandwifiCallback () ;
       }
     }
    }
   last_C_W_state = C_W_read;            
 }
 
 switch (s) {    //    ------------------------------------------------ Status ------------------------------------
  case 17:      // -----     STATUS_REGISTERED_AND_READY
  if (state10 == true){
    if (Power_Off){
      SuplaDevice.channelSetRGBWvalues(0,memRed,memGreen,memBlue,0,0); // off _brightness
      SuplaDevice.channelSetRGBWvalues(1,0,0,0,0,0); // off _brightness
      SuplaDevice.channelSetRGBWvalues(2,0,0,0,0,0); // off _brightness 
      //SuplaDevice.channelValueChanged(0, 0); // off _brightness
    }else{
      SuplaDevice.channelSetRGBWvalues(0,memRed,memGreen,memBlue,memCBr,memBr); // restore _color_brightness & _brightness
      SuplaDevice.channelSetRGBWvalues(1,0,0,0,0,memBr1); //  _brightness
      SuplaDevice.channelSetRGBWvalues(2,0,0,0,0,memBr2); //  _brightness
    }
   state10 = false; 
    } 
    break; 
    case 10:      // --------------------- REGISTER_IN_PROGRESS  ----------------------
    state10 = true; 
    break;
   
  }
}
