#include "LittleFS.h"    
#include <WiFiManager.h>
#include <ArduinoJson.h> 
#include <EEPROM.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h> 
#include <WiFiClientSecure.h>
#include <Ticker.h>
#include <Button3.h>
#include <ESP8266TrueRandom.h>
#include <SuplaDevice.h>
#include <supla/control/dimmer_base.h>
#include <supla/sensor/DS18B20.h>
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

#define CONFIG_PIN      0  // WiFiconfig 
#define PWM_PIN1       14
#define PWM_PIN2       12
#define PWM_PIN3       13
#define LEDPIN         15
#define BUTTON_PIN1     5
#define BUTTON_PIN2     4
#define BUTTON_PIN3    16
#define DS_PIN          2

int web_port =81;
int s;
char GUID[SUPLA_GUID_SIZE];
char AUTHKEY[SUPLA_AUTHKEY_SIZE];
byte uuidNumber[16];
char Supla_server[81]=("svr?.supla.org");
char Email[81]=("your@email");                        
char Supla_name[51]=("dimm"); 
char Supla_status[51]=("No server address"); 
int C_W_state = HIGH; 
int last_C_W_state = HIGH;
unsigned long time_last_C_W_change = 0; 
long C_W_delay = 5000;                      // config delay 5 seconds        
bool shouldSaveConfig = false;
bool initialConfig = false;
bool starting = true;
bool tikOn = true;
unsigned long mem_milis;
bool mem_update = false;
static const char logo[] PROGMEM = "<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>";
 
ESP8266WebServer httpServer(81);
ESP8266HTTPUpdateServer httpUpdater;
Ticker ticker; 
Ticker ticker2; 
Ticker ticker3; 
#define BTN_COUNT 3
Button3 buttons[BTN_COUNT] =
{
  Button3(BUTTON_PIN1),
  Button3(BUTTON_PIN2),
  Button3(BUTTON_PIN3)
};

bool out_on = true;

Supla::Control::RGBWBase *miDim[3] = {nullptr};

typedef struct {  //------------------------------------------- BTN ----------------------------------------------------
  int pin;                    // Button pin
  int out_pin;                // Pwm output pin
  unsigned long dim_milis;    // longPress loop delay
  unsigned long last_change;  // longPress last time
  char dim_state;              // Button pin state
  char last_dim_state;         // Button pin last state
  bool btn_hold;              // is longPress  yes/not 
  bool btn_dim ;              // Din up/down  
  bool btn_config;            // Button config mode on/off
  bool long_mode;             // Button long config mode
  bool scene_mode;            // Button scene on/off
  uint8_t hw_brightness;
  bool hw_on;                 // on/off
  char config_loop;
} _btn_t;

_btn_t btn[BTN_COUNT];

class Dimm : public Supla::Control::DimmerBase {  //------------------------------------------- DIMM ----------------------------------------------------
 public:
  Dimm(int brightnessPin)
      : brightnessPin(brightnessPin) {
  }

  void setRGBWValueOnDevice(uint8_t red,
                            uint8_t green,
                            uint8_t blue,
                            uint8_t colorBrightness,
                            uint8_t brightness) {
                         
   if (out_on) analogWrite(brightnessPin,brightness);
   if (brightnessPin == PWM_PIN1) btn[0].hw_brightness = brightness;
   if (brightnessPin == PWM_PIN2) btn[1].hw_brightness = brightness;
   if (brightnessPin == PWM_PIN3) btn[2].hw_brightness = brightness;
    mem_update = true;
    mem_milis = (millis()+3000); 
  }

 protected:
  int redPin;
  int greenPin;
  int bluePin;
  int brightnessPin;
  int colorBrightnessPin;
};

void supla_btn_init() {  //------------------------------------------- BTN ----------------------------------------------------
  for(int a=0;a<BTN_COUNT;a++)
    if (btn[a].pin > 0) {
      pinMode(btn[a].pin - 1, INPUT_PULLUP);
      btn[a].dim_milis = 0; 
      btn[a].last_change = millis();       
      btn[a].dim_state = digitalRead(btn[a].pin-1);
      btn[a].last_dim_state = digitalRead(btn[a].pin-1);
      btn[a].btn_hold = false;
      btn[a].btn_dim = true;
      btn[a].btn_config = false;
      btn[a].config_loop = 0;  
    }
}

void saveConfigCallback () {          
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void tick(){
  int state = digitalRead(LEDPIN);  
  digitalWrite(LEDPIN, !state);     
}
void disableLed(){
  digitalWrite(LEDPIN, LOW);                        
  ticker.detach();
  tikOn = false; 
  s = 0;                                    
}

void config_mode(char x){   //------------------------------------------- BTN CONFIG LOOP----------------------------------------------------

  switch (btn[x].config_loop) { 
    case  1 : miDim[x]->setRGBW(-1, -1, -1, -1, 100);                 break;
    case  3 : miDim[x]->setRGBW(-1, -1, -1, -1, 0);                   break;
    case  5 : miDim[x]->setRGBW(-1, -1, -1, -1, 100);                 break;
    case  7 : if (btn[x].hw_on){
      miDim[x]->setRGBW(-1, -1, -1, -1, EEPROM.read(x + 10));         break;
    }else{
      miDim[x]->setRGBW(-1, -1, -1, -1, 0);                           break;
    }
    case 40 : miDim[x]->setRGBWValueOnDevice(-1, -1, -1, -1, 0);      break;
    case 41 : miDim[x]->setRGBWValueOnDevice(-1, -1, -1, -1, 100);    break;
    case 42 : miDim[x]->setRGBWValueOnDevice(-1, -1, -1, -1, 0);      break;
    case 43 : miDim[x]->setRGBWValueOnDevice(-1, -1, -1, -1, 100);    break;
    case 44 : if (btn[x].hw_on){
      miDim[x]->setRGBWValueOnDevice(-1, -1, -1, -1, EEPROM.read(x + 10));     break;    
    }else{
      miDim[x]->setRGBWValueOnDevice(-1, -1, -1, -1, 0);              break;
    }
  }
  btn[x].config_loop ++;   
  if (btn[x].config_loop >= 45){
   btn[x].config_loop = 0;
   ticker3.detach();
   btn[x].btn_config = false;
   Serial.println("btn config Off");                                    
  }
}

void iterate_buttons(){  //------------------------------------------- BTN LOOP---------------------------------------------------- 

 unsigned long _now = millis();
 char _pin_read; 
 for(int a=0;a<BTN_COUNT;a++){
  buttons[a].loop();  
  if (btn[a].pin > 0) { 
   _pin_read = digitalRead(btn[a].pin - 1);  
   if (_pin_read != btn[a].last_dim_state) {            
     btn[a].last_change = _now;
   }
   if ((_now - btn[a].last_change) > 500) {     
     if (_pin_read != btn[a].dim_state) {     
       btn[a].dim_state = _pin_read;       
       if (btn[a].dim_state == LOW) {
          btn[a].btn_hold = true;
          btn[a].btn_dim = !btn[a].btn_dim;  
       }
     }
    }
   btn[a].last_dim_state = _pin_read;    
   if ((btn[a].btn_hold) && (btn[a].dim_milis < _now)){
    btn[a].dim_milis = (_now + 80);
    if (!digitalRead(btn[a].pin - 1)){
         if (btn[a].hw_brightness >= 100){
          if (btn[a].long_mode){
            btn[a].btn_dim = true; 
          }else{
            btn[a].hw_brightness = 100; 
          }
        }
        if (btn[a].hw_brightness <= 2){
          if (btn[a].long_mode){
            btn[a].btn_dim = false; 
          }else{
            btn[a].hw_brightness = 0; 
          }
        }          
        if (btn[a].btn_dim){
           miDim[a]->runAction(0, Supla::DIM_W);
           Serial.print("Long DIM ");Serial.println(a);
          } else {
           miDim[a]->runAction(0, Supla::BRIGHTEN_W);
           Serial.print("Long BRIGHTEN ");Serial.println(a);
         }  
      } else {
        btn[a].btn_hold = false;
        btn[a].last_change = (_now + 200);
      }
    }
  }
 }                                    
}
bool ondemandwifiCallback(bool resetConf) {  //------------------------------------------- WiFiConfig ----------------------------------------------------
   ticker.detach();
   digitalWrite(LEDPIN, HIGH);
             
   WiFiManager wm;

   if (resetConf){  
     wm.resetSettings();
     delay(3000);
     ESP.restart();   
     delay(1000);
   }
  
   WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 81,"required");
   WiFiManagerParameter custom_Email("email", "Email", Email, 81,"required");
   WiFiManagerParameter custom_Supla_name("name", "Supla Device Name", Supla_name, 51,"required");

   wm.setBreakAfterConfig(true);
   wm.setSaveConfigCallback(saveConfigCallback);
  
   wm.addParameter(&custom_Supla_server);
   wm.addParameter(&custom_Email);
   wm.addParameter(&custom_Supla_name);

   wm.setCustomHeadElement(logo);
   wm.setMinimumSignalQuality(8);
   //wm.setShowStaticFields(true); // force show static ip fields
   //wm.setShowDnsFields(true);    // force show dns field always
   wm.setConfigPortalTimeout(180);

   if (!wm.startConfigPortal("Dimm3x")) { Serial.println("Not connected to WiFi but continuing anyway.");} else { Serial.println("connected...yeey :)");}                
    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(Email, custom_Email.getValue());
    strcpy(Supla_name, custom_Supla_name.getValue());
    if(strcmp(Supla_server, "get_new_guid_and_authkey") == 0){
      Serial.println("new guid & authkey.");
      EEPROM.write(300, 0);
      EEPROM.commit();
      delay(100);
      ESP.reset(); 
    }  
  WiFi.softAPdisconnect(true);
  initialConfig = false;
  digitalWrite(LEDPIN, LOW);
  tikOn == false;
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
  for (int x = 0; x < 50; ++x) {
      EEPROM.write(x, 50);
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
     Serial.print("Supla connection state: "); 
     Serial.println(s); 
         if (status != 10){
      strcpy(Supla_status, msg);
   }  
  }                                        
}

void setup() {
  
  wifi_set_sleep_type(NONE_SLEEP_T);
  
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY, 1);
  delay(200);
  Serial.println(" ");
  Serial.println(" ");
  EEPROM.begin(1024);
  pinMode(CONFIG_PIN, INPUT_PULLUP); 
  pinMode(LEDPIN, OUTPUT);
  ticker.attach(0.5, tick);
  analogWriteRange(100); 
  analogWriteFreq(880);
  
  guid_authkey();
  
  wifi_station_set_hostname("Dimm3x");

  if (LittleFS.begin()) {  // ------------------------- wificonfig read -----------------
    Serial.println("mounted file system");
    if (LittleFS.exists("/config.json")) {
      Serial.println("reading config file");
       File configFile = LittleFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
         size_t size = configFile.size();
         std::unique_ptr<char[]> buf(new char[size]);
         configFile.readBytes(buf.get(), size);
        DynamicJsonDocument json(1024);
        DeserializationError deserializeError = deserializeJson(json, buf.get());
        serializeJsonPretty(json, Serial);
        if (!deserializeError) {Serial.println("\nparsed json");         
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
   
  if (WiFi.SSID()==""){ ondemandwifiCallback(false);}
  if (EEPROM.read(300) != 60){ondemandwifiCallback(false);}  
  WiFi.mode(WIFI_STA); 
  
  memset(btn, 0, sizeof(btn));
  btn[0].pin =BUTTON_PIN1 +1;          // pin gpio buton  +1
  btn[0].out_pin =PWM_PIN1 +1;         // pin gpio PwmOut +1
  btn[0].long_mode = EEPROM.read(71);
  btn[0].scene_mode = EEPROM.read(70);
  btn[1].pin =BUTTON_PIN2 +1;          // pin gpio buton  +1
  btn[1].out_pin =PWM_PIN2 +1;         // pin gpio PwmOut +1
  btn[1].long_mode = EEPROM.read(75);
  btn[1].scene_mode = EEPROM.read(74);
  btn[2].pin =BUTTON_PIN3 +1;          // pin gpio buton  +1
  btn[2].out_pin =PWM_PIN3 +1;         // pin gpio PwmOut +1
  btn[2].long_mode = EEPROM.read(79);
  btn[2].scene_mode = EEPROM.read(78);    
  supla_btn_init();

  miDim[0] = new Dimm(btn[0].out_pin-1);
  miDim[1] = new Dimm(btn[1].out_pin-1);
  miDim[2] = new Dimm(btn[2].out_pin-1);
  new Supla::Sensor::DS18B20(DS_PIN);

  wifi.enableSSL(false);
  SuplaDevice.setName(Supla_name);
  SuplaDevice.setStatusFuncImpl(&status_func);

  read_dimm_mem(10);            

  SuplaDevice.begin(GUID,Supla_server,Email,AUTHKEY);

  for(int a=0;a<BTN_COUNT;a++){
     buttons[a].setClickHandler(clicked);
     buttons[a].setDoubleClickHandler(doubleClick);
     buttons[a].setTripleClickHandler(tripleClick);
     buttons[a].setQuintleClickHandler(quintleClick);
     buttons[a].setConfigClickHandler(configClick);    
    if (!btn[a].hw_on){
      out_on = false;
      miDim[a]->setRGBW(0, 0, 0, 0, btn[a].hw_brightness);
      out_on = true;
      miDim[a]->setRGBW(-1, -1, -1, -1, 0);    
    }else{
      miDim[a]->setRGBWValueOnDevice(0, 0, 0, 0, btn[a].hw_brightness);
      miDim[a]->setRGBW(0, 0, 0, 0, btn[a].hw_brightness);
    }
    miDim[a]->setStep(2);
  }
  ticker2.attach_ms(50, iterate_buttons);  

}

void loop() {

  if (initialConfig){ initialConfig = false;ondemandwifiCallback(false);}
   
  int C_W_read = digitalRead(CONFIG_PIN);  //------------------------------------------- READ WiFiConfig PIN ----------------------------------------------------
   if (C_W_read != last_C_W_state) {            
     time_last_C_W_change = millis();
   }
   if ((millis() - time_last_C_W_change) > C_W_delay) {     
     if (C_W_read != C_W_state) {     
       Serial.println("Triger sate changed");
       C_W_state = C_W_read;       
       if (C_W_state == LOW) {
       // ondemandwifiCallback(true);
        ondemandwifiCallback(false);
       }
     }
    }
   last_C_W_state = C_W_read;

  if (millis() > mem_milis){if (mem_update) save_dim(10);mem_milis = (millis()+3000);}
      
  if (WiFi.status() == WL_CONNECTED){
    if (starting){
       httpUpdater.setup(&httpServer, "/update", "admin", "pass");
       httpServer.begin();      
       starting = false;         
     }
     httpServer.handleClient();
   }

  if (shouldSaveConfig) {
    Serial.println(" config...");  
    DynamicJsonDocument json(1024);
    json["Supla_server"] = Supla_server;
    json["Email"] = Email;
    json["Supla_name"] = Supla_name;
    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) {Serial.println("failed to open config file for writing");}
    serializeJsonPretty(json, Serial);
    serializeJson(json, configFile);
    configFile.close();
    Serial.println("Config written successfully");
    shouldSaveConfig = false;
    initialConfig = false; 
    WiFi.mode(WIFI_STA);   
    delay(5000);
    ESP.restart();     
  }

  SuplaDevice.iterate();
  delay(22);
   
  if ((s == 17)&&(tikOn)){
      ticker.detach();
      digitalWrite(LEDPIN, LOW);
      tikOn = false;    
  }else if ((s != 17) && (!tikOn)){
      ticker.attach(0.5, tick);
      tikOn = true;    
  }

}

void save_dim(int eepoffset){

  int eepset = eepoffset;
  
  bool save = false;
  if ((btn[0].hw_brightness != EEPROM.read(eepset)) && (btn[0].hw_brightness > 3 )){ EEPROM.write(eepset,btn[0].hw_brightness);save = true;}     
  if ((EEPROM.read(eepset + 5) != 33) && (btn[0].hw_brightness < 4 )){EEPROM.write(eepset + 5, 33);save = true;}     
  if ((EEPROM.read(eepset + 5) == 33) && (btn[0].hw_brightness > 3 )){EEPROM.write(eepset + 5, 10);save = true;}  
  
  if ((btn[1].hw_brightness != EEPROM.read(eepset + 1)) && (btn[1].hw_brightness > 3 )){ EEPROM.write(eepset + 1,btn[1].hw_brightness);save = true;}     
  if ((EEPROM.read(eepset + 6) != 33) && (btn[1].hw_brightness < 4 )){EEPROM.write(eepset + 6, 33);save = true;}     
  if ((EEPROM.read(eepset + 6) == 33) && (btn[1].hw_brightness > 3 )){EEPROM.write(eepset + 6, 10);save = true;} 
       
  if ((btn[2].hw_brightness != EEPROM.read(eepset + 2)) && (btn[2].hw_brightness > 3 )){ EEPROM.write(eepset + 2,btn[2].hw_brightness);save = true;}     
  if ((EEPROM.read(eepset + 7) != 33) && (btn[2].hw_brightness < 4 )){EEPROM.write(eepset + 7, 33);save = true;}     
  if ((EEPROM.read(eepset + 7) == 33) && (btn[2].hw_brightness > 3 )){EEPROM.write(eepset + 7, 10);save = true;} 
   
  if (save == true){ 
    EEPROM.commit();
    Serial.print("SAVE to eep: ");Serial.print(EEPROM.read(eepset));Serial.print(",");Serial.print(EEPROM.read(eepset + 1));Serial.print(",");Serial.print(EEPROM.read(eepset + 2));Serial.print(" offset: ");Serial.println(eepset);
    mem_update = false;
    mem_milis = (millis()+3000); 
  }  
}
void read_dimm_mem(int eepoffset){

  int eepset = eepoffset;
  
  btn[0].hw_brightness = EEPROM.read(eepset);btn[1].hw_brightness = EEPROM.read(eepset + 1);btn[2].hw_brightness = EEPROM.read(eepset + 2);
  if (EEPROM.read(eepset + 5) == 33){btn[0].hw_on = false;}else{btn[0].hw_on = true;}
  if (EEPROM.read(eepset + 6) == 33){btn[1].hw_on = false;}else{btn[1].hw_on = true;}
  if (EEPROM.read(eepset + 7) == 33){btn[2].hw_on = false;}else{btn[2].hw_on = true;}
  Serial.print("Epp Read PWM 1 ");Serial.println(btn[0].hw_brightness);Serial.print("Epp Read PWM 2 ");Serial.println(btn[1].hw_brightness);Serial.print("Epp Read PWM 3 ");Serial.println(btn[2].hw_brightness);
}

void clicked(Button3& btn) {  
    if (btn == buttons[0]) {
      miDim[0]->runAction(0, Supla::TOGGLE_W);
      Serial.println("Click TOGGLE 0");
    } else if (btn == buttons[1]) {
      miDim[1]->runAction(0, Supla::TOGGLE_W);
      Serial.println("Click TOGGLE 1");
    } else if (btn == buttons[2]) {
      miDim[2]->runAction(0, Supla::TOGGLE_W);
      Serial.println("Click TOGGLE 2");
    }
}
void doubleClick(Button3& btn) {  
   if (btn == buttons[0]) {doubl_A();}      
    else if (btn == buttons[1]) {doubl_B();}      
     else if (btn == buttons[2]) {doubl_C();}    
}
void tripleClick(Button3& btn) {  
   if (btn == buttons[0]) {tripl_A();}      
    else if (btn == buttons[1]) {tripl_B();}      
     else if (btn == buttons[2]) {tripl_C();}    
}
void quintleClick(Button3& btn) {  
   if (btn == buttons[0]) {quint_A();}      
    else if (btn == buttons[1]) {quint_B();}      
     else if (btn == buttons[2]) {quint_C();}    
}
void configClick(Button3& btn) {  
   if (btn == buttons[0]) {config_A();}      
    else if (btn == buttons[1]) {config_B();}      
     else if (btn == buttons[2]) {config_C();}    
}
void doubl_A() {
  if(btn[0].btn_config){
    btn[0].scene_mode = !btn[0].scene_mode;
    Serial.println("toggle scene mode 0"); 
    EEPROM.write(70, btn[0].scene_mode);
    EEPROM.commit();
    digitalWrite(LEDPIN, HIGH);                                  
    ticker.attach(0.5, disableLed); 
  }else{
   miDim[0]->setRGBW(-1, -1, -1, -1, 100);
   Serial.println("doubleClick 100% On 0");
  }
}
void tripl_A() {  
  if(btn[0].btn_config){
    save_dim(20);
    Serial.println("save sene 0");
    digitalWrite(LEDPIN, HIGH);                                  
    ticker.attach(0.5, disableLed);
  }else if (btn[0].scene_mode){     
      read_dimm_mem(20);
      if (btn[0].scene_mode){if (btn[0].hw_on){miDim[0]->setRGBW(-1, -1, -1, -1, btn[0].hw_brightness);}else{miDim[0]->setRGBW(-1, -1, -1, -1, 0);}}
      if (btn[1].scene_mode){if (btn[1].hw_on){miDim[1]->setRGBW(-1, -1, -1, -1, btn[1].hw_brightness);}else{miDim[1]->setRGBW(-1, -1, -1, -1, 0);}}
      if (btn[2].scene_mode){if (btn[2].hw_on){miDim[2]->setRGBW(-1, -1, -1, -1, btn[2].hw_brightness);}else{miDim[2]->setRGBW(-1, -1, -1, -1, 0);}}
      Serial.println("sene 0"); 
  }  
}
void quint_A() {  
  if(btn[0].btn_config){
    btn[0].long_mode = !btn[0].long_mode;
    Serial.println("toggle long mode 0"); 
    EEPROM.write(71, btn[0].long_mode);
    EEPROM.commit();
    digitalWrite(LEDPIN, HIGH);                                  
    ticker.attach(0.5, disableLed);
  }
}
void config_A() {
  if((!btn[0].btn_config) && (!btn[1].btn_config) && (!btn[2].btn_config)){
    Serial.println("Config Button 0");                           
    ticker3.attach(0.5, config_mode, (char)0);
    miDim[0]->setRGBW(-1, -1, -1, -1, 0); 
    btn[0].btn_config = true; 
  } 
}

void doubl_B() {
  if(btn[1].btn_config){
    btn[1].scene_mode = !btn[1].scene_mode;
    Serial.println("toggle scene mode 1"); 
    EEPROM.write(74, btn[1].scene_mode);
    EEPROM.commit();
    digitalWrite(LEDPIN, HIGH);                                  
    ticker.attach(0.5, disableLed); 
  }else{
   miDim[1]->setRGBW(-1, -1, -1, -1, 100);
   Serial.println("doubleClick 100% On 1");
  }
}
void tripl_B() {  
  if(btn[1].btn_config){
    save_dim(30);
    Serial.println("save sene 1");
    digitalWrite(LEDPIN, HIGH);                                  
    ticker.attach(0.5, disableLed);
  }else if (btn[1].scene_mode){     
      read_dimm_mem(30);
      if (btn[0].scene_mode){if (btn[0].hw_on){miDim[0]->setRGBW(-1, -1, -1, -1, btn[0].hw_brightness);}else{miDim[0]->setRGBW(-1, -1, -1, -1, 0);}}
      if (btn[1].scene_mode){if (btn[1].hw_on){miDim[1]->setRGBW(-1, -1, -1, -1, btn[1].hw_brightness);}else{miDim[1]->setRGBW(-1, -1, -1, -1, 0);}}
      if (btn[2].scene_mode){if (btn[2].hw_on){miDim[2]->setRGBW(-1, -1, -1, -1, btn[2].hw_brightness);}else{miDim[2]->setRGBW(-1, -1, -1, -1, 0);}}
      Serial.println("sene 1"); 
  }  
}
void quint_B() {  
  if(btn[1].btn_config){
    btn[1].long_mode = !btn[1].long_mode;
    Serial.println("toggle long mode 1"); 
    EEPROM.write(75, btn[1].long_mode);
    EEPROM.commit();
    digitalWrite(LEDPIN, HIGH);                                  
    ticker.attach(0.5, disableLed);
  }
}
void config_B() {
  if((!btn[0].btn_config) && (!btn[1].btn_config) && (!btn[2].btn_config)){
    Serial.println("Config Button 1");                           
    ticker3.attach(0.5, config_mode, (char)1);
    miDim[1]->setRGBW(-1, -1, -1, -1, 0); 
    btn[1].btn_config = true; 
  } 
}

void doubl_C() {  
  if(btn[2].btn_config){
    btn[2].scene_mode = !btn[2].scene_mode;
    Serial.println("toggle scene mode 2"); 
    EEPROM.write(78, btn[2].scene_mode);
    EEPROM.commit();
    digitalWrite(LEDPIN, HIGH);                                  
    ticker.attach(0.5, disableLed); 
  }else{
   miDim[2]->setRGBW(-1, -1, -1, -1, 100);
   Serial.println("doubleClick 100% On 2");
  }
}
void tripl_C() {  
  if(btn[2].btn_config){
    save_dim(40);
    Serial.println("save sene 2");
    digitalWrite(LEDPIN, HIGH);                                  
    ticker.attach(0.5, disableLed);
  }else if (btn[2].scene_mode){     
      read_dimm_mem(40);
      if (btn[0].scene_mode){if (btn[0].hw_on){miDim[0]->setRGBW(-1, -1, -1, -1, btn[0].hw_brightness);}else{miDim[0]->setRGBW(-1, -1, -1, -1, 0);}}
      if (btn[1].scene_mode){if (btn[1].hw_on){miDim[1]->setRGBW(-1, -1, -1, -1, btn[1].hw_brightness);}else{miDim[1]->setRGBW(-1, -1, -1, -1, 0);}}
      if (btn[2].scene_mode){if (btn[2].hw_on){miDim[2]->setRGBW(-1, -1, -1, -1, btn[2].hw_brightness);}else{miDim[2]->setRGBW(-1, -1, -1, -1, 0);}}
      Serial.println("sene 2"); 
  }  
} 
void quint_C() {  
  if(btn[2].btn_config){
    btn[2].long_mode = !btn[2].long_mode;
    Serial.println("toggle long mode 2"); 
    EEPROM.write(79, btn[2].long_mode);
    EEPROM.commit();
    digitalWrite(LEDPIN, HIGH);                                  
    ticker.attach(0.5, disableLed);
  }
}
void config_C() {
  if((!btn[0].btn_config) && (!btn[1].btn_config) && (!btn[2].btn_config)){
    Serial.println("Config Button 2");                           
    ticker3.attach(0.5, config_mode, (char)2);
    miDim[2]->setRGBW(-1, -1, -1, -1, 0); 
    btn[2].btn_config = true; 
  } 
}
