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

#include <SPI.h>
#include <SuplaDevice.h>
#include <supla/control/rgbw_base.h>
#include <Button3.h>
#include <WiFiManager.h>
#include <ESP8266mDNS.h> 
#include <ArduinoJson.h> //--------- https://github.com/bblanchon/ArduinoJson/tree/v5.13.2 ------
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266TrueRandom.h>
#include <IRremoteESP8266.h>                                 
#include <IRrecv.h>
#include <IRutils.h>
#include <Ticker.h>

// ESP8266 based board:
 #include <supla/network/esp_wifi.h>
 Supla::ESPWifi wifi("", "");
//
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

Supla::Control::RGBWBase *mirgbw = nullptr;

#define CONFIG_PIN 0  // WiFiconfig 
#define RED_PIN              14
#define GREEN_PIN            12
#define BLUE_PIN             13
#define BRIGHTNESS_PIN       5
#define COLOR_BRIGHTNESS_PIN 101
#define ledpin               16
#define pinr1                4 
const unsigned int captureBufSize = 100; 
Button3 buttonA = Button3(CONFIG_PIN);
DynamicJsonBuffer jsonBuffer;
JsonObject& deviceState = jsonBuffer.createObject();
WiFiClient client;
ESP8266WebServer *server = NULL;
ESP8266HTTPUpdateServer httpUpdater;
Ticker ticker;                                          
IRrecv irrecv(pinr1, captureBufSize);
File fsUploadFile; 
int s;
int web_port = 80;
char GUID[SUPLA_GUID_SIZE];
char AUTHKEY[SUPLA_AUTHKEY_SIZE];
byte uuidNumber[16];
char Supla_server[81]=("Set server address");
char Email[81]=("set email address");                        
char Supla_name[51];
char Supla_status[51];
int C_W_state = HIGH; 
int last_C_W_state = HIGH;
unsigned long time_last_C_W_change = 0; 
long C_W_delay = 10000;                      // config delay 10 seconds        
bool shouldSaveConfig = false;
bool initialConfig = false;
bool starting = true;
bool pr_wifi = true; 
bool holdReceive;   
unsigned long mem_milis;
bool rgbw_update = false;
uint8_t hw_red;
uint8_t hw_green;
uint8_t hw_blue;
uint8_t hw_colorBrightness;
uint8_t hw_brightness;
char limpio[40];
char toogle_rgb_data[40];
char toogle_w_data[40]; 
char brighten_rgb_data[40];
char dim_rgb_data[40];
char brighten_w_data[40];
char dim_w_data[40]; 
char dim_on_data[40];
char toogle_data[40]; 
char red_data[40]; 
char green_data[40]; 
char blue_data[40];
char mem_1_data[40];
char mem_2_data[40];
char mem_3_data[40];
char white_data[40]; 
/*
enum Action {
  TURN_ON,
  TURN_OFF,
  TOGGLE,
  BRIGHTEN_ALL,
  DIM_ALL,
  BRIGHTEN_R,
  BRIGHTEN_G,
  BRIGHTEN_B,
  BRIGHTEN_W,
  BRIGHTEN_RGB,
  DIM_R,
  DIM_G,
  DIM_B,
  DIM_W,
  DIM_RGB,
  TURN_ON_RGB,
  TURN_OFF_RGB,
  TOGGLE_RGB,
  TURN_ON_W,
  TURN_OFF_W,
  TOGGLE_W,
  TURN_ON_RGB_DIMMED,
  TURN_ON_W_DIMMED,
  TURN_ON_ALL_DIMMED,
  ITERATE_DIM_RGB,
  ITERATE_DIM_W,
  ITERATE_DIM_ALL
};*/
class Code {
  public:
    char encoding[14] = "";
    char data[40] = ""; 
    int bits = 0;
    bool valid = false;
};

Code last_recv;
Code last_recv_2;
Code last_recv_3;
Code last_recv_4;
Code last_recv_5;

class returnMessage {
  public:
    String message = "";
    String title = "";
    int httpcode = 0;
    int type = 1;
};
returnMessage rtnMessage;

class RgbwLeds : public Supla::Control::RGBWBase {
 public:
  RgbwLeds(int redPin,
           int greenPin,
           int bluePin,
           int colorBrightnessPin,
           int brightnessPin)
      : redPin(redPin),
        greenPin(greenPin),
        bluePin(bluePin),
        colorBrightnessPin(colorBrightnessPin),
        brightnessPin(brightnessPin){

  }

  void setRGBWValueOnDevice(uint8_t red,               
                            uint8_t green, 
                            uint8_t blue,
                            uint8_t colorBrightness,
                            uint8_t brightness) {
     hw_red = red;
     hw_green = green;
     hw_blue = blue;
     hw_colorBrightness = colorBrightness;
     hw_brightness = brightness;

    analogWrite(brightnessPin, (brightness * 1023) / 100);   
    int out_br =((colorBrightness * 1023) / 100);
    analogWrite(redPin, map(red, 0, 255, 0,out_br));
    analogWrite(greenPin,  map(green, 0, 255, 0,out_br));
    analogWrite(bluePin, map(blue, 0, 255, 0,out_br));

    mem_milis = (millis()+3000);rgbw_update = true; 
  }

 protected:
  int redPin;
  int greenPin;
  int bluePin;
  int brightnessPin;
  int colorBrightnessPin;
};

void saveConfigCallback () {          
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void tick()
{
  int state = digitalRead(ledpin);  
  digitalWrite(ledpin, !state);     
}

void disableLed()
{
  digitalWrite(ledpin, HIGH);                        
  ticker.detach();                                     
}
void ondemandwifiCallback () {
  
  ticker.attach(0.5, tick);
  
  if (server != NULL) { delete server;}
             
  WiFiManager wifiManager;
  
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

   if (!wifiManager.startConfigPortal("RGB_W_IR")) { Serial.println("Not connected to WiFi but continuing anyway.");} else { Serial.println("connected...yeey :)");}                
    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(Email, custom_Email.getValue());
    strcpy(Supla_name, custom_Supla_name.getValue()); 
    if(strcmp(Supla_server, "get_new_guid_and_authkey") == 0){
      Serial.println("new guid & authkey.");
      EEPROM.write(300, 0);
      EEPROM.commit();
      ESP.reset(); 
    }
  if (shouldSaveConfig) {
    Serial.println(" config...");
    DynamicJsonBuffer jsonWifiBuffer;
    JsonObject& json = jsonWifiBuffer.createObject();
    json["Supla_server"] = Supla_server;
    json["Email"] = Email;
    json["Supla_name"] = Supla_name;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    Serial.println("");
    Serial.println("Writing config file");
    json.printTo(configFile);
    configFile.close();
    jsonWifiBuffer.clear();
    Serial.println("Config written successfully");
    shouldSaveConfig = false;
    initialConfig = false; 
    WiFi.mode(WIFI_STA);   
    delay(5000);
    ESP.restart();     
  }
    
  ticker.detach();
  WiFi.softAPdisconnect(true);
  digitalWrite(ledpin, LOW);
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
void setup() { // ========================================================================== setup ================================================
  wifi_set_sleep_type(NONE_SLEEP_T);
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  delay(200);
  Serial.println(" ");
  Serial.println(" ");
  EEPROM.begin(1024);
  pinMode(CONFIG_PIN, INPUT_PULLUP); 
  pinMode(ledpin, OUTPUT);
  
  guid_authkey();
  read_rgbw_mem();
  
  if (WiFi.SSID()==""){ initialConfig = true;}
  if (EEPROM.read(300) != 60){initialConfig = true;}  

  if (SPIFFS.begin()) {  // ------------------------- wificonfig read -----------------
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
        if (json.success()) {Serial.println("\nparsed json");         
          if (json.containsKey("Supla_server")) strcpy(Supla_server, json["Supla_server"]);
          if (json.containsKey("Email")) strcpy(Email, json["Email"]);
          if (json.containsKey("Supla_name")) strcpy(Supla_name, json["Supla_name"]);       
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

  read_stored_command();
  wifi_station_set_hostname(Supla_name);
   
  WiFi.mode(WIFI_STA); 

  // CHANNEL0 - RGB controller and dimmer (RGBW)
  mirgbw = new RgbwLeds(RED_PIN, GREEN_PIN, BLUE_PIN, COLOR_BRIGHTNESS_PIN, BRIGHTNESS_PIN);
  
  wifi.enableSSL(false);
  SuplaDevice.setName(Supla_name);
  SuplaDevice.setStatusFuncImpl(&status_func);
  mirgbw->setRGBWValueOnDevice(hw_red, hw_green, hw_blue, hw_colorBrightness, hw_brightness);
  mirgbw->setRGBW(hw_red, hw_green, hw_blue, hw_colorBrightness, hw_brightness);
  
     SuplaDevice.begin(GUID,Supla_server,Email,AUTHKEY);

  buttonA.setClickHandler(click);
  buttonA.setLongClickHandler(longClick);
  buttonA.setDoubleClickHandler(doubleClick);
  buttonA.setTripleClickHandler(tripleClick);
  buttonA.setQuatleClickHandler(quatleClick);
  buttonA.setQuintleClickHandler(quintleClick);

  digitalWrite(ledpin, LOW);
  ticker.attach(2, disableLed);

  irrecv.enableIRIn();
  Serial.println("Ready to receive IR signals");
  mirgbw->setStep(5);
}
void sendHeader() {
  sendHeader(200);
}
void sendHeader(int httpcode) {
  server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  server->send(httpcode, "text/html; charset=utf-8", "");
  server->sendContent("<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Strict//EN' 'http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd'>\n");
  server->sendContent("<html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en'>\n");
  server->sendContent("  <div style='background: #01DF3A;'>\n");  
  server->sendContent("  <head>\n");
  server->sendContent("    <meta name='viewport' content='width=device-width, initial-scale=1.0' />\n");
  server->sendContent("    <link rel='stylesheet' href='https://stackpath.bootstrapcdn.com/bootstrap/3.4.1/css/bootstrap.min.css' />\n");
  server->sendContent("    <script src='https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js'></script>\n");  
  server->sendContent("    <script src='https://stackpath.bootstrapcdn.com/bootstrap/3.4.1/js/bootstrap.min.js'></script>\n");
  server->sendContent("    <style>@media (max-width: 991px) {.nav-pills>li {float: none; margin-left: 0; margin-top: 5px; text-align: center;}}</style>\n");
  server->sendContent("    <title>Supla RGBW IR  (" + String(Supla_name) + ")</title>\n");
  server->sendContent("  <link rel='shortcut icon' href='data:image/x-icon;base64,AAABAAEAEBAQAAEABAAoAQAAFgAAACgAAAAQAAAAIAAAAAEABAAAAAAAgAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAA/4QAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEAERABAAEAEBAQABAQEBABAAEAEQARABAQABAQABAAAQARAAEQARAAAAAAAAAAAAAAAAAAAAAAEREAEQAQAAAQAAEAEBAAABAAAAAQEAAAERAAEQAREAAQAAEAABABABAAAQAQEAEAEREAEQAREAAAAAAAAAAAD//wAA2N0AAKuqAADdmQAArrsAANnMAAD//wAA//8AAIZvAAC9rwAAv68AAI5jAAC97QAAva0AAIZjAAD//wAA' type='image/x-icon' />\n");
  server->sendContent("  <link rel='icon' href='data:image/x-icon;base64,AAABAAEAEBAQAAEABAAoAQAAFgAAACgAAAAQAAAAIAAAAAEABAAAAAAAgAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAA/4QAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEAERABAAEAEBAQABAQEBABAAEAEQARABAQABAQABAAAQARAAEQARAAAAAAAAAAAAAAAAAAAAAAEREAEQAQAAAQAAEAEBAAABAAAAAQEAAAERAAEQAREAAQAAEAABABABAAAQAQEAEAEREAEQAREAAAAAAAAAAAD//wAA2N0AAKuqAADdmQAArrsAANnMAAD//wAA//8AAIZvAAC9rwAAv68AAI5jAAC97QAAva0AAIZjAAD//wAA' type='image/x-icon' />\n");
  server->sendContent("  </head>\n");
  server->sendContent("  <body>\n");
  server->sendContent("   <nav class='navbar navbar-inverse'>\n");
  server->sendContent("      <a class='navbar-brand' href='/'>Supla RGBW IR</a>\n");
  server->sendContent("      <ul class='nav navbar-nav'>\n");
  server->sendContent("       <li class='dropdown'><a class='dropdown-toggle' data-toggle='dropdown' href='#'>Tools <span class='caret'></span></a>\n");
  server->sendContent("        <ul class='dropdown-menu'>\n");
  server->sendContent("         <li><a href='/update'>ESP8266 firmware update</a></li>\n");
  server->sendContent("         <li><a href='#' data-toggle='modal' data-target='#myModal'>Upload IR remote code file</a></li>\n");
  server->sendContent("         <li class='divider'></li>\n");
  server->sendContent("         <li><a target='_blank' style='background-color:red;' href='/deleteall'>!! Delete all Stored Codes !!</a></li>\n");  
  server->sendContent("        </ul>\n");
  server->sendContent("       </li>\n");
  server->sendContent("      </ul>\n");
  server->sendContent("   </nav>\n");
  server->sendContent("   <!-- Modal -->\n");
  server->sendContent("   <form method='POST' action='/listcodes' enctype='multipart/form-data' id='modal_form_id'>\n");  
  server->sendContent("   <div id='myModal' class='modal fade' role='dialog'>\n");
  server->sendContent("    <div class='modal-dialog'>\n");
  server->sendContent("      <!-- Modal content-->\n");
  server->sendContent("     <div class='modal-content'>\n");
  server->sendContent("       <div class='modal-header'>\n");
  server->sendContent("          <button type='button' class='close' data-dismiss='modal'>&times;</button>\n");
  server->sendContent("          <h4 class='modal-title'>Upload remote control button code</h4>\n");
  server->sendContent("       </div>\n");
  server->sendContent("       <div class='modal-body'>\n");
  server->sendContent("        <p>Select the json file you wish to upload.</p>\n");
  server->sendContent("        <div class='input-group'>\n");
  server->sendContent("          <label class='input-group-btn'>\n");
  server->sendContent("           <span class='btn btn-primary btn-file'>\n");  
  server->sendContent("            Browse&hellip; <input type='file' accept='.json' name='name' style='display: none' single>\n");
  server->sendContent("           </span>\n");
  server->sendContent("          </label>\n"); 
  server->sendContent("          <input type='text' class='form-control' readonly>\n");    
  server->sendContent("          <p></p>\n");  
  server->sendContent("        </div>\n");
  server->sendContent("        <div class='label label-warning'>Warning:</div>\n");
  server->sendContent("        <p>This will overrite any currently stored codes, this operation cannot be reversed.</p>\n"); 
  server->sendContent("        <p>Please ensure the selected file is in the correct format (preferably based on a previously downloaded code file), and ensure the files name (xx.json) matches the device name of the json object in the json file - (1.json should have <code>{\"device\":\"1\"}</code> as the json object).</p>\n");    
  server->sendContent("       </div>\n");
  server->sendContent("       <div class='modal-footer'>\n");
  server->sendContent("          <input class='btn btn-default' type='submit' value='Upload'>\n");
  server->sendContent("          <button type='button' class='btn btn-default' data-dismiss='modal'>Close</button>\n");
  server->sendContent("        </div>\n");
  server->sendContent("     </div>\n");
  server->sendContent("    </div>\n");
  server->sendContent("   </div>\n"); 
  server->sendContent("   </form>\n"); 
  server->sendContent("   <script type='text/javascript'>\n");
  server->sendContent("      $(document).on('change', '.btn-file :file', function() {\n");
  server->sendContent("        var input = $(this),\n");
  server->sendContent("            numFiles = 1,\n");
  server->sendContent("            label = input.val().replace(/\\\\/g , '/').replace(/.*\\//, '');\n");
  server->sendContent("        input.trigger('fileselect', [numFiles, label]);\n");
  server->sendContent("      });\n");
  server->sendContent("      $(document).ready( function() {\n");
  server->sendContent("          $('.btn-file :file').on('fileselect', function(event, numFiles, label) {\n");
  server->sendContent("              var input = $(this).parents('.input-group').find(':text'),\n");
  server->sendContent("                  log = label;\n");       
  server->sendContent("             if( input.length ) {\n");
  server->sendContent("                  input.val(log);\n");
  server->sendContent("             }\n");        
  server->sendContent("          });\n");
  server->sendContent("      });\n");
  server->sendContent("   </script>\n");  
  server->sendContent("   <div class='container'>\n");
  server->sendContent("      <div class='row'>\n");
  server->sendContent("        <div class='col-md-12'>\n");
  server->sendContent("          <ul class='nav nav-pills'>\n");
  server->sendContent("            <li class='active'>\n");
  server->sendContent("              <a href='http://" + WiFi.localIP().toString() + ":" + int(web_port) + "'>Local<span class='badge'>" + WiFi.localIP().toString() + ":" + String(web_port) + "</span></a></li>\n");
  server->sendContent("            <li class='active'>\n");
  server->sendContent("              <a href='http://" + WiFi.localIP().toString() + ":"  + int(web_port) + "/listcodes"  + "'>Stored Codes<span class='badge'>" + WiFi.localIP().toString() + ":"  + String(web_port) + "/listcodes"  + "</span></a></li>\n");
  server->sendContent("            <li class='active'>\n");
  server->sendContent("              <a href='https://cloud.supla.org/login'>Supla state<span class='badge'>" + String(Supla_status) + "</span></a></li>\n");
  server->sendContent("            <li class='active'>\n");
  server->sendContent("              <a href='http://" + String(Supla_name) + ".local" + ":" + String(web_port) + "'>mDNS<span class='badge'>" + String(Supla_name) + ".local" + ":" + String(web_port) + "</span></a></li>\n"); 
  server->sendContent("          </ul>\n");
  server->sendContent("        </div>\n");
  server->sendContent("      </div><hr />\n");
}

void sendFooter() {

  server->sendContent("      <hr />\n");
  server->sendContent("      <div class='row'><div class='col-md-12'><em>memory heap: " + String(ESP.getFreeHeap()) + "</em></div></div>");
  server->sendContent("     <hr />\n");
  server->sendContent("    <hr />\n");
  server->sendContent("   </div>\n");
  server->sendContent("  </body>\n");
  server->sendContent("  </div>\n");
  server->sendContent("</html>\n");
  server->client().stop();
  Serial.println("client stop");
}

void sendHomePage() {
  sendHomePage("", "");
}
void sendHomePage(String message, String header) {
  sendHomePage(message, header, 0);
}
void sendHomePage(String message, String header, int type) {
  sendHomePage(message, header, type, 200);
}
void sendHomePage(String message, String header, int type, int httpcode) {
  String jsonTest;
  sendHeader(httpcode);
  if (type == 1)
  server->sendContent("      <div class='row'><div class='col-md-12'><div class='alert alert-success'><strong>" + header + "!</strong> " + message + "</div></div></div>\n");
  if (type == 2)
  server->sendContent("      <div class='row'><div class='col-md-12'><div class='alert alert-warning'><strong>" + header + "!</strong> " + message + "</div></div></div>\n");
  if (type == 3)
  server->sendContent("      <div class='row'><div class='col-md-12'><div class='alert alert-danger'><strong>" + header + "!</strong> " + message + "</div></div></div>\n");
  server->sendContent("      <div class='row'>\n");
  server->sendContent("        <div class='col-md-12'>\n");
  server->sendContent("          <h3>Codes Received</h3>\n");
  server->sendContent("          <table class='table table-striped' style='table-layout: fixed;'>\n");
  server->sendContent("            <thead><tr><th>Data</th><th>Type</th><th>Length</th><th>Action</th></tr></thead>\n"); //Title
  server->sendContent("            <tbody>\n");
  if (last_recv.valid) {
    server->sendContent("              <form style='display:inline-block; margin:0; padding:0' action='/json' method='post'><tr class='text-uppercase'><td>" + String(last_recv.data) + "</td><td>" + String(last_recv.encoding) + "</td><td>" + String(last_recv.bits) + "</td><td> <a class='btn btn-warning btn-xs' href='/store?id=1' role='button'>Store</a></td></tr></form>\n");
  }  
  if (last_recv_2.valid) {
    server->sendContent("              <form style='display:inline-block; margin:0; padding:0' action='/json' method='post'><tr class='text-uppercase'><td>" + String(last_recv_2.data) + "</td><td>" + String(last_recv_2.encoding) + "</td><td>" + String(last_recv_2.bits) + "</td><td> <a class='btn btn-warning btn-xs' href='/store?id=2' role='button'>Store</a></td></tr></form>\n");
    }
    if (last_recv_3.valid) {
    server->sendContent("              <form style='display:inline-block; margin:0; padding:0' action='/json' method='post'><tr class='text-uppercase'><td>" + String(last_recv_3.data) + "</td><td>" + String(last_recv_3.encoding) + "</td><td>" + String(last_recv_3.bits) + "</td><td> <a class='btn btn-warning btn-xs' href='/store?id=3' role='button'>Store</a></td></tr></form>\n");
    }
    if (last_recv_4.valid) {
    server->sendContent("              <form style='display:inline-block; margin:0; padding:0' action='/json' method='post'><tr class='text-uppercase'><td>" + String(last_recv_4.data) + "</td><td>" + String(last_recv_4.encoding) + "</td><td>" + String(last_recv_4.bits) + "</td><td> <a class='btn btn-warning btn-xs' href='/store?id=4' role='button'>Store</a></td></tr></form>\n");
    }
    if (last_recv_5.valid) {
    server->sendContent("              <form style='display:inline-block; margin:0; padding:0' action='/json' method='post'><tr class='text-uppercase'><td>" + String(last_recv_5.data) + "</td><td>" + String(last_recv_5.encoding) + "</td><td>" + String(last_recv_5.bits) + "</td><td> <a class='btn btn-warning btn-xs' href='/store?id=5' role='button'>Store</a></td></tr></form>\n");
    }
  if (!last_recv.valid && !last_recv_2.valid && !last_recv_3.valid && !last_recv_4.valid && !last_recv_5.valid)
  server->sendContent("              <tr><td colspan='6' class='text-center'><em>No codes received</em></td></tr>");
  server->sendContent("            </tbody></table>\n"); 
  server->sendContent("      <div class='row'>\n");
  server->sendContent("        <div class='col-md-12'>\n");
  server->sendContent("          <h3>Store Preset 1 - 3</h3>\n");
  server->sendContent("          <table class='table table-striped' style='table-layout: fixed;'>\n");
  server->sendContent("            <thead><tr><th>STORE</th><th>Red Green Blue</th><th>ColBrigh</th><th>Brigh(W)</th></tr></thead>\n"); //Title
  server->sendContent("            <tbody>\n");
  server->sendContent("           <tr class='text-uppercase'><td> <a class='btn btn-primary btn-xs' href='/controlmem?id=1' role='button'>set mem 1</a></td><td>" + String(EEPROM.read(25)) + "," + String(EEPROM.read(26)) + "," + String(EEPROM.read(27)) + "</td><td>" + String(EEPROM.read(28)) + "</td><td>" + String(EEPROM.read(29)) + "</td></tr>\n");
  server->sendContent("           <tr class='text-uppercase'><td> <a class='btn btn-primary btn-xs' href='/controlmem?id=2' role='button'>set mem 2</a></td><td>" + String(EEPROM.read(35)) + "," + String(EEPROM.read(36)) + "," + String(EEPROM.read(37)) + "</td><td>" + String(EEPROM.read(38)) + "</td><td>" + String(EEPROM.read(39)) + "</td></tr>\n");
  server->sendContent("           <tr class='text-uppercase'><td> <a class='btn btn-primary btn-xs' href='/controlmem?id=3' role='button'>set mem 3</a></td><td>" + String(EEPROM.read(45)) + "," + String(EEPROM.read(46)) + "," + String(EEPROM.read(47)) + "</td><td>" + String(EEPROM.read(48)) + "</td><td>" + String(EEPROM.read(49)) + "</td></tr>\n");
  //server->sendContent("           <tr class='text-uppercase'><td>" + String(hw_red) + "," + String(hw_green) + "," + String(hw_blue) + "</td><td>" + String(hw_colorBrightness) + "</td><td>" + String(hw_brightness) + "</td><td> <a class='btn btn-primary btn-xs' href='/controlmem?id=1' role='button'>mem 1</a>  <a class='btn btn-primary btn-xs' href='/controlmem?id=2' role='button'>mem 2</a>  <a class='btn btn-primary btn-xs' href='/controlmem?id=3' role='button'>mem 3</a> </td></tr>\n");
  //server->sendContent("           <tr class='text-uppercase'><td>mem_2</td><td>" + String(hw_red) + "</td><td>" + String(hw_green) + "</td><td>" + String(hw_blue) + "</td><td>" + String(hw_colorBrightness) + "</td><td>" + String(hw_brightness) + "</td><td> <a class='btn btn-primary btn-xs' href='/controlmem?id=1' role='button'>Set</a> </td></tr>\n");
  //server->sendContent("           <tr class='text-uppercase'><td>mem_3</td><td>" + String(hw_red) + "</td><td>" + String(hw_green) + "</td><td>" + String(hw_blue) + "</td><td>" + String(hw_colorBrightness) + "</td><td>" + String(hw_brightness) + "</td><td> <a class='btn btn-primary btn-xs' href='/controlmem?id=2' role='button'>Set</a> </td></tr>\n"); 
  server->sendContent("            </tbody>\n");
  server->sendContent("          </table>\n");
  server->sendContent("         </div></div>\n");
  server->sendContent("       <p></p>\n");
  server->sendContent("        </div>\n");
  server->sendContent("      </div>\n");
  sendFooter();
}

void storeCodePage(Code selCode, int id) {
  storeCodePage(selCode, id, 200);
}
void storeCodePage(Code selCode, int ids, int httpcode){
         
  sendHeader(httpcode);
  server->sendContent("      <div class='row'>\n");
  server->sendContent("        <div class='col-md-12'>\n");  
  server->sendContent("          <h2><span class='label label-success'>" + String(selCode.data) + ":" + String(selCode.encoding) + ":" + String(selCode.bits) + "</span></h2>\n");
  server->sendContent("        </div></div>\n");
  server->sendContent("          <form action='/store' method='POST'>\n");
  server->sendContent("          <div class='form-group row'>\n");
  server->sendContent("              <label for='name_input' class='col-sm-2 col-form-label col-form-label-sm'>Select the remote channel:</label>\n");
  server->sendContent("              <div class='col-sm-10'>\n");
  server->sendContent("              <select class='form-control form-control-sm' name='name_input' name='id'>\n");
  server->sendContent("               <option>toogle_rgb</option>\n");
  server->sendContent("               <option>toogle_w</option>\n");  
  server->sendContent("               <option>brighten_rgb</option>\n");
  server->sendContent("               <option>din_rgb</option>\n");
  server->sendContent("               <option>brighten_w</option>\n");
  server->sendContent("               <option>din_w</option>\n");
  server->sendContent("               <option>din_on</option>\n");
  server->sendContent("               <option>toogle</option>\n");
  server->sendContent("               <option>red</option>\n");
  server->sendContent("               <option>green</option>\n");
  server->sendContent("               <option>blue</option>\n");
  server->sendContent("               <option>white</option>\n");
  server->sendContent("               <option>mem_1</option>\n");
  server->sendContent("               <option>mem_2</option>\n");
  server->sendContent("               <option>mem_3</option>\n");
  server->sendContent("               </select>\n");
  server->sendContent("              </div>\n");
  server->sendContent("          </div>\n");
  server->sendContent("          <div class='form-group row'>\n");
  server->sendContent("              <div class='col-sm-10'>\n");
  server->sendContent("              <button type='submit' formaction='/store?id=" + String(ids) + "' class='btn btn-danger'>Store</button>\n");    
  server->sendContent("              </div>\n");
  server->sendContent("          </div>\n");     
  server->sendContent("          </form>\n");
  server->sendContent("      <hr />\n");      
  sendFooter();
}

void listStoredCodes() {

  sendHeader(200);
  server->sendContent("      <div class='row'>\n");
  server->sendContent("        <div class='col-md-12'>\n");
  server->sendContent("          <h3>Codes Stored</h3>\n");
  server->sendContent("          <table class='table table-striped' style='table-layout: fixed;'>\n");
  server->sendContent("            <thead><tr><th>Button Name</th><th>Data</th><th>Action</th></tr></thead>\n"); //Title
  server->sendContent("            <tbody>\n");

  DynamicJsonBuffer jsonCodeBtnBuffer;
  JsonArray& rootbtn = jsonCodeBtnBuffer.parseArray(listcodefiles("/codes/"));
  if (rootbtn.size() != 0) {
    if (SPIFFS.begin()) {
      for (auto v : rootbtn) {
        String vtxt = v["name"];
        vtxt.toLowerCase();
        if (SPIFFS.exists(vtxt)) {
          File codeFileRead = SPIFFS.open(vtxt, "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            String jsonStringdata = filebtn["data"];
            String jsonStringbtn = filebtn["name"];
            jsonReadCodeFileBuffer.clear();
            if(filebtn.containsKey("data") && filebtn.containsKey("name")) server->sendContent("<tr class='text-uppercase'><td>" + jsonStringbtn + "</td><td>" + jsonStringdata + "</td><td> <a class='btn btn-primary btn-xs' href='" + vtxt + "' role='button'>Download</a> <a class='btn btn-danger btn-xs' href='/listcodes?item=" + jsonStringbtn + "' role='button'>Delete</a></td></tr>\n");
          }
        }   
      }
    }
  } else {
    server->sendContent("              <tr><td colspan='3' class='text-center'><em>No codes stored</em></td></tr>");
  }
  jsonCodeBtnBuffer.clear();
  
  server->sendContent("            </tbody>\n");
  server->sendContent("          </table>\n");
  server->sendContent("         </div></div>\n");
  server->sendContent("       <p></p>\n");
  sendFooter();


}

static void send_redirect(const String &redirect) {
  String html;
  html += F("HTTP/1.1 301 OK\r\n");
  html += F("Location: ");
  html += redirect;
  html += F("\r\n");
  html += F("Cache-Control: no-cache\r\n\r\n");
  server->sendContent(html);
}

void read_stored_command(){
  
  DynamicJsonBuffer jsonCodeBtnBuffer;
  JsonArray& rootbtn = jsonCodeBtnBuffer.parseArray(listcodefiles("/codes/"));
  if (rootbtn.size() != 0) {
    if (SPIFFS.begin()) {
        if (SPIFFS.exists("/codes/toogle_rgb.json")) {
          File codeFileRead = SPIFFS.open("/codes/toogle_rgb.json", "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            if (filebtn.containsKey("data")) strcpy(toogle_rgb_data ,filebtn["data"]);
            Serial.print("toogle_rgb_data: ");Serial.println(toogle_rgb_data);
            jsonReadCodeFileBuffer.clear();
        }   
      } else { strcpy(toogle_rgb_data ,limpio);
      }  
        if (SPIFFS.exists("/codes/toogle_w.json")) {
          File codeFileRead = SPIFFS.open("/codes/toogle_w.json", "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            if (filebtn.containsKey("data")) strcpy(toogle_w_data ,filebtn["data"]);
            Serial.print("toogle_w_data: ");Serial.println(toogle_w_data);
            jsonReadCodeFileBuffer.clear();
        }   
      } else { strcpy(toogle_w_data ,limpio);
      }       
        if (SPIFFS.exists("/codes/brighten_rgb.json")) {
          File codeFileRead = SPIFFS.open("/codes/brighten_rgb.json", "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            if (filebtn.containsKey("data")) strcpy(brighten_rgb_data ,filebtn["data"]);
            Serial.print("brighten_rgb_data: ");Serial.println(brighten_rgb_data);
            jsonReadCodeFileBuffer.clear();
        }   
      } else { strcpy(brighten_rgb_data ,limpio);
      }  
        if (SPIFFS.exists("/codes/din_rgb.json")) {
          File codeFileRead = SPIFFS.open("/codes/din_rgb.json", "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            if (filebtn.containsKey("data")) strcpy(dim_rgb_data ,filebtn["data"]);
            Serial.print("dim_rgb_data: ");Serial.println(dim_rgb_data);
            jsonReadCodeFileBuffer.clear();
        }   
      } else { strcpy(dim_rgb_data ,limpio);
      }  
        if (SPIFFS.exists("/codes/brighten_w.json")) {
          File codeFileRead = SPIFFS.open("/codes/brighten_w.json", "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            if (filebtn.containsKey("data")) strcpy(brighten_w_data ,filebtn["data"]);
            Serial.print("brighten_w_data: ");Serial.println(brighten_w_data);
            jsonReadCodeFileBuffer.clear();
        }   
      } else { strcpy(brighten_w_data ,limpio);
      }  
        if (SPIFFS.exists("/codes/din_w.json")) {
          File codeFileRead = SPIFFS.open("/codes/din_w.json", "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            if (filebtn.containsKey("data")) strcpy(dim_w_data ,filebtn["data"]);
            Serial.print("dim_w_data: ");Serial.println(dim_w_data);
            jsonReadCodeFileBuffer.clear();
        }   
      } else { strcpy(dim_w_data ,limpio);
      }
        if (SPIFFS.exists("/codes/din_on.json")) {
          File codeFileRead = SPIFFS.open("/codes/din_on.json", "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            if (filebtn.containsKey("data")) strcpy(dim_on_data ,filebtn["data"]);
            Serial.print("dim_on_data: ");Serial.println(dim_on_data);
            jsonReadCodeFileBuffer.clear();
        }   
      } else { strcpy(dim_on_data ,limpio);
      } 
        if (SPIFFS.exists("/codes/toogle.json")) {
          File codeFileRead = SPIFFS.open("/codes/toogle.json", "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            if (filebtn.containsKey("data")) strcpy(toogle_data ,filebtn["data"]);
            Serial.print("toogle_data: ");Serial.println(toogle_data);
            jsonReadCodeFileBuffer.clear();
        }   
      } else { strcpy(toogle_data ,limpio);
      } 
        if (SPIFFS.exists("/codes/red.json")) {
          File codeFileRead = SPIFFS.open("/codes/red.json", "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            if (filebtn.containsKey("data")) strcpy(red_data ,filebtn["data"]);
            Serial.print("red_data: ");Serial.println(red_data);
            jsonReadCodeFileBuffer.clear();
        }   
      } else { strcpy(red_data ,limpio);
      }
        if (SPIFFS.exists("/codes/green.json")) {
          File codeFileRead = SPIFFS.open("/codes/green.json", "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            if (filebtn.containsKey("data")) strcpy(green_data ,filebtn["data"]);
            Serial.print("green_data: ");Serial.println(green_data);
            jsonReadCodeFileBuffer.clear();
        }   
      } else { strcpy(green_data ,limpio);
      }
        if (SPIFFS.exists("/codes/blue.json")) {
          File codeFileRead = SPIFFS.open("/codes/blue.json", "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            if (filebtn.containsKey("data")) strcpy(blue_data ,filebtn["data"]);
            Serial.print("blue_data: ");Serial.println(blue_data);
            jsonReadCodeFileBuffer.clear();
        }   
      } else { strcpy(blue_data ,limpio);
      }
        if (SPIFFS.exists("/codes/white.json")) {
          File codeFileRead = SPIFFS.open("/codes/white.json", "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            if (filebtn.containsKey("data")) strcpy(white_data ,filebtn["data"]);
            Serial.print("white_data: ");Serial.println(white_data);
            jsonReadCodeFileBuffer.clear();
        }   
      } else { strcpy(white_data ,limpio);
      }
        if (SPIFFS.exists("/codes/mem_1.json")) {
          File codeFileRead = SPIFFS.open("/codes/mem_1.json", "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            if (filebtn.containsKey("data")) strcpy(mem_1_data ,filebtn["data"]);
            Serial.print("mem_1_data: ");Serial.println(mem_1_data);
            jsonReadCodeFileBuffer.clear();
        }   
      } else { strcpy(mem_1_data ,limpio);
      }
        if (SPIFFS.exists("/codes/mem_2.json")) {
          File codeFileRead = SPIFFS.open("/codes/mem_2.json", "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            if (filebtn.containsKey("data")) strcpy(mem_2_data ,filebtn["data"]);
            Serial.print("mem_2_data: ");Serial.println(mem_2_data);
            jsonReadCodeFileBuffer.clear();
        }   
      } else { strcpy(mem_2_data ,limpio);
      }
        if (SPIFFS.exists("/codes/mem_3.json")) {
          File codeFileRead = SPIFFS.open("/codes/mem_3.json", "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            if (filebtn.containsKey("data")) strcpy(mem_3_data ,filebtn["data"]);
            Serial.print("mem_3_data: ");Serial.println(mem_3_data);
            jsonReadCodeFileBuffer.clear();
        }   
      } else { strcpy(mem_3_data ,limpio);
      }                                                                      
    }
  } else {
    Serial.println("No Stored Data");
  }
  jsonCodeBtnBuffer.clear();
}

String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

String encoding(decode_results *results) {
  String output;
  switch (results->decode_type) {
    default:
    case UNKNOWN:      output = "UNKNOWN";            break;
    case NEC:          output = "NEC";                break;
    case SONY:         output = "SONY";               break;
    case RC5:          output = "RC5";                break;
    case RC5X:         output = "RC5X";               break;    
    case RC6:          output = "RC6";                break;
    case DISH:         output = "DISH";               break;
    case SHARP:        output = "SHARP";              break;
    case JVC:          output = "JVC";                break;
    case SANYO:        output = "SANYO";              break;
    case SANYO_LC7461: output = "SANYO_LC7461";       break;
    case MITSUBISHI:   output = "MITSUBISHI";         break;
    case SAMSUNG:      output = "SAMSUNG";            break;
    case LG:           output = "LG";                 break;
    case WHYNTER:      output = "WHYNTER";            break;
    case AIWA_RC_T501: output = "AIWA_RC_T501";       break;
    case PANASONIC:    output = "PANASONIC";          break;
    case DENON:        output = "DENON";              break;
    case COOLIX:       output = "COOLIX";             break;
    case GREE:         output = "GREE";               break;
    case HITACHI_AC1:  output = "HITACHI_AC1";        break;
    case HITACHI_AC:   output = "HITACHI_AC";         break;
    case HAIER_AC:     output = "HAIER_AC";           break;
    case CARRIER_AC:   output = "CARRIER_AC";         break;
    case LASERTAG:     output = "LASERTAG";           break;
    case FUJITSU_AC:   output = "FUJITSU_AC";         break;
    case MIDEA:        output = "MIDEA";              break;
    case TROTEC:       output = "TROTEC";             break;
    case TOSHIBA_AC:   output = "TOSHIBA_AC";         break;
    case DAIKIN:       output = "DAIKIN";             break;
    case KELVINATOR:   output = "KELVINATOR";         break;
    case MAGIQUEST:    output = "MAGIQUEST";          break;
    case NIKAI:        output = "NIKAI";              break;
    case RCMM:         output = "RCMM";               break;
    case GICABLE:      output = "GICABLE";            break;
    case MITSUBISHI_AC:output = "MITSUBISHI_AC";      break;
    case MITSUBISHI2:  output = "MITSUBISHI2";        break;
    case ARGO:         output = "ARGO";               break;
  }
  return output;
}

void storeReceivedCode (Code code, String btn) {
  //generate the nested json from the code
  DynamicJsonBuffer jsonWriteBuffer;  
  JsonObject& jsonadd = jsonWriteBuffer.createObject();
  btn.toLowerCase();
  jsonadd["name"] = btn;       
   
    jsonadd["type"] = code.encoding;
    jsonadd["data"] = code.data;
    jsonadd["length"] = code.bits;

  if (saveJSON(jsonadd, btn)) sendHomePage("Code stored to " + btn, "Alert", 2, 200); 
  jsonWriteBuffer.clear(); 
}

bool DeleteJSONitem (String btn) {
  btn.toLowerCase();
  if (SPIFFS.begin()) {
    SPIFFS.remove("/codes/"+ btn +".json");
    read_stored_command();
    return true;
  } else {
    return false;
  }
}

bool deletecodefiles(String path) {
  if(!path) path = "/codes/";
  path.toLowerCase();
  Serial.println("deleteFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  while(dir.next()){
    SPIFFS.remove(dir.fileName());
  }
  read_stored_command();
  return true;  
}

bool saveJSON (JsonObject& json, String btn){
  btn.toLowerCase();
  if (SPIFFS.begin()) {     

    if (SPIFFS.exists("/codes/" + btn +".json")) DeleteJSONitem(btn);
    File codeFileWrite = SPIFFS.open("/codes/" + btn +".json", "w"); 
    if (codeFileWrite) {
      json.printTo(codeFileWrite);
      json.prettyPrintTo(Serial);
      codeFileWrite.close();      
      Serial.println("Codes written successfully");
      read_stored_command();
      return true;
    }
  }
}

String listcodefiles(String path) {
  path.toLowerCase();

  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while(dir.next()){
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"name\":\"";
    output += String(dir.fileName());
    output += "\"}";
  }
  output += "]"; 
  return output;
}

void handleFileUpload(){
  if(server->uri() != "/listcodes") return;
  HTTPUpload& upload = server->upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/codes/")) filename = "/codes/"+filename;
    filename.toLowerCase();
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile) { 
      File codeFileCheck = SPIFFS.open(fsUploadFile.name(), "r");
      DynamicJsonBuffer jsonCheckBuffer;
      JsonObject& filebtn = jsonCheckBuffer.parseObject(codeFileCheck);  
      codeFileCheck.close();           
      String jsonStringname = filebtn["name"];
      jsonStringname = "/codes/" + jsonStringname + ".json";
      jsonStringname.toLowerCase();
      jsonCheckBuffer.clear();
      Serial.println(jsonStringname + " vs. " + fsUploadFile.name());
      if(jsonStringname != fsUploadFile.name()) {
        SPIFFS.remove(fsUploadFile.name()); 
        Serial.println("Object doesnt match, file deleted");
        send_redirect("/?message=Code file mismatch&type=2&header=Alert&httpcode=400"); 
      } else {
        send_redirect("/listcodes"); 
      }
      fsUploadFile.close();
    } else {
      Serial.print("Error uploading ");
      send_redirect("/?message=Error uploading file&type=2&header=Alert&httpcode=200"); 
    }
  }
}

bool handleFileRead(String path){
  path.toLowerCase();
  Serial.println("handleFileRead: " + path);
  if(SPIFFS.exists(path)){
    File file = SPIFFS.open(path, "r");
    size_t sent = server->streamFile(file, "application/octet-stream");
    file.close();
    return true;
  }
  return false;
}

void cvrtCode(Code& codeData, decode_results *results)
{
  strncpy(codeData.data, uint64ToString(results->value, 16).c_str(), 40);
  strncpy(codeData.encoding, encoding(results).c_str(), 14);
  codeData.bits = results->bits;

}

void copyCode (Code& c1, Code& c2) {
  strncpy(c2.data, c1.data, 40);
  strncpy(c2.encoding, c1.encoding, 14);
  c2.bits = c1.bits;
  c2.valid = c1.valid;
}
void loop() { // ============================================================================= LOOP ===========================================================

  if (initialConfig == true){ondemandwifiCallback();}
  
  buttonA.loop(); 
  SuplaDevice.iterate();
  delay(22);
  
  if (millis() > mem_milis){
      if (rgbw_update) save_rgbw_mem();
      mem_milis = (millis()+3000); 
    }
   
  int C_W_read = digitalRead(CONFIG_PIN);  
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
  decode_results  results;                                        

  if (irrecv.decode(&results) && !holdReceive) {                 
    Serial.println("Signal received:");

   if (digitalRead(ledpin)) {
    
    copyCode(last_recv_4, last_recv_5);                          
    copyCode(last_recv_3, last_recv_4);                         
    copyCode(last_recv_2, last_recv_3);                        
    copyCode(last_recv, last_recv_2);                            
    cvrtCode(last_recv, &results);                               
    last_recv.valid = true;

     Serial.print("value_ir: ");Serial.println(last_recv.data);

       if(strcmp(last_recv.data, toogle_rgb_data) == 0)
        {                      
            
                 mirgbw->trigger(0, Supla::TOGGLE_RGB);
                 Serial.println("IR TOGGLE_RGB"); 
             
         }
       else if(strcmp(last_recv.data, toogle_w_data) == 0) 
         {
           
                 mirgbw->trigger(0, Supla::TOGGLE_W);
                 Serial.println("IR TOGGLE_W");         
         }
       else if(strcmp(last_recv.data, brighten_rgb_data) == 0)  
        {                      
            
                 mirgbw->trigger(0, Supla::BRIGHTEN_RGB);
                 Serial.println("IR BRIGHTEN_RGB");  
           
         }           
       else if(strcmp(last_recv.data, dim_rgb_data) == 0) 
         {
           
                 mirgbw->trigger(0, Supla::DIM_RGB);
                 Serial.println("IR DIM_RGB");         
         }
       else if(strcmp(last_recv.data, brighten_w_data) == 0)
        {                      
            
                 mirgbw->trigger(0, Supla::BRIGHTEN_W);
                 Serial.println("IR BRIGHTEN_W");   
             
         }
       else if(strcmp(last_recv.data, dim_w_data) == 0) 
         {
            
                 mirgbw->trigger(0, Supla::DIM_W);
                 Serial.println("IR DIM_W"); 
                         
         }
       else if(strcmp(last_recv.data, dim_on_data) == 0)  
         {

                 mirgbw->setRGBW(-1, -1, -1, 20, 20);
                 Serial.println("IR TURN_ON_ALL_DIMMED"); 
                 
         }            
       else if(strcmp(last_recv.data, toogle_data) == 0) 
         {

                 mirgbw->trigger(0, Supla::TOGGLE);
                 Serial.println("IR TOGGLE"); 
                       
         }
       else if(strcmp(last_recv.data, red_data) == 0)  
         {

                 mirgbw->setRGBW(255, 0, 0, -1, -1);
                 Serial.println("IR RED"); 
                 
         }
       else if(strcmp(last_recv.data, green_data) == 0)  
         {

                 mirgbw->setRGBW(0, 255, 0, -1, -1);
                 Serial.println("IR GREEN"); 
                 
         }
       else if(strcmp(last_recv.data, blue_data) == 0)  
         {

                 mirgbw->setRGBW(0, 0, 255, -1, -1);
                 Serial.println("IR BLUE"); 
                 
         }
       else if(strcmp(last_recv.data, white_data) == 0)  
         {

                 mirgbw->setRGBW(255, 255, 255, -1, -1);
                 Serial.println("IR WHITE"); 
                 
         }
       else if(strcmp(last_recv.data, mem_1_data) == 0)  
         {

                 mirgbw->setRGBW(EEPROM.read(25), EEPROM.read(26), EEPROM.read(27), EEPROM.read(28), EEPROM.read(29));
                 Serial.println("IR MEM 1"); 
                 
         }
       else if(strcmp(last_recv.data, mem_2_data) == 0)  
         {

                 mirgbw->setRGBW(EEPROM.read(35), EEPROM.read(36), EEPROM.read(37), EEPROM.read(38), EEPROM.read(39));
                 Serial.println("IR MEM 2"); 
                 
         }
       else if(strcmp(last_recv.data, mem_3_data) == 0)  
         {

                 mirgbw->setRGBW(EEPROM.read(45), EEPROM.read(46), EEPROM.read(47), EEPROM.read(48), EEPROM.read(49));
                 Serial.println("IR MEM 3"); 
             
         }
                               
   }
    irrecv.resume();                                              
    digitalWrite(ledpin, LOW);                                  
    ticker.attach(0.3, disableLed);
  } 

  if (WiFi.status() == WL_CONNECTED){    
    if (pr_wifi == true){
     Serial.println("");
     Serial.println("CONNECTED"); 
     pr_wifi = false;        
      if (server != NULL) {
        delete server;
      }
     server = new ESP8266WebServer(web_port);
     httpUpdater.setup(server, "/update", "admin", "pass");
     if (!MDNS.begin(Supla_name)) {Serial.println("Error setting up MDNS responder!");}
     MDNS.addService("http", "tcp", web_port); 
     Serial.println("MDNS http service added. Hostname is set to http://" + String(Supla_name) + ".local:" + String(web_port));

  server->on("/store", []() {
    Serial.println("Connection received to store code page");
    int id = server->arg("id").toInt();
    Serial.print("store id: ");Serial.println(id);
    String btn = server->arg("name_input");
    btn.toLowerCase();         
    String output;
    if (id == 1 && last_recv.valid) {if (btn != "") {storeReceivedCode(last_recv, btn);  } else { storeCodePage(last_recv,1,200);}                              
    } else if (id == 2 && last_recv_2.valid) {if (btn != "") {storeReceivedCode(last_recv_2, btn); } else {storeCodePage(last_recv_2,2,200);}                                 
    } else if (id == 3 && last_recv_3.valid) {if (btn != "") {storeReceivedCode(last_recv_3, btn); } else {storeCodePage(last_recv_3,3,200);}                               
    } else if (id == 4 && last_recv_4.valid) {if (btn != "") {storeReceivedCode(last_recv_4, btn); } else {storeCodePage(last_recv_4,4,200);}                                 
    } else if (id == 5 && last_recv_5.valid) {if (btn != "") {storeReceivedCode(last_recv_5, btn); } else {storeCodePage(last_recv_5,5,200);}                                    
    } else { sendHomePage("Code does not exist", "Alert", 2, 404);}         
  });
  
  server->on("/listcodes", HTTP_POST, []() {server->send(200, "text/plain", "");}, {handleFileUpload});
  
  server->on("/listcodes", []() {
    String item = server->arg("item");
    if (server->hasArg("item")) {
      Serial.println("Connection received - delete item " + item);
      if (DeleteJSONitem(item)) listStoredCodes();     
    }
    listStoredCodes();
  });
  
  server->on("/deleteall", []() {
    if(deletecodefiles("/codes/")) sendHomePage("All Stored Codes DELETED", "Alert", 2, 404);
  });
  
  server->on("/controlmem", []() {
    Serial.println("Connection received to control page");
    int canal = server->arg("id").toInt();
    canal = canal * 10;
    Serial.print("MEM: ");Serial.println(canal);
    EEPROM.write(15 + canal,hw_red);EEPROM.write(16 + canal,hw_green);EEPROM.write(17 + canal,hw_blue);EEPROM.write(18 + canal,hw_colorBrightness);EEPROM.write(19 + canal,hw_brightness);
    EEPROM.commit();
 
    sendHomePage("MEM Set", "Alert", 2, 404);
  });
  
  server->on("/", []() {
    if (server->hasArg("message")) {
        rtnMessage.message = server->arg("message"); 
        rtnMessage.type = server->arg("type").toInt();
        rtnMessage.title = server->arg("header");
        rtnMessage.httpcode = server->arg("httpcode").toInt();
        sendHomePage(rtnMessage.message, rtnMessage.title, rtnMessage.type, rtnMessage.httpcode);      
    } 
    Serial.println("Connection received");
    sendHomePage();
  });
  
  server->onNotFound( []() {
    if(!handleFileRead(server->uri()))
      sendHomePage("Resource not found", "Alert", 2, 404);
  });
     
     server->begin();
     Serial.println("HTTP Server started on port " + String(web_port));
    }
    MDNS.update();
    server->handleClient();
     
   }
        
}
void click(Button3& btn) {
   mirgbw->trigger(0, Supla::TOGGLE_RGB);
   Serial.println("click TOGGLE_RGB");
}
void doubleClick(Button3& btn) {
   mirgbw->trigger(0, Supla::TOGGLE_W);
   Serial.println("doubleClick TOGGLE_W");
}
void tripleClick(Button3& btn) {
   mirgbw->trigger(0, Supla::DIM_ALL);
   Serial.println("tripleClick DIM_ALL");
}
void quatleClick(Button3& btn) {
   mirgbw->trigger(0, Supla::BRIGHTEN_ALL);
   Serial.println("quatleClick BRIGHTEN_ALL");
}
void quintleClick(Button3& btn) {
  // mirgbw->trigger(0, Supla::TURN_ON_ALL_DIMMED);
  mirgbw->setRGBW(-1, -1, -1, 20, 20);
   Serial.println("quintleClick set: -1, -1, -1, 20, 20");
}
void longClick(Button3& btn) {
   mirgbw->toggle();
   Serial.println("longClick toggle");
}
   //mirgbw->setDefaultDimmedBrightness(int dimmedBrightness);
   //mirgbw->setFadeEffectTime(int timeMs);
   //mirgbw->onTimer();
   //mirgbw->setStep(int step);
   //mirgbw->trigger(int trigger, int action);
   //mirgbw->setRGBW(int red, int green, int blue, int colorBrightness, int brightness)
   
void save_rgbw_mem(){
  bool save = false;
  if (hw_red != EEPROM.read(10)){ EEPROM.write(10,hw_red);bool save = true;}
  if (hw_green != EEPROM.read(11)){ EEPROM.write(11,hw_green);save = true;}
  if (hw_blue != EEPROM.read(12)){ EEPROM.write(12,hw_blue);save = true;} 
  if (hw_colorBrightness != EEPROM.read(13)){ EEPROM.write(13,hw_colorBrightness);save = true;} 
  if (hw_brightness != EEPROM.read(14)){ EEPROM.write(14,hw_brightness);save = true;} 
  if (save == true){ 
    EEPROM.commit();
    Serial.print("SAVE RGBW to Mem: ");Serial.print(hw_red);Serial.print(",");Serial.print(hw_green);Serial.print(",");Serial.print(hw_blue);Serial.print(",");Serial.print(hw_colorBrightness);Serial.print(",");Serial.print(hw_brightness);Serial.println(",");   
    rgbw_update = false;
  }  
}
void read_rgbw_mem(){
  hw_red = EEPROM.read(10);hw_green = EEPROM.read(11);hw_blue = EEPROM.read(12);hw_colorBrightness = EEPROM.read(13);hw_brightness = EEPROM.read(14);
  Serial.println("Epp Read ");Serial.println(hw_red);Serial.println(hw_green);Serial.println(hw_blue);Serial.println(hw_colorBrightness);Serial.println(hw_brightness);

}
  
