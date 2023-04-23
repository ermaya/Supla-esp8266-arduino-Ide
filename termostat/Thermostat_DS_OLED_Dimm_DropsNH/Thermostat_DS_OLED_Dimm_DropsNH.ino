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
#include <ESP8266TrueRandom.h>
#include <Ticker.h>
#include <ESPRotary.h>
#include <SuplaDevice.h>
#include <supla/control/relay.h>
#include <supla/control/virtual_relay.h>
#include <supla/control/button.h>
#include <supla/sensor/DS18B20.h>
#include <supla/control/dimmer_base.h>
#include <supla/network/esp_wifi.h>
#define STORAGE_OFFSET 512
#include <supla/storage/eeprom.h>
Supla::Eeprom eeprom(STORAGE_OFFSET);
Supla::ESPWifi wifi("SSID", "PASS");  //------ Do not change----wifimanager takes care------
//#include "SSD1306Wire.h" //----- 0.96 Oled --- https://github.com/ThingPulse/esp8266-oled-ssd1306
//SSD1306Wire  display(0x3c, 0, 2);  // D3-SDA  D4-SCL ----- 0.96 Oled ---
#include "SH1106Wire.h" //----- 1.3 Oled ---
SH1106Wire display(0x3c, 0, 2);  // D3-SDA  D4-SCL ----- 1.3 Oled ---
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


int relay_pin = 5;       //D1 --  5
int ds_pin = 4;          //D2 --  4
int button_pin = 14;     //D5 --  14
#define ROTARY_PIN1 13   //D7 --  13
#define ROTARY_PIN2 12   //D6 --  12

bool relayOnLevel = LOW; // "HIGH" for active high relay, "LOW" for active low relay.
bool Protected = false;   // if "true" disables the possibility of activating the relay in manual mode.
long C_W_delay = 10000;   // ---------------------- config delay 10 seconds ---------------------------

int C_W_state = HIGH;
int last_C_W_state = HIGH;
unsigned long time_last_C_W_change = 0;
char Supla_server[81] = ("svr?.supla.org");
char Email[81] = ("your@email");
char Supla_name[51] = ("Thermostat");
char term_hist[5] = ("1.0");
char Supla_status[51] = ("No server address");
char Router_SSID[32];
char Router_Pass[64];
bool shouldSaveConfig = false;
bool initialConfig = false;
bool starting = true;
int s;
char GUID[SUPLA_GUID_SIZE];
char AUTHKEY[SUPLA_AUTHKEY_SIZE];
byte uuidNumber[16];
static const char logo[] PROGMEM = "<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>";
double ThermostatTemperature = 20.5;
double actualTemp = -127.0;
//bool doSaveTermSetTemp = false;
float Term_Hist = 1.0;
int TempDisp_mtbs = 5000;
unsigned long TempDisp_lasttime;
int dimm_mtbs = 15000;
unsigned long dimm_lasttime;
bool WiFiConected = false;
bool dimm = false;

WiFiManager wifiManager;
Ticker ticker;
ESPRotary r = ESPRotary(ROTARY_PIN1, ROTARY_PIN2, 4);

Supla::Control::VirtualRelay *relay_0 = nullptr;
Supla::Sensor::DS18B20 *DsTemp = nullptr;
Supla::Control::RGBWBase *miDimmToTemp = nullptr;

const uint8_t logo32_glcd_bmp[] PROGMEM =  //logo supla 32
{
  0x00, 0x00, 0x00, 0x00, 0xC0, 0x1F, 0x00, 0x00, 0xE0, 0x7F, 0x00, 0x00,
  0xF0, 0x70, 0x00, 0x00, 0x30, 0xE0, 0x00, 0x00, 0x38, 0xC0, 0x00, 0x00,
  0x18, 0xC0, 0x01, 0x00, 0x18, 0xC0, 0x01, 0x00, 0x38, 0xC0, 0x00, 0x00,
  0x38, 0xE0, 0x01, 0x00, 0x70, 0xF0, 0x07, 0x00, 0xE0, 0x7F, 0x0E, 0x00,
  0xC0, 0x3F, 0x38, 0x00, 0x00, 0x1F, 0xE0, 0x0F, 0x00, 0x18, 0xC0, 0x1F,
  0x00, 0x18, 0xC0, 0x30, 0x00, 0x18, 0xC0, 0x30, 0x00, 0x30, 0xC0, 0x30,
  0x00, 0x30, 0x80, 0x1F, 0x00, 0x30, 0xC0, 0x0F, 0x00, 0x20, 0x60, 0x00,
  0x00, 0x60, 0x20, 0x00, 0x00, 0x60, 0x30, 0x00, 0x00, 0x40, 0x18, 0x00,
  0x00, 0xC0, 0x0D, 0x00, 0x00, 0xC0, 0x07, 0x00, 0x00, 0x60, 0x04, 0x00,
  0x00, 0x20, 0x0C, 0x00, 0x00, 0x20, 0x0C, 0x00, 0x00, 0x60, 0x06, 0x00,
  0x00, 0xC0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00
};
const uint8_t logo16_wifi_bmp[] PROGMEM =  //logo wifi 16
{
  0x00, 0x00, 0x00, 0x00, 0xE0, 0x07, 0x38, 0x1C, 0xC4, 0x23, 0x72, 0x4E,
  0x08, 0x10, 0xE4, 0x27, 0x10, 0x0C, 0x90, 0x09, 0x40, 0x02, 0x60, 0x06,
  0x40, 0x02, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00
};
const uint8_t logo16_supla_bmp[] PROGMEM =  //logo supla 16
{
  0x30, 0x00, 0x7C, 0x00, 0xC4, 0x00, 0x86, 0x00, 0x84, 0x00, 0xDC, 0x03,
  0x78, 0x36, 0x60, 0x78, 0x40, 0x48, 0x40, 0x38, 0x40, 0x04, 0x80, 0x06,
  0x80, 0x03, 0x40, 0x02, 0xC0, 0x03, 0x00, 0x01
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
const uint8_t hand16_bmp[] PROGMEM =  //logo hand 16
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x03, 0x30, 0x02, 0x08, 0x3F,
  0x04, 0x40, 0x04, 0x7C, 0x02, 0x40, 0x02, 0x7C, 0x02, 0x60, 0x04, 0x3C,
  0x18, 0x10, 0xE0, 0x0F, 0x00, 0x00, 0x00, 0x00
};
class ManualRelay : public Supla::Control::Relay {
  public:
    ManualRelay() : Relay(-1, true, 32) {}
    void onInit() {
      digitalWrite(relay_pin, !relayOnLevel);
      pinMode(relay_pin, OUTPUT);
    }
    void turnOn(_supla_int_t duration) {
      if (!relay_0->isOn() && Protected) {
        digitalWrite(relay_pin, !relayOnLevel);
        channel.setNewValue(false);
        Serial.println("*** Protected!! relay will not be activated in manual mode ***");
      } else {
        digitalWrite(relay_pin, relayOnLevel);
        channel.setNewValue(true);
        Serial.println("*** relay On ***");
      }
    }
    void turnOff(_supla_int_t duration) {
      digitalWrite(relay_pin, !relayOnLevel);
      channel.setNewValue(false);
      Serial.println("*** relay Off ***");
    }
    bool isOn() {
      if (relayOnLevel) {
        return digitalRead(relay_pin);
      } else {
        return !digitalRead(relay_pin);
      }
    }
}; ManualRelay *relay_1 = nullptr;
class TermostatTemp : public Supla::Sensor::Thermometer {
  public:
    TermostatTemp() {}

    void onInit() {
      channel.setNewValue(getValue() );
    }
    double getValue() {
      return ThermostatTemperature;
    }
    void iterateAlways() {
      channel.setNewValue(getValue() );
      if ( millis() - lastReadTime > 2500) {
        lastReadTime = millis();
        actualTemp = DsTemp->getValue();
        if (relay_0->isOn()) {
          if (((relay_1->isOn()) && ( actualTemp >= ThermostatTemperature)) || ( actualTemp < -100.0)) {
            relay_1->turnOff(0);
          } else if ((!relay_1->isOn()) && ( actualTemp <= (ThermostatTemperature - Term_Hist))) {
            relay_1->turnOn(0);
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
      ThermostatTemperature += 0.5;
      if (ThermostatTemperature > 50.0) ThermostatTemperature = 50.0;
      channel.setNewValue(false);
      miDimmToTemp->setRGBW(-1, -1, -1, -1, (ThermostatTemperature * 2));
    }
    void turnOff(_supla_int_t duration) {
      ThermostatTemperature -= 0.5;
      if (ThermostatTemperature < 0.0) ThermostatTemperature = 0.0;
      channel.setNewValue(false);
      miDimmToTemp->setRGBW(-1, -1, -1, -1, (ThermostatTemperature * 2));
    }
    bool isOn() {
      return false;
    }
}; TermostatSet *TerSet = nullptr;

enum addedActions {unDimm};
class addedActionsClass : public Supla::ActionHandler {
  public: addedActionsClass() {};
    void handleAction(int event, int action) {
      if (action == unDimm) {
        if (dimm) {
          dimm = false ;
          dimm_lasttime = millis();
          display.setContrast(200, 241, 64);
        } else {
          dimm_lasttime = millis();
        }
      }
    }
};
addedActionsClass *custAct = new addedActionsClass;

class DimmToTemp : public Supla::Control::DimmerBase {
  public:
    DimmToTemp() { }

    void setRGBWValueOnDevice(uint32_t red,
                              uint32_t green,
                              uint32_t blue,
                              uint32_t colorBrightness,
                              uint32_t brightness) {
      float dimmTemp = map(brightness, 0, 1023, 0, 100);
      ThermostatTemperature = dimmTemp * 0.5;
    }
};

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void ondemandwifiCallback () {

  display.clear();
  display.setContrast(200, 241, 64);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 16, "wifi config connect");
  display.drawString(64, 28, "to wifi hotspot");
  display.setFont(ArialMT_Plain_16);
  display.drawString(64, 44, "Thermostat");
  display.display();

  wifi.setSsid("");
  wifi.setPassword("");
  wifiManager.setConfigPortalBlocking(true);
  wifiManager.setCleanConnect(true);
  wifiManager.setBreakAfterConfig(true);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setCustomHeadElement(logo);
  wifiManager.setMinimumSignalQuality(8);
  wifiManager.setConfigPortalTimeout(300);
  std::vector<const char *> menu = {"wifi", "wifinoscan", "sep", "update", "sep", "info", "restart"};
  wifiManager.setMenu(menu);
  wifiManager.setTitle("&#66&#121&#46&#46&#69&#108&#77&#97&#121&#97");

  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 81, "required");
  WiFiManagerParameter custom_Email("email", "Email", Email, 81, "required");
  WiFiManagerParameter custom_Supla_name("name", "Supla Device Name", Supla_name, 51, "required");
  WiFiManagerParameter custom_term_hist("histerisis", "Histerisis", term_hist, 5, "required");
  WiFiManagerParameter custom_html_id21("<div><h4> - Supla State -   ");
  WiFiManagerParameter custom_html_id22(Supla_status);
  WiFiManagerParameter custom_html_id23("</h4></div>");

  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_Email);
  wifiManager.addParameter(&custom_Supla_name);
  wifiManager.addParameter(&custom_term_hist);
  wifiManager.addParameter(&custom_html_id21);
  wifiManager.addParameter(&custom_html_id22);
  wifiManager.addParameter(&custom_html_id23);

  if (!wifiManager.startConfigPortal("Thermostat")) {
    Serial.println("Not connected to WiFi but continuing anyway.");
  } else {
    Serial.println("connected...yeey :)");
  }
  strcpy(Supla_server, custom_Supla_server.getValue());
  strcpy(Email, custom_Email.getValue());
  strcpy(Supla_name, custom_Supla_name.getValue());
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
  if (initialConfig == true) {
    shouldSaveConfig = false;
    initialConfig = false;
    WiFi.mode(WIFI_STA);
    delay(5000);
    ESP.restart();
  }
}
void status_func(int status, const char *msg) {    //    ------------------------ Status --------------------------
  if (s != status) {
    s = status;
    if (s != 10) {
      strcpy(Supla_status, msg);
    }
  }
}

void TH_Overlay() {

  display.clear();
  if (actualTemp > -100) {
    display.setFont(Arimo_Bold_32);
    //display.setFont(ArialMT_Plain_24);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 14, String(actualTemp, 1) + "ºC");
  } else {
    display.setFont(Arimo_Bold_32);
    //display.setFont(ArialMT_Plain_24);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 14, "-----");
  }

  if (relay_0->isOn()) {
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 48, "Nastawa" + String(ThermostatTemperature, 1) + "ºC");
    display.drawString(61, 0, "Auto");
  } else {
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 48, "Nastawa" + String(ThermostatTemperature, 1) + "ºC");
    display.drawXbm(52, 0, 16, 16, hand16_bmp);
  }
  if (WiFiConected) {
    display.drawXbm(1, 0, 16, 16, logo16_wifi_bmp);// -------------------------------------------------- oled wifi ok  --------

    if (s == STATUS_REGISTERED_AND_READY) {
      display.drawXbm(22, 0, 16, 16, logo16_supla_bmp);// ------------------------------------------------ oled supla ok --------
    } else if (s != 10) {
      display.setFont(ArialMT_Plain_16);
      display.drawString(30, 0, String(s));
    }
  }
  if (relay_1->isOn()) {
    display.drawXbm(86, 0, 40, 16, logo_Power_on);// -------------------------------------------------- Relay on  ---------
  } else {
    display.drawXbm(86, 0, 40, 16, logo_Power_off);// ------------------------------------------------- Relay off  --------
  }
  display.display();
  TempDisp_lasttime = millis();
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

void encoder() {
  r.loop();
}

void setup() {
  wifi_set_sleep_type(NONE_SLEEP_T);
  pinMode(button_pin, INPUT_PULLUP);
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY, 1);
  display.init();
  display.flipScreenVertically();
  display.clear();
  display.setFont(ArialMT_Plain_24);
  display.drawString(42, 6, "Supla");
  display.setFont(ArialMT_Plain_16);
  display.drawString(33, 40, "Thermostat");
  display.drawXbm(0, 16, 32, 32, logo32_glcd_bmp );
  display.display();
  delay(10);
  EEPROM.begin(1024);
  if (EEPROM.read(300) != 60) {
    initialConfig = true;
  }
  guid_authkey();

  if (LittleFS.begin()) {  // ------------------------- wificonfig read -----------------
    if (LittleFS.exists("/config.json")) {
      File configFile = LittleFS.open("/config.json", "r");
      if (configFile) {
        DynamicJsonDocument json(1024);
        DeserializationError deserializeError = deserializeJson(json, configFile);
        serializeJsonPretty(json, Serial);
        if (!deserializeError) {
          Serial.println("\nparsed json");
          if (json.containsKey("Supla_server")) strcpy(Supla_server, json["Supla_server"]);
          if (json.containsKey("Email")) strcpy(Email, json["Email"]);
          if (json.containsKey("Supla_name")) strcpy(Supla_name, json["Supla_name"]);
          if (json.containsKey("term_hist")) {
            strcpy(term_hist, json["term_hist"]);
            Term_Hist = (String(term_hist).toFloat());
            if (Term_Hist < 0.1) {
              Term_Hist = 0.1;
            }
            else if (Term_Hist > 10.0) {
              Term_Hist = 10.0;
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

  Serial.print("Term_Hist: "); Serial.println(Term_Hist);

  wifi_station_set_hostname(Supla_name);
  WiFi.mode(WIFI_STA);

  if (initialConfig) {
    ondemandwifiCallback();
  }

  relay_0 = new Supla::Control::VirtualRelay(32);
  relay_0->getChannel()->setDefault(SUPLA_CHANNELFNC_POWERSWITCH);
  relay_0->setDefaultStateRestore();
  relay_0->disableChannelState();
  relay_1 = new ManualRelay();
  relay_1->getChannel()->setDefault(SUPLA_CHANNELFNC_POWERSWITCH);
  relay_1->setDefaultStateOff();
  relay_1->disableChannelState();
  DsTemp = new Supla::Sensor::DS18B20(ds_pin);
  TerTemp = new TermostatTemp();
  TerTemp->disableChannelState();
  TerSet = new TermostatSet();
  TerSet->getChannel()->setDefault(SUPLA_CHANNELFNC_POWERSWITCH);
  TerSet->disableChannelState();
  auto button_1 = new Supla::Control::Button(button_pin, true, true);
  button_1->setMulticlickTime(800, false);
  button_1->addAction(Supla::TOGGLE, relay_0, Supla::ON_CLICK_1);
  if (!Protected)button_1->addAction(Supla::TOGGLE, relay_1, Supla::ON_CLICK_2);
  button_1->addAction(unDimm, custAct, Supla::ON_PRESS);
  relay_0->addAction(Supla::TURN_OFF, relay_1, Supla::ON_TURN_OFF);
  miDimmToTemp = new DimmToTemp();
  miDimmToTemp->disableChannelState();

  r.setLeftRotationHandler(DirectionLeft);
  r.setRightRotationHandler(DirectionRight);

  SuplaDevice.setName(Supla_name);
  wifi.setSsid(Router_SSID);
  wifi.setPassword(Router_Pass);
  SuplaDevice.setStatusFuncImpl(&status_func);
  wifi.enableSSL(false);
  SuplaDevice.begin(GUID, Supla_server, Email, AUTHKEY);

  ticker.attach_ms(8, encoder);

}

void loop() {

  int C_W_read = digitalRead(button_pin); {
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
  delay(15);

  if ((millis() -  TempDisp_lasttime) > TempDisp_mtbs)  {
    WiFiConected = (WiFi.status() == WL_CONNECTED);
    TempDisp_mtbs = 500;
    TH_Overlay();
  }
  if ((millis() - dimm_lasttime) > dimm_mtbs) {
    if (!dimm) {
      display.setContrast(50, 50, 30);
      display.display();
      dimm = true ;
    }
    dimm_lasttime = millis();
  }
}

void DirectionLeft(ESPRotary& r) {
  if (dimm) {
    dimm = false ;
    dimm_lasttime = millis();
    display.setContrast(200, 241, 64);
  } else {
    TerSet->turnOff(0);
    dimm_lasttime = millis();
  }
}
void DirectionRight(ESPRotary& r) {
  if (dimm) {
    dimm = false ;
    dimm_lasttime = millis();
    display.setContrast(200, 241, 64);
  } else {
    TerSet->turnOn(0);
    dimm_lasttime = millis();
  }
}
