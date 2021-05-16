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
#define supla_lib_config_h_  // silences debug messages
#include "LittleFS.h"
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266TrueRandom.h>
#include <WiFiClientSecure.h>
#include <Ticker.h>
#include <SuplaDevice.h>
#include <supla/control/relay.h>
#include <supla/control/virtual_relay.h>
#include <supla/control/button.h>
#include <supla/sensor/MAXThermocouple.h>
#include <supla/network/esp_wifi.h>
#define STORAGE_OFFSET 512
#include <supla/storage/eeprom.h>
Supla::Eeprom eeprom(STORAGE_OFFSET);
Supla::ESPWifi wifi("SSID", "PASS");  //------ Do not change----wifimanager takes care------
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

int wificonfig_pin = 0;  //D3 --  0
int LED_PIN = 2;         //D4 --  2
int relay_pin = 5;       //D1 --  5
int pin_CLK = 13;        //D7 -- 13
int pin_CS = 12;         //D6 -- 12
int pin_DO = 14;         //D5 -- 14
int button_pin = 0;      //D3 --  0
int C_W_state = HIGH;
int last_C_W_state = HIGH;
unsigned long time_last_C_W_change = 0;
long C_W_delay = 10000;               // ---------------------- config delay 5 seconds ---------------------------
char Supla_server[81] = ("svr?.supla.org");
char Email[81] = ("your@email");
char Supla_name[51] = ("Thermostat");
char term_temp[5] = ("0.5");
char term_hist[5] = ("1.0");
char Supla_status[51] = ("No server address");
char Router_SSID[32];
char Router_Pass[64];
bool shouldSaveConfig = false;
bool initialConfig = false;
bool starting = true;
unsigned long wifi_checkDelay = 30000;
unsigned long wifimilis;
int s;
char GUID[SUPLA_GUID_SIZE];
char AUTHKEY[SUPLA_AUTHKEY_SIZE];
byte uuidNumber[16];
static const char logo[] PROGMEM = "<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>";
double ThermostatTemperature = 20.5;
bool doSaveTermSetTemp = false;
float Term_Temp = 0.5;
float Term_Hist = 1.0;
bool tikOn = true;
bool statusLedOn = LOW;

ESP8266WebServer httpServer(81);
ESP8266HTTPUpdateServer httpUpdater;
WiFiManager wifiManager;
Ticker ticker;

Supla::Control::VirtualRelay *relay_0 = nullptr;
Supla::Control::Relay *relay_1 = nullptr;
Supla::Sensor::MAXThermocouple *MaxTemp = nullptr;

class TermostatTemp : public Supla::Sensor::Thermometer {
  public:
    TermostatTemp() {}

    void onInit() {
      // retrieve value from eeprom to variable "ThermostatTemperature"
      channel.setNewValue(getValue() );
    }
    double getValue() {
      // returns the variable "ThermostatTemperature"
      return ThermostatTemperature;
    }
    void iterateAlways() {
      channel.setNewValue(getValue() );
      if ( millis() - lastReadTime > 5000) {
        lastReadTime = millis();
        if (relay_0->isOn()) {
          double actualTemp = MaxTemp->getValue();
          if (((relay_1->isOn()) && ( actualTemp >= (ThermostatTemperature))) || ( actualTemp < -100.0)) {
            relay_1->turnOff();
          } else if ((!relay_1->isOn()) && ( actualTemp <= ThermostatTemperature - Term_Hist)) {
            relay_1->turnOn();
          }
        }
      }
    }
}; TermostatTemp *TerTemp = nullptr;

class TermostatSet : public Supla::Control::Relay {
  public:
    TermostatSet() : Relay(-1, true, 32) {}

    void onInit() {
      // do nothing here
    }
    void turnOn(_supla_int_t duration) {
      //  increase "ThermostatTemperature" by Term_Temp
      ThermostatTemperature += Term_Temp;
      channel.setNewValue(false);
      doSaveTermSetTemp = true;
    }
    void turnOff(_supla_int_t duration) {
      //  decrease "ThermostatTemperature" by Term_Temp
      ThermostatTemperature -= Term_Temp;
      channel.setNewValue(false);
      doSaveTermSetTemp = true;
    }
    bool isOn() {
      return false;
    }
}; TermostatSet *TerSet = nullptr;

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void ondemandwifiCallback () {
  ticker.detach();
  digitalWrite(LED_PIN, statusLedOn);

  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 81, "required");
  WiFiManagerParameter custom_Email("email", "Email", Email, 81, "required");
  WiFiManagerParameter custom_Supla_name("name", "Supla Device Name", Supla_name, 51, "required");
  WiFiManagerParameter custom_term_temp("step", "Step +/-", term_temp, 5, "required");
  WiFiManagerParameter custom_term_hist("histerisis", "Histerisis", term_hist, 5, "required");
  WiFiManagerParameter custom_html_id21("<div><h4> - Supla State -   ");
  WiFiManagerParameter custom_html_id22(Supla_status);
  WiFiManagerParameter custom_html_id23("</h4></div>");

  wifiManager.setBreakAfterConfig(true);
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_Email);
  wifiManager.addParameter(&custom_Supla_name);
  wifiManager.addParameter(&custom_term_temp);
  wifiManager.addParameter(&custom_term_hist);
  wifiManager.addParameter(&custom_html_id21);
  wifiManager.addParameter(&custom_html_id22);
  wifiManager.addParameter(&custom_html_id23);

  wifiManager.setCustomHeadElement(logo);
  wifiManager.setMinimumSignalQuality(8);
  //wifiManager.setShowStaticFields(true); // force show static ip fields
  //wifiManager.setShowDnsFields(true);    // force show dns field always
  wifiManager.setConfigPortalTimeout(300);

  if (!wifiManager.startConfigPortal("MAX_Thermostat")) {
    Serial.println("Not connected to WiFi but continuing anyway.");
  } else {
    Serial.println("connected...yeey :)");
  }
  strcpy(Supla_server, custom_Supla_server.getValue());
  strcpy(Email, custom_Email.getValue());
  strcpy(Supla_name, custom_Supla_name.getValue());
  strcpy(term_temp, custom_term_temp.getValue());
  strcpy(term_hist, custom_term_hist.getValue());
  wifiManager.getWiFiSSID().toCharArray(Router_SSID, 33);
  wifiManager.getWiFiPass().toCharArray(Router_Pass, 65);
  if (strcmp(Supla_server, "get_new_guid_and_authkey") == 0) {
    Serial.println("new guid & authkey.");
    EEPROM.write(300, 0);
    EEPROM.commit();
    delay(100);
    ESP.reset();
  }
  if (shouldSaveConfig) {
    DynamicJsonDocument json(1024);
    json["Supla_server"] = Supla_server;
    json["Email"] = Email;
    json["Supla_name"] = Supla_name;
    json["term_temp"] = term_temp;
    json["term_hist"] = term_hist;
    json["Router_SSID"] = Router_SSID;
    json["Router_Pass"] = Router_Pass;
    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    serializeJsonPretty(json, Serial);
    serializeJson(json, configFile);
    configFile.close();
    shouldSaveConfig = false;
    initialConfig = false;
    WiFi.mode(WIFI_STA);
    delay(5000);
    ESP.restart();
  }
  WiFi.softAPdisconnect(true);   //  close AP
}
void status_func(int status, const char *msg) {    //    ------------------------ Status --------------------------
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
  //Serial.print("GUID : ");Serial.println(read_guid());
  //Serial.print("AUTHKEY : ");Serial.println(read_authkey());
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
void tick() {
  int state = digitalRead(LED_PIN);
  digitalWrite(LED_PIN, !state);
}
void setup() {
  wifi_set_sleep_type(NONE_SLEEP_T);
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY, 1);
  //WiFi.setOutputPower(17.5);
  delay(10);
  pinMode(wificonfig_pin, INPUT_PULLUP);
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
    if (LittleFS.exists("/config.json")) {
      File configFile = LittleFS.open("/config.json", "r");
      if (configFile) {
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
          if (json.containsKey("term_temp")) {
            strcpy(term_temp, json["term_temp"]);
            Term_Temp = (String(term_temp).toFloat());
            if (Term_Temp < 0.1) {
              Term_Temp = 0.1;
            }
            else if (Term_Temp > 50.0) {
              Term_Temp = 50.0;
            }
          }
          if (json.containsKey("term_temp")) {
            strcpy(term_hist, json["term_hist"]);
            Term_Hist = (String(term_hist).toFloat());
            if (Term_Hist < 0.1) {
              Term_Hist = 0.1;
            }
            else if (Term_Hist > 50.0) {
              Term_Hist = 50.0;
            }
          }
          if (json.containsKey("Router_SSID")) strcpy(Router_SSID, json["Router_SSID"]);
          if (json.containsKey("Router_Pass")) strcpy(Router_Pass, json["Router_Pass"]);
        } else {
          initialConfig = true;
        }
        configFile.close();
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }

  Serial.print("Term_Temp: "); Serial.println(Term_Temp);
  Serial.print("Term_Hist: "); Serial.println(Term_Hist);

  wifi_station_set_hostname(Supla_name);
  WiFi.mode(WIFI_STA);
  EEPROM.get(381, ThermostatTemperature);
  if ((ThermostatTemperature > 1000.0) || isnan(ThermostatTemperature)) {
    ThermostatTemperature = 20.0;
    EEPROM.put(381, ThermostatTemperature);
    EEPROM.commit();
    Serial.println("ThermostatTemperature reset to 20.0ºC");
  }

  relay_0 =  new Supla::Control::VirtualRelay(32);
  relay_0->getChannel()->setDefault(SUPLA_CHANNELFNC_POWERSWITCH);
  relay_0->setDefaultStateRestore();
  relay_0->disableChannelState();
  relay_1 =  new Supla::Control::Relay(relay_pin, true);
  relay_1->getChannel()->setDefault(SUPLA_CHANNELFNC_POWERSWITCH);
  relay_1->setDefaultStateOff();
  relay_1->disableChannelState();
  MaxTemp = new Supla::Sensor::MAXThermocouple(pin_CLK, pin_CS, pin_DO);
  TerTemp = new TermostatTemp();
  TerSet = new TermostatSet();
  TerSet->getChannel()->setDefault(SUPLA_CHANNELFNC_POWERSWITCH);
  TerSet->disableChannelState();
  auto button_1 = new Supla::Control::Button(button_pin, true, true);
  button_1->addAction(Supla::TOGGLE, relay_0, Supla::ON_PRESS);
  button_1->setSwNoiseFilterDelay(50);
  relay_0->addAction(Supla::TURN_OFF, relay_1, Supla::ON_TURN_OFF);

  SuplaDevice.setName(Supla_name);
  wifi.setSsid(Router_SSID);
  wifi.setPassword(Router_Pass);
  SuplaDevice.setStatusFuncImpl(&status_func);
  wifi.enableSSL(false);
  SuplaDevice.begin(GUID, Supla_server, Email, AUTHKEY);

}

void loop() {
  if (initialConfig == true) {
    ondemandwifiCallback();
  }

  int C_W_read = digitalRead(wificonfig_pin); {
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

  if (WiFi.status() == WL_CONNECTED) {
    if (starting) {
      httpUpdater.setup(&httpServer, "/update", "admin", "pass");
      httpServer.begin();
      starting = false;
      Serial.print("get MAXThermocouple temperature: ");
      Serial.println(MaxTemp->getValue());
    }
    httpServer.handleClient();
  } else {
    if ((millis() - wifimilis) > wifi_checkDelay) {
      WiFi.begin(Router_SSID, Router_Pass);
      Serial.println("CONNECT WIFI!!");
      wifimilis = millis() ;
      ticker.attach(0.5, tick);
      tikOn = true;
    }
  }

  SuplaDevice.iterate();
  delay(25);

  if (doSaveTermSetTemp) {
    EEPROM.put(381, ThermostatTemperature);
    EEPROM.commit();
    doSaveTermSetTemp = false;
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
