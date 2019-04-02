#include <FS.h>       // ---- esp board manager 2.4.2 --- iwip Variant V2 higher Bandwidth
#include <ESP8266WiFi.h>
#define SUPLADEVICE_CPP
#include "SuplaDevice.h"
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <math.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h> //--------- https://github.com/tzapu/WiFiManager/tree/0.14 -------------
#include <ArduinoJson.h> //--------- https://github.com/bblanchon/ArduinoJson/tree/v5.13.2 ------
#include "uEEPROMLib.h"
#include <DoubleResetDetector.h> 
#include <Ticker.h>      //for LED status
extern "C"
{
#include "user_interface.h"
}
Ticker ticker;
#define status_led 16 //D4 
#define DRD_TIMEOUT 30 // Number of seconds after reset during which a  subseqent reset will be considered a double reset.
#define DRD_ADDRESS 0 // RTC Memory Address for the DoubleResetDetector to use
DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);

byte input=0;
byte btnlast_val=0;
unsigned long btnlast_time;
int s;                   //             ---------------- Status ------------------
unsigned long wifi_checkDelay = 60000;  // wifi reconect delay
unsigned long wifimilis;
#define BEGIN_PIN 100
Adafruit_MCP23017 mcp;
unsigned int Rs = 100000;
double Vcc = 3.3;
// uEEPROMLib eeprom;
uEEPROMLib eeprom(0x50);
int inttmp0 = -1;
bool savers0 = false;
int inttmp1 = -1;
bool savers1 = false;
int inttmp2 = -1;
bool savert0 =false; 
bool savers2 = false;
int inttmp3 = -1;
bool savers3 = false;
WiFiClient client;
char Supla_server[40];
char Location_id[15];
char Location_Pass[20];
byte mac[6];

//flag for saving data
bool shouldSaveConfig = false;
bool initialConfig = false;
int timeout           = 180; // seconds to run for wifi config

void supla_rs_SavePosition(int channelNumber, int position) {
   
          switch(channelNumber)
         {   
   case 0:
    if ((!mcp.digitalRead(0))&&(!mcp.digitalRead(1))){
      inttmp0 = position;
      savers0 = true;
     }
    if (position <= 100 || position >= 10100){
      inttmp0 = position;
      savers0 = true;   
     }
    break;
   case 1:
    if ((!mcp.digitalRead(2))&&(!mcp.digitalRead(3))){
      inttmp1 = position;
      savers1 = true;
     }
    if (position <= 100 || position >= 10100){
      inttmp1 = position;
      savers1 = true;   
     }
   break;
  case 2:
    if ((!mcp.digitalRead(4))&&(!mcp.digitalRead(5))){
      inttmp2 = position;
      savers2 = true;
     }
    if (position <= 100 || position >= 10100){
      inttmp2 = position;
      savers2 = true;   
     } 
   break;
   case 3:
    if ((!mcp.digitalRead(6))&&(!mcp.digitalRead(7))){
      inttmp3 = position;
      savers3 = true;
     }
    if (position <= 100 || position >= 10100){
      inttmp3 = position;
      savers3 = true;   
     }
   break;
   } 
}

void supla_rs_LoadPosition(int channelNumber, int *position) {
  switch(channelNumber)
         {   
   case 0: 
      SuplaDevice.StopTimer();                   
      eeprom.eeprom_read(1, &*position); 
      Serial.print("read position 0: ");       
      Serial.println(*position);
      SuplaDevice.StartTimer();
    break;
    case 1:
    SuplaDevice.StopTimer();       
      eeprom.eeprom_read(11, &*position); 
      Serial.print("read position 1: ");       
      Serial.println(*position);
      SuplaDevice.StartTimer();   
    break;
    case 2:
    SuplaDevice.StopTimer();      
      eeprom.eeprom_read(21, &*position); 
      Serial.print("read position 2: ");       
      Serial.println(*position);
      SuplaDevice.StartTimer();   
    break;
    case 3: 
    SuplaDevice.StopTimer();        
      eeprom.eeprom_read(31, &*position); 
      Serial.print("read position 3: ");       
      Serial.println(*position);
      SuplaDevice.StartTimer();   
    break;
         }    
}

void supla_rs_SaveSettings(int channelNumber, unsigned int full_opening_time, unsigned int full_closing_time) {
  switch(channelNumber)
         {   
   case 0:
     SuplaDevice.StopTimer();
       if (!eeprom.eeprom_write(50, full_opening_time)) {
       Serial.println("Failed full_opening_time 0");
       } else {
       Serial.println("0 ut ok");
     }
     if (!eeprom.eeprom_write(55, full_closing_time)) {
       Serial.println("Failed full_closing_time 0");
       } else {
       Serial.println("0 ct ok");
     }
     SuplaDevice.StartTimer();  
  break;
  case 1:
    SuplaDevice.StopTimer();
    if (!eeprom.eeprom_write(60, full_opening_time)) {
       Serial.println("Failed full_opening_time 1");
       } else {
       Serial.println("1 ut ok");
     }
     if (!eeprom.eeprom_write(65, full_closing_time)) {
       Serial.println("Failed full_closing_time 1");
       } else {
       Serial.println("1 ct ok");
     }
    SuplaDevice.StartTimer();
  break;
  case 2:
    SuplaDevice.StopTimer();
    if (!eeprom.eeprom_write(70, full_opening_time)) {
       Serial.println("Failed full_opening_time 2");
       } else {
       Serial.println("2 ut ok");
     }
     if (!eeprom.eeprom_write(75, full_closing_time)) {
       Serial.println("Failed full_closing_time 2");
       } else {
       Serial.println("2 ct ok");
     }
    SuplaDevice.StartTimer();
  break;
  case 3:
    SuplaDevice.StopTimer();       
    if (!eeprom.eeprom_write(80, full_opening_time)) {
       Serial.println("Failed full_opening_time 0");
       } else {
       Serial.println("3 ut ok");
     }delay(0);       
     if (!eeprom.eeprom_write(85, full_closing_time)) {
       Serial.println("Failed full_closing_time 3");
       } else {
       Serial.println("3 ct ok");
     }
    SuplaDevice.StartTimer();
  break;
  }   
}

void supla_rs_LoadSettings(int channelNumber, unsigned int *full_opening_time, unsigned int *full_closing_time) {
   switch(channelNumber)
         {   
  case 0: 
   SuplaDevice.StopTimer();  
    eeprom.eeprom_read(50, &*full_opening_time);
    eeprom.eeprom_read(55, &*full_closing_time);
    Serial.print(" read.");Serial.print((channelNumber));Serial.print(" opening_time ");Serial.print((*full_opening_time));Serial.print(" closing_time ");Serial.println((*full_closing_time));   
   SuplaDevice.StartTimer();
  break;
  case 1:
   SuplaDevice.StopTimer(); 
    eeprom.eeprom_read(60, &*full_opening_time);
    eeprom.eeprom_read(65, &*full_closing_time);
    Serial.print(" read.");Serial.print((channelNumber));Serial.print(" opening_time ");Serial.print((*full_opening_time));Serial.print(" closing_time ");Serial.println((*full_closing_time));
   SuplaDevice.StartTimer();   
  break;
  case 2:
   SuplaDevice.StopTimer(); 
    eeprom.eeprom_read(70, &*full_opening_time);
    eeprom.eeprom_read(75, &*full_closing_time);
    Serial.print(" read.");Serial.print((channelNumber));Serial.print(" opening_time ");Serial.print((*full_opening_time));Serial.print(" closing_time ");Serial.println((*full_closing_time));
   SuplaDevice.StartTimer();   
  break;
  case 3:
   SuplaDevice.StopTimer(); 
    eeprom.eeprom_read(80, &*full_opening_time);
    eeprom.eeprom_read(85, &*full_closing_time);
    Serial.print(" read.");Serial.print((channelNumber));Serial.print(" opening_time ");Serial.print((*full_opening_time));Serial.print(" closing_time ");Serial.println((*full_closing_time)); 
   SuplaDevice.StartTimer();  
  break;
         }             
}

void tick(){
  //toggle state
  int state = digitalRead(status_led);  // get the current state of D4 pin
  digitalWrite(status_led, !state);     // set pin to the opposite state
}
void saveConfigCallback () {                 //callback notifying us of the need to save config
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void ondemandwifiCallback () {
  ticker.attach(0.2, tick);
  
  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 40);
  WiFiManagerParameter custom_Location_id("ID", "Location_id", Location_id, 15);
  WiFiManagerParameter custom_Location_Pass("Password", "Location_Pass", Location_Pass, 20);

  WiFiManager wifiManager;
  wifiManager.setBreakAfterConfig(true);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  //add all your parameters here
  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_Location_id);
  wifiManager.addParameter(&custom_Location_Pass);

  wifiManager.setCustomHeadElement("<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>");

  wifiManager.setMinimumSignalQuality();

  // set configportal timeout
    wifiManager.setConfigPortalTimeout(timeout);

    if (!wifiManager.startConfigPortal("Supla R4")) {
     Serial.println("Not connected to WiFi but continuing anyway.");
    } else {
      
      Serial.println("connected...yeey :)");    //if you get here you have connected to the WiFi
    }   
    //read updated parameters
    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(Location_id, custom_Location_id.getValue());
    strcpy(Location_Pass, custom_Location_Pass.getValue());
   
    if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Supla_server"] = Supla_server;
    json["Location_id"] = Location_id;
    json["Location_Pass"] = Location_Pass;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    Serial.println("config saved");
    shouldSaveConfig = false;
    WiFi.softAPdisconnect(true);   //  close AP
    WiFi.mode(WIFI_STA);
    ticker.detach();
    digitalWrite(status_led, HIGH);
    ESP.restart();
    delay(5000); 
  }    
  WiFi.softAPdisconnect(true);   //  close AP
}
double get_temperature(int channelNumber, double last_val) {  // 10k Ntc Adc to Gnd and 100k Resistor Adc to +3,3v
    
      int val = 0;
      for(int i = 0; i < 10; i++) {
      val += analogRead(A0);
      delay(1);
      }
      val = val / 10;
      double V_NTC = (double)val / 1023;
      double R_NTC = (Rs * V_NTC) / (Vcc - V_NTC);
      R_NTC = log(R_NTC);
      double t = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * R_NTC * R_NTC ))* R_NTC );
      t = t - 273.15;
         
      return t;  
}
void supla_timer() {

  unsigned long now = millis();  {
   
        input= mcp.readGPIO(1);
        if (input != btnlast_val && now - btnlast_time ) {
           btnlast_val = input;
           btnlast_time = now+100;
  
    if (input < 255) {
      Serial.print("Mcp B input ");  Serial.println(input);
      delay(200);
      input= mcp.readGPIO(1);
      if (input < 255) {
      
      if ( input == 254 ) {      
        if ( SuplaDevice.rollerShutterMotorIsOn(0) ) {
          Serial.print("0 Stop");
            SuplaDevice.rollerShutterStop(0);
        } else {
          Serial.print("0 Reveal");
            SuplaDevice.rollerShutterReveal(0);
        }        
     } 
      if ( input == 253 ) {
        if ( SuplaDevice.rollerShutterMotorIsOn(0) ) {
          Serial.print("0 Stop");
            SuplaDevice.rollerShutterStop(0);
        } else {
          Serial.print("0 Shut");
            SuplaDevice.rollerShutterShut(0);
        }        
     }
     if ( input == 251 ) {      
        if ( SuplaDevice.rollerShutterMotorIsOn(1) ) {
          Serial.print("1 Stop");
            SuplaDevice.rollerShutterStop(1);
        } else {
          Serial.print("1 Reveal");
            SuplaDevice.rollerShutterReveal(1);
        }        
     } 
      if ( input == 247 ) {
        if ( SuplaDevice.rollerShutterMotorIsOn(1) ) {
          Serial.print("1 Stop");
            SuplaDevice.rollerShutterStop(1);
        } else {
          Serial.print("1 Shut");
            SuplaDevice.rollerShutterShut(1);
        }        
     }
     if ( input == 239 ) {      
        if ( SuplaDevice.rollerShutterMotorIsOn(2) ) {
          Serial.print("2 Stop");
            SuplaDevice.rollerShutterStop(2);
        } else {
          Serial.print("2 Reveal");
            SuplaDevice.rollerShutterReveal(2);
        }        
     } 
      if ( input == 223 ) {
        if ( SuplaDevice.rollerShutterMotorIsOn(2) ) {
          Serial.print("2 Stop");
            SuplaDevice.rollerShutterStop(2);
        } else {
          Serial.print("2 Shut");
            SuplaDevice.rollerShutterShut(2);
        }        
     }
     if ( input == 191 ) {      
        if ( SuplaDevice.rollerShutterMotorIsOn(3) ) {
          Serial.print("3 Stop");
            SuplaDevice.rollerShutterStop(3);
        } else {
          Serial.print("3 Reveal");
            SuplaDevice.rollerShutterReveal(3);
        }        
     } 
      if ( input == 127 ) {
        if ( SuplaDevice.rollerShutterMotorIsOn(3) ) {
          Serial.print("3 Stop");
            SuplaDevice.rollerShutterStop(3);
        } else {
          Serial.print("3 Shut");
            SuplaDevice.rollerShutterShut(3);
        }
      }        
     }       
    }
   }
  }    
}
int supla_DigitalRead(int channelNumber, uint8_t pin) {
 switch(channelNumber)
         {  
  case 0:
    if (pin == 101) {
    return mcp.digitalRead(0);
    }
    if (pin == 102) {
    return mcp.digitalRead(1);
    }
    break;
  case 1:
    if (pin == 103) {
    return mcp.digitalRead(2);
    }
    if (pin == 104) {
    return mcp.digitalRead(3);
    }
    break;
  case 2:
    if (pin == 105) {
    return mcp.digitalRead(4);
    }
    if (pin == 106) {
    return mcp.digitalRead(5);
    }
    break;
  case 3:
    if (pin == 107) {
    return mcp.digitalRead(6);
    }
    if (pin == 108) {
    return mcp.digitalRead(7);
    }
    break;
 case 4: 
    return digitalRead(3);
     break;    
  case 5:
    return digitalRead(14);
     break;    
  case 6:
    return digitalRead(12);
     break;    
  case 7:
    return digitalRead(13);
     break; 
         }  
}
void suplaDigitalWrite(int channelNumber, uint8_t pin, uint8_t val) {  //------------------------------------------------  Virtual ----------------------------
  
  Serial.print("Pin:"); Serial.println(pin);
  Serial.print("Value:"); Serial.println(val);

    if (channelNumber == 0) {
     if (pin == 101){
       mcp.digitalWrite(0, val);
       return;
     }
     if (pin == 102){
       mcp.digitalWrite(1, val);
       return;
     }}
    if (channelNumber == 1) {
     if (pin == 103){
       mcp.digitalWrite(2, val);
       return;
     }
     if (pin == 104){
       mcp.digitalWrite(3, val);
       return;
     }}
    if (channelNumber == 2) {
     if (pin == 105){
       mcp.digitalWrite(4, val);
       return;
     }
     if (pin == 106){
       mcp.digitalWrite(5, val);
       return;
     }}
    if (channelNumber == 3) {
     if (pin == 107){
       mcp.digitalWrite(6, val);
       return;
     }
     if (pin == 108){
       mcp.digitalWrite(7, val);
       return;
     }}
     
  return;
        
}
/*void sensorRead(){
  SuplaDevice.channelValueChanged(sensor_restore_channel, val == HIGH ? 1 : 0);
}*/

void status_func(int status, const char *msg) {     //    ------------------------ Status --------------------------
 s=status;                                          //    -- to check if we are registered and ready before restore from memory
}
void setup() {  //------------------------------------------------ Setup ----------------------------------------------

  wifi_set_sleep_type(NONE_SLEEP_T);

  Serial.begin(115200);
  SPIFFS.begin();
  pinMode(status_led,OUTPUT); 
  ticker.attach(0.8, tick);
  Wire.begin();
  Wire.setClock(100000L);
  mcp.begin();      // use default address 0
 
  mcp.pinMode(0, OUTPUT);
  mcp.pinMode(1, OUTPUT);
  mcp.pinMode(2, OUTPUT);
  mcp.pinMode(3, OUTPUT);
  mcp.pinMode(4, OUTPUT);
  mcp.pinMode(5, OUTPUT);
  mcp.pinMode(6, OUTPUT);
  mcp.pinMode(7, OUTPUT);
  mcp.pinMode(8, INPUT);
  mcp.pullUp(8, HIGH);  // turn on a 100K pullup internally
  mcp.pinMode(9, INPUT);
  mcp.pullUp(9, HIGH);  // turn on a 100K pullup internally
  mcp.pinMode(10, INPUT);
  mcp.pullUp(10, HIGH);  // turn on a 100K pullup internally
  mcp.pinMode(11, INPUT);
  mcp.pullUp(11, HIGH);  // turn on a 100K pullup internally
  mcp.pinMode(12, INPUT);
  mcp.pullUp(12, HIGH);  // turn on a 100K pullup internally
  mcp.pinMode(13, INPUT);
  mcp.pullUp(13, HIGH);  // turn on a 100K pullup internally
  mcp.pinMode(14, INPUT);
  mcp.pullUp(14, HIGH);  // turn on a 100K pullup internally
  mcp.pinMode(15, INPUT);
  mcp.pullUp(15, HIGH);  // turn on a 100K pullup internally

  if (WiFi.SSID()==""){
    //Serial.println("We haven't got any access point credentials, so get them now");   
    initialConfig = true;
  } 
  //read configuration from FS json
    Serial.println("mounting Fs...");
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
        //json.printTo(Serial);   //print config data to serial on startup
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(Supla_server, json["Supla_server"]);
          strcpy(Location_id, json["Location_id"]);
          strcpy(Location_Pass, json["Location_Pass"]);

        } else {
          Serial.println("failed to load json config");
          
        }
      }
    }
    
  if (drd.detectDoubleReset()) {
    Serial.println("Double Reset Detected");
    ondemandwifiCallback ();
  } else {
    Serial.println("No Double Reset Detected");
  }
  
  WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.

   uint8_t mac[WL_MAC_ADDR_LENGTH];
   WiFi.macAddress(mac);
   char GUID[SUPLA_GUID_SIZE] = {mac[WL_MAC_ADDR_LENGTH - 6], mac[WL_MAC_ADDR_LENGTH - 5], mac[WL_MAC_ADDR_LENGTH - 4], mac[WL_MAC_ADDR_LENGTH - 3],                                
                                 mac[WL_MAC_ADDR_LENGTH - 2], mac[WL_MAC_ADDR_LENGTH - 1], mac[WL_MAC_ADDR_LENGTH - 1], mac[WL_MAC_ADDR_LENGTH - 2], 
                                 mac[WL_MAC_ADDR_LENGTH - 3], mac[WL_MAC_ADDR_LENGTH - 4], mac[WL_MAC_ADDR_LENGTH - 5], mac[WL_MAC_ADDR_LENGTH - 6]};
          
     // CHANNEL0 - TWO RELAYS (Roller shutter operation)
  SuplaDevice.addRollerShutterRelays(101,     // Pin number where the 1st relay is connected
                                     102, false);    // Pin number where the 2nd relay is connected

   // CHANNEL1 - TWO RELAYS (Roller shutter operation)
  SuplaDevice.addRollerShutterRelays(103,     // Pin number where the 1st relay is connected
                                     104, false);    // Pin number where the 2nd relay is connected

      // CHANNEL2 - TWO RELAYS (Roller shutter operation)
  SuplaDevice.addRollerShutterRelays(105,     // Pin number where the 1st relay is connected
                                     106, false);    // Pin number where the 2nd relay is connected

      // CHANNEL3 - TWO RELAYS (Roller shutter operation)
  SuplaDevice.addRollerShutterRelays(107,     // Pin number where the 1st relay is connected
                                     108, false);    // Pin number where the 2nd relay is connected
  
  // CHANNEL4 - Opening sensor (Normal Open)
  SuplaDevice.addSensorNO(3); //  ﻿Pin number where the sensor is connected

  // CHANNEL5 - Opening sensor (Normal Open)
  SuplaDevice.addSensorNO(14); //  ﻿Pin number where the sensor is connected

  // CHANNEL6 - Opening sensor (Normal Open)
  SuplaDevice.addSensorNO(12); //  ﻿Pin number where the sensor is connected

  // CHANNEL7 - Opening sensor (Normal Open)
  SuplaDevice.addSensorNO(13); //  ﻿Pin number where the sensor is connected

  // CHANNEL8 - Ntc temperature sensor 
  SuplaDevice.addDS18B20Thermometer();

  
  SuplaDevice.setDigitalReadFuncImpl(&supla_DigitalRead);    //            ------Send Value to server -------
  SuplaDevice.setDigitalWriteFuncImpl(&suplaDigitalWrite);   //        -------  Read Value from server   -------
  SuplaDevice.setTimerFuncImpl(&supla_timer);
  SuplaDevice.setStatusFuncImpl(&status_func);    //   ----------------------------- Status -----------------------------
  SuplaDevice.setRollerShutterFuncImpl(&supla_rs_SavePosition, &supla_rs_LoadPosition, &supla_rs_SaveSettings, &supla_rs_LoadSettings);
  SuplaDevice.setName("Supla R4");

  int LocationID = atoi(Location_id);
  SuplaDevice.begin(GUID,              // Global Unique Identifier 
                    mac,               // Ethernet MAC address
                    Supla_server,      // SUPLA server address
                    LocationID,        // Location ID 
                    Location_Pass);    // Location Password
        
}

void loop() {

  drd.loop();

  if (initialConfig){
    ondemandwifiCallback () ;
    initialConfig = false; 
  } 

      if (savers0){
       SuplaDevice.StopTimer();
      Serial.print("write position 0:");       
       Serial.println(inttmp0);       
       eeprom.eeprom_write(1, inttmp0); 
       Serial.print("read position 0:");
       eeprom.eeprom_read(1, &inttmp0);
       Serial.println(inttmp0);      
       savers0 = false;
       SuplaDevice.StartTimer();
      }
      if (savers1){
        SuplaDevice.StopTimer();
      Serial.print("write position 1:");       
       Serial.println(inttmp1);              
       eeprom.eeprom_write(11, inttmp1); 
       Serial.print("read position 1:");
       eeprom.eeprom_read(11, &inttmp1);
       Serial.println(inttmp1);      
       savers1 = false;
       SuplaDevice.StartTimer();
      }
      if (savers2){
        SuplaDevice.StopTimer();
      Serial.print("write position 2:");       
       Serial.println(inttmp2);        
       eeprom.eeprom_write(21, inttmp2); 
       Serial.print("read position 2:");
       eeprom.eeprom_read(21, &inttmp2);
       Serial.println(inttmp2);      
       savers2 = false;
       SuplaDevice.StartTimer();
      }
      if (savers3){
        SuplaDevice.StopTimer();
      Serial.print("write position 3:");       
       Serial.println(inttmp3);   
       delay(0);           
       eeprom.eeprom_write(31, inttmp3); 
       Serial.print("read position 3:");
       eeprom.eeprom_read(31, &inttmp3);
       Serial.println(inttmp3);      
       savers3 = false;
       SuplaDevice.StartTimer();
      }      
       
  if (WiFi.status() != WL_CONNECTED) { 
    ticker.attach(0.8, tick);
    WiFi_up();
    } 
  
  SuplaDevice.iterate();
  //SuplaDevice.setTemperatureCallback(&get_temperature);
 
  switch (s) {    //    ------------------------------------------------ Status ------------------------------------
    case 17:      // -----     STATUS_REGISTERED_AND_READY
      ticker.detach();
    break;

    case 10:      // --------------------- REGISTER_IN_PROGRESS  ----------------------
   
    break;
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
          cb.get_temperature = &get_temperature;
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
  }
  wifimilis = (millis() + wifi_checkDelay) ;
}

