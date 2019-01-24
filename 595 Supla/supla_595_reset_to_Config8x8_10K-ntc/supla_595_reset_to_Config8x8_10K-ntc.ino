#include <FS.h>       // ---- esp board manager 2.4.2 --- iwip Variant V2 higher Bandwidth
#include <ShiftRegister74HC595.h>   //https://github.com/Simsso/ShiftRegister74HC595
#include <ESP8266WiFi.h>
#define SUPLADEVICE_CPP
#include <SuplaDevice.h>
// create shift register object (number of shift registers, data pin 14 on 74595, clock pin 11 on 74595, latch pin 12 on 74595)
ShiftRegister74HC595 sr (1, 15, 16, 0); 
#include <math.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h> //--------- https://github.com/tzapu/WiFiManager/tree/0.14 -------------
#include <ArduinoJson.h> //--------- https://github.com/bblanchon/ArduinoJson/tree/v5.13.2 ------
#include <EEPROM.h>
#include <DoubleResetDetector.h> 
extern "C"
{
#include "user_interface.h"
}
#define DRD_TIMEOUT 30 // Number of seconds after reset during which a  subseqent reset will be considered a double reset.
#define DRD_ADDRESS 0 // RTC Memory Address for the DoubleResetDetector to use
DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);
int uno = 1;
bool eep = LOW;          //             ---------------- Eeprom ------------------
bool startEeprom = true; //             ---------------- Eeprom ------------------
int epr = 0;             //             ----------- Eepron read loops ------------
int s;                   //             ---------------- Status ------------------
unsigned long svr_update = 1800000; //mean time between update  30 minutes
unsigned long svr_update_lasttime;   //last time update
unsigned long wifi_checkDelay = 60000;  // wifi reconect delay
unsigned long wifimilis;
#define BEGIN_PIN 100

unsigned int Rs = 100000;
double Vcc = 3.3;

#define BTN_COUNT 8

WiFiClient client;
char Supla_server[40];
char Location_id[15];
char Location_Pass[20];
byte mac[6];

//flag for saving data
bool shouldSaveConfig = false;
bool initialConfig = false;
int timeout           = 120; // seconds to run for wifi config

void saveConfigCallback () {                 //callback notifying us of the need to save config
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void ondemandwifiCallback () {
// The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 40);
  WiFiManagerParameter custom_Location_id("ID", "Location_id", Location_id, 15);
  WiFiManagerParameter custom_Location_Pass("Password", "Location_Pass", Location_Pass, 20);

  WiFiManager wifiManager;

  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  //add all your parameters here
  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_Location_id);
  wifiManager.addParameter(&custom_Location_Pass);

  wifiManager.setCustomHeadElement("<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>");

  wifiManager.setMinimumSignalQuality();

  // set configportal timeout
    wifiManager.setConfigPortalTimeout(timeout);

    if (!wifiManager.startConfigPortal("Supla x8")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.restart();
      delay(5000);
    }
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    
    //read updated parameters
    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(Location_id, custom_Location_id.getValue());
    strcpy(Location_Pass, custom_Location_Pass.getValue());
    
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
      
           Eeprom_save() ;   //        ------------------------------Eepron save callback -----------------------------
                             //        ---- this happens every few seconds

    
      return t;  
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

void supla_timer() {
  char v;
  unsigned long now = millis();
  {
  for(int a=0;a<BTN_COUNT;a++)
    if (btn[a].pin > 0) {
        v = digitalRead(btn[a].pin);
        if (v != btn[a].last_val && now - btn[a].last_time ) {
           btn[a].last_val = v;
           btn[a].last_time = now;
           if (v==0)
             {
              if ( btn[a].ms > 0 ) {
                     SuplaDevice.relayOn(btn[a].channel, btn[a].ms);
                     Serial.println(" monostable");
                 } else {
                 if ( (btn[a].mem) == 1 ) {   //   ----------------- == 1 if channel is false... == 0 if channel is true -----------------------
                  SuplaDevice.relayOff(btn[a].channel);
                  Serial.print("BTN Switsh off relay ");
                  Serial.println(btn[a].relay_pin);
                 } else {
                  SuplaDevice.relayOn(btn[a].channel, 0);
                  Serial.print("BTN Switsh on relay ");
                  Serial.println(btn[a].relay_pin);
                 }        
             }
             }}
      }
    }
    
}
void supla_btn_init() {
  for(int a=0;a<BTN_COUNT;a++)
    if (btn[a].pin > 0) {
        pinMode(btn[a].pin, INPUT_PULLUP);
        btn[a].last_val = digitalRead(btn[a].pin);
        btn[a].last_time = millis();
    }
}
int supla_DigitalRead(int channelNumber, uint8_t pin) {
   
      if (channelNumber == 0){
        if (btn[0].mem ==0) return 0;      
        else return 1;
      }
      if (channelNumber == 1){
        if (btn[1].mem ==0) return 0;      
        else return 1;
      }
      if (channelNumber == 2){
        if (btn[2].mem ==0) return 0;      
        else return 1;
      }
      if (channelNumber == 3){
        if (btn[3].mem ==0) return 0;      
        else return 1;
      }
      if (channelNumber == 4){
        if (btn[4].mem ==0) return 0;      
        else return 1;
      }
      if (channelNumber == 5){
        if (btn[5].mem ==0) return 0;      
        else return 1;
      }
      if (channelNumber == 6){
        if (btn[6].mem ==0) return 0;      
        else return 1;
      }
      if (channelNumber == 7){
        if (btn[7].mem ==0) return 0;      
        else return 1;      
      }      
}
void suplaDigitalWrite(int channelNumber, uint8_t pin, uint8_t val) {  //------------------------------------------------  Virtual ----------------------------
  
     if (channelNumber == 0){
       btn[0].mem =val;
       sr.set(0, val); // set single pin 
     }
     if (channelNumber == 1){
       btn[1].mem =val;
       sr.set(1, val); // set single pin 
     }
     if (channelNumber == 2){
       btn[2].mem =val;
       sr.set(2, val); // set single pin 
     }
     if (channelNumber == 3){
       btn[3].mem =val;
       sr.set(3, val); // set single pin 
     }
     if (channelNumber == 4){
       btn[4].mem =val;
       sr.set(4, val); // set single pin 
     }
     if (channelNumber == 5){
       btn[5].mem =val;
       sr.set(5, val); // set single pin 
     }
     if (channelNumber == 6){
       btn[6].mem =val;
       sr.set(6, val); // set single pin 
     }
     if (channelNumber == 7){
       btn[7].mem =val;
       sr.set(7, val); // set single pin 
     }
     
  return;
        
}
void Eeprom_save() {                  //----------EEPROM write  ---------------------- EEprom
    if (startEeprom == true){             // ----- don't change memorized state until connected and restored all the channels

      return;
      }                        
      for(int i=0;i<BTN_COUNT;i++) {  //  ---check relay except it have delay (staircase)
         if ( btn[i].ms > 0 ) {
                     continue;
         } else {
        eep = (btn[i].mem);                    //  --- read relay state
        if (eep != EEPROM.read(i)){            //  --- compare relay state with memorized state
         EEPROM.write(i,eep);                  //  --- if different write memory
         Serial.print("EEPROM.");
         Serial.print(i);
         Serial.print(" write.");
         Serial.println((eep));
         Serial.print(" channel ");
         Serial.println((btn[i].channel));
         EEPROM.commit();
        }
      }
    }
}
void Eepron_read() {                  //----------EEPROM read  ---------------------- EEprom
                                
   // for(int i=0;i<BTN_COUNT;i++){      //  ---check relay except der have delay (staircase)
     //if ( (btn[i].ms) > 0 ) {
                //     continue;
        // } else {
        eep = EEPROM.read(epr);               //  ---read relay state
       
         Serial.print("EEPROM.");
         Serial.print(epr);
         Serial.print(" read.");
         Serial.println((eep));
         Serial.print(" channel ");
         Serial.println((btn[epr].channel));
       
        if (eep == HIGH){                    //  --- if 1 send relay on
          SuplaDevice.relayOn(epr, 0);       //  --- only one channel in each pass
          }
        
    if (epr == (BTN_COUNT-1)) {
      startEeprom = false;                //  --- once finished we do not need more
      }     
}
void status_func(int status, const char *msg) {     //    ------------------------ Status --------------------------
 s=status;                                          //    -- to check if we are registered and ready before restore from memory
}
void setup() {  //------------------------------------------------ Setup ----------------------------------------------

  wifi_set_sleep_type(NONE_SLEEP_T);

  Serial.begin(115200);
  
  EEPROM.begin(512);

   if (drd.detectDoubleReset()) {
    Serial.println("Double Reset Detected");
    ondemandwifiCallback ();
  } else {
    Serial.println("No Double Reset Detected");
  }
  if (WiFi.SSID()==""){
    //Serial.println("We haven't got any access point credentials, so get them now");   
    initialConfig = true;
  } 
  //read configuration from FS json
  Serial.println("mounting FS...");
  
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
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
  } else {
    Serial.println("failed to mount FS");
  }
  WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.

   uint8_t mac[WL_MAC_ADDR_LENGTH];
   WiFi.macAddress(mac);
   char GUID[SUPLA_GUID_SIZE] = {mac[WL_MAC_ADDR_LENGTH - 6], mac[WL_MAC_ADDR_LENGTH - 5], mac[WL_MAC_ADDR_LENGTH - 4], mac[WL_MAC_ADDR_LENGTH - 3],                                
                                 mac[WL_MAC_ADDR_LENGTH - 2], mac[WL_MAC_ADDR_LENGTH - 1], mac[WL_MAC_ADDR_LENGTH - 1], mac[WL_MAC_ADDR_LENGTH - 2], 
                                 mac[WL_MAC_ADDR_LENGTH - 3], mac[WL_MAC_ADDR_LENGTH - 4], mac[WL_MAC_ADDR_LENGTH - 5], mac[WL_MAC_ADDR_LENGTH - 6]};
          
  SuplaDevice.addRelay(101, false);   
  SuplaDevice.addRelay(102, false);  
  SuplaDevice.addRelay(103, false); 
  SuplaDevice.addRelay(104, false);   
  SuplaDevice.addRelay(105, false);  
  SuplaDevice.addRelay(106, false); 
  SuplaDevice.addRelay(107, false);   
  SuplaDevice.addRelay(108, false);       
  // CHANNEL8 - Thermometer DS18B20
  SuplaDevice.addDS18B20Thermometer();

  memset(btn, 0, sizeof(btn));
  btn[0].pin =5;          // pin gpio buton  0 = no buton
  btn[0].relay_pin =101;  // pin gpio Relay
  btn[0].channel =0;      // channel
  btn[0].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[0].mem =0;
  btn[1].pin =4;          // pin gpio buton  0 = no buton
  btn[1].relay_pin =102;  // pin gpio Relay
  btn[1].channel =1;      // channel
  btn[1].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[1].mem =0;
  btn[2].pin =2;          // pin gpio buton  0 = no buton
  btn[2].relay_pin =103;  // pin gpio Relay
  btn[2].channel =2;      // channel
  btn[2].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[2].mem =0;
  btn[3].pin =14;         // pin gpio buton  0 = no buton 
  btn[3].relay_pin =104;  // pin gpio Relay
  btn[3].channel =3;      // channel
  btn[3].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[3].mem =0;
  btn[4].pin =12;         // pin gpio buton  0 = no buton
  btn[4].relay_pin =105;  // pin gpio Relay
  btn[4].channel =4;      // channel
  btn[4].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[4].mem =0;
  btn[5].pin =13;         // pin gpio buton  0 = no buton
  btn[5].relay_pin =106;  // pin gpio Relay
  btn[5].channel =5;      // channel
  btn[5].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[5].mem =0;
  btn[6].pin =3;         // pin gpio buton  0 = no buton
  btn[6].relay_pin =107;    // pin gpio Relay
  btn[6].channel =6;      // channel
  btn[6].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[6].mem =0;
  btn[7].pin =1;          // pin gpio buton  0 = no buton
  btn[7].relay_pin =108;  // pin gpio Relay
  btn[7].channel =7;      // channel
  btn[7].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[7].mem =0;
  supla_btn_init();

  SuplaDevice.setDigitalReadFuncImpl(&supla_DigitalRead);    //            ------Send Value to server -------
  SuplaDevice.setDigitalWriteFuncImpl(&suplaDigitalWrite);   //        -------  Read Value from server   -------
  SuplaDevice.setTimerFuncImpl(&supla_timer);
  SuplaDevice.setStatusFuncImpl(&status_func);    //   ----------------------------- Status -----------------------------
  SuplaDevice.setName("Supla x 8");

  int LocationID = atoi(Location_id);
  SuplaDevice.begin(GUID,              // Global Unique Identifier 
                    mac,               // Ethernet MAC address
                    Supla_server,      // SUPLA server address
                    LocationID,        // Location ID 
                    Location_Pass);    // Location Password
// read_initial_relay_state();
    for(int i=0;i<BTN_COUNT;i++){      //  ---check relay except der have delay (staircase)
     if ( (btn[i].ms) > 0 ) {
                     continue;
         } else {
        eep = EEPROM.read(i);               //  ---read relay state
       if (eep > uno ){
         EEPROM.write(i,uno);                  
         Serial.println("epp correct");
         EEPROM.commit();
       }
         Serial.print("Recover.");
         Serial.print(i);
         Serial.print(" read.");
         Serial.println((eep));
         Serial.print(" channel ");
         Serial.println((btn[i].channel));
       
       // if (eep == HIGH){                    //  --- if 1 send relay on
          sr.set(btn[i].channel, eep); // set single pin 
          //SuplaDevice.relayOn(epr, 0);       //  --- only one channel in each pass
         // }
         }
    }         
}

void loop() {

  drd.loop();

  if (initialConfig){
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

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    Serial.println("config saved");
    shouldSaveConfig = false;
    WiFi.mode(WIFI_STA);
    ESP.restart();
    delay(5000); 
  }
  if (WiFi.status() != WL_CONNECTED) { 
    WiFi_up();
    } 
  
  SuplaDevice.iterate();
  SuplaDevice.setTemperatureCallback(&get_temperature);
  
  switch (s) {    //    ------------------------------------------------ Status ------------------------------------
  case 17:      // -----     STATUS_REGISTERED_AND_READY
  if (epr<BTN_COUNT){
  Serial.print("Eepron_read...");
  Serial.println(epr);
   Eepron_read() ;                // ------------------------------- Eeprom read callback -------------------------------
   epr = epr+1;                   // -------- 1 loop for each output 8 in total ----------
   
  } 
    break;

    case 10:      // --------------------- REGISTER_IN_PROGRESS  ----------------------
    epr = 0 ;
     startEeprom = true;
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

void WiFi_up() // conect to wifi
{ 
  if (millis() > wifimilis)  {
  WiFi.begin(); 
  for (int x = 10; x > 0; x--) 
  {
    if (x == 1){
    wifimilis = (millis() + wifi_checkDelay) ; 
    }
    if (WiFi.status() == WL_CONNECTED) 
    {    
     break;                           
    }
    else                              
    {
     yield();
     delay(500);   
      Serial.print(".");                
                         
    }
    
  }

  if (WiFi.status() == WL_CONNECTED)
  {
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
  }
  else    
  {
    Serial.println("");
    Serial.println("not connected");
  }
  }
}
