#include <FS.h>       // ---- esp board manager 2.4.2 --- iwip Variant V2 higher Bandwidth
#include <ESP8266WiFi.h>
#define CUSTOMSUPLA_CPP
#include <CustomSupla.h>  //  ------ V 1.6.2C -------
#include <DNSServer.h>
#include <WiFiManager.h> 
#include <ArduinoJson.h> //--------- https://github.com/bblanchon/ArduinoJson/tree/v5.13.2 ------
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>  
#include <Ticker.h>      //for LED status
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

/*
GPIO #  Component
GPIO00  Button1
GPIO01  User
GPIO02  User
GPIO03  User
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
#define led_on HIGH
#define led_off LOW
#define ONE_WIRE_BUS 3
#define status_led 13   //      ----------------------- status Led -----------------------
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
int button_5 = 9; //9
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress Thermometer_1,Thermometer_2,Thermometer_3,Thermometer_4;
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
long C_W_delay = 10000;               // config delay 10 seconds           ----------        opóźnienie konfiguracji 10 sekund
bool tikOn = false;
bool booton_action = false;
WiFiClient client;
ESP8266WebServer httpServer(81);
ESP8266HTTPUpdateServer httpUpdater;
const char* update_path = "/update";
char Supla_server[80];
char Location_id[15];
char Location_Pass[34];
char Supla_name[51];
char up_host[21];
char update_username[21];
char update_password[21];
char Supla_status[51];
char temp1[6];
char temp2[6];
char temp3[6];
float Temp1;
float Temp2;
float Temp3;
byte mac[6];
bool shouldSaveConfig = false;
bool initialConfig = false;
int timeout = 180;          // seconds to run the wifi config

void tick(){
  //toggle Led state
  int state = digitalRead(status_led);  
  digitalWrite(status_led, !state);  
}

void saveConfigCallback () {                 //callback notifying us of the need to save config
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void ondemandwifiCallback () {
 ticker.attach(1.0, tick);
  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 80);
  WiFiManagerParameter custom_Location_id("ID", "Location id", Location_id, 15);
  WiFiManagerParameter custom_Location_Pass("Password", "Location Pass", Location_Pass, 34);
  WiFiManagerParameter custom_Supla_name("name", "Supla Device Name", Supla_name, 51,"required");
  WiFiManagerParameter custom_update_username("updateUsername", "update username", update_username, 21,"required");
  WiFiManagerParameter custom_update_password("updatePassword", "update password", update_password, 21,"required");
  WiFiManagerParameter custom_Supla_tempa("tempa", "Temp 1", temp1, 6,"required");
  WiFiManagerParameter custom_Supla_tempb("tempb", "Temp 2", temp2, 6,"required");
  WiFiManagerParameter custom_Supla_tempc("tempc", "Temp 3", temp3, 6,"required");
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
  wifiManager.addParameter(&custom_Supla_tempa);
  wifiManager.addParameter(&custom_Supla_tempb);
  wifiManager.addParameter(&custom_Supla_tempc);
  wifiManager.addParameter(&custom_Supla_status);

  wifiManager.setCustomHeadElement("<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>");
  wifiManager.setMinimumSignalQuality(8);
  wifiManager.setConfigPortalTimeout(timeout);

    if (!wifiManager.startConfigPortal("Supla_Lector")) {
      Serial.println("Not connected to WiFi but continuing anyway.");
    } else {
      
      Serial.println("connected...yeey :)");    //if you get here you have connected to the WiFi
    }
    //read updated parameters
    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(Location_id, custom_Location_id.getValue());
    strcpy(Location_Pass, custom_Location_Pass.getValue());
    strcpy(Supla_name, custom_Supla_name.getValue());
    strcpy(update_username, custom_update_username.getValue());
    strcpy(update_password, custom_update_password.getValue());
    strcpy(temp1, custom_Supla_tempa.getValue());
    strcpy(temp2, custom_Supla_tempb.getValue());
    strcpy(temp3, custom_Supla_tempc.getValue());
   
  WiFi.softAPdisconnect(true);   //  close AP
    ticker.detach();
    digitalWrite(status_led, led_off);
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
         float Thermometer1 = sensors.getTempCByIndex(0);     
         float Thermometer2 = sensors.getTempCByIndex(1);     
         float Thermometer3 = sensors.getTempCByIndex(2);                 
             
    if (btn[4].mem == true){  // is auto on?
     if ((Thermometer1 > 0)&& (Thermometer2 > 0)&& (Thermometer3 > 0)){  // are temps > 0ºC
      if (((Thermometer2 - Thermometer1) > Temp2) && (Thermometer1 < Temp1)){  
        if ( btn[1].ms > 10 ) {
           Serial.println("Monostable Auto on by temp for: " + String(btn[1].ms / 1000) + " S");
           CustomSupla.relayOn(1, btn[1].ms);
        } else {
          Serial.println(" Auto on by temp");
           booton_action = true;
          CustomSupla.relayOn(1, 0);
        } 
       }
     }
     if (btn[1].mem == true){     // is pump on?     
      if (((Thermometer3 - Thermometer1) < Temp3) || (Thermometer1 > Temp1)){
          Serial.println(" Auto off by temp");
           booton_action = true;
          CustomSupla.relayOff(1);
      }
     }
    }
  sensors.requestTemperatures(); 
}
double get_temperature(int channelNumber, double last_val) {
   double t = -275;    

   switch(channelNumber)
          {
            case 5:
         return t = sensors.getTempCByIndex(0);     
                    break;
            case 6:
         return t = sensors.getTempCByIndex(1);     
                    break;
            case 7:
         return t = sensors.getTempCByIndex(2);     
                    break;       
            case 8:            
         return t = sensors.getTempCByIndex(3);
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
              if ( btn[a].ms > 10 ) {
                 if ( (btn[a].mem) == 1 ) {  
                  CustomSupla.relayOff(a);
                  Serial.print("BTN Switsh off monostable relay: ");
                  Serial.println(a);
                 } else { 
                  CustomSupla.relayOn(btn[a].channel, btn[a].ms);
                  Serial.print("monostable time: ");Serial.println(btn[a].ms);                 
                 }                                                            
              } else {
                 if ( (btn[a].mem) == 1 ) { 
                  CustomSupla.relayOff(btn[a].channel);
                  Serial.print("BTN Switsh off relay ");
                  Serial.println(btn[a].relay_pin-1);
                 } else {
                  CustomSupla.relayOn(btn[a].channel, 0);
                  Serial.print("BTN Switsh on relay ");
                  Serial.println(btn[a].relay_pin-1);
                 }        
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
   
     return btn[channelNumber].mem;      
 
}

void suplaDigitalWrite(int channelNumber, uint8_t pin, uint8_t val) {       
     btn[channelNumber].mem =val;
      if (channelNumber == 4) {
        if ( (btn[channelNumber].mem) == 0 ) {                    
                  Serial.print("Manual Mode ");                          
                  eep_milis = millis() + 2000 ;
                  Serial.println(" Relay off Manual Mode");
                  CustomSupla.relayOff(1);                  
                 }else {
                  Serial.println("Auto Mode ");       
                  eep_milis = millis() + 2000 ;                   
                 }

        }else if ( (btn[channelNumber].mem) == 0 ) {                    
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
    
     if ((booton_action == false) && (CustomSupla.channel_pin[channelNumber].DurationMS != btn[channelNumber].ms) && (val == 0)) {
            int durationMS = CustomSupla.channel_pin[channelNumber].DurationMS;
             if (durationMS < 0 ) durationMS = 0;
               Serial.print("button_duration: ");Serial.print(durationMS);           
               Serial.print(" button: ");Serial.println(channelNumber);               
               EEPROM.put((channelNumber * 10) + 200, durationMS);
               EEPROM.commit();
               btn[channelNumber].ms = durationMS;
           }
      booton_action = false;
  return; 
}

void Eeprom_save() {                  //----------EEPROM write  ---------------------- EEprom

        if (start){
          return;
        }
      for(int e=0;e<BTN_COUNT;e++) {  //  ---check relay except it have delay (staircase)
         if ( btn[e].ms > 0 ) {
        eep = (btn[e].mem);                    //  --- read relay state
        if (EEPROM.read(e) != 0){            //  --- compare relay state with memorized state
         EEPROM.write(e,0);                  //  --- if different write memory
         Serial.print(" channel monostable, mem off: ");
         Serial.println((btn[e].channel));
         EEPROM.commit();
        }          
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
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
  Serial.println("");
}
void setup() {  //------------------------------------------------ Setup ----------------------------------------------

  wifi_set_sleep_type(NONE_SLEEP_T);

  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY, 1); 
    delay(5000);
  pinMode(status_led,OUTPUT); 
  digitalWrite(status_led, led_on);
  EEPROM.begin(512);
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

  int btn0ms;EEPROM.get(200,btn0ms);Serial.print("initial_button_duration 0: ");Serial.println(btn0ms);
  int btn1ms;EEPROM.get(210,btn1ms);Serial.print("initial_button_duration 1: ");Serial.println(btn1ms);
  int btn2ms;EEPROM.get(220,btn2ms);Serial.print("initial_button_duration 2: ");Serial.println(btn2ms); 
  int btn3ms;EEPROM.get(230,btn3ms);Serial.print("initial_button_duration 3: ");Serial.println(btn3ms); 
     
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
        json.printTo(Serial);   //print config data to serial on startup
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
          Temp1 = atof(temp1);Serial.print("Temp1: ");Serial.println(Temp1);
          Temp2 = atof(temp2);Serial.print("Temp2: ");Serial.println(Temp2);
          Temp3 = atof(temp3);Serial.print("Temp3: ");Serial.println(Temp3);
        } else {
          Serial.println("failed to load json config");
          initialConfig = true;
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }

  wifi_station_set_hostname(Supla_name);  //nazwa w sieci lokalnej  @cino111
  CustomSupla.setDigitalReadFuncImpl(&supla_DigitalRead);    //            ------Send Value to server -------
  CustomSupla.setDigitalWriteFuncImpl(&suplaDigitalWrite);   //        -------  Read Value from server   -------
  CustomSupla.setStatusFuncImpl(&status_func);    //   ----------------------------- Status -----------------------------
  CustomSupla.setTemperatureCallback(&get_temperature);
  CustomSupla.setName(Supla_name);

  int LocationID = atoi(Location_id);
  CustomSupla.begin(GUID, mac, Supla_server, LocationID, Location_Pass);
                        
}

void loop() {

  if (initialConfig){
    for(int i=0;i<BTN_COUNT;i++){      
        EEPROM.write(i, 0);
        EEPROM.put((i * 10) + 200, cero); 
     }
      EEPROM.commit();
    ondemandwifiCallback () ;
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
         Serial.println((btn[i].channel));
         btn[i].mem =eep;
         digitalWrite(btn[i].relay_pin-1, eep);       //--------------- relay active High          
                  Serial.print((btn[i].relay_pin) -1);
                  Serial.print(" set ");
                  Serial.println(eep);                    
         }
    }
    start = false;
  } 
  
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

  if (millis() > eep_milis){
     itrate_Term();
     Eeprom_save() ;   //        ------------------------------Eepron save callback -----------------------------   
     eep_milis = millis() + 5000 ;
   }
   
  switch (s) {    //    ------------------------------------------------ Status ------------------------------------
    case 17:      // -----     STATUS_REGISTERED_AND_READY
    if (epr<BTN_COUNT){
     Eepron_read() ;                // ------------------------------- Eeprom read callback -------------------------------
     epr = epr+1;// -------- 1 loop for each output  ----------   
      if (tikOn == true){            
        digitalWrite(status_led, led_off);
        tikOn = false;
        Serial.println("supla.Ready");}
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
          ondemandwifiCallback () ;
         }
       }
    }
    last_C_W_state = C_W_read;            
  }

    CustomSupla.iterate(); 
    delay(50);
    iterate_botton(); 
   
   if (WiFi.status() == WL_CONNECTED){    
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
    }
    httpServer.handleClient(); 
   } else {
    WiFi_up();  
   }

   if (s != 17){
    CustomSupla.iterateOfline();
    digitalWrite(status_led, led_on);
    tikOn = true; 
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
