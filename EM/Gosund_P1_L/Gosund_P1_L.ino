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
#define supla_lib_config_h_  // silences debug messages "should be
                             // disabled by default"

#include <ArduinoJson.h>  //--------- V6 ------
#include <Button3.h>
#include <EEPROM.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266TrueRandom.h>
#include <ESP8266WebServer.h>
#include <SuplaDevice.h>
#include <Ticker.h>
#include <WiFiManager.h>
#include <supla/control/relay.h>
#include <supla/io.h>
#include <supla/sensor/one_phase_electricity_meter.h>

#include "CSE7766.h"
#include "LittleFS.h"
#define STORAGE_OFFSET 512
#include <supla/storage/eeprom.h>
Supla::Eeprom eeprom(STORAGE_OFFSET);
#include <supla/network/esp_wifi.h>
Supla::ESPWifi wifi("a","b");  //- Do not edit-wifimanager takes care-
extern "C" {
#include "user_interface.h"
}

/*
 * Gosund P1
GPIO #  Component
GPIO 01  CSE7766 Tx
GPIO 02  LedLink
GPIO 03  CSE7766 Rx
GPIO 04  USB overload
GPIO 05  Relay4i
GPIO 12  Relay2
GPIO 13  Relay3
GPIO 14  Relay1
GPIO 16  Button1
 */

#define CSE7766_RX_PIN 3
#define status_led     2
#define power_led      0
#define overload_pin   4
#define relay_pin_1    14
#define relay_pin_2    12
#define relay_pin_3    13
#define relay_pin_4    5
#define wificonfig_pin 16
int C_W_state = HIGH;
int last_C_W_state = HIGH;
unsigned long time_last_C_W_change = 0;
long C_W_delay = 5000;  // config delay 5 seconds
int overload_state = HIGH;
long overload_delay = 100;
int last_overload_state = HIGH;
unsigned long time_last_overload_change = 0;
bool usb_overload = false;
bool tik_overload = false;
unsigned long mem_energy_milis = 30000;
bool noButton = false;
char Supla_server[81] = ("Set server address");
char Email[81] = ("set email address");
char Supla_name[51];
char Supla_status[51];
char Router_SSID[32];
char Router_Pass[64];
char Volt[8];
char Wats[8];
char setWh[10];
bool shouldSaveConfig = false;
bool initialConfig = false;
int s = 0;
bool starting = true;
bool Tik = true;
uint64_t mem_energy = 0;
uint64_t _mem_energy = 0;
char GUID[SUPLA_GUID_SIZE];
char AUTHKEY[SUPLA_AUTHKEY_SIZE];
byte uuidNumber[16];
static const char logo[] PROGMEM =
    "<style>html{ background-color: #01DF3A;}</style><div class='s'><svg "
    "version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' "
    "xml:space='preserve'><path "
    "d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-"
    "0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,"
    "1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0."
    "1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-"
    "0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,"
    "16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12."
    "2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8."
    "6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-"
    "19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-"
    "14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1."
    "5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z "
    "M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35."
    "5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,"
    "0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6."
    "1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3."
    "5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,"
    "4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z "
    "M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,"
    "2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67."
    "1,89,55.9,89,42.6z "
    "M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-"
    "9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,"
    "188.6z "
    "M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,"
    "10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></"
    "svg>";

ESP8266WebServer httpServer(81);
ESP8266HTTPUpdateServer httpUpdater;
Ticker ticker;
Ticker powerLed;
Ticker analogBtn;
Button3 buttonA = Button3(wificonfig_pin);

Supla::Control::Relay *relay1 = nullptr;
Supla::Control::Relay *relay2 = nullptr;
Supla::Control::Relay *relay3 = nullptr;
Supla::Control::Relay *relay4 = nullptr;

#define CSE7766_CURRENT_RATIO 16030
#define CSE7766_VOLTAGE_RATIO 190770
#define CSE7766_POWER_RATIO   5195000
CSE7766 myCSE7766;

class MyCSE7766 : public Supla::Sensor::OnePhaseElectricityMeter {
 public:
  MyCSE7766() {
  }
  void onInit() {
    readValuesFromDevice();
    updateChannelValues();
  }

  virtual void readValuesFromDevice() {
    myCSE7766.handle();
    // mem_energy = 0.01 Wh
    mem_energy = _mem_energy + (myCSE7766.getEnergy() / 36);
    // voltage in 0.01 V
    setVoltage(0, myCSE7766.getVoltage() * 100);
    // current in 0.001 A
    setCurrent(0, myCSE7766.getCurrent() * 1000);
    // power in 0.00001 kW
    setPowerActive(0, myCSE7766.getActivePower() * 100000);
    // energy in 0.00001 kWh
    setFwdActEnergy(0, mem_energy);
    // power in 0.00001 kVA
    setPowerApparent(0, myCSE7766.getApparentPower() * 100000);
    // power in 0.00001 kvar
    setPowerReactive(0, myCSE7766.getReactivePower() * 100000);
    // power in 0.001
    setPowerFactor(0, myCSE7766.getPowerFactor() * 1000);
    // float powerFactor = 0; setPowerFactor(0, powerFactor * 1000);
  }
};
MyCSE7766 *cse = nullptr;

class CustomControl : public Supla::Io {
 public:
  void customDigitalWrite(int channelNumber, uint8_t pin, uint8_t val) {
    if ((pin == relay_pin_4) && (usb_overload)) {
      usb_overload = false;  // reset usb overload
    }
    return ::digitalWrite(pin, val);
  }
} CustomControl;

void tick() {
  int state = digitalRead(status_led);
  digitalWrite(status_led, !state);
}

void tikPowerLed() {
  int state = digitalRead(power_led);
  digitalWrite(power_led, !state);
}

void tikAnalogBtn() {
  buttonA.loop();

  int a0in = analogRead(A0);
  if (a0in > 930) {
    noButton = true;
    return;
  } else if ((a0in > 600) && (a0in < 920) && (noButton)) {
    relay1->toggle();
    noButton = false;
    return;
  } else if ((a0in > 350) && (a0in < 600) && (noButton)) {
    relay2->toggle();
    noButton = false;
    return;
  } else if ((a0in > 150) && (a0in < 350) && (noButton)) {
    relay3->toggle();
    noButton = false;
    return;
  }
}

void calibrate_CSE7766() {
  int ex_volt = atoi(Volt);
  int ex_pow = atoi(Wats);
  double ex_amps = ((float)ex_pow / (float)ex_volt);
  digitalWrite(relay_pin_1, HIGH);
  digitalWrite(relay_pin_2, HIGH);
  digitalWrite(relay_pin_3, HIGH);
  digitalWrite(relay_pin_4, HIGH);
  myCSE7766.handle();
  unsigned long timeout1 = millis();
  while ((millis() - timeout1) < 10000) {
    delay(10);
  }
  myCSE7766.handle();
  myCSE7766.expectedPower(ex_pow);
  myCSE7766.expectedVoltage(ex_volt);
  myCSE7766.expectedCurrent(ex_amps);
  unsigned long timeout2 = millis();
  while ((millis() - timeout2) < 2000) {
    delay(10);
  }
  double current_multi = myCSE7766.getCurrentRatio();
  double voltage_multi = myCSE7766.getVoltageRatio();
  double power_multi = myCSE7766.getPowerRatio();
  EEPROM.put(400, current_multi);
  EEPROM.put(410, voltage_multi);
  EEPROM.put(420, power_multi);
  EEPROM.write(430, 60);
  EEPROM.commit();
  yield();
}

void get_CSE7766_config() {
  if (EEPROM.read(430) == 60) {
    double p_m;
    double v_m;
    double c_m;
    EEPROM.get(400, c_m);
    EEPROM.get(410, v_m);
    EEPROM.get(420, p_m);
    myCSE7766.setCurrentRatio(c_m);
    myCSE7766.setVoltageRatio(v_m);
    myCSE7766.setPowerRatio(p_m);
  }
}

void energy_save() {
  EEPROM.put(460, mem_energy);
  EEPROM.commit();
}

void loopPowerLed() {
  if (usb_overload) {
    if (!tik_overload) {
      powerLed.attach(0.8, tikPowerLed);
      tik_overload = true;
    }
  } else {
    if (tik_overload) {
      powerLed.detach();
      tik_overload = false;
    }
    if (digitalRead(power_led) == HIGH) {
      if ((relay1->isOn()) || (relay2->isOn()) || (relay3->isOn()) ||
          (relay4->isOn())) {
        digitalWrite(power_led, LOW);
        mem_energy_milis = millis() + 600000;
      }
    } else {
      if ((!relay1->isOn()) && (!relay2->isOn()) && (!relay3->isOn()) &&
          (!relay4->isOn())) {
        digitalWrite(power_led, HIGH);
        energy_save();
      }
    }
  }
}

void saveConfigCallback() {
  shouldSaveConfig = true;
}

void ondemandwifiCallback() {
  ticker.detach();
  digitalWrite(status_led, LOW);
  WiFiManager wifiManager;

  WiFiManagerParameter custom_Supla_server(
      "server", "supla server", Supla_server, 80, "required");
  WiFiManagerParameter custom_Email("email", "Email", Email, 80, "required");
  WiFiManagerParameter custom_Supla_name(
      "name", "Supla Device Name", Supla_name, 50, "required");
  WiFiManagerParameter custom_Volt("Volt", "expectedVoltage", Volt, 3);
  WiFiManagerParameter custom_Wats("Wats", "expectedActivePower", Wats, 4);
  WiFiManagerParameter custom_setWh("setWh", "set counter energy Wh", setWh, 9);
  WiFiManagerParameter custom_html_id21("<div><h4> - Supla State -   ");
  WiFiManagerParameter custom_html_id22(Supla_status);
  WiFiManagerParameter custom_html_id23("</h4></div>");

  wifiManager.setBreakAfterConfig(true);
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_Email);
  wifiManager.addParameter(&custom_Supla_name);
  wifiManager.addParameter(&custom_Volt);
  wifiManager.addParameter(&custom_Wats);
  wifiManager.addParameter(&custom_setWh);
  wifiManager.addParameter(&custom_html_id21);
  wifiManager.addParameter(&custom_html_id22);
  wifiManager.addParameter(&custom_html_id23);

  wifiManager.setCustomHeadElement(logo);
  wifiManager.setMinimumSignalQuality(8);
  wifiManager.setConfigPortalTimeout(300);

  if (!wifiManager.startConfigPortal("GosundP1")) {
    Serial.println("Not connected to WiFi but continuing anyway.");
  } else {
    Serial.println("connected...yeey :)");
  }
  strcpy(Supla_server, custom_Supla_server.getValue());
  strcpy(Email, custom_Email.getValue());
  strcpy(Supla_name, custom_Supla_name.getValue());
  strcpy(Volt, custom_Volt.getValue());
  strcpy(Wats, custom_Wats.getValue());
  strcpy(setWh, custom_setWh.getValue());
  wifiManager.getWiFiSSID().toCharArray(Router_SSID, 33);
  wifiManager.getWiFiPass().toCharArray(Router_Pass, 65);

  if (strcmp(Supla_server, "get_new_guid_and_authkey") == 0) {
    EEPROM.write(300, 0);
    EEPROM.commit();
    ESP.reset();
  }
  if (strlen(setWh) != 0) {
    _mem_energy = atol(setWh);
    _mem_energy = _mem_energy * 100;
    mem_energy = _mem_energy;
    energy_save();
  }
  if ((strlen(Volt) != 0) && (strlen(Wats) != 0)) {
    calibrate_CSE7766();
  }

  if (shouldSaveConfig ==  true) { 
    DynamicJsonDocument json(1024);
    json["Supla_server"] = Supla_server;
    json["Email"] = Email;
    json["Supla_name"] = Supla_name;
    json["Router_SSID"] = Router_SSID;
    json["Router_Pass"] = Router_Pass;
    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    serializeJson(json, configFile);
    delay(2000);
    configFile.close();
    shouldSaveConfig = false;
    initialConfig = false;
    WiFi.mode(WIFI_STA);
    ticker.detach();
    digitalWrite(status_led, LOW);
    delay(5000);
    ESP.restart();
  }
  shouldSaveConfig = false;
  initialConfig = false;
  WiFi.mode(WIFI_STA);
  ticker.detach();
  digitalWrite(status_led, LOW);
  delay(5000);
  ESP.restart();
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
    EEPROM.put(460, mem_energy);
    EEPROM.commit();
  }
  read_guid();
  read_authkey();
}

String read_guid(void) {
  String read_eeprom = "";
  int i, ii = 0;
  int eep_star = 301;
  int end_guid = eep_star + SUPLA_GUID_SIZE;
  String temp_read = "0x";
  for (i = eep_star; i < end_guid + 16; i = i + 1) {
    temp_read += char(EEPROM.read(i));
    read_eeprom += char(EEPROM.read(i));
    if ((i % 2) == 0) {
      char *_guid =
          strcpy((char *)malloc(temp_read.length() + 1), temp_read.c_str());
      GUID[ii] = strtoul(_guid, NULL, 16);
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
  for (i = eep_star; i < end_authkey + 16; i = i + 1) {
    temp_read += char(EEPROM.read(i));
    read_eeprom += char(EEPROM.read(i));
    if ((i % 2) == 0) {
      char *_authkey =
          strcpy((char *)malloc(temp_read.length() + 1), temp_read.c_str());
      AUTHKEY[ii] = strtoul(_authkey, NULL, 16);
      temp_read = "0x";
      ii++;
    }
  }
  return read_eeprom;
}
void setup() {
  wifi_set_sleep_type(NONE_SLEEP_T);
  EEPROM.begin(1024);
  pinMode(wificonfig_pin, INPUT_PULLUP);
  pinMode(overload_pin, INPUT_PULLUP);
  pinMode(status_led, OUTPUT);
  pinMode(power_led, OUTPUT);

  guid_authkey();
  ticker.attach(0.5, tick);

  if (WiFi.SSID() == "") {
    initialConfig = true;
  }

  if (LittleFS.begin()) {  // ------------------------- wificonfig read
                           // -----------------
    if (LittleFS.exists("/config.json")) {
      File configFile = LittleFS.open("/config.json", "r");
      if (configFile) {
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonDocument json(1024);
        DeserializationError deserializeError =
            deserializeJson(json, buf.get());
        if (!deserializeError) {
          Serial.println("\nparsed json");
          if (json.containsKey("Supla_server"))
            strcpy(Supla_server, json["Supla_server"]);
          if (json.containsKey("Email")) strcpy(Email, json["Email"]);
          if (json.containsKey("Supla_name"))
            strcpy(Supla_name, json["Supla_name"]);
          if (json.containsKey("Router_SSID"))
            strcpy(Router_SSID, json["Router_SSID"]);
          if (json.containsKey("Router_Pass"))
            strcpy(Router_Pass, json["Router_Pass"]);
        } else {
          initialConfig = true;
        }
        configFile.close();
      }
    }
  }

  wifi_station_set_hostname(Supla_name);

  myCSE7766.setCurrentRatio(CSE7766_CURRENT_RATIO);
  myCSE7766.setVoltageRatio(CSE7766_VOLTAGE_RATIO);
  myCSE7766.setPowerRatio(CSE7766_POWER_RATIO);
  myCSE7766.setRX(CSE7766_RX_PIN);
  myCSE7766.begin();  // will initialize serial to 4800 bps
  get_CSE7766_config();

  EEPROM.get(460, _mem_energy);
  mem_energy = _mem_energy;
  eeprom.setStateSavePeriod(600000);

  relay1 = new Supla::Control::Relay(relay_pin_1, true, 224);
  relay1->setDefaultStateRestore();
  relay1->keepTurnOnDuration();
  relay2 = new Supla::Control::Relay(relay_pin_2, true, 224);
  relay2->setDefaultStateRestore();
  relay2->keepTurnOnDuration();
  relay3 = new Supla::Control::Relay(relay_pin_3, true, 224);
  relay3->setDefaultStateRestore();
  relay3->keepTurnOnDuration();
  relay4 = new Supla::Control::Relay(relay_pin_4, false, 224);
  relay4->setDefaultStateRestore();
  relay4->keepTurnOnDuration();

  cse = new MyCSE7766();
  analogBtn.attach_ms(100, tikAnalogBtn);
  buttonA.setClickHandler(clickA);
  buttonA.setLongClickHandler(longClickA);
  buttonA.setDoubleClickHandler(doubleClickA);
  buttonA.setTripleClickHandler(tripleClickA);
  buttonA.setQuatleClickHandler(quatleClickA);
  buttonA.setConfigClickHandler(configClickA);

  SuplaDevice.setName(Supla_name);
  wifi.setSsid(Router_SSID);
  wifi.setPassword(Router_Pass);
  SuplaDevice.setStatusFuncImpl(&status_func);
  wifi.enableSSL(false);
  SuplaDevice.begin(GUID, Supla_server, Email, AUTHKEY);
}
char buffer[50];
void loop() {
  if (initialConfig == true) {
    ondemandwifiCallback();
    EEPROM.write(431, 0);
    EEPROM.commit();
  }

  if (relay4->isOn()) {
    int overload_read = digitalRead(overload_pin);
    {
      if (overload_read != last_overload_state) {
        time_last_overload_change = millis();
      }
      if ((millis() - time_last_overload_change) > overload_delay) {
        if (overload_read != overload_state) {
          overload_state = overload_read;
          if (overload_state == LOW) {
            relay4->turnOff();
            usb_overload = true;
          }
        }
      }
      last_overload_state = overload_read;
    }
  }

  SuplaDevice.iterate();
  delay(25);
  loopPowerLed();

  if (WiFi.status() == WL_CONNECTED) {
    if (starting) {
      httpUpdater.setup(
          &httpServer,
          "/update",
          "admin",
          "pass");  // -- set here: path, username and password for OTA. --
      httpServer.begin();
      starting = false;
    }
    if ((Tik == true) && (s == 17)) {
      ticker.detach();
      digitalWrite(status_led, HIGH);
      Tik = false;
    }
    httpServer.handleClient();
  }
  if (((s != 17) || (WiFi.status() != WL_CONNECTED)) && (Tik == false)) {
    ticker.attach(0.5, tick);
    Tik = true;
  }

  if (millis() > mem_energy_milis) {
    if ((mem_energy > _mem_energy) &&
        ((relay1->isOn()) || (relay2->isOn()) || (relay3->isOn()))) {
      energy_save();
      mem_energy_milis = millis() + 600000;
    }
  }
}

void clickA(Button3 &btn) {
  relay1->toggle();
}
void longClickA(Button3 &btn) {
  if ((relay1->isOn()) || (relay2->isOn()) || (relay3->isOn()) ||
      (relay4->isOn())) {
    relay1->turnOff();
    relay2->turnOff();
    relay3->turnOff();
    relay4->turnOff();
  } else {
    relay1->turnOn();
    relay2->turnOn();
    relay3->turnOn();
    relay4->turnOn();
  }
}
void doubleClickA(Button3 &btn) {
  relay2->toggle();
}
void tripleClickA(Button3 &btn) {
  relay3->toggle();
}
void quatleClickA(Button3 &btn) {
  relay4->toggle();
}
void configClickA(Button3 &btn) {
  initialConfig = true;
}
