#include <FS.h>       // ---- esp board manager 2.4.2 --- iwip Variant V2 higher Bandwidth
#include <OneWire.h>
#include <DallasTemperature.h> 
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#define CUSTOMSUPLA_CPP
#include <CustomSupla.h>
#include <DNSServer.h>
#include <WiFiManager.h> 
#include <ArduinoJson.h>     //--------- https://github.com/bblanchon/ArduinoJson/tree/v5.13.2 ------
#include <Ticker.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h> 
#include <Button3.h>
extern "C"
{
#include "user_interface.h"
}

//#define D0 16  //no internal pullup resistor  Überprüfe die serielle schnittstelle "PuTTY"
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

#define button_PIN 12 
#define button2_PIN 14 // D5
#define TRIGGER_PIN 12  // D6    wifi configuration pin --------pin konfiguracji wifi
#define relay1 13      // D7
#define relay2 101     // 
#define relay3 15      // D8
#define sensor1 5      // D1
#define sensor2 4      // D2
#define ONE_WIRE_BUS 3 // RX
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress Thermometer_1, Thermometer_2;

int resolution = 12;
WiFiClient client;
ESP8266WebServer httpServer(81);
ESP8266HTTPUpdateServer httpUpdater;
Button3 button = Button3(button_PIN);
Button3 button2 = Button3(button2_PIN);
const char* host = "supla12bar";
Ticker ticker;
Ticker btn_loop;
unsigned long wifi_checkDelay = 30000;
unsigned long wifimilis; 
int C_W_state = HIGH; 
int last_C_W_state = HIGH;
unsigned long time_last_C_W_change = 0; 
long C_W_delay = 5000;               // config delay 5 seconds           ----------        opóźnienie konfiguracji 5 sekund
int MPX_mtbs = 5000;                // mean time between MPX update   
int mpx_zeroCal ;
unsigned long MPX_lasttime;   
double MPX_Value = 0;
double MPV_Send = -1;
double distance = 0;
double rssi = 0;
double rssi_last = 0;
double temp_1;
double temp_2;
int MPX_zero_offset = 0;
int mpx_Compensation =0;
int s;                   //             ---------------- Status ------------------
int timeout           = 120;
char Supla_server[40];
char Location_id[15];
char Location_Pass[34];
char MPX_Compensation[5];
char Supla_name[51];
char update_path[21] = "/update";
char update_username[21];
char update_password[21];
byte mac[6];
bool pr_wifi = true;
bool pr_supla = true;
bool shouldSaveConfig = false;
bool initialConfig = false;
bool manual = false;
bool dimm = false;         
unsigned long dimm_milis ;

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
const uint8_t logo_Power_off[] PROGMEM =  //Power off
{
  0xe0, 0xff, 0xff, 0xff, 0x03, 0x38, 0xfc, 0xff, 0xff, 0x0f, 0x0c, 0xf0,
   0xff, 0xff, 0x3f, 0x06, 0xe0, 0xff, 0xff, 0x3f, 0x02, 0xc0, 0xcf, 0x43,
   0x78, 0x03, 0xc0, 0x33, 0x43, 0x78, 0x01, 0xc0, 0x7b, 0x7b, 0xff, 0x01,
   0x80, 0xfd, 0x42, 0xf8, 0x01, 0x80, 0xfd, 0x42, 0xf8, 0x01, 0x80, 0x7b,
   0x7b, 0xff, 0x03, 0xc0, 0x33, 0x7b, 0xff, 0x02, 0xc0, 0xcf, 0x7b, 0x7f,
   0x06, 0xe0, 0xff, 0xff, 0x7f, 0x0c, 0xf0, 0xff, 0xff, 0x3f, 0x18, 0xfc,
   0xff, 0xff, 0x1f, 0xf0, 0xff, 0xff, 0xff, 0x07, 
};
const uint8_t logo_Power_on[] PROGMEM =  //Power on
{
   0xe0, 0xff, 0xff, 0xff, 0x07, 0x38, 0x00, 0x00, 0x30, 0x0c, 0x0c, 0x00,
   0x00, 0x18, 0x18, 0x06, 0x00, 0x00, 0x0c, 0x30, 0x02, 0x0e, 0x00, 0x06,
   0x60, 0x03, 0x9f, 0x19, 0x02, 0x40, 0x81, 0xb1, 0x1b, 0x03, 0xc0, 0x81,
   0xa0, 0x1a, 0x01, 0x80, 0x81, 0xa0, 0x1e, 0x01, 0x80, 0x81, 0xb1, 0x1c,
   0x01, 0xc0, 0x03, 0x9b, 0x18, 0x03, 0x40, 0x02, 0x8e, 0x18, 0x02, 0x60,
   0x06, 0x00, 0x00, 0x06, 0x20, 0x0c, 0x00, 0x00, 0x0c, 0x38, 0x38, 0x00,
   0x00, 0x18, 0x0c, 0xe0, 0xff, 0xff, 0xff, 0x07,
};

void get_MPX(){
  int val = 0;
      for(int i = 0; i < 10; i++) {
      val += analogRead(A0);
      delay(1);
      }
      val = val / 10; 
  yield();
  MPX_Value = map(val,MPX_zero_offset,(mpx_Compensation+MPX_zero_offset),0,1000) / 83.3;
  yield(); 
  Serial.print("bar: ");  
  Serial.println(MPX_Value,2);
  yield();
  if (MPX_Value != MPV_Send){ 
    MPV_Send = MPX_Value;
   // CustomSupla.channelDoubleValueChanged(0, MPV_Send);       
  }
   TH_Overlay();
}

void Btn_loop(){
  button.loop();  
  button2.loop();
  CustomSupla.iterateOfline();  
}
void saveConfigCallback () {
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
          display.drawString(64, 44, "Supla_12Bar"); 
          display.display();   
  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 40);
  WiFiManagerParameter custom_Location_id("ID", "Location_id", Location_id, 15);
  WiFiManagerParameter custom_Location_Pass("Password", "Location_Pass", Location_Pass, 34 );
  WiFiManagerParameter custom_MPX_Compensation("Compensation", "Compensation 500-1000", MPX_Compensation, 5,"required");
  WiFiManagerParameter custom_Supla_name("name", "Supla Device Name", Supla_name, 51,"required");
  WiFiManagerParameter custom_update_username("updateUsername", "update username", update_username, 21,"required");
  WiFiManagerParameter custom_update_password("updatePassword", "update password", update_password, 21,"required");

  WiFiManager wifiManager;
  wifiManager.setBreakAfterConfig(true);
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_Location_id);
  wifiManager.addParameter(&custom_Location_Pass);
  wifiManager.addParameter(&custom_MPX_Compensation);
  wifiManager.addParameter(&custom_Supla_name);
  wifiManager.addParameter(&custom_update_username);
  wifiManager.addParameter(&custom_update_password);

  wifiManager.setCustomHeadElement("<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>");
  wifiManager.setMinimumSignalQuality();

    wifiManager.setConfigPortalTimeout(timeout);

    if (!wifiManager.startConfigPortal("Supla_12Bar")) {   //    ----wifi configuration ap name---
      Serial.println("Not connected to WiFi but continuing anyway.");
    } else {     
      Serial.println("connected...yeey :)");    //if you get here you have connected to the WiFi
    }
    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(Location_id, custom_Location_id.getValue());
    strcpy(Location_Pass, custom_Location_Pass.getValue());
    strcpy(MPX_Compensation, custom_MPX_Compensation.getValue());
    strcpy(Supla_name, custom_Supla_name.getValue());
    strcpy(update_username, custom_update_username.getValue());
    strcpy(update_password, custom_update_password.getValue());
}
void TH_Overlay() {

  display.clear();
   if (MPX_Value > -1){
         display.setFont(ArialMT_Plain_24);
         display.setTextAlignment(TEXT_ALIGN_CENTER);
         display.drawString(64, 20, String(MPX_Value, 2) + "bar"); 
   } 
   if (MPX_Value < -1){
         display.setFont(ArialMT_Plain_24);
         display.setTextAlignment(TEXT_ALIGN_CENTER);
         display.drawString(64, 20, "ERROR"); 
   } 

         display.setFont(ArialMT_Plain_16);
         display.setTextAlignment(TEXT_ALIGN_CENTER);
         display.drawString(64, 48,"A" + String(temp_1, 1) + "º B" + " " + String(temp_2, 1) + "º"); 

  if (pr_wifi == false){
               display.drawXbm(1, 0, 16, 16, logo16_wifi_bmp);// -------------------------------------------------- oled wifi ok  -------- 
     if (s == 17){            
               display.drawXbm(18, 0, 16, 16, logo16_supla_bmp);// ------------------------------------------------ oled supla ok --------                                 
        }else{
               display.setFont(ArialMT_Plain_16);
               display.setTextAlignment(TEXT_ALIGN_LEFT);
               display.drawString(18, 0, "E" + String(s)); 
        }
  }
 if (!digitalRead(relay1)){
         display.drawXbm(42, 0, 40, 16, logo_Power_off);// ------------------------------------------------- Relay off  --------   
  } else {
         display.drawXbm(42, 0, 40, 16, logo_Power_on);// -------------------------------------------------- Relay on  --------- 
  }
 if (!digitalRead(relay3)){
         display.drawXbm(86, 0, 40, 16, logo_Power_off);// ------------------------------------------------- Relay off  --------   
  } else {
         display.drawXbm(86, 0, 40, 16, logo_Power_on);// -------------------------------------------------- Relay on  --------- 
  }

  display.display();
  yield();  
}
double get_temperature(int channelNumber, double last_val) {
   double t = -275;    

   switch(channelNumber)
          {
            case 0:
         return MPV_Send;      
                    break;
            case 1:
         //t = sensors.getTempC(Thermometer_1);
         t = sensors.getTempCByIndex(0);
         temp_1 = t;
         Serial.print("DS18B20 0 Temp: ");
         Serial.println(t);
                    break;
            case 2:        
         //t = sensors.getTempC(Thermometer_2);
         t = sensors.getTempCByIndex(1);
         temp_2 = t;
         Serial.print("DS18B20 1 Temp: ");
         Serial.println(t);
      sensors.requestTemperatures();
      
                    break;
          }
    return t; 
}
int supla_DigitalRead(int channelNumber, uint8_t pin) {
   if (channelNumber == 4){
    manual = digitalRead(relay1);
    return manual;
   }else{
    return digitalRead(pin); 
   }
}
void suplaDigitalWrite(int channelNumber, uint8_t pin, uint8_t val) {       

      if ( channelNumber == 4 ) {                    
          digitalWrite(relay1, val);        
          manual = val;
          TH_Overlay(); 
           return;   
      }          
      if ( channelNumber == 3 ) { 
         if (manual == false){
            digitalWrite(relay1, val);
            
             return; 
         }
      }else {
          digitalWrite(pin, val);
          TH_Overlay();         
           return;        
      }
  return; 
}    
void status_func(int status, const char *msg) { 
  if (s != status){
     s=status; 
     Serial.print("status: "); 
     Serial.println(s);   
  }                                        
}
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
  Serial.println("");
}
void setup() {    //    ------------------------ Setup --------------------------

  wifi_set_sleep_type(NONE_SLEEP_T);

  //Serial.begin(115200);
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY, 1); 
    delay(1000);
  EEPROM.begin(128);  
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
    
  if (WiFi.SSID()==""){
    initialConfig = true;
  }
    display.init();
  display.flipScreenVertically();
   display.setFont(ArialMT_Plain_10);
     display.clear();
      display.setFont(ArialMT_Plain_24);
       display.drawString(33, 40, "SUPLA");
         display.drawXbm(0, 16, 32, 32,logo32_glcd_bmp );
          display.display();
           delay(2000);

  button.setQuintleClickHandler(quintleClick);
  button2.setClickHandler(click2);
  button2.setLongClickHandler(longClick2); 

  
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
        Serial.println(jsonBuffer.size());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          if (json.containsKey("Supla_server")) strcpy(Supla_server, json["Supla_server"]);
          if (json.containsKey("Location_id")) strcpy(Location_id, json["Location_id"]);
          if (json.containsKey("Location_Pass")) strcpy(Location_Pass, json["Location_Pass"]);
          if (json.containsKey("MPX_Compensation")) strcpy(MPX_Compensation, json["MPX_Compensation"]);
          if (json.containsKey("Supla_name")) strcpy(Supla_name, json["Supla_name"]);         
          if (json.containsKey("update_username")) strcpy(update_username, json["update_username"]);
          if (json.containsKey("update_password")) strcpy(update_password, json["update_password"]);
          mpx_Compensation = String(MPX_Compensation).toInt();
          Serial.print("Compensation: ");
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
  Serial.print("Read zero offset: ");
  Serial.println(MPX_zero_offset);

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
  sensors.setResolution(Thermometer_1, resolution); 
  sensors.setResolution(Thermometer_2, resolution);   
  sensors.setWaitForConversion(false);
  sensors.requestTemperatures(); 

  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.macAddress(mac);
  char GUID[SUPLA_GUID_SIZE] = {mac[WL_MAC_ADDR_LENGTH - 6], mac[WL_MAC_ADDR_LENGTH - 5], mac[WL_MAC_ADDR_LENGTH - 4], mac[WL_MAC_ADDR_LENGTH - 3],                                
                                mac[WL_MAC_ADDR_LENGTH - 2], mac[WL_MAC_ADDR_LENGTH - 1], mac[WL_MAC_ADDR_LENGTH - 1], mac[WL_MAC_ADDR_LENGTH - 2], 
                                mac[WL_MAC_ADDR_LENGTH - 3], mac[WL_MAC_ADDR_LENGTH - 4], mac[WL_MAC_ADDR_LENGTH - 5], mac[WL_MAC_ADDR_LENGTH - 6]};
  
  CustomSupla.addDS18B20Thermometer();
  CustomSupla.addDS18B20Thermometer();
  CustomSupla.addDS18B20Thermometer();
    CustomSupla.addRelay(relay1, false); 
    CustomSupla.addRelay(relay2, false); 
    CustomSupla.addRelay(relay3, false);
    CustomSupla.addSensorNO(sensor1, true);
    CustomSupla.addSensorNO(sensor2, true);   
  
  CustomSupla.setStatusFuncImpl(&status_func);    //   ----------------------------- Status -----------------------------
  CustomSupla.setDigitalReadFuncImpl(&supla_DigitalRead);    //            ------Send Value to server -------
  CustomSupla.setDigitalWriteFuncImpl(&suplaDigitalWrite);   //        -------  Read Value from server   ------- 
  CustomSupla.setName(Supla_name);    // Supla device name
  wifi_station_set_hostname(Supla_name);

  int LocationID = atoi(Location_id);
  CustomSupla.begin(GUID,              // Global Unique Identifier 
                    mac,               // Ethernet MAC address
                    Supla_server,      // SUPLA server address
                    LocationID,        // Location ID 
                    Location_Pass);    // Location Password
                    
 WiFi.mode(WIFI_STA);
 btn_loop.attach_ms(40, Btn_loop);
 
 dimm_milis = millis() + 15000;
 
}

void loop() {
  
 if (WiFi.status() == WL_CONNECTED) 
  {    
    httpServer.handleClient();
 }
        
 if (s == 17){
    button.loop();  
    button2.loop();
       if (pr_supla == true){
          btn_loop.detach();
          pr_supla = false;
          Serial.println("Supla connected");        
         }        
      }else{       
         delay(25);
          if (pr_supla == false){      
          pr_supla = true;
          Serial.println("Not connected");
          btn_loop.attach_ms(40, Btn_loop);
       }
 } 
  if (millis() > MPX_lasttime + MPX_mtbs)  {    //--------------MPX callback--------------------
       get_MPX();
       MPX_lasttime = millis();
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
          display.clear();
          display.setTextAlignment(TEXT_ALIGN_CENTER);
          display.setFont(ArialMT_Plain_10);
          display.drawString(64, 0, "config saved");
          display.display();     
    Serial.println("config saved");
    shouldSaveConfig = false;
    WiFi.mode(WIFI_STA);
    ESP.restart();
      delay(5000);
 }
  
 if ((WiFi.status() != WL_CONNECTED) && ( pr_wifi == false)) { 
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
    MDNS.begin(host);
    httpUpdater.setup(&httpServer,update_path, update_username, update_password);
    httpServer.begin();
    MDNS.addService("http", "tcp", 81);
    Serial.printf("HTTPUpdateServer ready! Open http://%s.local:81%s in your browser and login with username '%s' and password '%s'\n", host, update_path, update_username, update_password);      
 }
 
  CustomSupla.iterate();
  delay(25);

  if (millis() > dimm_milis){ 
    if (dimm == false){
    display.setContrast(50, 50, 30);
    display.display();
    dimm = true ;     
    }    
    dimm_milis = millis() + 15000 ;
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
          cb.get_temperature = &get_temperature;  
          cb.get_temperature_and_humidity = NULL;
          return cb;
}
void click(Button3& btn) {
            dimm_milis = millis() + 15000 ;
            display.setContrast(100, 230, 60);
            dimm = false;
            TH_Overlay();  
}
void quintleClick(Button3& btn) {
  
    int val = 0;
      for(int i = 0; i < 10; i++) {
       val += analogRead(A0);
       delay(1);
      }
      val = val / 10;
      
      MPX_zero_offset = val;
         display.clear();
         display.setContrast(100, 241, 64);       
         display.setFont(ArialMT_Plain_24);
         display.setTextAlignment(TEXT_ALIGN_CENTER);
         display.drawString(64, 20, "MPX Call.."); 
         display.display();
         dimm = false;     
         Serial.print("Write zero offset: ");
         Serial.println(MPX_zero_offset);
         EEPROM.write(1, MPX_zero_offset); 
         EEPROM.commit();
           delay (3000);
            TH_Overlay();
          
}
void longClick2(Button3& btn) {
  
               if (manual == false){ 
                 CustomSupla.relayOn(3, 1000);
                 Serial.println("BTN Switsh on 1 second");
               } 
}
void click2(Button3& btn) {
  
           if (dimm == true){
            dimm_milis = millis() + 15000 ;
            display.setContrast(100, 230, 60);
            dimm = false;
            TH_Overlay(); 
           }
               if (digitalRead(relay1) == 1 ) {  
                  CustomSupla.relayOff(4);
                  Serial.println("BTN Switsh off relay");
               } else {
                  CustomSupla.relayOn(4, 0);
                  Serial.println("BTN Switsh on relay");
               }
}
void WiFi_up(){ // conect to wifi
 
  if (millis() > wifimilis)  {
  WiFi.begin();
  Serial.println("CONNECTING WIFI");
  wifimilis = (millis() + wifi_checkDelay) ; 
  }  
}
