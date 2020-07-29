#include <FS.h>           // ---- esp board manager 2.4.2 --- 
#include <ESP8266WiFi.h>
#define CUSTOMSUPLA_CPP
#include <CustomSupla.h>  //  ------ V 1.6.2C -------
#include <DNSServer.h>
#include <WiFiManager.h> 
#include <ArduinoJson.h> //--------- https://github.com/bblanchon/ArduinoJson/tree/v5.13.2 ------
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>  
#include <Ticker.h>      
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Button3.h>
#include <ESP8266mDNS.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
extern "C"
{
#include "user_interface.h"
}

/*
//#define D0 16  //no internal pullup resistor
//#define D1  5
//#define D2  4
//#define D3  0  //must not be pulled low during power on/reset, toggles value during boot
//#define D4  2  //must not be pulled low during power on/reset, toggles value during boot
//#define D5 14
//#define D6 12
//#define D7 13
//#define D8 15  //must not be pulled high during power on/reset


GPIO #  Component
GPIO00  Button1
GPIO01  TX
GPIO02  SDA
GPIO03  RX
GPIO04  Relay3
GPIO05  Relay2
GPIO09  Button2
GPIO10  Button3
GPIO12  Relay1
GPIO13  Led1i
GPIO14  Button4
GPIO15  Relay4
GPIO16  None
S6: 1
K5: all 1
K6: all 0
 */

#define      NTPfastReq        10                   // NTP time request in seconds when  time not set
#define      NTPslowReq        3600                 // NTP time request in seconds after time is  set
#define      Version           "0.87 "               // Firmware version
 
#define LONGCLICK_MS    1000
#define DOUBLECLICK_MS  2000
#define led_on HIGH
#define led_off LOW
#define ONE_WIRE_BUS 3
#define status_led 13   
#define BTN_COUNT 5
int relay_1 = 12;   //------------------- set relay Gpio -----
int relay_2 = 5;
int relay_3 = 4;
int relay_4 = 15;
int relay_5 = 110;
int button_1 = 0;   //------------------- set button Gpio ----
int button_2 = 111;  
int button_3 = 10;
int button_4 = 14; 
int button_5 = 112; //9
Button3 button = Button3(9);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress Thermometer_1,Thermometer_2,Thermometer_3,Thermometer_4,id1,id2,id3,id4;
int  resolution = 10;
Ticker ticker;
bool pr_wifi = true;
bool start = true;
bool eep = LOW;     
int epr = 0;         
int s;
int cero = 0;             
unsigned long wifi_checkDelay = 60000;
unsigned long wifimilis;
unsigned long eep_milis;
int C_W_state = HIGH; 
int last_C_W_state = HIGH;
unsigned long time_last_C_W_change = 0; 
long C_W_delay = 10000;               // -----------------  config delay 10 seconds  ----------------------
int C_W_state2 = HIGH; 
int last_C_W_state2 = HIGH;
unsigned long time_last_C_W_change2 = 0;       
bool tikOn = false;
bool booton_action = false;
WiFiClient client;
WiFiServer server(80);
ESP8266WebServer *server2 = NULL;
ESP8266HTTPUpdateServer httpUpdater;
// NTP Server details
//-------------------
IPAddress timeServer(129, 6, 15, 28);              // time.nist.gov NTP server
WiFiUDP Udp;
unsigned int localPort       = 3232;               // Local port to listen for UDP packets


const char* update_path = "/update";
char Supla_server[80];
char Location_id[15];
char Location_Pass[34];
char Supla_name[33];
char update_username[21];
char update_password[21];
char Supla_status[51];
char temp1[6];
char temp2[6];
char temp3[6];
float Temp1;
float Temp2;
float Temp3;
char ds_id1[17];
char ds_id2[17];
char ds_id3[17];
char ds_id4[17];
char on_time[15];
long time_on = 0;
byte mac[6];
bool shouldSaveConfig = false;
bool initialConfig = false;
unsigned long last_on = 0;
// Variables
//----------
#define      ErrMsg            "<font color=\"red\"> < < Error !</font>"
#define      EEPROM_chk        60
float        TimeZone        = 0;
byte         TimeZoneH       = 0;
byte         TimeZoneM       = 0;
boolean      ResetWiFi       = false;
long         timeNow         = 0;
long         timeOld         = 300000;
boolean      TimeOk          = false;
boolean      NTPtimeOk       = false;
String       request         = "";
byte         Page            = 1;
int          IP_1            = 129;
int          IP_2            = 6;
int          IP_3            = 15;
int          IP_4            = 28;
boolean      TimeCheck       = false;
int          NewHH           = 0;
int          NewMM           = 0;
int          Newdd           = 0;
int          Newmm           = 0;
int          Newyy           = 0;
int          PgmNr           = 0;
int          OnHour          = 0;
int          OnMinute        = 0;
int          OffHour         = 0;
int          OffMinute       = 0;
boolean      ManualOff       = false;
boolean      ManualOn        = false;
int          LastHH          = 0;
int          LastMM          = 0;
int          Lastdd          = 0;
int          Lastmm          = 0;
int          Lastyy          = 0;
byte         old_sec         = 0;
long         old_ms          = 0;
boolean      PgmPrev         = false;
boolean      PgmNext         = false;
boolean      PgmSave         = false;
boolean      Error1          = false;
boolean      Error2          = false;
boolean      Error3          = false;
boolean      Error3b         = false;
boolean      Error4          = false;
boolean      Error5          = false;
boolean      Error6          = false;
boolean      Error7          = false;
boolean      Armed           = false;
boolean      D[8]            = {false, false, false, false, false, false, false, false};
boolean      C[5]            = {false, false, false, false, false};
bool         Output[5]       = {false, false, false, false, false};
bool         Output_is[5]    = {false, false, false, false, false};
bool         Man_on[5]       = {false, false, false, false, false};
bool         Man_off[5]      = {false, false, false, false, false};
bool         WebButton[5]    = {false, false, false, false, false};
int          On_Time[7];
int          Off_Time[7];
boolean      On_Days[7][8];
boolean      On_Channel[7][5];
bool         local_timer    = false;

void tick(){
  //toggle Led state
  int state = digitalRead(status_led);  
  digitalWrite(status_led, !state);  
}

void saveConfigCallback () {                 
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

bool ondemandwifiCallback(bool resetConf) {

 ticker.attach(1.0, tick);
 
 WiFiManager wifiManager;

  if (resetConf){  
    wifiManager.resetSettings();
    delay(5000);
    ESP.reset();
    delay(1000);
   }
 
  char ds_temp_1[8] = (" --.-- ");
  char ds_temp_2[8] = (" --.-- ");
  char ds_temp_3[8] = (" --.-- ");
  char ds_temp_4[8] = (" --.-- ");
  float _temp1 = sensors.getTempC(Thermometer_1);if (_temp1 > -100){dtostrf(_temp1, 6, 2, ds_temp_1);}
  float _temp2 = sensors.getTempC(Thermometer_2);if (_temp2 > -100){dtostrf(_temp2, 6, 2, ds_temp_2);}
  float _temp3 = sensors.getTempC(Thermometer_3);if (_temp3 > -100){dtostrf(_temp3, 6, 2, ds_temp_3);}
  float _temp4 = sensors.getTempC(Thermometer_4);if (_temp4 > -100){dtostrf(_temp4, 6, 2, ds_temp_4);}
  char _ds_id1[17];
  char _ds_id2[17];
  char _ds_id3[17];
  char _ds_id4[17];
  array_to_string(Thermometer_1, 8, _ds_id1);
  array_to_string(Thermometer_2, 8, _ds_id2);
  array_to_string(Thermometer_3, 8, _ds_id3);
  array_to_string(Thermometer_4, 8, _ds_id4);
  
  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 80,"required");
  WiFiManagerParameter custom_Location_id("ID", "Location id", Location_id, 15,"required");
  WiFiManagerParameter custom_Location_Pass("Password", "Location Pass", Location_Pass, 34,"required");
  WiFiManagerParameter custom_Supla_name("name", "Supla Device Name", Supla_name, 33,"required");
  WiFiManagerParameter custom_update_username("updateUsername", "update username", update_username, 21,"required");
  WiFiManagerParameter custom_update_password("updatePassword", "update password", update_password, 21,"required");
  WiFiManagerParameter custom_Supla_tempa("tempa", "desired temperature", temp1, 6,"required");
  WiFiManagerParameter custom_Supla_tempb("tempb", "turn on when more than", temp2, 6,"required");
  WiFiManagerParameter custom_Supla_tempc("tempc", "turn off when less than", temp3, 6,"required");
  WiFiManagerParameter custom_html_id04("<div>");
  WiFiManagerParameter custom_html_id05( ds_temp_1);
  WiFiManagerParameter custom_html_id06("&deg;C - Ds18b20 1 id - "); 
  WiFiManagerParameter custom_html_id07( _ds_id1);
  WiFiManagerParameter custom_html_id08( "</div><div>");
  WiFiManagerParameter custom_html_id09( ds_temp_2);
  WiFiManagerParameter custom_html_id10("&deg;C - Ds18b20 2 id - ");
  WiFiManagerParameter custom_html_id11( _ds_id2);
  WiFiManagerParameter custom_html_id12( "</div><div>");
  WiFiManagerParameter custom_html_id13( ds_temp_3);
  WiFiManagerParameter custom_html_id14("&deg;C - Ds18b20 3 id - ");
  WiFiManagerParameter custom_html_id15( _ds_id3);
  WiFiManagerParameter custom_html_id16( "</div><div>");
  WiFiManagerParameter custom_html_id17( ds_temp_4);
  WiFiManagerParameter custom_html_id18("&deg;C - Ds18b20 4 id - ");
  WiFiManagerParameter custom_html_id19( _ds_id4);
  WiFiManagerParameter custom_html_id20( "</div>");
  WiFiManagerParameter custom_ds_id1("ds_ida", "Pool water temperature", ds_id1, 17,"required");
  WiFiManagerParameter custom_ds_id2("ds_idb", "Solar panel temperature", ds_id2, 17,"required");
  WiFiManagerParameter custom_ds_id3("ds_idc", "Return water temperature", ds_id3, 17,"required");
  WiFiManagerParameter custom_ds_id4("ds_idd", "Air temperature", ds_id4, 17);
  WiFiManagerParameter custom_on_time("on", "minimum running time in seconds", on_time, 15,"required");
  WiFiManagerParameter custom_html_id21("<div><h4> - Supla State -   ");
  WiFiManagerParameter custom_html_id22( Supla_status);
  WiFiManagerParameter custom_html_id23( "</h4></div>");

  wifiManager.setBreakAfterConfig(true);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_Location_id);
  wifiManager.addParameter(&custom_Location_Pass);
  wifiManager.addParameter(&custom_Supla_name);
  wifiManager.addParameter(&custom_update_username);
  wifiManager.addParameter(&custom_update_password);
  wifiManager.addParameter(&custom_Supla_tempa);
  wifiManager.addParameter(&custom_Supla_tempb);
  wifiManager.addParameter(&custom_Supla_tempc);
  wifiManager.addParameter(&custom_html_id04);
  wifiManager.addParameter(&custom_html_id05);
  wifiManager.addParameter(&custom_html_id06);
  wifiManager.addParameter(&custom_html_id07);
  wifiManager.addParameter(&custom_html_id08);
  wifiManager.addParameter(&custom_html_id09);
  wifiManager.addParameter(&custom_html_id10);
  wifiManager.addParameter(&custom_html_id11);
  wifiManager.addParameter(&custom_html_id12);
  wifiManager.addParameter(&custom_html_id13);
  wifiManager.addParameter(&custom_html_id14);
  wifiManager.addParameter(&custom_html_id15);
  wifiManager.addParameter(&custom_html_id16);
  wifiManager.addParameter(&custom_html_id17);
  wifiManager.addParameter(&custom_html_id18);
  wifiManager.addParameter(&custom_html_id19);
  wifiManager.addParameter(&custom_html_id20);
  wifiManager.addParameter(&custom_ds_id1);
  wifiManager.addParameter(&custom_ds_id2);
  wifiManager.addParameter(&custom_ds_id3);
  wifiManager.addParameter(&custom_ds_id4);
  wifiManager.addParameter(&custom_on_time);
  wifiManager.addParameter(&custom_html_id21);
  wifiManager.addParameter(&custom_html_id22);
  wifiManager.addParameter(&custom_html_id23);

  wifiManager.setCustomHeadElement("<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>");
  wifiManager.setMinimumSignalQuality(8);
  wifiManager.setConfigPortalTimeout(180);

    if (!wifiManager.startConfigPortal("Supla_Lector")) {
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
    strcpy(temp1, custom_Supla_tempa.getValue());
    strcpy(temp2, custom_Supla_tempb.getValue());
    strcpy(temp3, custom_Supla_tempc.getValue());
    strcpy(ds_id1, custom_ds_id1.getValue());
    strcpy(ds_id2, custom_ds_id2.getValue());
    strcpy(ds_id3, custom_ds_id3.getValue());
    strcpy(ds_id4, custom_ds_id4.getValue());
    strcpy(on_time, custom_on_time.getValue());

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
    json["temp1"] = temp1;
    json["temp2"] = temp2;
    json["temp3"] = temp3;
    json["ds_id1"] = ds_id1;
    json["ds_id2"] = ds_id2;
    json["ds_id3"] = ds_id3;
    json["ds_id4"] = ds_id4;
    json["on_time"] = on_time;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {Serial.println("failed to open config file for writing");}    
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    Serial.println("config saved");
    shouldSaveConfig = false;
    initialConfig = false; 
    ticker.detach();
    digitalWrite(status_led, led_off);
    WiFi.mode(WIFI_STA);
    ESP.restart();
    delay(5000); 
  }    
   
  WiFi.softAPdisconnect(true);  
    ticker.detach();
    digitalWrite(status_led, led_off);
    ESP.restart();
    delay(5000);
}
void array_to_string(byte array[], unsigned int len, char buffer[])
{
    for (unsigned int i = 0; i < len; i++)
    {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}
void hexCharacterStringToBytes(byte *byteArray, const char *hexString)
{
  bool oddLength = strlen(hexString) & 1;
  byte currentByte = 0;
  byte byteIndex = 0;

  for (byte charIndex = 0; charIndex < strlen(hexString); charIndex++)
  {
    bool oddCharIndex = charIndex & 1;

    if (oddLength)
    {
      if (oddCharIndex)
      {
        currentByte = nibble(hexString[charIndex]) << 4;
      }
      else
      {
        currentByte |= nibble(hexString[charIndex]);
        byteArray[byteIndex++] = currentByte;
        currentByte = 0;
      }
    }
    else
    {
      if (!oddCharIndex)
      {
        currentByte = nibble(hexString[charIndex]) << 4;
      }
      else
      {
        currentByte |= nibble(hexString[charIndex]);
        byteArray[byteIndex++] = currentByte;
        currentByte = 0;
      }
    }
  }
}
byte nibble(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';

  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;

  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;

  return 0;  // Not a valid hexadecimal character
}
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
  Serial.println("");
}
typedef struct {  //------------------------------------------- BTN ----------------------------------------------------
  int pin;
  int relay_pin;
  int channel;
  char last_val;
  int ms;
  unsigned long last_time;
  bool mem;
} _btn_t;

_btn_t btn[BTN_COUNT];

void itrate_Term() {
  unsigned int Now =  millis();
         float Thermometer1 = sensors.getTempC(id1);     
         float Thermometer2 = sensors.getTempC(id2);     
         float Thermometer3 = sensors.getTempC(id3);                 
             
    if (btn[4].mem == true){  // is auto on ?
     if ((Thermometer1 > 0)&& (Thermometer2 > 0)&& (Thermometer3 > 0) && (btn[1].mem == false)){  // are temps > 0ºC & relay 2 off
      if (((Thermometer2 - Thermometer1) > Temp2) && (Thermometer1 < Temp1)){  
        if ( time_on > 10 ) {
           Serial.println("Min run time: " + String(time_on / 1000) + " seconds");
           } 
          Serial.println(" Auto on by temp");
           booton_action = true;
           last_on = Now;
          //CustomSupla.relayOn(1, 0);
       if (Output_is[1] == 0) { 
        if (Man_on[1] == false) {
            Man_on[1] = true;
            Man_off[1] = false;
            goto doLED;
          }
      }
      else {
        if (Man_on[1] == true) {
          Man_on[1] = false;
          Man_off[1] = false;
          goto doLED;
        }
      }
       }
     }
     if (btn[1].mem == true){     // is pump on?
      if  ( time_on > 10 ){ 
        if (( (Now - last_on) > time_on) && (((Thermometer3 - Thermometer1) < Temp3) || (Thermometer1 > Temp1))){
          Serial.println("Auto off by temp + min run time");
           booton_action = true;
          //CustomSupla.relayOff(1); 
      if (Output_is[1] == 1) {  
        if (Man_off[1] == false) {
            Man_off[1] = true;
            Man_on[1] = false;
            goto doLED;
        }
      }
      else {
        if (Man_off[1] == true) {
          Man_off[1] = false;
          Man_on[1] = false;
          goto doLED;
        }
     }         
        }
      }
       else if (((Thermometer3 - Thermometer1) < Temp3) || (Thermometer1 > Temp1)){
          Serial.println("Auto off by temp");
           booton_action = true;
          //CustomSupla.relayOff(1);
       if (Output_is[1] == 1) {  
        if (Man_off[1] == false) {
            Man_off[1] = true;
            Man_on[1] = false;
            goto doLED;
        }
      }
      else {
        if (Man_off[1] == true) {
          Man_off[1] = false;
          Man_on[1] = false;
          goto doLED;
        }
     }
      }
     }
    }
   doLED:
   eep_milis = Now + 2000 ;
   timeOld = 0;  
  sensors.requestTemperatures(); 
}

double get_temperature(int channelNumber, double last_val) {
   double t = -275;
   double _temp1 = -275; 
   double _temp2 = -275; 
   double _temp3 = -275; 
   double _temp4 = -275;   

   switch(channelNumber)
          {
            case 5:
          _temp1 = sensors.getTempC(id1);if (_temp1 > -100)t = _temp1;         
            return t;  
                    break;
            case 6:
          _temp2 = sensors.getTempC(id2); if (_temp2 > -100)t = _temp2;        
            return t;     
                    break;
            case 7:
          _temp3 = sensors.getTempC(id3);if (_temp3 > -100)t = _temp3;         
            return t;    
                    break;       
            case 8:            
          _temp4 = sensors.getTempC(id4);if (_temp4 > -100)t = _temp4;         
            return t;  
              break;                    
          }
    return t; 
}

void iterate_botton() {
  char v;
  unsigned long now = millis();
  {
  for(int a=0;a<BTN_COUNT;a++)
    if (btn[a].pin > 0) {
        v = digitalRead(btn[a].pin-1);
        if (v != btn[a].last_val && now - btn[a].last_time ) {
           btn[a].last_val = v;
           btn[a].last_time = now;
           delay(30);
           v = digitalRead(btn[a].pin-1);
           if (v==0)
             {
              booton_action = true;
              
              WebButton[a]= true;
              
         }}
      }
    }
}

void supla_btn_init() {
  for(int a=0;a<BTN_COUNT;a++)
    if (btn[a].pin > 0) {
        pinMode(btn[a].pin-1, INPUT_PULLUP);
        btn[a].last_val = digitalRead(btn[a].pin-1);
        btn[a].last_time = millis();
        pinMode(btn[a].relay_pin-1,OUTPUT);
    }
}

int supla_DigitalRead(int channelNumber, uint8_t pin) {

   if (channelNumber == 9){
     return local_timer;
   }else{
     return btn[channelNumber].mem;      
   }
}

void suplaDigitalWrite(int channelNumber, uint8_t pin, uint8_t val) {       

  if (channelNumber == 9){
    if (EEPROM.read(1001) != val){
         EEPROM.write(1001,val);                  
         EEPROM.commit();
        }
      if ( val >=1){local_timer = true;} else {local_timer = false;}
     return;    
  }
    // Check manual OFF
    if (val == false) {
     if (Man_on[channelNumber] == true) {
      Man_on[channelNumber] = false;
      Serial.print("Manon = 0");
      goto doLED;
     }
      if (btn[channelNumber].mem  == true) {  
        if (Man_off[channelNumber] == false) {
            Man_off[channelNumber] = true;
            Man_on[channelNumber] = false;
            Serial.print("mem = 1 Manoff = 1");
            goto doLED;
        }
      }
   }
    
    // Check manual ON
    if  (val == true) {
     if (Man_off[channelNumber] == true) {
      Man_off[channelNumber] = false;
      Serial.print("Manoff = 0");
      goto doLED;
     }
      if (btn[channelNumber].mem == false) { 
        if (Man_on[channelNumber] == false) {
            Man_on[channelNumber] = true;
            Man_off[channelNumber] = false;
            Serial.print("mem = 0 Manon = 1");
            goto doLED;
          }
      }
    }

    doLED:
    eep_milis = millis() + 2000 ;
    timeOld = 0;


    
     if ((booton_action == false) && (CustomSupla.channel_pin[channelNumber].DurationMS != btn[channelNumber].ms) && (val == 0)) {
            int durationMS = CustomSupla.channel_pin[channelNumber].DurationMS;
             if (durationMS < 0 ) durationMS = 0;
               Serial.print("button_duration: ");Serial.print(durationMS);           
               Serial.print(" button: ");Serial.println(channelNumber);               
               EEPROM.put((channelNumber * 10) + 600, durationMS);
               EEPROM.commit();
               btn[channelNumber].ms = durationMS;
           }
      booton_action = false;
  return; 
}

void Eeprom_save() {                  

        if (start){
          return;
        }
      for(int e=0;e<BTN_COUNT;e++) {  //  ---check relay except it have delay (staircase)
         if ( btn[e].ms > 0 ) {
        eep = (btn[e].mem);                    //  --- read relay state
        if (EEPROM.read(e) != 0){            //  --- compare relay state with stored state
         EEPROM.write(e,0);                  //  --- if different write memory
         Serial.print(" channel monostable, mem off: ");
         Serial.println((btn[e].channel));
         EEPROM.commit();
        }          
                     continue;
         } else {
        eep = (btn[e].mem);                    //  --- read relay state
     if (eep != EEPROM.read(e)){            //  --- compare relay state with stored state 
       if ((eep == true) &&  (Man_on[e] == true)) {
          EEPROM.write(e,true);
          Serial.print("EEPROM set on channel: ");
          Serial.println((btn[e].channel));
          EEPROM.commit();         
        }else if (eep == false){
          EEPROM.write(e,false);
          Serial.print("EEPROM set off channel: ");
          Serial.println((btn[e].channel));
          EEPROM.commit();
        }  
        }
     }
   }
}

void Eepron_read() {                  //----------EEPROM read  ---------------------- EEprom                                

       if ( btn[epr].ms > 10 ) {
                     return; 
         } else {
         eep = EEPROM.read(epr);               //  ---read relay state       
         Serial.print("EEPROM.");
         Serial.print(epr);
         Serial.print(" read.");
         Serial.print((eep));
         Serial.print(" channel ");
         Serial.println((btn[epr].channel));
       
        if (eep == HIGH){                    //  --- if 1 send relay on
          booton_action = true;
          CustomSupla.relayOn(epr, 0);       //  --- only one channel on each pass
          } 
         } 
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

  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY, 1); 
  WiFi.hostname("pool-control");
   delay(5000);
  pinMode(status_led,OUTPUT); 
  digitalWrite(status_led, led_on);
  EEPROM.begin(1024);
  sensors.begin();
  Serial.println(".");
  Serial.println(".");
  sensors.begin();

  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  oneWire.reset_search();
  if (!oneWire.search(Thermometer_1)) Serial.println("Unable to find address for Thermometer 1"); else printAddress(Thermometer_1);
  if (!oneWire.search(Thermometer_2)) Serial.println("Unable to find address for Thermometer 2"); else printAddress(Thermometer_2);
  if (!oneWire.search(Thermometer_3)) Serial.println("Unable to find address for Thermometer 3"); else printAddress(Thermometer_3);
  if (!oneWire.search(Thermometer_4)) Serial.println("Unable to find address for Thermometer 4"); else printAddress(Thermometer_4);  
  sensors.setResolution(Thermometer_1, resolution); 
  sensors.setResolution(Thermometer_2, resolution);
  sensors.setResolution(Thermometer_3, resolution); 
  sensors.setResolution(Thermometer_4, resolution);     
  sensors.setWaitForConversion(false);
  sensors.requestTemperatures();

  if (WiFi.SSID()==""){
    initialConfig = true;
  }

  WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.

   uint8_t mac[WL_MAC_ADDR_LENGTH];
   WiFi.macAddress(mac);
   char GUID[SUPLA_GUID_SIZE] = {mac[WL_MAC_ADDR_LENGTH - 6], mac[WL_MAC_ADDR_LENGTH - 5], mac[WL_MAC_ADDR_LENGTH - 4], mac[WL_MAC_ADDR_LENGTH - 3],                                
                                 mac[WL_MAC_ADDR_LENGTH - 2], mac[WL_MAC_ADDR_LENGTH - 1], mac[WL_MAC_ADDR_LENGTH - 1], mac[WL_MAC_ADDR_LENGTH - 2], 
                                 mac[WL_MAC_ADDR_LENGTH - 3], mac[WL_MAC_ADDR_LENGTH - 4], mac[WL_MAC_ADDR_LENGTH - 5], mac[WL_MAC_ADDR_LENGTH - 6]};
          
  CustomSupla.addRelay(101, false);   
  CustomSupla.addRelay(102, false);  
  CustomSupla.addRelay(103, false);
  CustomSupla.addRelay(104, false); 
  CustomSupla.addRelay(105, false);
  CustomSupla.addDS18B20Thermometer(); 
  CustomSupla.addDS18B20Thermometer(); 
  CustomSupla.addDS18B20Thermometer(); 
  CustomSupla.addDS18B20Thermometer();
  CustomSupla.addRelay(106, false); 

  int btn0ms;EEPROM.get(600,btn0ms);Serial.print("initial_button_duration 0: ");Serial.println(btn0ms);
  int btn1ms;EEPROM.get(610,btn1ms);Serial.print("initial_button_duration 1: ");Serial.println(btn1ms);
  int btn2ms;EEPROM.get(620,btn2ms);Serial.print("initial_button_duration 2: ");Serial.println(btn2ms); 
  int btn3ms;EEPROM.get(630,btn3ms);Serial.print("initial_button_duration 3: ");Serial.println(btn3ms); 
     
  memset(btn, 0, sizeof(btn));
  btn[0].pin =button_1 +1;          // pin gpio buton  +1
  btn[0].relay_pin =relay_1 +1;  // pin gpio Relay   +1
  btn[0].channel =0;      // channel
  btn[0].ms = btn0ms;
  btn[0].mem =0;
  btn[1].pin =button_2 +1;          // pin gpio buton  +1
  btn[1].relay_pin =relay_2 +1;  // pin gpio Relay   +1
  btn[1].channel =1;      // channel
  btn[1].ms = btn1ms;
  btn[1].mem =0;
  btn[2].pin =button_3 +1;          // pin gpio buton  +1
  btn[2].relay_pin =relay_3 +1;  // pin gpio Relay   +1
  btn[2].channel =2;      // channel
  btn[2].ms = btn2ms;
  btn[2].mem =0;
  btn[3].pin =button_4 +1;          // pin gpio buton  +1
  btn[3].relay_pin =relay_4 +1;  // pin gpio Relay   +1
  btn[3].channel =3;      // channel
  btn[3].ms = btn3ms;
  btn[3].mem =0;
  btn[4].pin =button_5 +1;          // pin gpio buton  +1
  btn[4].relay_pin =relay_5 +1;  // pin gpio Relay   +1
  btn[4].channel =4;      // channel
  btn[4].ms = 0;
  btn[4].mem =0;  
  supla_btn_init();

  button.setClickHandler(Click);
  button.setLongClickHandler(Click);
  button.setDoubleClickHandler(doubleClick);  

  //read configuration from FS json
  Serial.println("mounting FS...");
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
        json.prettyPrintTo(Serial);
        //json.printTo(Serial);   //print config data to serial on startup
        if (json.success()) {
          Serial.println("\nparsed json");

          if (json.containsKey("Supla_server")) strcpy(Supla_server, json["Supla_server"]);
          if (json.containsKey("Location_id")) strcpy(Location_id, json["Location_id"]);
          if (json.containsKey("Location_Pass")) strcpy(Location_Pass, json["Location_Pass"]);
          if (json.containsKey("Supla_name")) strcpy(Supla_name, json["Supla_name"]);         
          if (json.containsKey("update_username")) strcpy(update_username, json["update_username"]);
          if (json.containsKey("update_password")) strcpy(update_password, json["update_password"]);
          if (json.containsKey("temp1")) strcpy(temp1, json["temp1"]);
          if (json.containsKey("temp2")) strcpy(temp2, json["temp2"]);
          if (json.containsKey("temp3")) strcpy(temp3, json["temp3"]);
          if (json.containsKey("ds_id1")) strcpy(ds_id1, json["ds_id1"]);
          if (json.containsKey("ds_id2")) strcpy(ds_id2, json["ds_id2"]);
          if (json.containsKey("ds_id3")) strcpy(ds_id3, json["ds_id3"]);
          if (json.containsKey("ds_id4")) strcpy(ds_id4, json["ds_id4"]);
          if (json.containsKey("on_time")) strcpy(on_time, json["on_time"]);
          time_on = (atoi(on_time) * 1000);
          Temp1 = atof(temp1);Serial.print("Temp1: ");Serial.println(Temp1);
          Temp2 = atof(temp2);Serial.print("Temp2: ");Serial.println(Temp2);
          Temp3 = atof(temp3);Serial.print("Temp3: ");Serial.println(Temp3);
          hexCharacterStringToBytes(id1, ds_id1);
          hexCharacterStringToBytes(id2, ds_id2);
          hexCharacterStringToBytes(id3, ds_id3);
          hexCharacterStringToBytes(id4, ds_id4);
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

  local_timer = EEPROM.read(1001);
  Serial.print("local timer: ");Serial.println(local_timer);
  
  printAddress(id1);printAddress(id2);printAddress(id3);printAddress(id4);
 
  CustomSupla.setDigitalReadFuncImpl(&supla_DigitalRead);    //            ------Send Value to server -------
  CustomSupla.setDigitalWriteFuncImpl(&suplaDigitalWrite);   //        -------  Read Value from server   -------
  CustomSupla.setStatusFuncImpl(&status_func);    //   ----------------------------- Status -----------------------------
  CustomSupla.setTemperatureCallback(&get_temperature);
  CustomSupla.setName(Supla_name);

  int LocationID = atoi(Location_id);
  CustomSupla.begin(GUID, mac, Supla_server, LocationID, Location_Pass);

  ReadData();
                        
}

void loop() {
 
  if (initialConfig){
    ondemandwifiCallback (false) ;
  }
    
  if (start){
    // read_initial_relay_state
    for(int i=0;i<BTN_COUNT;i++){      //  ---check relay except der have delay (staircase)
     if ( (btn[i].ms) > 0 ) {
      digitalWrite(btn[i].relay_pin-1, LOW);       //--------------- relay active High     
                     continue;
         } else {
        eep = EEPROM.read(i); 
       if (eep >= 2 ){
         EEPROM.write(i,0);                  
         Serial.println("epp correct");
         EEPROM.commit();
       }
         Serial.print("Recover.");
         Serial.print(i);
         Serial.print(" read.");
         Serial.print((eep));
         Serial.print(" channel ");
         Serial.print((btn[i].channel));
         Serial.print(" Gpio ");         
         btn[i].mem =eep;
         digitalWrite(btn[i].relay_pin-1, eep);       //--------------- relay active High          
                  Serial.print((btn[i].relay_pin) -1);
                  Serial.print(" set ");
                  Serial.println(eep);                    
         }
    }
    start = false;
  } 

  if (millis() > eep_milis){
     itrate_Term();
     Eeprom_save() ;   //        ------------------------------Eepron save callback -----------------------------   
     eep_milis = millis() + 5000 ;
   }
   
  switch (s) {    //    ------------------------------------------------ Status ------------------------------------
    case 17:      // -----     STATUS_REGISTERED_AND_READY
    if ((epr<BTN_COUNT) && (TimeOk != false)) {
     Eepron_read() ;                // ------------------------------- Eeprom read callback -------------------------------
     epr = epr+1;// -------- 1 loop for each output  ----------   
      if (tikOn == true){            
        digitalWrite(status_led, led_off);
        tikOn = false;
        if (local_timer){CustomSupla.relayOn(9, 0);}
        }
       }
      break;   
     case 10:      // --------------------- REGISTER_IN_PROGRESS  ----------------------
      epr = 0 ;
      break;
  }
  
  int C_W_read = digitalRead(btn[0].pin-1);{  
   if (C_W_read != last_C_W_state) {time_last_C_W_change = millis(); }
     if ((millis() - time_last_C_W_change) > C_W_delay) {     
       if (C_W_read != C_W_state) {     
         Serial.println("Triger sate changed");
         C_W_state = C_W_read;       
         if (C_W_state == LOW) {
          ondemandwifiCallback (true) ;
         }
       }
    }
    last_C_W_state = C_W_read;            
  }

  int C_W_read2 = digitalRead(btn[3].pin-1);{  
   if (C_W_read2 != last_C_W_state2) {time_last_C_W_change2 = millis(); }
     if ((millis() - time_last_C_W_change2) > C_W_delay) {     
       if (C_W_read2 != C_W_state2) {     
         C_W_state2 = C_W_read2;       
         if (C_W_state2 == LOW) {
          Serial.println(" Btn 4  RESTART");
          ESP.restart();
          delay(5000); 
         }
       }
    }
    last_C_W_state2 = C_W_read2;            
  }
   
    iterate_botton(); 
    button.loop();  
       
   if (WiFi.status() == WL_CONNECTED){ 
       CustomSupla.iterate(); 
        delay(50);  
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
     MDNS.begin(Supla_name);        

     MDNS.addService("http", "tcp", 80);
     Serial.printf("HTTPUpdateServer ready! Open http://%s.local:81%s in your browser and login with username '%s' and password '%s'\n", Supla_name, update_path, update_username, update_password);      
     if (server2 != NULL) {
     delete server2;
    }
    server2 = new ESP8266WebServer(81);
    httpUpdater.setup(server2, update_path, update_username, update_password);
      // Setup NTP time requests
      //----------------------------------------------------------------------
     Udp.begin(localPort);
     setSyncProvider(getNtpTime);
     setSyncInterval(NTPfastReq);
     // Begin IoT server
     //----------------------------------------------------------------------
     server.begin();
     server2->begin(); 
    }
   // MDNS.update();
    server2->handleClient();
   } else {
    WiFi_up();  
   }

   if (s != 17){
    CustomSupla.iterateOfline();
    digitalWrite(status_led, led_on);
    tikOn = true; 
   }

   
   
  // Scan Button
  //----------------------------------------------------------------------
  ScanButton();
  // Scan for NTP time changes
  //----------------------------------------------------------------------
  CheckNTPtime();
  // See if time has changed
  //----------------------------------------------------------------------
  DoTimeCheck();
  // Update LED
  //----------------------------------------------------------------------
  UpdateLED();
  
  // See if any data was received via WiFi
  //----------------------------------------------------------------------
  WiFiClient client = server.available();
  String  request = "";
  boolean DataAvailable = false;
  if (client != 0) {
    // Read requests from web page if available
    //----------------------------------------------------------------------
    request = client.readStringUntil('\r');
    DataAvailable = true;
    client.flush();
  }

  // Respond to requests from web page
  //----------------------------------------------------------------------
  if ( (DataAvailable == true) and (request.indexOf("/") != -1) ) {
    Serial.println(request);
    if (request.indexOf("Link=1")     != -1) Page = 1;
    if (request.indexOf("Link=2")     != -1) Page = 2;
    if (request.indexOf("Link=4")     != -1) Page = 4;
    if (request.indexOf("Link=5")     != -1) Page = 5;
    if (request.indexOf("GET / HTTP") != -1) Page = 1;
    Error1 = false;
    Error2 = false;
    Error3 = false;
    Error3b= false;
    Error4 = false;
    Error5 = false;
    Error6 = false;
    Error7 = false;


    // Respond to Buttons
    //==================================

    // PAGE 1 - STATUS
    //----------------
    // See if Save Button was presed
    //----------------------------------------------------------------------
    if (request.indexOf("Over1=") != -1) {
      Page = 1;
      WebButton[0] = true;
      ScanButton();
    }
    if (request.indexOf("Over2=") != -1) {
      Page = 1;
      WebButton[1] = true;
      ScanButton();
    }
    if (request.indexOf("Over3=") != -1) {
      Page = 1;
      WebButton[2] = true;
      ScanButton();
    }
    if (request.indexOf("Over4=") != -1) {
      Page = 1;
      WebButton[3] = true;
      ScanButton();
    }
    if (request.indexOf("Over5=") != -1) {
      Page = 1;
      WebButton[4] = true;
      ScanButton();
    }
    // See if Refresh Button was pressed
    //----------------------------------------------------------------------
    if (request.indexOf("RefreshBtn1=") != -1) {
      Page = 1;
    }

    // PAGE 2 - PROGRAMS
    //------------------
    // See if Previous Buttomn was pressed
    //----------------------------------------------------------------------
    if (request.indexOf("SaveBtnPrev=") != -1) {
      Page = 2;
      PgmPrev = true;
      PgmNext = false;
      PgmSave = true;
    }
    // See if Next Button was pressed
    //----------------------------------------------------------------------
    if (request.indexOf("SaveBtnNext=") != -1) {
      Page = 2;
      PgmPrev = false;
      PgmNext = true;
      PgmSave = true;
    }
    // See if Save Button was pressed
    //----------------------------------------------------------------------
    if (request.indexOf("SaveBtn2=") != -1) {
      Page = 2;
      PgmSave = true;
      PgmPrev = false;
      PgmNext = false;
    }
    // See if Clear Button was pressed
    //----------------------------------------------------------------------
    if (request.indexOf("ClearBtn1=") != -1) {
      Page = 2;
      On_Time[PgmNr]  = 0;
      Off_Time[PgmNr] = 0;
      for (byte i = 0; i < 8; i++ ) {
        On_Days[PgmNr][i] = false;
      }
      for (byte i = 0; i < 5; i++ ) {
        On_Channel[PgmNr][i] = false;
      }
      PgmPrev = false;
      PgmNext = false;
      // Save program data
      SaveProgram();
    }
    // Get program data if any button was pressed
    //----------------------------------------------------------------------
    if (PgmSave == true) {
      PgmSave = false;
      // On Hour
      if (request.indexOf("OnH=") != -1)  {
        String Tmp = request;
        int t1 = Tmp.indexOf("OnH=");
        Tmp.remove(0,t1+4);
        t1 = Tmp.indexOf("&");
        Tmp.remove(t1);
        OnHour = Tmp.toInt();
        if ( (OnHour < 0) or (OnHour > 23) ) Error1 = true;
      }
      // On Minute
      if (request.indexOf("OnM=") != -1)  {
        String Tmp = request;
        int t1 = Tmp.indexOf("OnM=");
        Tmp.remove(0,t1+4);
        t1 = Tmp.indexOf("&");
        Tmp.remove(t1);
        OnMinute = Tmp.toInt();
        if ( (OnMinute < 0) or (OnMinute > 59) ) Error1 = true;
      }
      // Off Hour
      if (request.indexOf("OffH=") != -1)  {
        String Tmp = request;
        int t1 = Tmp.indexOf("OffH=");
        Tmp.remove(0,t1+5);
        t1 = Tmp.indexOf("&");
        Tmp.remove(t1);
        OffHour = Tmp.toInt();
        if ( (OffHour < 0) or (OffHour > 23) ) Error2 = true;
      }
      // Off Minute
      if (request.indexOf("OffM=") != -1)  {
        String Tmp = request;
        int t1 = Tmp.indexOf("OffM=");
        Tmp.remove(0,t1+5);
        t1 = Tmp.indexOf("&");
        if (t1 == -1) {
          t1 = Tmp.indexOf(" ");
        }
        Tmp.remove(t1);
        OffMinute = Tmp.toInt();
        if ( (OffMinute < 0) or (OffMinute > 59) ) Error2 = true;
      }
      // Reset day flags
      D[0] = false;
      D[1] = false;
      D[2] = false;
      D[3] = false;
      D[4] = false;
      D[5] = false;
      D[6] = false;
      D[7] = false;
      // Day 1
      if (request.indexOf("D1=on") != -1) D[0] = true;
      // Day 2
      if (request.indexOf("D2=on") != -1) D[1] = true;
      // Day 3
      if (request.indexOf("D3=on") != -1) D[2] = true;
      // Day 4
      if (request.indexOf("D4=on") != -1) D[3] = true;
      // Day 5
      if (request.indexOf("D5=on") != -1) D[4] = true;
      // Day 6
      if (request.indexOf("D6=on") != -1) D[5] = true;
      // Day 7
      if (request.indexOf("D7=on") != -1) D[6] = true;  
      // Reset channel flags
      C[0] = false;
      C[1] = false;
      C[2] = false;
      C[3] = false;
      C[4] = false;
      C[5] = false;
      // Channel 1
      if (request.indexOf("C1=on") != -1) C[0] = true;
      // Channel 2
      if (request.indexOf("C2=on") != -1) C[1] = true;
      // Channel 3
      if (request.indexOf("C3=on") != -1) C[2] = true;
      // Channel 4
      if (request.indexOf("C4=on") != -1) C[3] = true;
      // Channel 5
      if (request.indexOf("C5=on") != -1) C[4] = true;
      // Timer on
      if (request.indexOf("D8=on") != -1) D[7] = true;  
      // Update program if no errors
      if ( (Error1 == false) and (Error2 == false) ) {
        On_Time[PgmNr]  = (OnHour  * 100) + OnMinute;
        Off_Time[PgmNr] = (OffHour * 100) + OffMinute;        
        for (byte i = 0; i < 8; i++) {
          if (D[i] == true) On_Days[PgmNr][i] = true; else On_Days[PgmNr][i] = false;
        }  
        for (byte i = 0; i < 5; i++) {
          if (C[i] == true) On_Channel[PgmNr][i] = true; else On_Channel[PgmNr][i] = false;
          Man_on[i] = false;
          Man_off[i] = false;
        }  
        // Save program data
        ManualOff = false;
        ManualOn = false;
        SaveProgram();
        timeOld = 0;
      }
      else {
        PgmPrev = false;
        PgmNext = false;
      }
    }
    // Change to Prev/Next Program
    if (PgmPrev == true) {
      PgmPrev = false;
      PgmNr = PgmNr - 1;
      if (PgmNr <0) PgmNr = 6;
    }
    if (PgmNext == true) {
      PgmNext = false;
      PgmNr = PgmNr + 1;
      if (PgmNr > 6) PgmNr = 0;        
    }

    // PAGE 4 - NTP Setup
    //-------------------
    // See if Save Button was pressed
    //----------------------------------------------------------------------
    if (request.indexOf("SaveBtn4=") != -1) {
      Page = 4;
      // Time Zone
      if (request.indexOf("TZH=") != -1)  {
        String Tmp = request;
        int t1 = Tmp.indexOf("TZH=");
        Tmp.remove(0,t1+4);
        t1 = Tmp.indexOf("&");
        if (t1 == -1) {
          t1 = Tmp.indexOf(" ");
        }
        Tmp.remove(t1);
        TimeZone = Tmp.toFloat();
        if ( (TimeZone < -12) or (TimeZone > 12) ) {
          Error4 = true;
          TimeZone = 0;
        }
      }
      // Get NTP IP address
      //--------------------------------
      if (request.indexOf("IP_1=") != -1) {
        String Tmp = request;
        int t1 = Tmp.indexOf("IP_1=");
        Tmp.remove(0,t1+5);
        t1 = Tmp.indexOf("&");
        Tmp.remove(t1);
        IP_1 = Tmp.toInt();
        if ( (IP_1 < 0) or (IP_1 > 255) ) Error5 = true;
      }
      if (request.indexOf("IP_2=") != -1) {
        String Tmp = request;
        int t1 = Tmp.indexOf("IP_2=");
        Tmp.remove(0,t1+5);
        t1 = Tmp.indexOf("&");
        Tmp.remove(t1);
        IP_2 = Tmp.toInt();
        if ( (IP_2 < 0) or (IP_2 > 255) ) Error5 = true;
      }
      if (request.indexOf("IP_3=") != -1) {
        String Tmp = request;
        int t1 = Tmp.indexOf("IP_3=");
        Tmp.remove(0,t1+5);
        t1 = Tmp.indexOf("&");
        Tmp.remove(t1);
        IP_3 = Tmp.toInt();
        if ( (IP_3 < 0) or (IP_3 > 255) ) Error5 = true;
      }
      if (request.indexOf("IP_4=") != -1) {
        String Tmp = request;
        int t1 = Tmp.indexOf("IP_4=");
        Tmp.remove(0,t1+5);
        t1 = Tmp.indexOf("&");
        Tmp.remove(t1);
        IP_4 = Tmp.toInt();
        if ( (IP_4 < 0) or (IP_4 > 255) ) Error5 = true;
      }
      if ( (Error4 == false) and (Error5 == false) ) {
        // Set new NTP IP
        timeServer[0] = IP_1;
        timeServer[1] = IP_2;
        timeServer[2] = IP_3;
        timeServer[3] = IP_4;
        for (byte i = 0; i < 5; i++) {
          Man_on[i] = false;
          Man_off[i] = false;
        }
        //Save Time Server Settings
        SaveNTP();
        NTPtimeOk = false;
        setSyncInterval(NTPfastReq);
      }
    }
    // See if Save Button was pressed
    //----------------------------------------------------------------------
    if (request.indexOf("SaveBtn5=") != -1) {
      Page = 4;
      //Get new hour
      String Tmp = request;
      int t1 = Tmp.indexOf("TimeHour=");
      Tmp.remove(0,t1+9);
      t1 = Tmp.indexOf("&");
      Tmp.remove(t1);
      NewHH = Tmp.toInt();
      if ( (NewHH < 0) or (NewHH > 23) ) Error6 = true;
      //Get new minute
      Tmp = request;
      t1 = Tmp.indexOf("TimeMinute=");
      Tmp.remove(0,t1+11);
      t1 = Tmp.indexOf("&");
      Tmp.remove(t1);
      NewMM = Tmp.toInt();
      if ( (NewMM < 0) or (NewMM > 59) ) Error6 = true;
      //Get new date
      Tmp = request;
      t1 = Tmp.indexOf("TimeDate=");
      Tmp.remove(0,t1+9);
      t1 = Tmp.indexOf("&");
      Tmp.remove(t1);
      Newdd = Tmp.toInt();
      if ( (Newdd < 1) or (Newdd > 31) ) Error7 = true;
      //Get new month
      Tmp = request;
      t1 = Tmp.indexOf("TimeMonth=");
      Tmp.remove(0,t1+10);
      t1 = Tmp.indexOf("&");
      Tmp.remove(t1);
      Newmm = Tmp.toInt();
      if ( (Newmm < 1) or (Newmm > 12) ) Error7 = true;
      //Get new year
      Tmp = request;
      t1 = Tmp.indexOf("TimeYear=");
      Tmp.remove(0,t1+9);
      t1 = Tmp.indexOf("&");
      if (t1 == -1) {
        t1 = Tmp.indexOf(" ");
      }
      Tmp.remove(t1);
      Newyy = Tmp.toInt();
      if ( (Newyy < 2000) or (Newyy > 2069) ) Error7 = true;
      // Update time
      //------------
      setTime(NewHH, NewMM, 0, Newdd, Newmm, Newyy);
      LastHH = NewHH;
      LastMM = NewMM;
      Lastdd = Newdd;
      Lastmm = Newmm;
      Lastyy = Newyy;
      TimeOk = true;
      for (byte i = 0; i < 5; i++) {
          Man_on[i] = false;
          Man_off[i] = false;
      }
      timeOld = 0;
      setSyncInterval(NTPslowReq);
    }
    // See if Refresh Button was pressed
    //----------------------------------------------------------------------
    if (request.indexOf("RefreshBtn5=") != -1) {
      Page = 4;
    }

    // Check time before updating web page
    //----------------------------------------------------------------------
    DoTimeCheck();

    // Web Page HTML Code
    //==================================
    client.println("<!doctype html>");
    client.println("<html lang='en'>");
    client.println("<head>");
    // Refresh home page every 60 sec
    //client.println("<META HTTP-EQUIV=""refresh"" CONTENT=""60"">");  // ----------------  was comentedout ------------------------
    client.print("<div style='margin-left: 20px;'>");
    client.print("<style> body {background-color: #01DF3A;Color: #2B276E;}</style>");  // <style> body {background-color: #C3FCF7;Color: #2B276E;}</style>
    client.println("<title>");
    client.println(String(Supla_name));
    client.println("</title>");
    client.println("  <link rel='shortcut icon' href='data:image/x-icon;base64,AAABAAEAEBAQAAEABAAoAQAAFgAAACgAAAAQAAAAIAAAAAEABAAAAAAAgAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAA/4QAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEAERABAAEAEBAQABAQEBABAAEAEQARABAQABAQABAAAQARAAEQARAAAAAAAAAAAAAAAAAAAAAAEREAEQAQAAAQAAEAEBAAABAAAAAQEAAAERAAEQAREAAQAAEAABABABAAAQAQEAEAEREAEQAREAAAAAAAAAAAD//wAA2N0AAKuqAADdmQAArrsAANnMAAD//wAA//8AAIZvAAC9rwAAv68AAI5jAAC97QAAva0AAIZjAAD//wAA' type='image/x-icon' />\n");
    client.println("  <link rel='icon' href='data:image/x-icon;base64,AAABAAEAEBAQAAEABAAoAQAAFgAAACgAAAAQAAAAIAAAAAEABAAAAAAAgAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAA/4QAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEAERABAAEAEBAQABAQEBABAAEAEQARABAQABAQABAAAQARAAEQARAAAAAAAAAAAAAAAAAAAAAAEREAEQAQAAAQAAEAEBAAABAAAAAQEAAAERAAEQAREAAQAAEAABABABAAAQAQEAEAEREAEQAREAAAAAAAAAAAD//wAA2N0AAKuqAADdmQAArrsAANnMAAD//wAA//8AAIZvAAC9rwAAv68AAI5jAAC97QAAva0AAIZjAAD//wAA' type='image/x-icon' />\n");
    client.println("</head>");
    client.print("<body>");
    client.print("<font size = \"5\"><b>");
    client.print(String(Supla_name));
    client.print("</font></b><br>"); 
    // Show time
    //----------------------------------------------------------------------
    client.print("<p style=\"color:#180BF4;\";>");  
    client.print("<font size = \"5\"><b>");
    if (hour() < 10) client.print(" ");
    client.print(hour());
    client.print(":");
    if (minute() < 10) client.print("0");
    client.print(minute());
    client.print("</font></p>"); 
    // Day of the week
    //----------------------------------------------------------------------
    switch (weekday()) {
      case 1: client.print("Sunday, ");
              break;
      case 2: client.print("Monday, ");
              break;
      case 3: client.print("Tuesday, ");
              break;  
      case 4: client.print("Wednesday, ");
              break;  
      case 5: client.print("Thursday, ");
              break;  
      case 6: client.print("Friday, ");
              break;  
      case 7: client.print("Saturday, ");
              break;  
      default: client.print("");
              break;        
    }
    // Date
    //----------------------------------------------------------------------
    client.print(day());
    // Month
    //----------------------------------------------------------------------
    switch (month()) {
      case  1: client.print(" January ");
               break;
      case  2: client.print(" February ");
               break;
      case  3: client.print(" March ");
               break;
      case  4: client.print(" April ");
               break;
      case  5: client.print(" May ");
               break;
      case  6: client.print(" June ");
               break;
      case  7: client.print(" July ");
               break;
      case  8: client.print(" August ");
               break;
      case  9: client.print(" September ");
               break;
      case 10: client.print(" October ");
               break;
      case 11: client.print(" November ");
               break;
      case 12: client.print(" December ");
               break;
      default: client.print(" ");
               break;        
    }
    // Year
    //----------------------------------------------------------------------
    client.print(year());
    client.print("<br><br>");
    // Show system status
    //----------------------------------------------------------------------
    client.print(" Main: </b>");
    if  (TimeOk == false) { 
      client.print("Blocked - Time not set!");
    }
    else {
      if  (Man_off[0] == true) {
        client.print("STANDBY until next event");
      }
      if  (Man_on[0] == true){
        client.print("ON until next event");
      }     
      if ((Man_off[0] == false) and (Man_on[0] == false)) {
        if (Output_is[0] == 1) client.print("ON"); else client.print("OFF");
      }
    }
    client.println("<br>");
    // Show system status Solar
    //----------------------------------------------------------------------
    client.print("<b> Solar: </b>");
    if  (TimeOk == false) { 
      client.print("Blocked - Time not set!");
    }else {
      if  (Man_off[1] == true) {
        client.print("STANDBY until next event");
      }
      if  (Man_on[1] == true){
        client.print("ON until next event");
      }     
      if ((Man_off[1] == false) and (Man_on[1] == false)) {
       if (Output_is[1] == 1) client.print("ON"); else client.print("OFF");
      }
     }
    client.println("<br>");
    // Show system status Aux 1
    //----------------------------------------------------------------------
    client.print("<b> Aux 1: </b>");
    if  (TimeOk == false) { 
      client.print("Blocked - Time not set!");
    }else {
      if  (Man_off[2] == true) {
        client.print("STANDBY until next event");
      }
      if  (Man_on[2] == true){
        client.print("ON until next event");
      }     
      if ((Man_off[2] == false) and (Man_on[2] == false)) {
       if (Output_is[2] == 1) client.print("ON"); else client.print("OFF");
      }
    }
    client.println("<br>");
    // Show system status Aux 2
    //----------------------------------------------------------------------
    client.print("<b> Aux 2: </b>");
    if  (TimeOk == false) { 
      client.print("Blocked - Time not set!");
    }else {
      if  (Man_off[3] == true) {
        client.print("STANDBY until next event");
      }
      if  (Man_on[3] == true){
        client.print("ON until next event");
      }     
      if ((Man_off[3] == false) and (Man_on[3] == false)) {
       if (Output_is[3] == 1) client.print("ON"); else client.print("OFF");
      }
    }
    client.println("<br>");
    // Show system status auto/man
    //----------------------------------------------------------------------
    client.print("<b>Auto/Man: </b>");
    if  (TimeOk == false) { 
      client.print("Blocked - Time not set!");
    }else {
      if  (Man_off[4] == true) {
        client.print("STANDBY until next event");
      }
      if  (Man_on[4] == true){
        client.print("ON until next event");
      }     
      if ((Man_off[4] == false) and (Man_on[4] == false)) {
       if (Output_is[4] == 1) client.print("ON"); else client.print("OFF");
      }
    }
    client.println("<br><br>");
    //Menu
    //----------------------------------------------------------------------
    client.print("<form action= method=\"get\"><b>");
    client.print("<a href=\"Link=1\">Home</a>&emsp;");
    client.print("<a href=\"Link=2\">Programs</a>&emsp;");
    client.print("<a href=\"Link=4\">Time</a><br>"); 
    // Draw line
    //----------------------------------------------------------------------
    client.print("</b><hr />");  
    
    // Status PAGE
    //============
    if (Page == 1) {
      client.print("<font size = \"4\"><b>Status</font></b><br><br>");
      //Button 1
      client.println("<input type=\"submit\" name =\"Over1\" value=\"Manual Override\">");        
      client.print("<b> Main: ");
      if ((Man_on[0]  == true) or (Man_off[0]   == true)) {
         client.print("Active");
      }else {client.print(" ----- ");}        
      client.print("</b><br>");
      //Button 2
      client.println("<input type=\"submit\" name =\"Over2\" value=\"Manual Override\">");       
      client.print("<b> Solar: ");
      if ((Man_on[1]  == true) or (Man_off[1]   == true)) {
        client.print("Active");
      }else {client.print(" ----- ");}          
      client.print("</b><br>");
      //Button 3
      client.println("<input type=\"submit\" name =\"Over3\" value=\"Manual Override\">");       
      client.print("<b> Aux 1: ");
      if ((Man_on[2]  == true) or (Man_off[2]   == true)) {
        client.print("Active");
      }else {client.print(" ----- ");}          
      client.print("</b><br>");
      //Button 4
      client.println("<input type=\"submit\" name =\"Over4\" value=\"Manual Override\">");       
      client.print("<b> Aux 2: ");
      if ((Man_on[3]  == true) or (Man_off[3]   == true)) {
        client.print("Active");
      }else {client.print(" ----- ");}          
      client.print("</b><br>");
      //Button 5
      client.println("<input type=\"submit\" name =\"Over5\" value=\"Manual Override\">");       
      client.print("<b> Auto/Man: ");
      if ((Man_on[4]  == true) or (Man_off[4]   == true)) {
        client.print("Active");
      }else {client.print(" --- ");}          
      client.print("</b><br><br>");      
      //Button Refrech
      client.println("&emsp;");
      client.println("<input type=\"submit\" name =\"RefreshBtn1\" value=\"Refresh\">");
      client.println("<br>");
      // Draw line
      client.print("<hr />");        
      // Show last time synch
      client.print("<font size = \"2\">"); 
      client.print("</b>Version ");
      client.print(Version);
      client.print("<br>");
      client.print("Time was last updated on ");
      client.print(Lastdd);
      client.print("/");
      if (Lastmm < 10) client.print("0");
      client.print(Lastmm);
      client.print("/");
      client.print(Lastyy);
      client.print(" at ");
      if (LastHH < 10) client.print("0");
      client.print(LastHH);
      client.print(":");
      if (LastMM < 10) client.print("0");
      client.print(LastMM);
      client.print("</font>");
    }
  
    //Program  PAGE
    //============
    if (Page == 2) {
      // Program number
      client.print("<font size = \"4\"><b>"); 
      client.print("Program ");
      client.print(PgmNr + 1);
      client.print(" of 7");
      //Previous Button
      client.println("&emsp;<input type=\"submit\" name =\"SaveBtnPrev\" value=\" << \">");
      client.println("&emsp;");
      //Next Button
      client.println("<input type=\"submit\" name =\"SaveBtnNext\" value=\" >> \"></font></b><br><br>");
      //Timer on
      client.print("<b> Timer  On / Off: </b>");
      client.print("<input type=\"Checkbox\" name=\"D8\"");
      if (On_Days[PgmNr][7]==true) client.print("checked"); else client.print("unchecked");
      client.print("> <br><br>");      
      //On time
      client.print("On  Time: <input type=\"text\"<input maxlength=\"2\" size=\"2\" name=\"OnH\"value =\"");
//      if ( (On_Time[PgmNr] / 100) < 10) client.print("0");
      client.print(On_Time[PgmNr]/100);    
      client.print("\"> : <input type=\"text\"<input maxlength=\"2\" size=\"2\" name=\"OnM\"value =\"");
      if ( (On_Time[PgmNr] % 100) < 10) client.print("0");
      client.print(On_Time[PgmNr]%100);    
      client.print("\">");
      if (Error1 == true) client.print(ErrMsg);
      client.print("<br><br>");
      //Off time
      client.print("Off Time: <input type=\"text\"<input maxlength=\"2\" size=\"2\" name=\"OffH\"value =\"");
//      if ( (Off_Time[PgmNr] / 100) < 10) client.print("0");
      client.print(Off_Time[PgmNr]/100);    
      client.print("\"> : <input type=\"text\"<input maxlength=\"2\" size=\"2\" name=\"OffM\"value =\"");
      if ( (Off_Time[PgmNr] % 100) < 10) client.print("0");
      client.print(Off_Time[PgmNr]%100);    
      client.print("\">");
      if (Error2 == true) client.print(ErrMsg);
      client.print("<br><br>");
      //Day 1
      client.print("<input type=\"Checkbox\" name=\"D1\"");
      if (On_Days[PgmNr][0]==true) client.print("checked"); else client.print("unchecked");
      client.print("> Sun<br>");
      //Day 2
      client.print("<input type=\"Checkbox\" name=\"D2\"");
      if (On_Days[PgmNr][1]==true) client.print("checked"); else client.print("unchecked");
      client.print("> Mon<br>");
      //Day 3
      client.print("<input type=\"Checkbox\" name=\"D3\"");
      if (On_Days[PgmNr][2]==true) client.print("checked"); else client.print("unchecked");
      client.print("> Tue<br>");
      //Day 4
      client.print("<input type=\"Checkbox\" name=\"D4\"");
      if (On_Days[PgmNr][3]==true) client.print("checked"); else client.print("unchecked");
      client.print("> Wed<br>");
      //Day 5
      client.print("<input type=\"Checkbox\" name=\"D5\"");
      if (On_Days[PgmNr][4]==true) client.print("checked"); else client.print("unchecked");
      client.print("> Thu<br>");
      //Day 6
      client.print("<input type=\"Checkbox\" name=\"D6\"");
      if (On_Days[PgmNr][5]==true) client.print("checked"); else client.print("unchecked");
      client.print("> Fri<br>");
      //Day 7
      client.print("<input type=\"Checkbox\" name=\"D7\"");
      if (On_Days[PgmNr][6]==true) client.print("checked"); else client.print("unchecked");
      client.print("> Sat<br><br>");
      //Channel
      client.print("</div><div style='margin-left: 120px;'>");
      client.print("<b> Active Output: </b><br><br>");
      client.print("</div><div style='margin-left: 20px;'>");
      client.print("<input type=\"Checkbox\" name=\"C1\"");
      if (On_Channel[PgmNr][0]==true) client.print("checked"); else client.print("unchecked");
      client.print("> Main ");
      client.print("<input type=\"Checkbox\" name=\"C2\"");
      if (On_Channel[PgmNr][1]==true) client.print("checked"); else client.print("unchecked");
      client.print("> Solar ");
      client.print("<input type=\"Checkbox\" name=\"C3\"");
      if (On_Channel[PgmNr][2]==true) client.print("checked"); else client.print("unchecked");
      client.print("> Aux 1 ");
      client.print("<input type=\"Checkbox\" name=\"C4\"");
      if (On_Channel[PgmNr][3]==true) client.print("checked"); else client.print("unchecked");
      client.print("> Aux 2 ");
      client.print("<input type=\"Checkbox\" name=\"C5\"");
      if (On_Channel[PgmNr][4]==true) client.print("checked"); else client.print("unchecked");
      client.print("> Auto/Man <br><br>");
      //Button
      client.println("<br>");
      client.println("<input type=\"submit\" name =\"SaveBtn2\" value=\"Save\">");
      client.print("&emsp;");
      client.println("<input type=\"submit\" name =\"ClearBtn1\" value=\"Clear\">");
    }
    
    // Time Server PAGE
    //=================
    if (Page == 4) {
      client.print("<font size = \"4\"><b>Time Setup</font></b><br><br>");  //  client.print("<font size = \"4\"><b><u>Time Setup</u></font></b><br><br>"); 
      
      //Time Zone
      client.print("<font size = \"3\"><b>NTP Network Setup</font></b><br><br>"); 
      client.print("Time Zone ");
      client.print("<input type=\"text\"<input maxlength=\"6\" size=\"7\" name=\"TZH\" value =\"");
      client.print(TimeZone,2);
      client.println("\">");
//      if (Error4 == true) client.print(ErrMsg); else client.print(" (hours)");
      client.print("<br><br>");
      //IP Addtess if time server
      client.print("Time Server IP : <i>(default 129.6.15.28 or 196.4.160.4)</i><br>");
      client.print(" <input type=\"text\"<input maxlength=\"3\" size=\"3\" name=\"IP_1\"value =\"");
      client.print(IP_1);    
      client.print("\">");
      client.print(" <input type=\"text\"<input maxlength=\"3\" size=\"3\" name=\"IP_2\"value =\"");
      client.print(IP_2);    
      client.print("\">");
      client.print(" <input type=\"text\"<input maxlength=\"3\" size=\"3\" name=\"IP_3\"value =\"");
      client.print(IP_3);    
      client.print("\">");
      client.print(" <input type=\"text\"<input maxlength=\"3\" size=\"3\" name=\"IP_4\"value =\"");
      client.print(IP_4);    
      client.print("\">");
//      if (Error5 == true) client.print(ErrMsg);
      //Button 1
      client.println("<br><br>");
      client.println("<input type=\"submit\" name =\"SaveBtn4\" value=\"Save\"><br>");
      // Draw line
      client.print("<hr />");        

      // Set Time Inputs
      client.print("<font size = \"3\"><b>Local Time Adjust</font></b><br>"); 
      client.print("<br>Time: <input type=\"text\"<input maxlength=\"2\" size=\"2\" name=\"TimeHour\"value =\"");
      client.print(hour());
      client.print("\">");
      client.print(" : <input type=\"text\"<input maxlength=\"3\" size=\"3\" name=\"TimeMinute\"value =\"");
      if (minute() < 10) client.print("0");
      client.print(minute());
      client.print("\">");
//      if (Error6 == true) client.print(ErrMsg); else client.print(" (hh:mm)");
      // Set Date Inputs
      client.print("<br><br>");
      client.print("Date: <input type=\"text\"<input maxlength=\"2\" size=\"2\" name=\"TimeDate\"value =\"");
      client.print(day());
      client.print("\">");
      client.print(" / <input type=\"text\"<input maxlength=\"3\" size=\"3\" name=\"TimeMonth\"value =\"");
      if (month() < 10) client.print("0");
      client.print(month());
      client.print("\">");
      client.print(" / <input type=\"text\"<input maxlength=\"4\" size=\"4\" name=\"TimeYear\"value =\"");
      client.print(year());
      client.print("\">");
//      if (Error7 == true) client.print(ErrMsg); else client.print(" (dd/mm/yyyy)");
      //Button 2
      client.println("<br><br>");
      client.println("<input type=\"submit\" name =\"SaveBtn5\" value=\"Update Time\">");
      //Button 3
      client.println("&emsp;");
      client.println("<input type=\"submit\" name =\"RefreshBtn\" value=\"Refresh\">");
      
      // Draw line
      client.print("<hr />");        
      // Show last time synch
      client.print("<font size = \"2\">"); 
      client.print("</b>Time last updated ");
      client.print(Lastdd);
      client.print("/");
      if (Lastmm < 10) client.print("0");
      client.print(Lastmm);
      client.print("/");
      client.print(Lastyy);
      client.print(" at ");
      if (LastHH < 10) client.print("0");
      client.print(LastHH);
      client.print(":");
      if (LastMM < 10) client.print("0");
      client.print(LastMM);
      client.print("</font>");
    }
    
    client.println("</div>");
    client.println("</body>");
    client.println("</html>");
    // End of Web Page
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
      client.setTimeout(1000);
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
          cb.get_temperature = &get_temperature;
          cb.get_temperature_and_humidity = NULL;
          cb.get_rgbw_value = NULL;
          cb.set_rgbw_value = NULL;          
          return cb;
}

void WiFi_up(){ // conect to wifi
 
  if (millis() > wifimilis)  {
    WiFi.begin();
    pr_wifi = true;
    tikOn = true;  
    Serial.println("CONNECTING WIFI");   
    wifimilis = (millis() + wifi_checkDelay) ;
  }
}
void Click(Button3& btn) {
  button_2x1();
}
void button_2x1() {  
              booton_action = true;
              WebButton[4]= true;

}
void doubleClick(Button3& btn) {
  button_2x2();
}
void button_2x2() {
              booton_action = true;
              WebButton[1]= true;

}
//###############################################################################################
// Save Program Data
// 
//###############################################################################################
void SaveProgram() {
  for (byte i = 0; i < 7; i++) {
    byte nr = i;
    byte t1 = 20 + (nr * 17);
    //On Time
    EEPROM.write(t1,On_Time[nr]/100);
    EEPROM.write(t1 +  1,On_Time[nr]%100);
    // Off time
    EEPROM.write(t1 +  2,Off_Time[nr]/100);
    EEPROM.write(t1 +  3,Off_Time[nr]%100);
    // On days
    EEPROM.write(t1 +  4,On_Days[nr][0]);  
    EEPROM.write(t1 +  5,On_Days[nr][1]);  
    EEPROM.write(t1 +  6,On_Days[nr][2]);  
    EEPROM.write(t1 +  7,On_Days[nr][3]);  
    EEPROM.write(t1 +  8,On_Days[nr][4]);  
    EEPROM.write(t1 +  9,On_Days[nr][5]);  
    EEPROM.write(t1 + 10,On_Days[nr][6]);  
    // Spare
    EEPROM.write(t1 + 11,On_Days[nr][7]);
    // Channel 
    EEPROM.write(t1 +  12,On_Channel[nr][0]);  
    EEPROM.write(t1 +  13,On_Channel[nr][1]);  
    EEPROM.write(t1 +  14,On_Channel[nr][2]);  
    EEPROM.write(t1 +  15,On_Channel[nr][3]);  
    EEPROM.write(t1 +  16,On_Channel[nr][4]); 
    EEPROM.commit();
    Serial.print("Save Program Data: ");
    Serial.println(nr);
  }
}


//###############################################################################################
// Save NTP Server Data
// 
//###############################################################################################
void SaveNTP() {
  int Tz = (TimeZone * 100);
  Tz = Tz + 1200;
  TimeZoneH = Tz/100;
  TimeZoneM = Tz%100;
  EEPROM.write( 8,TimeZoneH);
  EEPROM.write( 9,TimeZoneM); 
  EEPROM.write(10,IP_1);
  EEPROM.write(11,IP_2);
  EEPROM.write(12,IP_3);
  EEPROM.write(13,IP_4);
  EEPROM.commit();
}

//###############################################################################################
// READ program data from EEPROM
// 
//###############################################################################################
void ReadData() {
  // See if EEPROM contains valid data.
  // EEPROM location 0 will contain EEPROM_chk if data is valid
  // If not valid, store defult settings
  if (EEPROM.read(16) != EEPROM_chk) {
    EEPROM.write( 16,EEPROM_chk);  // EEPROM check
    EEPROM.write( 8,12 );         // Time Zone Hour
    EEPROM.write( 9,0  );         // TimZone Minute
    EEPROM.write(10,129);         // NTP IP 1
    EEPROM.write(11,6  );         // NTP IP 2
    EEPROM.write(12,15 );         // NTP IP 3
    EEPROM.write(13,28 );         // NTP IP 4
    // Clear all programs
    for (byte i = 20; i < 145; i++) {  
      EEPROM.write(i,0);
    }
    EEPROM.commit();
  }
  // Read programs
  for (byte i = 0; i < 7; i++) {
    byte nr = i;
    byte t1 = 20 + ( (i) * 17);
    //On Time
    //---------------------
    On_Time[nr]  = EEPROM.read(t1);
    On_Time[nr] = On_Time[nr] * 100;
    On_Time[nr] = On_Time[nr] + EEPROM.read(t1 + 1);
    // Off time
    //---------------------
    Off_Time[nr]  = EEPROM.read(t1 + 2);
    Off_Time[nr] = Off_Time[nr] * 100;
    Off_Time[nr] = Off_Time[nr] + EEPROM.read(t1 + 3);
    // On days
    //---------------------
    On_Days[nr][0] = EEPROM.read(t1 +  4);  
    On_Days[nr][1] = EEPROM.read(t1 +  5);  
    On_Days[nr][2] = EEPROM.read(t1 +  6);  
    On_Days[nr][3] = EEPROM.read(t1 +  7);  
    On_Days[nr][4] = EEPROM.read(t1 +  8);  
    On_Days[nr][5] = EEPROM.read(t1 +  9);  
    On_Days[nr][6] = EEPROM.read(t1 + 10);  
    // Spare
    //---------------------
    On_Days[nr][7] = EEPROM.read(t1 + 11);
    //Channel
    On_Channel[nr][0] = EEPROM.read(t1 +  12);
    On_Channel[nr][1] = EEPROM.read(t1 +  13);
    On_Channel[nr][2] = EEPROM.read(t1 +  14);
    On_Channel[nr][3] = EEPROM.read(t1 +  15);
    On_Channel[nr][4] = EEPROM.read(t1 +  16);
  }

  TimeZoneH      = EEPROM.read(8);
  TimeZoneM      = EEPROM.read(9);
  IP_1           = EEPROM.read(10);
  IP_2           = EEPROM.read(11);
  IP_3           = EEPROM.read(12);
  IP_4           = EEPROM.read(13);
  // Setup timeServer IP
  timeServer[0] = IP_1;
  timeServer[1] = IP_2;
  timeServer[2] = IP_3;
  timeServer[3] = IP_4;
  // Assemble Timezone value
  TimeZone = TimeZoneM;
  TimeZone = TimeZone / 100;
  TimeZone = TimeZone + TimeZoneH;
  TimeZone = TimeZone -12;
}

//###############################################################################################
// NTP Code - do not change
//
//###############################################################################################
const int NTP_PACKET_SIZE = 48;                 // NTP time is in the first 48 bytes of message
byte      packetBuffer[NTP_PACKET_SIZE];        //buffer to hold incoming & outgoing packets

time_t getNtpTime() {
  while (Udp.parsePacket() > 0) ;               // discard any previously received packets
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 5000) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      if (TimeOk ==false) {
        TimeOk = true;
      }
      TimeCheck   = true;
      return secsSince1900 - 2208988800UL + TimeZone * SECS_PER_HOUR;
    }
  }
  return 0; // return 0 if unable to get the time
}


//###############################################################################################
// send an NTP request to the time server at the given address
//
//###############################################################################################
void sendNTPpacket(IPAddress & address) {
  // set all bytes in the buffer to 0
  //------------------------------------------------
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  //------------------------------------------------
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;            // Stratum, or type of clock
  packetBuffer[2] = 6;            // Polling Interval
  packetBuffer[3] = 0xEC;         // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  //------------------------------------------------
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  //NTP requests are to port 123
  //------------------------------------------------
  Udp.beginPacket(address, 123); 
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
//###############################################################################################
// Scan for NTP Time changes
// 
//###############################################################################################
void CheckNTPtime() {

  // This line needed to keep NTP Time Synch active
  //------------------------------------------------
  timeNow = (10000 * hour()) + (minute() * 100) + second();

  // See if NTP time was set
  //------------------------
  if ( (TimeOk == true) and (NTPtimeOk == false) and (TimeCheck == true) ){
      setSyncInterval(NTPslowReq);
      NTPtimeOk = true;
  }
  // See if NTP Time was updated
  //----------------------------
  if (TimeCheck == true) {
    LastHH = hour();
    LastMM = minute();
    Lastdd = day();
    Lastmm = month();
    Lastyy = year();
    TimeCheck = false;
  }
}


//###############################################################################################
// Scan for Button presses
// 
//###############################################################################################
void ScanButton() {

  for (byte x = 0; x < 5; x++) {

  if  (WebButton[x] == true)  {
    WebButton[x]= false;
    
    // Check manual OFF
    if (Man_on[x] == false) {
      if (Output_is[x] == 1) {  
        if (Man_off[x] == false) {
            Man_off[x] = true;
            Man_on[x] = false;
            goto doLED;
        }
      }
      else {
        if (Man_off[x] == true) {
          Man_off[x] = false;
          Man_on[x] = false;
          goto doLED;
        }
     }
   }
    
    // Check manual ON
    if  (Man_off[x] == false) {
      if (Output_is[x] == 0) { 
        if (Man_on[x] == false) {
            Man_on[x] = true;
            Man_off[x] = false;
            goto doLED;
          }
      }
      else {
        if (Man_on[x] == true) {
          Man_on[x] = false;
          Man_off[x] = false;
          goto doLED;
        }
      }
    }

    doLED:

    timeOld = 0;
   }
  }
}


//###############################################################################################
// Update LED Status
// 
//###############################################################################################
void UpdateLED() {
  //LED Status
  if (TimeOk == false) {
    if ( (millis() - old_ms) > 3000) {
      
       // digitalWrite(LED,HIGH); //1  
      //  delay(50);
      //  digitalWrite(LED,LOW); //1
      //  delay(50);

      old_ms - millis();
    }
  }
}


//###############################################################################################
// See if time has changed and update output according to programs
// 
//###############################################################################################
void DoTimeCheck() {
      Output[0] = false;
      Output[1] = false;
      Output[2] = false;
      Output[3] = false;
      Output[4] = false;
      Output[5] = false;

  timeNow = (100 * hour()) + minute();
  if (timeOld != timeNow) {
    // Time changed - check outputs
    timeOld = timeNow;
  if (local_timer){
    for (byte i = 0; i < 7; i++) {
      // See if Buzzer can be controlled
      if (TimeOk != false) {
        //Time Ok, check if output must be on
        // See if Ontime < OffTime (same day)
        if (On_Time[i] < Off_Time[i]) {
          if ( (timeNow >= On_Time[i]) and (timeNow < Off_Time[i]) ) {
            // See if current day is selected
            if ((On_Days[i][weekday() - 1] == true) && (On_Days[i][7]==true)) { 
              for (byte s = 0; s < 5; s++) {
                if (On_Channel[i][s] == true){
                  Output[s] = true;
                }
              }
            }
          }
        }
        // See if Ontime > OffTime (over two days)
        if (On_Time[i] > Off_Time[i]) {
          if ( (timeNow < Off_Time[i]) or (timeNow>= On_Time[i]) ) {
            int PrevDay = weekday() - 2;
            if (PrevDay < 0) PrevDay = 6;
            // Check current day
            if (timeNow >= On_Time[i]) {
              if ((On_Days[i][weekday() - 1] == true) && (On_Days[i][7]==true)) { 
               for (byte s = 0; s < 5; s++) {
                if (On_Channel[i][s] == true){
                  Output[s] = true;
                }
              }
             }
            }
            // Check previous day
            if (timeNow < Off_Time[i]) {
              if ((On_Days[i][PrevDay] == true) && (On_Days[i][7]==true)) { 
                 for (byte s = 0; s < 5; s++) {
                  if (On_Channel[i][s] == true){
                    Output[s] = true;
                 }
                }
              }
            }
          }
        } 
      }
    }
  }
   for (byte z = 0; z < 5; z++) {
    // Check manual off 
    if  (Man_on[z] == false) {
      if (Man_off[z] == true) {
        if (Output[z] == true) {
          Output[z] = false;
        }
        else {
          Man_off[z] = false;
        }
      }
    }
    // Check manual on 
    if  (Man_off[z] == false) {
      if (Man_on[z] == true) {
        if (Output[z] == false) {
          Output[z] = true;
        }
        else {
          Man_on[z] = false;
        }
      }
    }
   }

    // Set output

    for (byte x = 0; x < 5; x++) {
     Output_is[x] = Output[x];
     if ((Output[x] == true) && (btn[x].mem != Output[x])){
      Serial.print("set on: ");Serial.println(x);
      booton_action = true;
      digitalWrite(btn[x].relay_pin-1, HIGH);
      btn[x].mem = true; 
      CustomSupla.channelValueChanged(x,true);
     }else if (btn[x].mem != Output[x]){
      Serial.print("set off: ");Serial.println(x);
      booton_action = true;
      digitalWrite(btn[x].relay_pin-1, LOW);
      btn[x].mem = false;
      CustomSupla.channelValueChanged(x,false);
     }
     //btn[x].mem = Output[x];
    }   
  } 

}
