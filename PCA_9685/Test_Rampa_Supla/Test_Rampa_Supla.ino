#define supla_lib_config_h_  // silences debug messages
#include "LittleFS.h"
#include <Wire.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266TrueRandom.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Button3.h>
#include <Ticker.h>
#include <Adafruit_PWMServoDriver.h>
#include <SuplaDevice.h>
#include <supla/control/dimmer_base.h>
#include <supla/control/relay.h>
#include <supla/network/esp_wifi.h>
Supla::ESPWifi wifi("SSID", "PASS");  //------ Do not change----wifimanager takes care------
#define STORAGE_OFFSET 512
#include <supla/storage/eeprom.h>
Supla::Eeprom eeprom(STORAGE_OFFSET);
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
//#define RX  3  //High at boot
//#define TX  1  //must not be pulled low during power on/reset

#define SDA_PIN 4
#define SCL_PIN 5
#define LED_PIN  2
#define button_0_pin  0
#define button_1_pin  14
#define  relay_0_pin 16
#define  relay_1_pin 15
#define  relay_2_pin 13   
bool statusLedOn = LOW;

const int PWM_SEQUENCE_STOP       = -1;
const int PWM_SEQUENCE_ON_0_TO_15  = 1;
const int PWM_SEQUENCE_OFF_15_TO_0 = 2;
const int PWM_SEQUENCE_ON_ALL      = 3;
const int PWM_SEQUENCE_OFF_ALL     = 4;
const int PWM_SEQUENCE_ON_15_TO_0  = 5;
const int PWM_SEQUENCE_OFF_0_TO_15 = 6;

int fadeEffect = 500;
int currentPwmChannel = 0;
int pwmSequence = PWM_SEQUENCE_STOP;
int pwmOutLevel[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int pwmMaxLevel = 4095; 
int pwmMinLevel = 0;
unsigned long ultimaPasada;
int C_W_state = HIGH;
int last_C_W_state = HIGH;
unsigned long time_last_C_W_change = 0;
long C_W_delay = 10000;               // ---------------------- config delay 10 seconds ---------------------------
char Supla_server[81] = ("Set server address");
char Email[81] = ("set email address");
char Supla_name[81] = ("SuplaPCA9685");
char FadeTime[8] = ("500");
char Router_SSID[32];
char Router_Pass[64];
char Supla_status[51];
bool shouldSaveConfig = false;
bool initialConfig = false;
bool starting = true;
bool tikOn = true;
int s;
char GUID[SUPLA_GUID_SIZE];
char AUTHKEY[SUPLA_AUTHKEY_SIZE];
byte uuidNumber[16];
static const char logo[] PROGMEM = "<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>";

ESP8266WebServer httpServer(81);
ESP8266HTTPUpdateServer httpUpdater;
WiFiManager wifiManager;
Ticker ticker;
Ticker tickerB;
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
Button3 buttonA = Button3(button_0_pin);
Button3 buttonB = Button3(button_1_pin);

Supla::Control::Relay *relay_0 = nullptr;
Supla::Control::Relay *relay_1 = nullptr;
Supla::Control::Relay *relay_2 = nullptr;

class PwmDimm : public Supla::Control::DimmerBase {
  public:
    PwmDimm(int pwmChannel)
      : pwmChannel(pwmChannel) {
    }
    void setRGBWValueOnDevice(uint32_t red,
                              uint32_t green,
                              uint32_t blue,
                              uint32_t colorBrightness,
                              uint32_t brightness) {

      uint32_t brightnessAdj = brightness;
      
      if ( pwmChannel == 0){
        brightnessAdj = map(brightnessAdj, 0, 1023, 2047, 4095);
            for (uint8_t i = 0; i < 16; i++) {
              if (pwmOutLevel[i] >= pwmMaxLevel){
                pwm.setPWM(i, 0, brightnessAdj);
                pwmOutLevel[i] = brightnessAdj;
              }
            }
           pwmMaxLevel = brightnessAdj;
      }    
      else if ( pwmChannel == 1){
        brightnessAdj = map(brightnessAdj, 0, 1023, 0, 2047);        
            for (uint8_t i = 0; i < 16; i++) {
              if (pwmOutLevel[i] <= pwmMinLevel){
                pwm.setPWM(i, 0, brightnessAdj);
                pwmOutLevel[i] = brightnessAdj;
              }
            }
           pwmMinLevel = brightnessAdj;
      }
                              }
  protected:
    int pwmChannel;
};
class CustomControl : public Supla::Io {
  public:
    void customDigitalWrite(int channelNumber, uint8_t pin, uint8_t val) {
      
      if (pin == relay_0_pin){
        if (val == 1) {
          if (relay_1->isOn())relay_1->turnOff();
          if (relay_2->isOn())relay_2->turnOff();
          currentPwmChannel = 0;   
          pwmSequence = PWM_SEQUENCE_ON_0_TO_15;
          Serial.println("Rampa + ON)");
        }
        if (val == 0){
          if (relay_1->isOn())return;
          if (relay_2->isOn())relay_2->turnOff();
          currentPwmChannel = 0;   
          pwmSequence = PWM_SEQUENCE_OFF_0_TO_15;
          Serial.println("Rampa + OFF)");
        }
      }
      if (pin == relay_1_pin){
        if (val == 1) {
          if (relay_0->isOn())relay_0->turnOff(); 
          if (relay_2->isOn())relay_2->turnOff(); 
          pwmSequence = PWM_SEQUENCE_ON_ALL;
          Serial.println("Todo)");
        }
        if (val == 0){
          if (relay_0->isOn())relay_0->turnOff();
          if (relay_2->isOn())relay_2->turnOff();  
          pwmSequence = PWM_SEQUENCE_OFF_ALL;
          Serial.println("Nada)");
        }
      }
      if (pin == relay_2_pin){
        if (val == 1) {
          if (relay_1->isOn())relay_1->turnOff();                      
          if (relay_0->isOn())relay_0->turnOff();                     
          currentPwmChannel = 15;   
          pwmSequence = PWM_SEQUENCE_ON_15_TO_0;
          Serial.println("Rampa - ON)");
        }
        if (val == 0){
          if (relay_1->isOn())return;
          if (relay_0->isOn())relay_0->turnOff();
          currentPwmChannel = 15;   
          pwmSequence = PWM_SEQUENCE_OFF_15_TO_0;
          Serial.println("Rampa - OFF)");
        }
      }                       
     return ::digitalWrite(pin, val);
   }    
} CustomControl;

void rampa() {
  unsigned long timePast = millis() - ultimaPasada;
  ultimaPasada = millis();
  if (timePast > 0) {
    double divisor = 1.0 * fadeEffect / timePast;
    if (divisor <= 0) {
      divisor = 1;
    }
    int psos = pwmMaxLevel / divisor;
    if (psos < 1) {
      psos = 1;
    }

    switch (pwmSequence) {

      case PWM_SEQUENCE_STOP: break;

      case PWM_SEQUENCE_ON_0_TO_15: {  
          pwmOutLevel[currentPwmChannel] += psos;         
          if (pwmOutLevel[currentPwmChannel] >= pwmMaxLevel) {
            int sobras = (pwmOutLevel[currentPwmChannel] - pwmMaxLevel) + pwmOutLevel[currentPwmChannel + 1];
            pwmOutLevel[currentPwmChannel] = pwmMaxLevel;
            if (currentPwmChannel < 15) {
              pwm.setPWM(currentPwmChannel, 0, pwmOutLevel[currentPwmChannel]);
              currentPwmChannel ++;
              if (sobras <= pwmMaxLevel){
                pwmOutLevel[currentPwmChannel] = sobras;
                pwm.setPWM(currentPwmChannel, 0, pwmOutLevel[currentPwmChannel]);
              } else {
                pwmOutLevel[currentPwmChannel] = pwmMaxLevel;
                pwm.setPWM(currentPwmChannel, 0, pwmOutLevel[currentPwmChannel]);
              }
            } else {
              pwm.setPWM(currentPwmChannel, 0, pwmMaxLevel);
              pwmOutLevel[currentPwmChannel] = pwmMaxLevel;
              pwmSequence = PWM_SEQUENCE_STOP;
            }
          } else {
            pwm.setPWM(currentPwmChannel, 0, pwmOutLevel[currentPwmChannel]);;
          }
        } break;

      case PWM_SEQUENCE_OFF_15_TO_0: {  
          pwmOutLevel[currentPwmChannel] -= psos;
          if (pwmOutLevel[currentPwmChannel] <= pwmMinLevel) {
            int faltas = (pwmOutLevel[currentPwmChannel] - pwmMinLevel) + pwmOutLevel[currentPwmChannel - 1];
            pwmOutLevel[currentPwmChannel] = pwmMinLevel;
            if (currentPwmChannel > 0) {
              pwm.setPWM(currentPwmChannel, 0, pwmOutLevel[currentPwmChannel]);
              currentPwmChannel --;
              if (faltas >= pwmMinLevel){
                pwmOutLevel[currentPwmChannel] = faltas;
                pwm.setPWM(currentPwmChannel, 0, pwmOutLevel[currentPwmChannel]);
              } else {
                pwmOutLevel[currentPwmChannel] = pwmMinLevel;
                pwm.setPWM(currentPwmChannel, 0, pwmOutLevel[currentPwmChannel]);
              }
            } else {
              pwm.setPWM(currentPwmChannel, 0, pwmMinLevel);
              pwmOutLevel[currentPwmChannel] = pwmMinLevel;
              pwmSequence = PWM_SEQUENCE_STOP;
            }
          } else {
            pwm.setPWM(currentPwmChannel, 0, pwmOutLevel[currentPwmChannel]);
          }
        } break;

      case PWM_SEQUENCE_ON_ALL: {  
         for (uint8_t i = 0; i < 16; i++) {
          pwmOutLevel[i] += psos;
          if (pwmOutLevel[i] >= pwmMaxLevel) {
            pwmOutLevel[i] = pwmMaxLevel;
              pwm.setPWM(i, 0, pwmOutLevel[i]);
              if (i >= 15){             
               pwmSequence = PWM_SEQUENCE_STOP;
              }
          } else {
              pwm.setPWM(i, 0, pwmOutLevel[i]);
          }
         }
        } break;

      case PWM_SEQUENCE_OFF_ALL: {  
         for (uint8_t i = 0; i < 16; i++) {
          pwmOutLevel[i] -= psos;
          if (pwmOutLevel[i] <= pwmMinLevel) {
            pwmOutLevel[i] = pwmMinLevel;
              pwm.setPWM(i, 0, pwmOutLevel[i]);
              if (i >= 15){             
               pwmSequence = PWM_SEQUENCE_STOP;
              }
          } else {
              pwm.setPWM(i, 0, pwmOutLevel[i]);
          }
         }
        } break;

      case PWM_SEQUENCE_ON_15_TO_0: {   
          pwmOutLevel[currentPwmChannel] += psos;         
          if (pwmOutLevel[currentPwmChannel] >= pwmMaxLevel) {
            int sobras = (pwmOutLevel[currentPwmChannel] - pwmMaxLevel) + pwmOutLevel[currentPwmChannel - 1];
            pwmOutLevel[currentPwmChannel] = pwmMaxLevel;
            if (currentPwmChannel > 0) {
              pwm.setPWM(currentPwmChannel, 0, pwmOutLevel[currentPwmChannel]);
              currentPwmChannel --;
              if (sobras <= pwmMaxLevel){
                pwmOutLevel[currentPwmChannel] = sobras;
                pwm.setPWM(currentPwmChannel, 0, pwmOutLevel[currentPwmChannel]);
              } else {
                pwmOutLevel[currentPwmChannel] = pwmMaxLevel;
                pwm.setPWM(currentPwmChannel, 0, pwmOutLevel[currentPwmChannel]);
              }
            } else {
              pwm.setPWM(currentPwmChannel, 0, pwmMaxLevel);
              pwmOutLevel[currentPwmChannel] = pwmMaxLevel;
              pwmSequence = PWM_SEQUENCE_STOP;
            }
          } else {
            pwm.setPWM(currentPwmChannel, 0, pwmOutLevel[currentPwmChannel]);;
          }
        } break;

      case PWM_SEQUENCE_OFF_0_TO_15: { 
          pwmOutLevel[currentPwmChannel] -= psos;
          if (pwmOutLevel[currentPwmChannel] <= pwmMinLevel) {
            int faltas = (pwmOutLevel[currentPwmChannel] - pwmMinLevel) + pwmOutLevel[currentPwmChannel + 1];
            pwmOutLevel[currentPwmChannel] = pwmMinLevel;
            if (currentPwmChannel < 15) {
              pwm.setPWM(currentPwmChannel, 0, pwmOutLevel[currentPwmChannel]);
              currentPwmChannel ++;
              if (faltas >= pwmMinLevel){
                pwmOutLevel[currentPwmChannel] = faltas;
                pwm.setPWM(currentPwmChannel, 0, pwmOutLevel[currentPwmChannel]);
              } else {
                pwmOutLevel[currentPwmChannel] = pwmMinLevel;
                pwm.setPWM(currentPwmChannel, 0, pwmOutLevel[currentPwmChannel]);
              }
            } else {
              pwm.setPWM(currentPwmChannel, 0, pwmMinLevel);
              pwmOutLevel[currentPwmChannel] = pwmMinLevel;
              pwmSequence = PWM_SEQUENCE_STOP;
            }
          } else {
            pwm.setPWM(currentPwmChannel, 0, pwmOutLevel[currentPwmChannel]);
          }
        } break;          
    }
  }
}

void tick() {
  int state = digitalRead(LED_PIN);
  digitalWrite(LED_PIN, !state);
}

void botones() {
  buttonA.loop();
  buttonB.loop();
}

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void ondemandwifiCallback () {
  ticker.detach();
  digitalWrite(LED_PIN, statusLedOn);
  tikOn = false;

  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 81, "required");
  WiFiManagerParameter custom_Email("email", "Email", Email, 81, "required");
  WiFiManagerParameter custom_Supla_name("name", "Supla Device Name", Supla_name, 81, "required");
  WiFiManagerParameter custom_FadeTime("FadeTime", "Fade Time for each Step", FadeTime, 5, "required");
  WiFiManagerParameter custom_html_id21("<div><h4> - Supla State -   ");
  WiFiManagerParameter custom_html_id22( Supla_status);
  WiFiManagerParameter custom_html_id23( "</h4></div>");

  wifiManager.setBreakAfterConfig(true);
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_Email);
  wifiManager.addParameter(&custom_Supla_name);
  wifiManager.addParameter(&custom_FadeTime);
  wifiManager.addParameter(&custom_html_id21);
  wifiManager.addParameter(&custom_html_id22);
  wifiManager.addParameter(&custom_html_id23);

  wifiManager.setCustomHeadElement(logo);
  wifiManager.setMinimumSignalQuality(8);
  wifiManager.setConfigPortalTimeout(300);

  if (!wifiManager.startConfigPortal("SuplaPCA9685")) {
    Serial.println("Not connected to WiFi but continuing anyway.");
  } else {
    Serial.println("connected...yeey :)");
  }
  strcpy(Supla_server, custom_Supla_server.getValue());
  strcpy(Email, custom_Email.getValue());
  strcpy(Supla_name, custom_Supla_name.getValue());
  strcpy(FadeTime, custom_FadeTime.getValue());
  wifiManager.getWiFiSSID().toCharArray(Router_SSID, 33);
  wifiManager.getWiFiPass().toCharArray(Router_Pass, 65);
  if (strcmp(Supla_server, "get_new_guid_and_authkey") == 0) {
    Serial.println("new guid & authkey.");
    EEPROM.write(300, 0);
    EEPROM.commit();
    delay(100);
    ESP.reset();
  }
  if (shouldSaveConfig == true) { // ------------------------ wificonfig save --------------
    Serial.println(" config...");
    DynamicJsonDocument json(1024);
    json["Supla_server"] = Supla_server;
    json["Email"] = Email;
    json["Supla_name"] = Supla_name;
    json["FadeTime"] = FadeTime;
    json["Router_SSID"] = Router_SSID;
    json["Router_Pass"] = Router_Pass;
    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
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
  WiFi.softAPdisconnect(true);
  initialConfig = false;
}

void status_func(int status, const char *msg) {  
  if (s != status) {
    s = status;
    if (s != 10) {
      strcpy(Supla_status, msg);
    }
  }
}

void guid_authkey(void) {
  if (EEPROM.read(300) != 60) {
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
    delay(0);
  }
  read_guid();
  read_authkey();
  Serial.print("GUID : "); Serial.println(read_guid());
  Serial.print("AUTHKEY : "); Serial.println(read_authkey());
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
    //wifi_set_sleep_type(NONE_SLEEP_T);
    Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY, 1);
    delay(10);
    Serial.println(" ");
    Serial.println(" ");
    pinMode(button_0_pin, INPUT_PULLUP);
    pinMode(button_1_pin, INPUT_PULLUP);
    digitalWrite(relay_0_pin,LOW);
    digitalWrite(relay_2_pin,LOW);
    pinMode(relay_0_pin, OUTPUT);
    pinMode(relay_2_pin, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    ticker.attach(0.5, tick);
    EEPROM.begin(1024);
    
  if (EEPROM.read(300) != 60) {
    initialConfig = true;
  }
  
  guid_authkey();

  if (WiFi.SSID() == "") {
    initialConfig = true;
  }
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
        if (!deserializeError) {
          Serial.println("\nparsed json");
          if (json.containsKey("Supla_server")) strcpy(Supla_server, json["Supla_server"]);
          if (json.containsKey("Email")) strcpy(Email, json["Email"]);
          if (json.containsKey("Supla_name")) strcpy(Supla_name, json["Supla_name"]);
          if (json.containsKey("FadeTime")) strcpy(FadeTime, json["FadeTime"]);
          if (json.containsKey("Router_SSID")) strcpy(Router_SSID, json["Router_SSID"]);
          if (json.containsKey("Router_Pass")) strcpy(Router_Pass, json["Router_Pass"]);
          fadeEffect = atoi(FadeTime);
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
  wifi_station_set_hostname(Supla_name);
  WiFi.mode(WIFI_STA);

    Wire.begin(SDA_PIN, SCL_PIN);
    pwm.begin();
    pwm.reset();
    pwm.setOscillatorFrequency(27000000);
    pwm.setPWMFreq(400);  // This is the PWM frequency
    Wire.setClock(400000);

  auto miDimA = new PwmDimm(0);
  auto miDimB = new PwmDimm(1);
  relay_0 = new Supla::Control::Relay(relay_0_pin, true, 192);
  relay_0->getChannel()->setDefault(SUPLA_CHANNELFNC_STAIRCASETIMER);
  relay_0->keepTurnOnDuration();
  relay_0->disableChannelState();
  relay_1 = new Supla::Control::Relay(relay_1_pin, true, 64);
  relay_1->getChannel()->setDefault(SUPLA_CHANNELFNC_LIGHTSWITCH);
  relay_1->setDefaultStateOff();
  relay_1->disableChannelState();
  relay_2 = new Supla::Control::Relay(relay_2_pin, true, 192);
  relay_2->getChannel()->setDefault(SUPLA_CHANNELFNC_STAIRCASETIMER);
  relay_2->keepTurnOnDuration();
  relay_2->disableChannelState();
  
  buttonA.setClickHandler(clickA);
  buttonA.setLongClickHandler(longClickA);
  buttonB.setClickHandler(clickB);
  buttonB.setLongClickHandler(longClickB);
    
  SuplaDevice.setName(Supla_name);
  SuplaDevice.setStatusFuncImpl(&status_func);
  wifi.enableSSL(false);
  wifi.setSsid(Router_SSID);
  wifi.setPassword(Router_Pass);
  SuplaDevice.begin(GUID, Supla_server, Email, AUTHKEY);

  if (initialConfig == true) {
    ondemandwifiCallback();
  }    

    for (uint8_t i = 0; i < 16; i++) {
      pwm.setPWM(i, 0, pwmMinLevel);
      pwmOutLevel[currentPwmChannel] = pwmMinLevel;
    }

   tickerB.attach_ms(50, botones);
}

void loop() {
  
  int C_W_read = digitalRead(button_0_pin); {
    if (C_W_read != last_C_W_state) {
      time_last_C_W_change = millis();
    }
    if ((millis() - time_last_C_W_change) > C_W_delay) {
      if (C_W_read != C_W_state) {
        C_W_state = C_W_read;
        if (C_W_state == LOW) {
          ondemandwifiCallback () ;
        }
      }
    }
    last_C_W_state = C_W_read;
  }

  SuplaDevice.iterate();
  
  delay(25);
   
  rampa();

  if (WiFi.status() == WL_CONNECTED) {
    if (starting) {
      httpUpdater.setup(&httpServer, "/update", "admin", "pass");
      httpServer.begin();
      starting = false;
    }
    httpServer.handleClient();
  }
  if (s == STATUS_REGISTERED_AND_READY) {
    if (tikOn) {
      ticker.detach();
      digitalWrite(LED_PIN, !statusLedOn);
      tikOn = false;
    }
  } else if (!tikOn) {
    ticker.attach(0.5, tick);
    tikOn = true;
  }
  
}
void clickA(Button3 & btn) {
    Serial.println("Click A");
    relay_0->toggle(); 
}

void longClickA(Button3& btn) {
    Serial.println("longClick A");
    relay_1->toggle(); 
}
  
void clickB(Button3 & btn) {
    Serial.println("Click B");
    relay_2->toggle(); 
}

void longClickB(Button3& btn) {
    Serial.println("longClick B");
    relay_1->toggle(); 
}
