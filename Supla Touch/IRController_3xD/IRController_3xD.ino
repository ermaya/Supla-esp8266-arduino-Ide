#include <FS.h>                                               // This needs to be first, or it all crashes and burns
#include <IRremoteESP8266.h>                                  // ---- esp board manager 2.4.2 ---
#include <IRrecv.h>
#include <IRutils.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>                                      
#include <ESP8266mDNS.h>                                     
#define CUSTOMSUPLA_CPP
#include <CustomSupla.h>                                      // 1.6.2 C
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266HTTPUpdateServer.h>  
#include <Ticker.h>
#include <EEPROM.h>
#include <ESP8266TrueRandom.h>                                            
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
 * Touch x 3 ESP8285
GPIO0 Touchpad # 1
GPIO9 Touchpad # 2
GPIO10 Touchpad # 3
GPIO12 Relay # 1
GPIO5 Relay # 2
GPIO4 Relay # 3
GPIO13 Blue LED
GPIO1 TX pin
GPIO3 RX pin Temperature DS18b20
 */
byte mac[6];
const int pinr1 = 3;                           // Receiving pin
const int configpin = 0;                                    
const int ledpin = 2;                           // Built in LED defined for WEMOS people
bool booton_action = false;
#define BTN_COUNT 3
int relay_1 = 16; //12;   //------------------- set relay Gpio -----
int relay_2 = 4; //5;
int relay_3 = 5;//4;
int button_1 = 0;   //------------------- set button Gpio ----
int button_2 = 12; //9;
int button_3 = 13; //10;
const unsigned int captureBufSize = 100;          // Size of the IR capture buffer.
bool tikOn = false;
bool pr_wifi = true;
bool start = true;
bool eep = LOW;     
int epr = 0;         
int cero = 0;
bool initialConfig = false;
bool shouldSaveConfig = false;                              
bool holdReceive;               
unsigned long eep_milis;
unsigned long wifi_checkDelay = 60000;
unsigned long wifimilis;
int C_W_state = HIGH; 
int last_C_W_state = HIGH;
unsigned long time_last_C_W_change = 0; 
long C_W_delay = 10000;               // ---------------------- config delay 10 seconds ---------------------------
int canal = 0;
int web_port = 80;
char Supla_server[40]= "";
char Location_id[15]= "";
char Location_Pass[20]= "";
char Supla_name[51]= "Supla_IR";
char port_str[6] = "80";
char Supla_status[51];
int s; 
char GUID[SUPLA_GUID_SIZE];
byte uuidNumber[16];
char limpio[40];
char supla1_on_data[40];
char supla1_off_data[40]; 
char supla2_on_data[40];
char supla2_off_data[40];
char supla3_on_data[40];
char supla3_off_data[40]; 
char supla4_on_data[40];
char supla4_off_data[40]; 
const char *wifi_config_name = "Touch IR WiFiComfig";

DynamicJsonBuffer jsonBuffer;
JsonObject& deviceState = jsonBuffer.createObject();
WiFiClient client;
ESP8266WebServer *server = NULL;
ESP8266HTTPUpdateServer httpUpdater;
Ticker ticker;                                          
IRrecv irrecv(pinr1, captureBufSize);

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

File fsUploadFile;            

typedef struct { 
  int pin;
  int relay_pin;
  int channel;
  char last_val;
  int ms;
  unsigned long last_time;
  bool mem;
} _btn_t;

_btn_t btn[BTN_COUNT];

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
           delay(60);
           v = digitalRead(btn[a].pin-1);
           if (v==0)
             {
              booton_action = true;
              if ( btn[a].ms > 10 ) {
                 if ( digitalRead(btn[a].relay_pin-1) == 1 ) {  
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
   
     return digitalRead(btn[channelNumber].relay_pin-1);        //--------------- relay active High        
 
}

void suplaDigitalWrite(int channelNumber, uint8_t pin, uint8_t val) {       
     btn[channelNumber].mem =val;
      
           if ( (btn[channelNumber].mem) == 0 ) {                    
                  Serial.print("Switsh off relay ");
                  Serial.println(btn[channelNumber].relay_pin-1);
                  digitalWrite(btn[channelNumber].relay_pin-1, LOW);        //--------------- relay active High       
                  eep_milis = millis() + 2000 ;                  
                 }else {
                  Serial.print("Switsh on relay ");
                  Serial.println(btn[channelNumber].relay_pin-1);
                  digitalWrite(btn[channelNumber].relay_pin-1, HIGH);        //--------------- relay active High       
                  eep_milis = millis() + 2000 ;                   
                 }
    
     if ((booton_action == false) && (CustomSupla.channel_pin[channelNumber].DurationMS != btn[channelNumber].ms)) {
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

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void resetReceive() {
  if (holdReceive) {
    Serial.println("Reenabling receiving");
    irrecv.resume();
    holdReceive = false;
  }
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

  wifiManager.setBreakAfterConfig(true);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setCustomHeadElement("<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>");
  wifiManager.setMinimumSignalQuality(8);

  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 40);
  WiFiManagerParameter custom_Location_id("ID", "Location_id", Location_id, 15);
  WiFiManagerParameter custom_Location_Pass("Password", "Location_Pass", Location_Pass, 20);
  WiFiManagerParameter custom_Supla_name("name", "Supla Device Name", Supla_name, 51,"required");
  WiFiManagerParameter custom_port("port_str", "Choose a IRWeb port", port_str, 6);
  
  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_Location_id);
  wifiManager.addParameter(&custom_Location_Pass);
  wifiManager.addParameter(&custom_Supla_name);  
  wifiManager.addParameter(&custom_port);

    if (!wifiManager.startConfigPortal("Supla_Touch_IR")) {
      Serial.println("Not connected to WiFi but continuing anyway.");
    } else {
      
      Serial.println("connected...yeey :)");   
    }

    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(Location_id, custom_Location_id.getValue());
    strcpy(Location_Pass, custom_Location_Pass.getValue());
    strcpy(Supla_name, custom_Supla_name.getValue());
    strncpy(port_str, custom_port.getValue(), 6);
    web_port = atoi(port_str);

     if(strcmp(Supla_server, "get_new_guid") == 0){
      Serial.println("new guid.");
      EEPROM.write(300, 0);
      EEPROM.commit();
      ESP.reset(); 
    }

  if (shouldSaveConfig) {
    Serial.println(" config...");
    DynamicJsonBuffer jsonWifiBuffer;
    JsonObject& json = jsonWifiBuffer.createObject();
    json["Supla_server"] = Supla_server;
    json["Location_id"] = Location_id;
    json["Location_Pass"] = Location_Pass;
    json["Supla_name"] = Supla_name;
    json["port_str"] = port_str;

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
  }
  ticker.detach();

  WiFi.softAPdisconnect(true);
  digitalWrite(ledpin, LOW);
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
void auto_guid(void) {
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
    EEPROM.put(200, cero);
    EEPROM.put(210, cero);
    EEPROM.put(220, cero);
    EEPROM.write(300, 60);
    EEPROM.commit();
  }
  read_guid();
  Serial.print("GUID : ");Serial.println(read_guid());  
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
//=============================================================================================================================

void setup() {
  
  wifi_set_sleep_type(NONE_SLEEP_T);
  WiFi.mode(WIFI_STA);
    
  //Serial.begin(115200);
  EEPROM.begin(512);
  pinMode(ledpin, OUTPUT);
  Serial.println("");
  pinMode(configpin, INPUT_PULLUP);

  if (WiFi.SSID()==""){initialConfig = true;}      
  if (EEPROM.read(300) != 60){initialConfig = true;} 
  auto_guid();
   
  CustomSupla.addRelay(101, false);   
  CustomSupla.addRelay(102, false);  
  CustomSupla.addRelay(103, false); 

  int btn0ms;EEPROM.get(200,btn0ms);Serial.print("initial_button_duration 0: ");Serial.println(btn0ms);
  int btn1ms;EEPROM.get(210,btn1ms);Serial.print("initial_button_duration 1: ");Serial.println(btn1ms);
  int btn2ms;EEPROM.get(220,btn2ms);Serial.print("initial_button_duration 2: ");Serial.println(btn2ms); 
     
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
  supla_btn_init();

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonWifiBuffer;
        JsonObject& json = jsonWifiBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          if (json.containsKey("Supla_server")) strncpy(Supla_server, json["Supla_server"], 40);
          if (json.containsKey("Location_id")) strncpy(Location_id, json["Location_id"], 15);
          if (json.containsKey("Location_Pass")) strncpy(Location_Pass, json["Location_Pass"], 20);
          if (json.containsKey("Supla_name")) strncpy(Supla_name, json["Supla_name"], 51);         
          if (json.containsKey("port_str")) {
            strncpy(port_str, json["port_str"], 6);
            web_port = atoi(json["port_str"]);
          }
        } else {
          Serial.println("failed to load json config");
        }
        jsonWifiBuffer.clear();
      }
     configFile.close();     
    }
  } else {
    Serial.println("failed to mount FS");
  }
  read_stored_command();
  wifi_station_set_hostname(Supla_name);

  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.macAddress(mac);          
  CustomSupla.setDigitalReadFuncImpl(&supla_DigitalRead);   
  CustomSupla.setDigitalWriteFuncImpl(&suplaDigitalWrite);  
  CustomSupla.setName(Supla_name);
  CustomSupla.setStatusFuncImpl(&status_func);
  int LocationID = atoi(Location_id);  
  CustomSupla.begin(GUID,mac,Supla_server, LocationID,Location_Pass);    

  digitalWrite(ledpin, LOW);
  ticker.attach(2, disableLed);

  irrecv.enableIRIn();
  Serial.println("Ready to receive IR signals");
}

//+=============================================================================
// Send header HTML
//
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
  server->sendContent("    <meta name='viewport' content='width=device-width, initial-scale=.75' />\n");
  server->sendContent("    <link rel='stylesheet' href='https://stackpath.bootstrapcdn.com/bootstrap/3.4.1/css/bootstrap.min.css' />\n");
  server->sendContent("    <script src='https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js'></script>\n");  
  server->sendContent("    <script src='https://stackpath.bootstrapcdn.com/bootstrap/3.4.1/js/bootstrap.min.js'></script>\n");
  server->sendContent("    <style>@media (max-width: 991px) {.nav-pills>li {float: none; margin-left: 0; margin-top: 5px; text-align: center;}}</style>\n");
  server->sendContent("    <title>Supla Touch IR  (" + String(Supla_name) + ")</title>\n");
  server->sendContent("  <link  rel='icon' href='data:image/x-icon;base64,AAABAAEAEBAQAAEABAAoAQAAFgAAACgAAAAQAAAAIAAAAAEABAAAAAAAgAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAA/4QAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEAERABAAEAEBAQABAQEBABAAEAEQARABAQABAQABAAAQARAAEQARAAAAAAAAAAAAAAAAAAAAAAEREAEQAQAAAQAAEAEBAAABAAAAAQEAAAERAAEQAREAAQAAEAABABABAAAQAQEAEAEREAEQAREAAAAAAAAAAAD//wAA2N0AAKuqAADdmQAArrsAANnMAAD//wAA//8AAIZvAAC9rwAAv68AAI5jAAC97QAAva0AAIZjAAD//wAA' type='image/x-icon' />\n");
  server->sendContent("  <link  rel='shortcut icon' href='data:image/x-icon;base64,AAABAAEAEBAQAAEABAAoAQAAFgAAACgAAAAQAAAAIAAAAAEABAAAAAAAgAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAA/4QAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEAERABAAEAEBAQABAQEBABAAEAEQARABAQABAQABAAAQARAAEQARAAAAAAAAAAAAAAAAAAAAAAEREAEQAQAAAQAAEAEBAAABAAAAAQEAAAERAAEQAREAAQAAEAABABABAAAQAQEAEAEREAEQAREAAAAAAAAAAAD//wAA2N0AAKuqAADdmQAArrsAANnMAAD//wAA//8AAIZvAAC9rwAAv68AAI5jAAC97QAAva0AAIZjAAD//wAA' type='image/x-icon' />\n");
  server->sendContent("  </head>\n");
  server->sendContent("  <body>\n");
  server->sendContent("   <nav class='navbar navbar-inverse'>\n");
  server->sendContent("      <a class='navbar-brand' href='/'>Supla Touch IR</a>\n");
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
  server->sendContent("              <a href='http://" + WiFi.localIP().toString() + ":" + int(web_port) + "'>Local <span class='badge'>" + WiFi.localIP().toString() + ":" + String(web_port) + "</span></a></li>\n");
  server->sendContent("            <li class='active'>\n");
  server->sendContent("              <a href='http://" + WiFi.localIP().toString() + ":"  + int(web_port) + "/listcodes"  + "'>Stored Codes <span class='badge'>" + WiFi.localIP().toString() + ":"  + String(web_port) + "/listcodes"  + "</span></a></li>\n");
  server->sendContent("            <li class='active'>\n");
  server->sendContent("              <a href='https://cloud.supla.org/login'> Supla state <span class='badge'>" + String(Supla_status) + "</span></a></li>\n");
  server->sendContent("            <li class='active'>\n");
  server->sendContent("              <a href='http://" + String(Supla_name) + ".local" + ":" + String(web_port) + "'>Hostname <span class='badge'>" + String(Supla_name) + ".local" + ":" + String(web_port) + "</span></a></li>\n"); 
  server->sendContent("          </ul>\n");
  server->sendContent("        </div>\n");
  server->sendContent("      </div><hr />\n");/*
  server->sendContent("      <div class='row'>\n");
  server->sendContent("        <div class='col-md-12'>\n");
  server->sendContent("          <ul class='nav nav-pills'>\n"); 
  server->sendContent("            <li class='active'>\n");
  server->sendContent("              <a href='http://" + WiFi.softAPIP().toString() + ":" + int(web_port) + "'>AP <span class='badge'>" + WiFi.softAPIP().toString() + ":" + String(web_port) + "</span></a></li>\n");
  server->sendContent("            <li class='active'>\n");
  server->sendContent("              <a href='http://" + WiFi.softAPIP().toString() + ":"  + int(web_port) + "/listcodes"  + "'>AP Stored Codes <span class='badge'>" + WiFi.softAPIP().toString() + ":"  + String(web_port) + "/listcodes"  + "</span></a></li>\n");;  
  server->sendContent("          </ul>\n");
  server->sendContent("        </div>\n");
  server->sendContent("      </div><hr />\n");*/
}

//+=============================================================================
// Send footer HTML
//
void sendFooter() {

  server->sendContent("      <hr />\n");
 // server->sendContent("      <div class='row'><div class='col-md-12'><em>Supla status:" + String(Supla_status) + "</em></div></div>");
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

//+=============================================================================
// Stream home page HTML
//
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
  String State0 = "OFF"; if ( btn[0].mem == 1){ State0 = "ON";}
  String State1 = "OFF"; if ( btn[1].mem == 1){ State1 = "ON";}
  String State2 = "OFF"; if ( btn[2].mem == 1){ State2 = "ON";} 
  server->sendContent("      <div class='row'>\n");
  server->sendContent("        <div class='col-md-12'>\n");
  server->sendContent("          <h3>Web Control</h3>\n");
  server->sendContent("          <table class='table table-striped' style='table-layout: fixed;'>\n");
  server->sendContent("            <thead><tr><th>Channel</th><th>State</th><th>Timer</th><th>Action</th></tr></thead>\n"); //Title
  server->sendContent("            <tbody>\n");
  server->sendContent("           <tr class='text-uppercase'><td>Channel 1</td><td>" + State0 + "</td><td> " + (btn[0].ms / 1000.0) + " Seconds</td><td> <a class='btn btn-primary btn-xs' href='/controlon?id=0' role='button'>On</a> <a class='btn btn-danger btn-xs' href='/controloff?id=0' role='button'>Off</a></td></tr>\n");
  server->sendContent("           <tr class='text-uppercase'><td>Channel 2</td><td>" + State1 + "</td><td> " + (btn[1].ms / 1000.0) + " Seconds</td><td> <a class='btn btn-primary btn-xs' href='/controlon?id=1' role='button'>On</a> <a class='btn btn-danger btn-xs' href='/controloff?id=1' role='button'>Off</a></td></tr>\n");
  server->sendContent("           <tr class='text-uppercase'><td>Channel 3</td><td>" + State2 + "</td><td> " + (btn[2].ms / 1000.0) + " Seconds</td><td> <a class='btn btn-primary btn-xs' href='/controlon?id=2' role='button'>On</a> <a class='btn btn-danger btn-xs' href='/controloff?id=2' role='button'>Off</a></td></tr>\n"); 
  server->sendContent("            </tbody>\n");
  server->sendContent("          </table>\n");
  server->sendContent("         </div></div>\n");
  server->sendContent("       <p></p>\n");
 // server->sendContent("          </div></div>\n");
 // server->sendContent("      <div class='row'>\n");
 // server->sendContent("        <div class='col-md-12'>\n");
 // server->sendContent("          <ul class='list-unstyled'>\n");
 // server->sendContent("            <li><span class='badge'>GPIO " + String(pinr1) + "</span> Receiving </li>\n");
 // server->sendContent("            <li><span class='badge'>GPIO " + String(pins1) + "</span> Transmitter 1 </li></ul>\n");
  server->sendContent("        </div>\n");
  server->sendContent("      </div>\n");
  sendFooter();
}

//+=============================================================================
// Stream store code page HTML
//
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
  server->sendContent("               <option>supla1_On</option>\n");
  server->sendContent("               <option>supla1_Off</option>\n");
  server->sendContent("               <option>supla2_On</option>\n");
  server->sendContent("               <option>supla2_Off</option>\n");
  server->sendContent("               <option>supla3_On</option>\n");
  server->sendContent("               <option>supla3_Off</option>\n");/*
  server->sendContent("               <option>supla4_On</option>\n");
  server->sendContent("               <option>supla4_Off</option>\n");
  server->sendContent("               <option>supla5_On</option>\n");
  server->sendContent("               <option>supla5_Off</option>\n");
  server->sendContent("               <option>supla6_On</option>\n");
  server->sendContent("               <option>supla6_Off</option>\n");
  server->sendContent("               <option>supla7_On</option>\n");
  server->sendContent("               <option>supla7_Off</option>\n");
  server->sendContent("               <option>supla8_On</option>\n");
  server->sendContent("               <option>supla8_Off</option>\n");*/
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
  //server->sendContent("      <div class='row'><div class='col-md-12'><em>set the name for the code as follow: <h4>supla1</h4> for the code that is transmitted with the first channel in Supla App </em></div></div>");
  //server->sendContent("      <div class='row'><div class='col-md-12'><em><h4>supla2</h4> for the second channel in Supla App <h4>supla8</h4>  and so on until last channel in Supla App</em></div></div>");
  
  sendFooter();
}

//+=============================================================================
// Stream list code page HTML
//
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


//+=============================================================================
// Redirect used for some pages - upload, so page refersh doesnt ask for a re-submit
//
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
        if (SPIFFS.exists("/codes/supla1_on.json")) {
          File codeFileRead = SPIFFS.open("/codes/supla1_on.json", "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            if (filebtn.containsKey("data")) strcpy(supla1_on_data ,filebtn["data"]);
            Serial.print("supla1_on_data: ");Serial.println(supla1_on_data);
            jsonReadCodeFileBuffer.clear();
        }   
      } else { strcpy(supla1_on_data ,limpio);
      }  
        if (SPIFFS.exists("/codes/supla1_off.json")) {
          File codeFileRead = SPIFFS.open("/codes/supla1_off.json", "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            if (filebtn.containsKey("data")) strcpy(supla1_off_data ,filebtn["data"]);
            Serial.print("supla1_off_data: ");Serial.println(supla1_off_data);
            jsonReadCodeFileBuffer.clear();
        }   
      } else { strcpy(supla1_off_data ,limpio);
      }       
        if (SPIFFS.exists("/codes/supla2_on.json")) {
          File codeFileRead = SPIFFS.open("/codes/supla2_on.json", "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            if (filebtn.containsKey("data")) strcpy(supla2_on_data ,filebtn["data"]);
            Serial.print("supla2_on_data: ");Serial.println(supla2_on_data);
            jsonReadCodeFileBuffer.clear();
        }   
      } else { strcpy(supla2_on_data ,limpio);
      }  
        if (SPIFFS.exists("/codes/supla2_off.json")) {
          File codeFileRead = SPIFFS.open("/codes/supla2_off.json", "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            if (filebtn.containsKey("data")) strcpy(supla2_off_data ,filebtn["data"]);
            Serial.print("supla2_off_data: ");Serial.println(supla2_off_data);
            jsonReadCodeFileBuffer.clear();
        }   
      } else { strcpy(supla2_off_data ,limpio);
      }  
        if (SPIFFS.exists("/codes/supla3_on.json")) {
          File codeFileRead = SPIFFS.open("/codes/supla3_on.json", "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            if (filebtn.containsKey("data")) strcpy(supla3_on_data ,filebtn["data"]);
            Serial.print("supla3_on_data: ");Serial.println(supla3_on_data);
            jsonReadCodeFileBuffer.clear();
        }   
      } else { strcpy(supla3_on_data ,limpio);
      }  
        if (SPIFFS.exists("/codes/supla3_off.json")) {
          File codeFileRead = SPIFFS.open("/codes/supla3_off.json", "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            if (filebtn.containsKey("data")) strcpy(supla3_off_data ,filebtn["data"]);
            Serial.print("supla3_off_data: ");Serial.println(supla3_off_data);
            jsonReadCodeFileBuffer.clear();
        }   
      } else { strcpy(supla3_off_data ,limpio);
      }  
      /* if (SPIFFS.exists("/codes/supla4_on.json")) {
          File codeFileRead = SPIFFS.open("/codes/supla4_on.json", "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            if (filebtn.containsKey("data")) strcpy(supla4_on_data ,filebtn["data"]);
            Serial.print("supla4_on_data: ");Serial.println(supla4_on_data);
            jsonReadCodeFileBuffer.clear();
        }   
      } else { strcpy(supla4_on_data ,limpio);
      }  
        if (SPIFFS.exists("/codes/supla4_off.json")) {
          File codeFileRead = SPIFFS.open("/codes/supla4_off.json", "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            if (filebtn.containsKey("data")) strcpy(supla4_off_data ,filebtn["data"]);
            Serial.print("supla4_off_data: ");Serial.println(supla4_off_data);
            jsonReadCodeFileBuffer.clear();
        }   
      } else { strcpy(supla4_off_data ,limpio);} */                           
    }
  } else {
    Serial.println("No Stored Data");
  }
  jsonCodeBtnBuffer.clear();
}

//+=============================================================================
// Split string by character
//
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

//+=============================================================================
// Display encoding type
//
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


//+=============================================================================
// Stores the selected code to JSON file
//
void storeReceivedCode (Code code, String btn) {
  //generate the nested json from the code
  DynamicJsonBuffer jsonWriteBuffer;  
  JsonObject& jsonadd = jsonWriteBuffer.createObject();
  btn.toLowerCase();
  jsonadd["name"] = btn;       
   
    jsonadd["type"] = code.encoding;
    jsonadd["data"] = code.data;
    jsonadd["length"] = code.bits;

  if (saveJSON(jsonadd, btn)) sendHomePage("Code stored to " + btn, "Alert", 2, 200); //send_redirect("/?message=Code stored to " + btn + "&type=1&header=Information&httpcode=200"); //
  jsonWriteBuffer.clear(); 
}

//+=============================================================================
// Deletes the codes JSON file
//
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

//+=============================================================================
// delete all stored codes
//
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


//+=============================================================================
// Saves the codes JSON file with code (called storeReceivedCode)
//
bool saveJSON (JsonObject& json, String btn){
  btn.toLowerCase();
  if (SPIFFS.begin()) {     //read current codes file

    if (SPIFFS.exists("/codes/" + btn +".json")) DeleteJSONitem(btn);
    File codeFileWrite = SPIFFS.open("/codes/" + btn +".json", "w"); //writes the updated json to code file
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

//+=============================================================================
// lists all stored codes for display in storedcodepage
//
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

//+=============================================================================
// Manages file upload for restoring remote button codes
//
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
        SPIFFS.remove(fsUploadFile.name());  //No match on object name and file name, file deleted
        Serial.println("Object doesnt match, file deleted");
        send_redirect("/?message=Code file mismatch&type=2&header=Alert&httpcode=400"); //sendHomePage("The JSON object doesn not match the file name, please try again", "Alert", 2, 200); //
      } else {
        send_redirect("/listcodes"); //sendHomePage("Codes restored", "Information", 1, 200); //
      }
      fsUploadFile.close();
    } else {
      Serial.print("Error uploading ");
      send_redirect("/?message=Error uploading file&type=2&header=Alert&httpcode=200"); //sendHomePage("Error uploading file", "Alert", 2, 200); //
    }
  }
}


//+=============================================================================
// Enables codesjson file download for backup
//
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


//+=============================================================================
// Code to JsonObject
//
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

void loop() {
  
  if (initialConfig){
    for(int i=0;i<BTN_COUNT;i++){      
        EEPROM.write(i, 0);
        EEPROM.put((i * 10) + 200, cero); 
     }
      EEPROM.commit();
    ondemandwifiCallback () ;
  }
  
  int C_W_read = digitalRead(configpin);  // ---------------------let's read the status of the Gpio to start wificonfig ---------------------
   if (C_W_read != last_C_W_state) {  time_last_C_W_change = millis();}    
    if ((millis() - time_last_C_W_change) > C_W_delay) {     
     if (C_W_read != C_W_state) {     
       C_W_state = C_W_read;       
        if (C_W_state == LOW) {
         Serial.println("WiFiConfig");
          ondemandwifiCallback ();} } }         
           last_C_W_state = C_W_read; 

    if (start){
    // read_initial_relay_state
    for(int i=0;i<BTN_COUNT;i++){      //  ---check relay except der have delay (staircase)
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
         digitalWrite(btn[i].relay_pin-1, eep);               
                  Serial.print((btn[i].relay_pin) -1);
                  Serial.print(" set ");
                  Serial.println(eep);                    
         }
    }
    start = false;
  } 
  
  CustomSupla.iterate();
    
  decode_results  results;                                        // Somewhere to store the results

  if (irrecv.decode(&results) && !holdReceive) {                  // Grab an IR code
    Serial.println("Signal received:");

   if (digitalRead(ledpin)) {
    
    copyCode(last_recv_4, last_recv_5);                           // Pass
    copyCode(last_recv_3, last_recv_4);                           // Pass
    copyCode(last_recv_2, last_recv_3);                           // Pass
    copyCode(last_recv, last_recv_2);                             // Pass
    cvrtCode(last_recv, &results);                                // Store the results
    last_recv.valid = true;

     Serial.print("value_ir: ");Serial.println(last_recv.data);

       if(strcmp(last_recv.data, supla1_on_data) == 0)
        {                      
            booton_action = true;
              if ( btn[0].ms > 10 ) {
                  CustomSupla.relayOn(btn[0].channel, btn[0].ms);
                  Serial.print("IR Switsh on 1 monostable time: ");Serial.println(btn[0].ms);                                                                           
              } else {
                  CustomSupla.relayOn(btn[0].channel, 0);
                  Serial.println("IR Switsh on 1 ");  
             }
         }
       else if(strcmp(last_recv.data, supla1_off_data) == 0) 
         {
            booton_action = true;
                  CustomSupla.relayOff(btn[0].channel);
                  Serial.println("IR Switsh off 1");        
         }
       else if(strcmp(last_recv.data, supla2_on_data) == 0)  
        {                      
            booton_action = true;
              if ( btn[1].ms > 10 ) {
                  CustomSupla.relayOn(btn[1].channel, btn[1].ms);
                  Serial.print("IR Switsh on 2 monostable time: ");Serial.println(btn[1].ms);                                                                           
              } else {
                  CustomSupla.relayOn(btn[1].channel, 0);
                  Serial.println("IR Switsh on 2 ");  
             }
         }           
       else if(strcmp(last_recv.data, supla2_off_data) == 0) 
         {
            booton_action = true;
                  CustomSupla.relayOff(btn[1].channel);
                  Serial.println("IR Switsh off 2");        
         }
       else if(strcmp(last_recv.data, supla3_on_data) == 0)
        {                      
            booton_action = true;
              if ( btn[0].ms > 10 ) {
                  CustomSupla.relayOn(btn[2].channel, btn[2].ms);
                  Serial.print("IR Switsh on 3 monostable time: ");Serial.println(btn[2].ms);                                                                           
              } else {
                  CustomSupla.relayOn(btn[2].channel, 0);
                  Serial.println("IR Switsh on 3 ");  
             }
         }
       else if(strcmp(last_recv.data, supla3_off_data) == 0) 
         {
            booton_action = true;
                  CustomSupla.relayOff(btn[2].channel);
                  Serial.println("IR Switsh off 3");        
         }
       else if(strcmp(last_recv.data, supla4_on_data) == 0)  
         {
             Serial.println("Signal supla4_on received:");
         }            
       else if(strcmp(last_recv.data, supla4_off_data) == 0) 
         {
            Serial.println("Signal supla4_off received:");      
         }             
   }
    irrecv.resume();                                              // Prepare for the next value
    digitalWrite(ledpin, LOW);                                    // Turn on the LED for 0.5 seconds
    ticker.attach(0.3, disableLed);
  }
  
  delay(22);

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
      if (server != NULL) {
        delete server;
      }
     server = new ESP8266WebServer(web_port);
     httpUpdater.setup(server, "/update", "admin", "pass");
     if (!MDNS.begin(Supla_name)) {Serial.println("Error setting up MDNS responder!");}
     MDNS.addService("http", "tcp", web_port); // Announce the ESP as an HTTP service
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
  
  server->on("/controlon", []() {
    Serial.println("Connection received to control page");
    int canal = server->arg("id").toInt();
    Serial.print("canal: ");Serial.println(canal);
      booton_action = true;
      if ( btn[canal].ms > 10 ) {
         CustomSupla.relayOn(btn[canal].channel, btn[canal].ms);                                                                           
      } else {
         CustomSupla.relayOn(btn[canal].channel, 0); 
      } 
    sendHomePage();
  });
    server->on("/controloff", []() {
    Serial.println("Connection received to control page");
    int canal = server->arg("id").toInt();
    Serial.print("canal: ");Serial.println(canal);
      booton_action = true;
      CustomSupla.relayOff(btn[canal].channel);      
    sendHomePage();
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
    server->handleClient();
    MDNS.update(); 
   } else {
    WiFi_up();  
   }
   
  if (millis() > eep_milis){
     Eeprom_save() ;   //        ------------------------------Eepron save callback -----------------------------   
     eep_milis = millis() + 5000 ;
   }
   
  switch (s) {    //    ------------------------------------------------ Status ------------------------------------
    case 17:      // -----     STATUS_REGISTERED_AND_READY
    if (epr<BTN_COUNT){
     Eepron_read() ;                // ------------------------------- Eeprom read callback -------------------------------
     epr = epr+1;// -------- 1 loop for each output  ----------   
      if (tikOn == true){            
        //digitalWrite(status_led, HIGH);
        tikOn = false;
        Serial.println("supla.Ready");}
       }
      break;   
     case 10:      // --------------------- REGISTER_IN_PROGRESS  ----------------------
      epr = 0 ;
      break;
  } 
                     
  if (s != 17){
    CustomSupla.iterateOfline();
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
    bool supla_arduino_svr_connect(const char *suplaserver, int port) {
          return client.connect(suplaserver, 2015);
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
          cb.get_temperature = NULL;
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
