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

#include <FS.h>       // ---- esp board manager 2.7.1 
#include <SuplaDevice.h>  // SoftVer, "2.3.1-mod"
#include <supla/io.h>
//#include <supla/sensor/DS18B20.h>
#include <WiFiManager.h>
#include <ArduinoJson.h> //--------- https://github.com/bblanchon/ArduinoJson/tree/v5.13.2 ------
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266TrueRandom.h>
#include <WiFiClientSecure.h>
#include <SoftwareSerial.h>
#include <Ticker.h>
#include <supla/network/esp_wifi.h>
Supla::ESPWifi wifi("", ""); 
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

#define BTN_COUNT 8
int wificonfig_pin = 0;   //D3
int ledpin = 2;          //D4
int C_W_state = HIGH; 
int last_C_W_state = HIGH;
unsigned long time_last_C_W_change = 0; 
long C_W_delay = 5000;               // ---------------------- config delay 5 seconds ---------------------------
char Supla_server[81]=("Set server address");
char Email[81]=("set email address");
char Supla_name[51]=("Supla_EDM");
char Supla_status[51];
bool shouldSaveConfig = false;
bool initialConfig = false;
bool state10 = true;
bool starting = true;
bool local_triger = false;
int s;
char GUID[SUPLA_GUID_SIZE];
char AUTHKEY[SUPLA_AUTHKEY_SIZE];
byte uuidNumber[16]; 
static const char logo[] PROGMEM = "<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>";
static const char str_a[] PROGMEM = "<br><div style='text-align:center;'><label for='pres1'>-- Relay 1 --</label><br><input type='radio'name='pres1'value='1'checked>Last State<input type='radio'name='pres1'value='2'>On<input type='radio'name='pres1'value='3'>Off</div>";
static const char str_b[] PROGMEM = "<br><div style='text-align:center;'><label for='pres2'>-- Relay 2 --</label><br><input type='radio'name='pres2'value='1'checked>Last State<input type='radio'name='pres2'value='2'>On<input type='radio'name='pres2'value='3'>Off</div>";
static const char str_c[] PROGMEM = "<br><div style='text-align:center;'><label for='pres3'>-- Relay 3 --</label><br><input type='radio'name='pres3'value='1'checked>Last State<input type='radio'name='pres3'value='2'>On<input type='radio'name='pres3'value='3'>Off</div>";
static const char str_d[] PROGMEM = "<br><div style='text-align:center;'><label for='pres4'>-- Relay 4 --</label><br><input type='radio'name='pres4'value='1'checked>Last State<input type='radio'name='pres4'value='2'>On<input type='radio'name='pres4'value='3'>Off</div>";
static const char str_e[] PROGMEM = "<br><div style='text-align:center;'><label for='pres5'>-- Relay 5 --</label><br><input type='radio'name='pres5'value='1'checked>Last State<input type='radio'name='pres5'value='2'>On<input type='radio'name='pres5'value='3'>Off</div>";
static const char str_f[] PROGMEM = "<br><div style='text-align:center;'><label for='pres6'>-- Relay 6 --</label><br><input type='radio'name='pres6'value='1'checked>Last State<input type='radio'name='pres6'value='2'>On<input type='radio'name='pres6'value='3'>Off</div>";
static const char str_g[] PROGMEM = "<br><div style='text-align:center;'><label for='pres7'>-- Relay 7 --</label><br><input type='radio'name='pres7'value='1'checked>Last State<input type='radio'name='pres7'value='2'>On<input type='radio'name='pres7'value='3'>Off</div>";
static const char str_h[] PROGMEM = "<br><div style='text-align:center;'><label for='pres8'>-- Relay 8 --</label><br><input type='radio'name='pres8'value='1'checked>Last State<input type='radio'name='pres8'value='2'>On<input type='radio'name='pres8'value='3'>Off</div>";
byte dataBusReceived[5] = {0x55, 0x00, 0x00, 0x00, 0x00};
byte dataBusSendrelays[5] = {0x55, 0x01, 0x53, 0x00, 0x00};
byte dataBusSetIn[5] = {0x55, 0x01, 0x49, 0xFF, 0xB7};  // all off(55 01 49 00 82) all on(55 01 49 FF B7)
uint8_t relays_state = B0;
uint8_t in_state = B0;
uint8_t last_in_state = B0;
uint8_t saved_relays_state = B0;
unsigned long save_milis = 0;

ESP8266WebServer httpServer(81);
ESP8266HTTPUpdateServer httpUpdater;
WiFiManager wifiManager;
SoftwareSerial swSer;
Ticker ticker;  
WiFiManagerParameter field_a;
WiFiManagerParameter field_b;
WiFiManagerParameter field_c;
WiFiManagerParameter field_d;
WiFiManagerParameter field_e;
WiFiManagerParameter field_f;
WiFiManagerParameter field_g;
WiFiManagerParameter field_h;

typedef struct { 
  int pin;
  char last_val;
  int ms;
  unsigned long last_time;
  int preset;
} _btn_t;

_btn_t btn[BTN_COUNT];

class RelayBoard : public Supla::Io {
  public:
  
  void customDigitalWrite(int channelNumber, uint8_t pin, uint8_t val) {

    if (pin <= 99) {
        return ::digitalWrite(pin,val);   
      }
      
        if (val == 1){
          bitSet(relays_state, channelNumber); 
        }else{
          bitClear(relays_state, channelNumber); 
        }
                   
         if ((state10 == false) && (saved_relays_state != relays_state)){
            dataBusSendrelays[3] = relays_state;
            dataBusSendrelays[4] = Relays_crc8(dataBusSendrelays, 4);
            swSer.write(dataBusSendrelays, sizeof(dataBusSendrelays));
            save_milis = millis() +5000;
            saved_relays_state = relays_state;
            Serial.print("RelayBoard Set = ");
            Serial.println(relays_state, BIN);
         }

           if ((state10 == false) && (local_triger == false) && (SuplaDevice.channel_pin[channelNumber].DurationMS != btn[channelNumber].ms) && (!val)) {
            int durationMS = SuplaDevice.channel_pin[channelNumber].DurationMS;
               Serial.print("button_duration: ");Serial.print(durationMS);           
               Serial.print(" button: ");Serial.println(channelNumber);               
               EEPROM.put((channelNumber * 10) + 600, durationMS);
               EEPROM.commit();
               btn[channelNumber].ms = durationMS;
           }  
        local_triger = false;               

    return;  
 }  
  
  int customDigitalRead(int channelNumber, uint8_t pin) {
    
    if (pin <= 99){
        return ::digitalRead(pin);  
    }
        return bitRead(relays_state, channelNumber);                       
  }
    
  uint8_t Relays_crc8(uint8_t* data, uint8_t length){

    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t mix = 0;
    uint8_t crc = 0;
    uint8_t byte = 0;

    for (i = 1; i < length; i++) {
        byte = data[i];

        for (j = 0; j < 8; j++) {
            mix = (crc ^ byte) & 0x01;
            crc >>= 1;
            if (mix)
                crc ^= 0x8C;
            byte >>= 1;
        }
    }
    return crc;
  }

}RelayBoard;

void supla_btn_init() { 
   for(int a=0;a<BTN_COUNT;a++)
    if (btn[a].pin > 0) {
        btn[a].last_val = LOW;
        btn[a].last_time = millis();
    }
}

void tick()
{
  int state = digitalRead(ledpin);  
  digitalWrite(ledpin, !state);     
}

void saveConfigCallback () {          
  shouldSaveConfig = true;
}

void ondemandwifiCallback () {
  
   ticker.attach(0.5, tick);

   httpServer.stop();
   
   WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 81,"required");
   WiFiManagerParameter custom_Email("email", "Email", Email, 81,"required");
   WiFiManagerParameter custom_Supla_name("name", "Supla Device Name", Supla_name, 51,"required");

  new (&field_a) WiFiManagerParameter(str_a); 
  new (&field_b) WiFiManagerParameter(str_b); 
  new (&field_c) WiFiManagerParameter(str_c); 
  new (&field_d) WiFiManagerParameter(str_d); 
  new (&field_e) WiFiManagerParameter(str_e); 
  new (&field_f) WiFiManagerParameter(str_f); 
  new (&field_g) WiFiManagerParameter(str_g); 
  new (&field_h) WiFiManagerParameter(str_h); 

   WiFiManagerParameter custom_html_id21("<div><h4> - Supla State -   ");
   WiFiManagerParameter custom_html_id22( Supla_status);
   WiFiManagerParameter custom_html_id23( "</h4></div>");  
  
  wifiManager.setSaveParamsCallback(saveParamCallback);
   
   wifiManager.setBreakAfterConfig(true);
   wifiManager.setSaveConfigCallback(saveConfigCallback);
   
   wifiManager.setCustomHeadElement(logo);  
   wifiManager.addParameter(&custom_Supla_server);
   wifiManager.addParameter(&custom_Email);
   wifiManager.addParameter(&custom_Supla_name);
   wifiManager.addParameter(&field_a);
   wifiManager.addParameter(&field_b);
   wifiManager.addParameter(&field_c);
   wifiManager.addParameter(&field_d);
   wifiManager.addParameter(&field_e);
   wifiManager.addParameter(&field_f);
   wifiManager.addParameter(&field_g);
   wifiManager.addParameter(&field_h);
   wifiManager.addParameter(&custom_html_id21);
   wifiManager.addParameter(&custom_html_id22);
   wifiManager.addParameter(&custom_html_id23);   


   wifiManager.setMinimumSignalQuality(8);
   //wifiManager.setShowStaticFields(true); // force show static ip fields
   //wifiManager.setShowDnsFields(true);    // force show dns field always
   wifiManager.setConfigPortalTimeout(300);

   if (!wifiManager.startConfigPortal("Supla_EDM")) { Serial.println("Not connected to WiFi but continuing anyway.");} else { Serial.println("connected...yeey :)");}             
    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(Email, custom_Email.getValue());
    strcpy(Supla_name, custom_Supla_name.getValue());
    if(strcmp(Supla_server, "get_new_guid_and_authkey") == 0){
      EEPROM.write(300, 0);
      EEPROM.commit();
      delay(100);
      ESP.reset(); 
    }
 
   ticker.detach();
   WiFi.softAPdisconnect(true);
   digitalWrite(ledpin, HIGH);
}
String getParam(String name){
  String value;
  if(wifiManager.server->hasArg(name)) {
    value = wifiManager.server->arg(name);
  }
  return value;
}

void saveParamCallback(){
  EEPROM.write(400, getParam("pres1").toInt());
  EEPROM.write(401, getParam("pres2").toInt());
  EEPROM.write(402, getParam("pres3").toInt());
  EEPROM.write(403, getParam("pres4").toInt()); 
  EEPROM.write(404, getParam("pres5").toInt());
  EEPROM.write(405, getParam("pres6").toInt());
  EEPROM.write(406, getParam("pres7").toInt());
  EEPROM.write(407, getParam("pres8").toInt());
  EEPROM.commit(); 
  Serial.println("PARAM preset Relay 1 = " + getParam("pres1"));
  Serial.println("PARAM preset Relay 2 = " + getParam("pres2"));
  Serial.println("PARAM preset Relay 3 = " + getParam("pres3"));
  Serial.println("PARAM preset Relay 4 = " + getParam("pres4"));
  Serial.println("PARAM preset Relay 5 = " + getParam("pres5"));
  Serial.println("PARAM preset Relay 6 = " + getParam("pres6"));
  Serial.println("PARAM preset Relay 7 = " + getParam("pres7"));
  Serial.println("PARAM preset Relay 8 = " + getParam("pres8"));
}

void status_func(int status, const char *msg) {    //    ------------------------ Status --------------------------
  if (s != status){
    s = status; 
      if (s != 10){
        strcpy(Supla_status, msg);
        Serial.println("Supla Status: " + String(Supla_status)); 
     }
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
    EEPROM.put(600, 0); EEPROM.put(610, 0); EEPROM.put(620, 0); EEPROM.put(630, 0); 
    EEPROM.put(640, 0); EEPROM.put(650, 0);EEPROM.put(660, 0); EEPROM.put(670, 0); 
    EEPROM.write(100, 0);        
    EEPROM.write(300, 60);
    EEPROM.commit();
    delay(0);
  }
  read_guid();
  read_authkey(); 
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
  
   Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY, 1);
   
   swSer.begin(57600, SWSERIAL_8N1, 4, 5);  // software serial : RX = digital pin 4, TX = digital pin 5

  delay(10);
  Serial.println(" ");
  Serial.println(" ");
  pinMode(wificonfig_pin, INPUT_PULLUP);
  pinMode(ledpin,OUTPUT);   
  digitalWrite(ledpin, HIGH);
  
  EEPROM.begin(1024);  
  if (EEPROM.read(300) != 60){initialConfig = true;} 
  
  guid_authkey();

  saved_relays_state = EEPROM.read(100);
  Serial.print("saved relays state: ");
  Serial.println(saved_relays_state, BIN);
    
  memset(btn, 0, sizeof(btn));
  btn[0].pin =1;
  btn[0].preset = EEPROM.read(400); 
  EEPROM.get(600,btn[0].ms);
  if (btn[0].ms > 10){
    Serial.print("Relay 1 time ms: ");Serial.println(btn[0].ms);
  }else{
    Serial.print("initial_state Relay 1 = ");
    if (btn[0].preset == 2){Serial.println("On");}else if (btn[0].preset == 3){Serial.println("Off");}else Serial.println("Last State");               
  }
  btn[1].pin =2;
  btn[1].preset = EEPROM.read(401);   
  EEPROM.get(610,btn[1].ms);
  if (btn[1].ms > 10){
    Serial.print("Relay 2 time ms: ");Serial.println(btn[1].ms);
  }else{
    Serial.print("initial_state Relay 2 = ");
    if (btn[1].preset == 2){Serial.println("On");}else if (btn[1].preset == 3){Serial.println("Off");}else Serial.println("Last State");         
  }
  btn[2].pin =3;
  btn[2].preset = EEPROM.read(402); 
  EEPROM.get(620,btn[2].ms);
  if (btn[2].ms > 10){
    Serial.print("Relay 3 time ms: ");Serial.println(btn[2].ms);
  }else{
    Serial.print("initial_state Relay 3 = ");
    if (btn[2].preset == 2){Serial.println("On");}else if (btn[2].preset == 3){Serial.println("Off");}else Serial.println("Last State");           
  }
  btn[3].pin =4;
  btn[3].preset = EEPROM.read(403);  
  EEPROM.get(630,btn[3].ms);
  if (btn[3].ms > 10){
    Serial.print("Relay 4 time ms: ");Serial.println(btn[3].ms);
  }else{
    Serial.print("initial_state Relay 4 = ");
    if (btn[3].preset == 2){Serial.println("On");}else if (btn[3].preset == 3){Serial.println("Off");}else Serial.println("Last State");          
  }
  btn[4].pin =5;
  btn[4].preset = EEPROM.read(404);   
  EEPROM.get(640,btn[4].ms);
  if (btn[4].ms > 10){
    Serial.print("Relay 5 time ms: ");Serial.println(btn[4].ms);
  }else{
    Serial.print("initial_state Relay 5 = ");
    if (btn[4].preset == 2){Serial.println("On");}else if (btn[4].preset == 3){Serial.println("Off");}else Serial.println("Last State");         
  }
  btn[5].pin =6;
  btn[5].preset = EEPROM.read(405);   
  EEPROM.get(650,btn[5].ms);
  if (btn[5].ms > 10){
    Serial.print("Relay 6 time ms: ");Serial.println(btn[5].ms);
  }else{
    Serial.print("initial_state Relay 6 = ");
    if (btn[5].preset == 2){Serial.println("On");}else if (btn[5].preset == 3){Serial.println("Off");}else Serial.println("Last State");        
  }
  btn[6].pin =7;
  btn[6].preset = EEPROM.read(406);  
  EEPROM.get(660,btn[6].ms);
  if (btn[6].ms > 10){
    Serial.print("Relay 7 time ms: ");Serial.println(btn[6].ms);
  }else{
    Serial.print("initial_state Relay 7 = ");
    if (btn[6].preset == 2){Serial.println("On");}else if (btn[6].preset == 3){Serial.println("Off");}else Serial.println("Last State");         
  }
  btn[7].pin =8;
  btn[7].preset = EEPROM.read(407);    
  EEPROM.get(670,btn[7].ms);
  if (btn[7].ms > 10){
    Serial.print("Relay 8 time ms: ");Serial.println(btn[7].ms);
  }else{
    Serial.print("initial_state Relay 8 = ");
    if (btn[7].preset == 2){Serial.println("On");}else if (btn[7].preset == 3){Serial.println("Off");}else Serial.println("Last State");         
  }
    supla_btn_init();
                
  if (WiFi.SSID()==""){ initialConfig = true;} 

  if (SPIFFS.begin()) {  // ------------------------- wificonfig read -----------------
    if (SPIFFS.exists("/config.json")) {
     // Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;         
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.prettyPrintTo(Serial);   //print config data to serial on startup
        Serial.println(" ");  
        if (json.success()) {//Serial.println("\nparsed json");         
          if (json.containsKey("Supla_server")) strcpy(Supla_server, json["Supla_server"]);
          if (json.containsKey("Email")) strcpy(Email, json["Email"]);
          if (json.containsKey("Supla_name")) strcpy(Supla_name, json["Supla_name"]);        
        } else {
          initialConfig = true;
        }
        configFile.close(); 
      }
    }
   } else {
    Serial.println("failed to mount FS");
  } 
  
   wifi_station_set_hostname(Supla_name);   
   WiFi.mode(WIFI_STA); 
   
      SuplaDevice.addRelay(100, false);
      SuplaDevice.addRelay(101, false); 
      SuplaDevice.addRelay(102, false); 
      SuplaDevice.addRelay(103, false); 
      SuplaDevice.addRelay(104, false);
      SuplaDevice.addRelay(105, false); 
      SuplaDevice.addRelay(106, false); 
      SuplaDevice.addRelay(107, false);
    
    SuplaDevice.setName(Supla_name);
    SuplaDevice.setStatusFuncImpl(&status_func);
    wifi.enableSSL(false);

    SuplaDevice.begin(GUID,Supla_server,Email,AUTHKEY);
         
    ticker.attach(1.0, tick); 

     swSer.write(dataBusSetIn, sizeof(dataBusSetIn)); // in1-in8 to ralays on
     for (int i = 0; i < 8; i++) { // update state   
       if (((bitRead(saved_relays_state, i)) == HIGH) && (btn[i].ms < 10) && (btn[i].preset != 3) || (btn[i].preset == 2) && (btn[i].ms < 10)){
            bitSet(relays_state, i);
            local_triger = true;
            SuplaDevice.relayOn(i, 0);
          }else{
            bitClear(relays_state, i);
            local_triger = true;
            SuplaDevice.relayOff(i);
          }
     }  
       dataBusSendrelays[3] = relays_state;
       dataBusSendrelays[4] = RelayBoard.Relays_crc8(dataBusSendrelays, 4);
       swSer.write(dataBusSendrelays, sizeof(dataBusSendrelays));          
}

void loop() {
  
  if (initialConfig == true){ondemandwifiCallback();}

  if (save_milis < millis()){
         if ((state10 == false) && (EEPROM.read(100) != relays_state)){
            EEPROM.write(100, relays_state);
            EEPROM.commit();
            saved_relays_state = relays_state;
            Serial.print(" Store Relay´s state = ");
            Serial.println(relays_state, BIN);
         }
    save_milis = millis() +5000;    
  }

  int C_W_read = digitalRead(wificonfig_pin);{  
   if (C_W_read != last_C_W_state) {  time_last_C_W_change = millis();}      
    if ((millis() - time_last_C_W_change) > C_W_delay) {     
     if (C_W_read != C_W_state) {     
       C_W_state = C_W_read;       
       if (C_W_state == LOW) {
        ondemandwifiCallback () ;} } }         
     last_C_W_state = C_W_read;            
   }
      
  if (shouldSaveConfig == true) { // ------------------------ wificonfig save --------------
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Supla_server"] = Supla_server;
    json["Email"] = Email;
    json["Supla_name"] = Supla_name;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) { }   
    json.printTo(configFile);
    configFile.close();    
    shouldSaveConfig = false;
    initialConfig = false; 
    WiFi.mode(WIFI_STA);   
    delay(5000);
    ESP.restart(); 
  }

  SuplaDevice.iterate();
   delay(25);
  receivedDataFromBus();

   if (WiFi.status() == WL_CONNECTED){
    if (starting){
      httpUpdater.setup(&httpServer, "/update", "admin", "pass");
      httpServer.begin(); 
      starting = false;         
    }
     httpServer.handleClient();
  }
   
   switch (s) { 
    case 17:  
     if (state10 == true){                 
      state10 = false;
        ticker.detach();
        digitalWrite(ledpin, HIGH); 
     }   
     break;     
    case 10:  
     state10 = true;
     break;  
  }

  if (s != 17){
    SuplaDevice.iterateOffline();
    ticker.attach(1.0, tick);
  }
  
}

void receivedDataFromBus(){

  while(swSer.available() >= 5){
  
    if(swSer.read() ==  0x55){  // Expect to receive the start Byte 0x55
      
     Serial.print("receive Byte 0x55");
     
      for (uint8_t i = 1; i<= 4; i++){
        dataBusReceived[i] = swSer.read();  // Store all remaining data 
        Serial.print(" 0x");
        Serial.print(dataBusReceived[i] < 16 ? "0" : "");
        Serial.print(dataBusReceived[i],HEX);
      }
      delay(30);  // Need to give time to empty the serial buffer
      }
   Serial.print(" & ");
   
   if (RelayBoard.Relays_crc8(dataBusReceived, 4) == dataBusReceived[4]){   // RelayBoard.Relays_crc8
    Serial.println("CRC OK ");
    
    if ((dataBusReceived[2] == 0x52) && (state10 == false)){
       Serial.print("Board send relay state ");
       Serial.print(dataBusReceived[3],BIN);
       Serial.println("");
      for (int i = 0; i < 8; i++) {
         if (bitRead(relays_state, i) != (bitRead(dataBusReceived[3], i))) {
          if ((bitRead(dataBusReceived[3], i)) == HIGH){
            local_triger = true;
            SuplaDevice.relayOn(i, btn[i].ms);
          }else{
            local_triger = true;
            SuplaDevice.relayOff(i);
          }
        }
      }       
    }
    if ((dataBusReceived[2] == 0x6B) && (state10 == false)){
       Serial.print("in state from Board ");
       Serial.print(dataBusReceived[3],BIN);
      for (int i = 0; i < 8; i++) {
          if ((bitRead(dataBusReceived[3], i)) == HIGH){
            Serial.print(" bord active in Nº ");
            Serial.print(i+1);
          }
        }
        Serial.println("");       
      }    
   }else{
    Serial.println("!! CRC error !!");
   }
   Serial.println(" end of serial data in.");
  }   
}

