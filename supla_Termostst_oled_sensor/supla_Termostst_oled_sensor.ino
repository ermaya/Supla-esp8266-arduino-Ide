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
#include <OneWire.h>
#include <DallasTemperature.h>
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

//#include "SSD1306Wire.h" //----- 0.96 Oled --- https://github.com/ThingPulse/esp8266-oled-ssd1306
//SSD1306Wire  display(0x3c, 0, 2);  // D3-SDA  D4-SCL ----- 0.96 Oled ---
#include "SH1106Wire.h" //----- 1.3 Oled ---
SH1106Wire display(0x3c, 0, 2);  // D3-SDA  D4-SCL ----- 1.3 Oled ---

#define ONE_WIRE_BUS 13  //D7 --------------------oneWire Dallas Temperature --------requires 4k7 resistance
int relay_1 = 15;        //D8 ------------------- thermometer Auto/Man Led ----- active high
int relay_2 = 12;        //D6 ------------------- relay output ----- active high
int relay_3 = 120;       // --------------------- thermostat temperature setting +/- 0.5º no Gpio -----
int relay_4 = 122;       // --------------------- no Gpio -----
int button_1 = 14;       //D5 ------------------- automatic / manual thermostat change  ---- WiFi config 10 seconds  --------
int button_2 = 16;       //D0 ------------------- manual on / off ----requires 10k pullup resistance due to lack of internal
int button_3 = 5;        //D1 ------------------- thermostat temperature setting - 0.5º----
int button_4 = 4;        //D2 ------------------- thermostat temperature setting + 0.5º----
#define sensor_pin 9
#define BTN_COUNT 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress;
int  resolution = 12;
bool pr_wifi = true;
bool start = true;
bool eep = LOW;     
int epr = 0;         
int s;             
int Term_Temp ;
int Term_Hist ;
double tr = -275;
double trs ;
unsigned long wifi_checkDelay = 20000;
unsigned long wifimilis;
unsigned long eep_milis;
unsigned long tr_milis;
unsigned long ds_milis;
int C_W_state = HIGH; 
int last_C_W_state = HIGH;
unsigned long time_last_C_W_change = 0; 
long C_W_delay = 10000;               // config delay 10 seconds           ----------        opóźnienie konfiguracji 10 sekund
bool tikOn = false;
WiFiClient client;
ESP8266WebServer httpServer(81);
ESPEFC httpUpdater;
const char* update_path = "/update";
char Supla_server[80];
char Location_id[15];
char Location_Pass[34];
char Supla_name[51];
char up_host[21];
char update_username[21];
char update_password[21];
char term_temp[5];
char term_hist[5];
byte mac[6];
bool shouldSaveConfig = false;
bool initialConfig = false;
bool dimm = false;
bool window = false;          
unsigned long dimm_milis ;
int timeout = 180;          // seconds to run the wifi config
const uint8_t logo32_glcd_bmp[] PROGMEM =  //logo supla 32
{
  0x00,0x00,0x00,0x00,0xC0,0x1F,0x00,0x00,0xE0,0x7F,0x00,0x00,
  0xF0,0x70,0x00,0x00,0x30,0xE0,0x00,0x00,0x38,0xC0,0x00,0x00,
  0x18,0xC0,0x01,0x00,0x18,0xC0,0x01,0x00,0x38,0xC0,0x00,0x00,
  0x38,0xE0,0x01,0x00,0x70,0xF0,0x07,0x00,0xE0,0x7F,0x0E,0x00,
  0xC0,0x3F,0x38,0x00,0x00,0x1F,0xE0,0x0F,0x00,0x18,0xC0,0x1F,
  0x00,0x18,0xC0,0x30,0x00,0x18,0xC0,0x30,0x00,0x30,0xC0,0x30,
  0x00,0x30,0x80,0x1F,0x00,0x30,0xC0,0x0F,0x00,0x20,0x60,0x00,
  0x00,0x60,0x20,0x00,0x00,0x60,0x30,0x00,0x00,0x40,0x18,0x00,
  0x00,0xC0,0x0D,0x00,0x00,0xC0,0x07,0x00,0x00,0x60,0x04,0x00,
  0x00,0x20,0x0C,0x00,0x00,0x20,0x0C,0x00,0x00,0x60,0x06,0x00,
  0x00,0xC0,0x03,0x00,0x00,0x00,0x00,0x00
};
const uint8_t logo16_wifi_bmp[] PROGMEM =  //logo wifi 16
{
  0x00,0x00,0x00,0x00,0xE0,0x07,0x38,0x1C,0xC4,0x23,0x72,0x4E,
  0x08,0x10,0xE4,0x27,0x10,0x0C,0x90,0x09,0x40,0x02,0x60,0x06,
  0x40,0x02,0x80,0x01,0x00,0x00,0x00,0x00
};
const uint8_t logo16_supla_bmp[] PROGMEM =  //logo supla 16
{
  0x30,0x00,0x7C,0x00,0xC4,0x00,0x86,0x00,0x84,0x00,0xDC,0x03,
  0x78,0x36,0x60,0x78,0x40,0x48,0x40,0x38,0x40,0x04,0x80,0x06,
  0x80,0x03,0x40,0x02,0xC0,0x03,0x00,0x01
};
const uint8_t logo16_Win_open[] PROGMEM =  //open window 16
{
  0xFE,0x7F,0xFE,0x7F,0x06,0x60,0x06,0x60,0x06,0x60,0x06,0x60,
  0x06,0x60,0x06,0x60,0x06,0x60,0x06,0x60,0x06,0x60,0x06,0x60,
  0x06,0x60,0x06,0x60,0xFE,0x7F,0xFE,0x7F
};
const uint8_t logo16_Win_close[] PROGMEM =  //close window 16
{
  0xFE,0x7F,0xFE,0x7F,0x86,0x61,0x86,0x61,0x86,0x61,0xFE,0x7F,
  0xFE,0x7F,0x86,0x61,0x86,0x61,0x86,0x61,0x86,0x61,0x86,0x61,
  0x86,0x61,0x86,0x61,0xFE,0x7F,0xFE,0x7F
};
const uint8_t logo_Power_off[] PROGMEM =  //logo supla 16
{
  0xe0, 0xff, 0xff, 0xff, 0x03, 0x38, 0xfc, 0xff, 0xff, 0x0f, 0x0c, 0xf0,
   0xff, 0xff, 0x3f, 0x06, 0xe0, 0xff, 0xff, 0x3f, 0x02, 0xc0, 0xcf, 0x43,
   0x78, 0x03, 0xc0, 0x33, 0x43, 0x78, 0x01, 0xc0, 0x7b, 0x7b, 0xff, 0x01,
   0x80, 0xfd, 0x42, 0xf8, 0x01, 0x80, 0xfd, 0x42, 0xf8, 0x01, 0x80, 0x7b,
   0x7b, 0xff, 0x03, 0xc0, 0x33, 0x7b, 0xff, 0x02, 0xc0, 0xcf, 0x7b, 0x7f,
   0x06, 0xe0, 0xff, 0xff, 0x7f, 0x0c, 0xf0, 0xff, 0xff, 0x3f, 0x18, 0xfc,
   0xff, 0xff, 0x1f, 0xf0, 0xff, 0xff, 0xff, 0x07, 
};
const uint8_t logo_Power_on[] PROGMEM =  //logo supla 16
{
   0xe0, 0xff, 0xff, 0xff, 0x07, 0x38, 0x00, 0x00, 0x30, 0x0c, 0x0c, 0x00,
   0x00, 0x18, 0x18, 0x06, 0x00, 0x00, 0x0c, 0x30, 0x02, 0x0e, 0x00, 0x06,
   0x60, 0x03, 0x9f, 0x19, 0x02, 0x40, 0x81, 0xb1, 0x1b, 0x03, 0xc0, 0x81,
   0xa0, 0x1a, 0x01, 0x80, 0x81, 0xa0, 0x1e, 0x01, 0x80, 0x81, 0xb1, 0x1c,
   0x01, 0xc0, 0x03, 0x9b, 0x18, 0x03, 0x40, 0x02, 0x8e, 0x18, 0x02, 0x60,
   0x06, 0x00, 0x00, 0x06, 0x20, 0x0c, 0x00, 0x00, 0x0c, 0x38, 0x38, 0x00,
   0x00, 0x18, 0x0c, 0xe0, 0xff, 0xff, 0xff, 0x07,
};
void saveConfigCallback () {                 //callback notifying us of the need to save config
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void ondemandwifiCallback () {
          display.clear();
          display.setContrast(100, 241, 64);
          display.setTextAlignment(TEXT_ALIGN_CENTER);
          display.setFont(ArialMT_Plain_10);
          display.drawString(64, 16, "wifi config connect");
          display.drawString(64, 28, "to wifi hotspot");
          display.setFont(ArialMT_Plain_16);
          display.drawString(64, 44, "Supla_Termostat"); 
          display.display();    
  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 80);
  WiFiManagerParameter custom_Location_id("ID", "Location id", Location_id, 15);
  WiFiManagerParameter custom_Location_Pass("Password", "Location Pass", Location_Pass, 34);
  WiFiManagerParameter custom_Supla_name("name", "Supla Device Name", Supla_name, 51,"required");
  WiFiManagerParameter custom_up_host("up_host", "xxxx (DHCP name)", up_host, 21,"required");
  WiFiManagerParameter custom_update_username("updateUsername", "update username", update_username, 21,"required");
  WiFiManagerParameter custom_update_password("updatePassword", "update password", update_password, 21,"required");
  WiFiManagerParameter custom_term_temp("Termostat", "Temp x 10", term_temp, 5,"required");
  WiFiManagerParameter custom_term_hist("Histerisis", "Histerisis x 10", term_hist, 5,"required");

  WiFiManager wifiManager;
  wifiManager.setBreakAfterConfig(true);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_Location_id);
  wifiManager.addParameter(&custom_Location_Pass);
  wifiManager.addParameter(&custom_Supla_name);
  wifiManager.addParameter(&custom_up_host);
  wifiManager.addParameter(&custom_update_username);
  wifiManager.addParameter(&custom_update_password);
  wifiManager.addParameter(&custom_term_temp);
  wifiManager.addParameter(&custom_term_hist);

  wifiManager.setCustomHeadElement("<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>");
  wifiManager.setMinimumSignalQuality(8);
  wifiManager.setConfigPortalTimeout(timeout);

    if (!wifiManager.startConfigPortal("Supla_Termostat")) {
      Serial.println("Not connected to WiFi but continuing anyway.");
    } else {
      
      Serial.println("connected...yeey :)");    //if you get here you have connected to the WiFi
    }
    //read updated parameters
    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(Location_id, custom_Location_id.getValue());
    strcpy(Location_Pass, custom_Location_Pass.getValue());
    strcpy(Supla_name, custom_Supla_name.getValue());
    strcpy(up_host, custom_up_host.getValue());
    strcpy(update_username, custom_update_username.getValue());
    strcpy(update_password, custom_update_password.getValue());
    strcpy(term_temp, custom_term_temp.getValue());
    strcpy(term_hist, custom_term_hist.getValue());
   
  WiFi.softAPdisconnect(true);   //  close AP
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

void TH_Overlay() {

  display.clear();
   if (tr > -100){
         display.setFont(ArialMT_Plain_24);
         display.setTextAlignment(TEXT_ALIGN_CENTER);
         display.drawString(64, 20, String(tr, 2) + "ºC"); 
   } 
   if (tr < -100){
         display.setFont(ArialMT_Plain_24);
         display.setTextAlignment(TEXT_ALIGN_CENTER);
         display.drawString(64, 20, "ERROR"); 
   } 
   if (window == true){
         display.setFont(ArialMT_Plain_16);
         display.setTextAlignment(TEXT_ALIGN_CENTER);
         display.drawString(64, 48, "HALTED " + String(trs, 1) + "ºC");            
   }else{
        if ((btn[0].mem) == 1){
         display.setFont(ArialMT_Plain_16);
         display.setTextAlignment(TEXT_ALIGN_CENTER);
         display.drawString(64, 48, "Auto Set " + String(trs, 1) + "ºC"); 
        }
       if ((btn[0].mem) == 0){
         display.setFont(ArialMT_Plain_16);
         display.setTextAlignment(TEXT_ALIGN_CENTER);
         display.drawString(64, 48, "Manual mode"); 
       } 
  } 
  if (pr_wifi == false){
               display.drawXbm(1, 0, 16, 16, logo16_wifi_bmp);// -------------------------------------------------- oled wifi ok  -------- 
     if (s == 17){            
               display.drawXbm(24, 0, 16, 16, logo16_supla_bmp);// ------------------------------------------------ oled supla ok --------                                 
        }else{
               display.setFont(ArialMT_Plain_16);
               display.setTextAlignment(TEXT_ALIGN_LEFT);
               display.drawString(18, 0, "E" + String(s)); 
        }
  }
  if (window == false){
               display.drawXbm(48, 0, 16, 16, logo16_Win_close);// -------------------------------------------------- window close  -------- 
          }else{            
               display.drawXbm(48, 0, 16, 16, logo16_Win_open);// -------------------------------------------------- window open --------                                 
  }
  if ((btn[1].mem) == 0){
         display.drawXbm(74, 0, 40, 16, logo_Power_off);// ------------------------------------------------- Relay off  --------   
  }
  if ((btn[1].mem) == 1){
         display.drawXbm(74, 0, 40, 16, logo_Power_on);// -------------------------------------------------- Relay on  --------- 
  }
  display.display();
  yield();  
}
double get_temperature(int channelNumber, double last_val) {
   double t = -275;    

   switch(channelNumber)
          {
            case 3:
         yield(); 
         t = sensors.getTempCByIndex(0);
         if (t < -55){t = -275;}      
                    break;
            case 4:
            double trd = (Term_Temp) ;
                  trd = (trd / 10) ; 
            t = (trd);
                    break;         
          }
    return t; 
}
void itrate_Term() {
    
    if (window == true){
      if ( digitalRead(btn[1].relay_pin-1) == 1){             
          (btn[1].mem) = 0 ;
          Serial.println("Window open");
          SuplaDevice.relayOff(1); 
          }          
          return ; 
    }
    if (tr<-100){
      if ( digitalRead(btn[1].relay_pin-1) == 1){             
          (btn[1].mem) = 0 ;
          Serial.println(" Relay off temp to low");
          SuplaDevice.relayOff(1); 
          }
          return ; 
    }    
     double trt = (Term_Temp + Term_Hist) ;
      trt = (trt / 10);
       Serial.print("tr ");
       Serial.println(tr);
         if (tr >= trt) {         
          if ( digitalRead(btn[1].relay_pin-1) == 1) {                     
          (btn[1].mem) = 0 ;
          Serial.println(" Relay off ");
          SuplaDevice.relayOff(1); 
         } }
     double trs = (Term_Temp - Term_Hist) ;
      trs = (trs / 10) ;
         if(tr <= trs) {
          if ( digitalRead(btn[1].relay_pin-1) == 0) {
         (btn[1].mem) = 1 ;
         Serial.println(" Relay on ");
          SuplaDevice.relayOn(1, 0); 
         } } 
}
void iterate_btn() {
  char v;
  unsigned long now = millis();
  {
  for(int a=0;a<4;a++)
    if (btn[a].pin > 0) {
        v = digitalRead(btn[a].pin-1);
        if (v != btn[a].last_val && now - btn[a].last_time ) {
           btn[a].last_val = v;
           btn[a].last_time = now;
           delay(75);
           v = digitalRead(btn[a].pin-1);
           if (v==0)
           if (dimm == true){
            dimm_milis = millis() + 15000 ;
            display.setContrast(100, 230, 60);
            dimm = false;
            TH_Overlay();
             return;  
           }else{  
              if ((btn[a].channel)== 2){
                  Serial.print("Temp - ");
                  Term_Temp = Term_Temp - 5 ;
                  Serial.println(Term_Temp);      
                  tr_milis = millis() + 5000 ;                  
                  trs = (Term_Temp) ;
                  trs = (trs / 10) ;
                  SuplaDevice.channelDoubleValueChanged(4,trs); 
                  TH_Overlay(); 
                  dimm_milis = millis() + 15000 ;
                   return;
              }
              if ((btn[a].channel)== 3){
                  Serial.print("Temp + ");
                  Term_Temp = Term_Temp + 5 ; 
                  Serial.println(Term_Temp);      
                  tr_milis = millis() + 5000 ;                   
                  trs = (Term_Temp) ;
                  trs = (trs / 10) ;
                  SuplaDevice.channelDoubleValueChanged(4,trs); 
                  TH_Overlay();
                  dimm_milis = millis() + 15000 ; 
                   return;                        
              }
                if ( (btn[a].mem) == 1 ) { 
                  SuplaDevice.relayOff(btn[a].channel);
                  Serial.print("BTN Switsh off relay ");
                  Serial.println(btn[a].relay_pin-1);
                  dimm_milis = millis() + 15000 ;
                  btn[a].mem = 0;
                   return;
                 } else {
                  SuplaDevice.relayOn(btn[a].channel, 0);
                  Serial.print("BTN Switsh on relay ");
                  Serial.println(btn[a].relay_pin-1);
                  dimm_milis = millis() + 15000 ;
                  btn[a].mem = 1;
                   return;
                 }        
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
    if (channelNumber == 5){
      return !window;
    }else{
      return btn[channelNumber].mem;
    }
}
void suplaDigitalWrite(int channelNumber, uint8_t pin, uint8_t val) {       
     btn[channelNumber].mem =val;
     switch(channelNumber)
          { 
             case 0:
         if ( (btn[channelNumber].mem) == 0 ) {                    
                  Serial.print("Manual Mode ");                  
                  digitalWrite(btn[channelNumber].relay_pin-1, LOW);        
                  eep_milis = millis() + 2000 ;
                  Serial.println(" Relay off Manual Mode");
                  SuplaDevice.relayOff(1);                  
                 }else {
                  Serial.print("Auto Mode ");
                  digitalWrite(btn[channelNumber].relay_pin-1, HIGH);        
                  eep_milis = millis() + 2000 ;                   
                 } 
                 TH_Overlay();     
                 break;
             case 1:
        
             if ( (btn[channelNumber].mem) == 0 ) {                    
                  Serial.print("Switsh off relay ");
                  Serial.println(btn[channelNumber].relay_pin-1);
                  digitalWrite(btn[channelNumber].relay_pin-1, LOW);        
                  eep_milis = millis() + 2000 ;                  
                 }else {
                  Serial.print("Switsh on relay ");
                  Serial.println(btn[channelNumber].relay_pin-1);
                  digitalWrite(btn[channelNumber].relay_pin-1, HIGH);      
                  eep_milis = millis() + 2000 ;                   
                 }
                 TH_Overlay();
                 break;
              
              case 2:
         if ( (btn[channelNumber].mem) == 0 ) {                    
                  Serial.print("Temp - ");
                  Term_Temp = Term_Temp - 5 ;
                  Serial.println(Term_Temp);      
                  tr_milis = millis() + 5000 ;                  
                 }else {
                  Serial.print("Temp + ");
                  Term_Temp = Term_Temp + 5 ; 
                  Serial.println(Term_Temp);      
                  tr_milis = millis() + 5000 ;                   
                 } 
                  trs = (Term_Temp) ;
                  trs = (trs / 10) ; 
                  SuplaDevice.channelDoubleValueChanged(4,trs);
                  TH_Overlay();   
                    break;
           }
  return; 
}

void Eeprom_save() {                  //----------EEPROM write  ---------------------- EEprom

        if (start){
          return;
        }
        if ( (btn[0].mem) == 0 ) { 
      for(int e=0;e<2;e++) {  
         if ( btn[e].ms > 0 ) {
                     continue;
         } else {
        eep = (btn[e].mem);                    //  --- read relay state
        if (eep != EEPROM.read(e)){            //  --- compare relay state with memorized state          
         EEPROM.write(e,eep);                  //  --- if different write memory
         Serial.print("EEPROM.");
         Serial.print(e);
         Serial.print(" write.");
         Serial.print((eep));
         Serial.print(" channel ");
         Serial.println((btn[e].channel));        
         EEPROM.commit();         
        }
     }
   }
  } 
          if ( (btn[0].mem) == 1 ) { 
         eep = (btn[0].mem);                    //  --- read relay state
        if (eep != EEPROM.read(0)){            //  --- compare relay state with memorized state          
         EEPROM.write(0,eep);                  //  --- if different write memory
         Serial.print("EEPROM.");
         Serial.print(0);
         Serial.print(" write.");
         Serial.print((eep));
         Serial.print(" channel ");
         Serial.println((btn[0].channel));        
         EEPROM.commit();            
          }
    }
}
void Eepron_read() {                  //----------EEPROM read  ---------------------- EEprom  
             
       
       if ( btn[epr].ms > 0 ) {
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
          (btn[epr].mem) = 1 ;
          SuplaDevice.relayOn(epr, 0);       //  --- only one channel on each pass
          } 
         }
         if (eep == 1) {
         bool stat = digitalRead(btn[1].relay_pin-1);
         if (stat) SuplaDevice.relayOn(1, 0);
         }   
}
void status_func(int status, const char *msg) {     //    ------------------------ Status --------------------------
 s=status;                                          //    -- to check if we are registered and ready before restore from memory
}
void setup() {  //------------------------------------------------ Setup ----------------------------------------------

  wifi_set_sleep_type(NONE_SLEEP_T);

  Serial.begin(115200);
  pinMode(sensor_pin, INPUT_PULLUP);
  delay(200);
  EEPROM.begin(512);
  sensors.begin();
  sensors.getAddress(tempDeviceAddress, 0);
  sensors.setResolution(tempDeviceAddress, resolution);  
  sensors.setWaitForConversion(false);
  sensors.requestTemperatures();

    display.init();
  display.flipScreenVertically();
   display.setFont(ArialMT_Plain_10);
     display.clear();
      display.setFont(ArialMT_Plain_24);
       display.drawString(33, 40, "SUPLA");
         display.drawXbm(0, 16, 32, 32,logo32_glcd_bmp );
          display.display();
           delay(2000);
   
  if (WiFi.SSID()==""){   
    initialConfig = true;
  } 
  
  Serial.println("mounting FS...");
  
  WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.

   uint8_t mac[WL_MAC_ADDR_LENGTH];
   WiFi.macAddress(mac);
   char GUID[SUPLA_GUID_SIZE] = {mac[WL_MAC_ADDR_LENGTH - 6], mac[WL_MAC_ADDR_LENGTH - 5], mac[WL_MAC_ADDR_LENGTH - 4], mac[WL_MAC_ADDR_LENGTH - 3],                                
                                 mac[WL_MAC_ADDR_LENGTH - 2], mac[WL_MAC_ADDR_LENGTH - 1], mac[WL_MAC_ADDR_LENGTH - 1], mac[WL_MAC_ADDR_LENGTH - 2], 
                                 mac[WL_MAC_ADDR_LENGTH - 3], mac[WL_MAC_ADDR_LENGTH - 4], mac[WL_MAC_ADDR_LENGTH - 5], mac[WL_MAC_ADDR_LENGTH - 6]};
          
  SuplaDevice.addRelay(101, false);
  SuplaDevice.addRelay(102, false);
  SuplaDevice.addRelay(103, false);  
  SuplaDevice.addDS18B20Thermometer();
  SuplaDevice.addDS18B20Thermometer();  
  SuplaDevice.addSensorNO(150); //  ﻿Pin number where the sensor is connected 
     
  memset(btn, 0, sizeof(btn));
  btn[0].pin =button_1 +1;          // pin gpio buton  +1
  btn[0].relay_pin =relay_1 +1;  // pin gpio Relay   +1
  btn[0].channel =0;      // channel
  btn[0].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[0].mem =0;
  btn[1].pin =button_2 +1;          // pin gpio buton  +1
  btn[1].relay_pin =relay_2 +1;  // pin gpio Relay   +1
  btn[1].channel =1;      // channel
  btn[1].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[1].mem =0;
  btn[2].pin =button_3 +1;          // pin gpio buton  +1
  btn[2].relay_pin =relay_3 +1;  // pin gpio Relay   +1
  btn[2].channel =2;      // channel
  btn[2].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[2].mem =0;
  btn[3].pin =button_4 +1;          // pin gpio buton  +1
  btn[3].relay_pin =relay_4 +1;  // pin gpio Relay   +1
  btn[3].channel =3;      // channel
  btn[3].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[3].mem =0;
  
  supla_btn_init();

  //read configuration from FS json
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_10);
    display.drawString(64, 0, "mounting FS...");
    display.display();
    
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
          strcpy(up_host, json["up_host"]);
          strcpy(update_username, json["update_username"]);
          strcpy(update_password, json["update_password"]);
          strcpy(term_temp, json["term_temp"]);
          Term_Temp = (String(term_temp).toInt());
          strcpy(term_hist, json["term_hist"]);
          Term_Hist = (String(term_hist).toInt());
        } else {
          Serial.println("failed to load json config");
          initialConfig = true;
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  wifi_station_set_hostname(up_host);  //nazwa w sieci lokalnej  @cino111

  SuplaDevice.setDigitalReadFuncImpl(&supla_DigitalRead);    //            ------Send Value to server -------
  SuplaDevice.setDigitalWriteFuncImpl(&suplaDigitalWrite);   //        -------  Read Value from server   -------
  SuplaDevice.setStatusFuncImpl(&status_func);    //   ----------------------------- Status -----------------------------
  SuplaDevice.setTemperatureCallback(&get_temperature);
  SuplaDevice.setName(Supla_name);

  int LocationID = atoi(Location_id);
  SuplaDevice.begin(GUID,              // Global Unique Identifier 
                    mac,               // Ethernet MAC address
                    Supla_server,      // SUPLA server address
                    LocationID,        // Location ID 
                    Location_Pass);    // Location Password   

  dimm_milis = millis() + 15000;

}
void loop() {

  if (start == true){
    // read_initial_relay_state
    for(int i=0;i<2;i++){      //  ---check relay except der have delay (staircase)
     if ( (btn[i].ms) > 0 ) {
      digitalWrite(btn[i].relay_pin-1, LOW);     
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
         Serial.println((btn[i].channel));
         btn[i].mem =eep;         
                  Serial.print((btn[i].relay_pin) -1);
                  Serial.print(" set ");
                  Serial.println(eep);                    
         }
    }
    start = false;
  }  
  if (initialConfig == true){
    for(int i=0;i<2;i++){      
        EEPROM.write(i, 0); 
     }
      EEPROM.commit();
      Serial.println("initial config triger");
    ondemandwifiCallback () ;
  }
  
  if (shouldSaveConfig == true) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Supla_server"] = Supla_server;
    json["Location_id"] = Location_id;
    json["Location_Pass"] = Location_Pass;
    json["Supla_name"] = Supla_name;
    json["up_host"] = up_host;
    json["update_username"] = update_username;
    json["update_password"] = update_password;
    json["term_temp"] = term_temp;
    json["term_hist"] = term_hist;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    Serial.println("config saved");
          display.clear();
          display.setTextAlignment(TEXT_ALIGN_CENTER);
          display.setFont(ArialMT_Plain_10);
          display.drawString(64, 0, "config saved");
          display.display();      
    shouldSaveConfig = false;
    initialConfig = false; 
    WiFi.mode(WIFI_STA);   
    delay(5000);
    ESP.restart(); 
  }
  
  if (millis() > ds_milis){  //        ------------------------------Termp update ----------------------------- 
    sensors.requestTemperatures();
    window = digitalRead(sensor_pin); 
    ds_milis = millis() + 5000 ;
     }  
   
  if (WiFi.status() != WL_CONNECTED) {    
    if (millis() > wifimilis)  {               
  WiFi.begin();
  pr_wifi = true;
  tikOn = false;
  Serial.println("CONNECTING WIFI"); 
  wifimilis = (millis() + wifi_checkDelay) ;
  }  
}
     
  if (millis() > eep_milis){        
     Eeprom_save() ;   //        ------------------------------Eepron save callback -----------------------------   
     eep_milis = millis() + 5000 ;
     } 
yield(); 
   if (millis() > dimm_milis){ 
    if (dimm == false){
    display.setContrast(50, 50, 30);
    display.display();
    dimm = true ;     
    }    
    dimm_milis = millis() + 15000 ;
     } 

    if (s != 17) if (tikOn == true)tikOn = false;
    
  switch (s) {    //    ------------------------------------------------ Status ------------------------------------
  case 17:      // -----     STATUS_REGISTERED_AND_READY
  if (epr<2){
   Eepron_read() ;                // ------------------------------- Eeprom read callback -------------------------------
   epr = epr+1;// -------- 1 loop for each output  ----------    
    }
    if (tikOn == false)tikOn = true;   
    break; 
    case 10:      // --------------------- REGISTER_IN_PROGRESS  ----------------------
    epr = 0 ;   
    break;
   
  }
  
  int C_W_read = digitalRead(btn[0].pin-1);{  
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
       
   iterate_btn();
   
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
     delay(100);
   }
    httpServer.handleClient(); 
   }

   if (millis() > tr_milis){  //        ------------------------------Termostat callback ----------------------------- 

    yield(); 
    tr = sensors.getTempCByIndex(0);
      trs = (Term_Temp) ;
      trs = (trs / 10) ;   
     TH_Overlay();        
     if ((btn[0].mem) == true) itrate_Term(); 
     if ((String(term_temp).toInt()) != Term_Temp){ 
      display.invertDisplay();// -------------------     oled imvert ---------------------------------------------------     
      Serial.println(term_temp);
      Serial.println(Term_Temp);
      Serial.println("save Temp_Term");
      itoa(Term_Temp ,term_temp ,10) ;
      Serial.println(term_temp);
      Serial.println("saving config");
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.createObject();
      json["Supla_server"] = Supla_server;
      json["Location_id"] = Location_id;
      json["Location_Pass"] = Location_Pass;
      json["Supla_name"] = Supla_name;
      json["up_host"] = up_host;
      json["update_username"] = update_username;
      json["update_password"] = update_password;
      json["term_temp"] = term_temp;
      json["term_hist"] = term_hist;
      File configFile = SPIFFS.open("/config.json", "w");
      if (!configFile) {
      Serial.println("failed to open config file for writing");
      }
      json.prettyPrintTo(Serial);
      json.printTo(configFile);
      configFile.close();
      Serial.println("config saved"); 
      delay(100);
      display.normalDisplay();//----------------------     oled normal ---------------------------------------------------    
     }
     tr_milis = millis() + 5000 ;
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
       //WiFi_up();
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
          //cb.get_temperature = NULL;
          cb.get_temperature_and_humidity = NULL;
          cb.get_rgbw_value = NULL;
          cb.set_rgbw_value = NULL;         
          return cb;
}
