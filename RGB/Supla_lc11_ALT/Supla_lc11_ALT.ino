#define supla_lib_config_h_  // silences debug messages
#include "LittleFS.h"
#include <WiFiManager.h>
#include <EEPROM.h>
#include <ESP8266TrueRandom.h>
#include <ArduinoJson.h>
#include <Button3.h>
#include <SuplaDevice.h>
#include <supla/control/button.h>
#include <supla/control/rgb_leds.h>
#include <supla/control/dimmer_leds.h>
#include <supla/network/esp_wifi.h>
Supla::ESPWifi wifi("SSID", "PASS");  //------ Do not change----wifimanager takes care------
#define STORAGE_OFFSET 512
#include <supla/storage/eeprom.h>
Supla::Eeprom eeprom(STORAGE_OFFSET);

#define RED_PIN              5
#define GREEN_PIN            4
#define BLUE_PIN             14
#define BRIGHTNESS_1_PIN     13
#define BRIGHTNESS_2_PIN     12
#define BUTTON_PIN           0
#define RF_IN_PIN            15
#define RF_ENABLE_PIN        2

int C_W_state = HIGH;
int last_C_W_state = HIGH;
unsigned long time_last_C_W_change = 0;
long C_W_delay = 10000;               // ---------------------- config delay 10 seconds ---------------------------
char Supla_server[81] = ("Set server address");
char Email[81] = ("set email address");
char Supla_name[81] = ("SuplaLC11");
char Router_SSID[32];
char Router_Pass[64];
char Supla_status[51];
bool shouldSaveConfig = false;
bool initialConfig = false;
int s;
char GUID[SUPLA_GUID_SIZE];
char AUTHKEY[SUPLA_AUTHKEY_SIZE];
byte uuidNumber[16];
static const char logo[] PROGMEM = "<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>";
Button3 buttonA = Button3(BUTTON_PIN);
WiFiManager wifiManager;

Supla::Control::RGBLeds *rgb = nullptr;
Supla::Control::DimmerLeds *dim1 = nullptr;
Supla::Control::DimmerLeds *dim2 = nullptr;

const uint32_t ARILUX_RF_TIME_AVOID_DUPLICATE = 1000;  // Milliseconds
const uint8_t ARILUX_RF_MAX_CHANGES = 51;              // Pulses (sync + 2 x 24 bits)
const uint32_t ARILUX_RF_SEPARATION_LIMIT = 4300;      // Microseconds
const uint32_t ARILUX_RF_RECEIVE_TOLERANCE = 60;       // Percentage
uint8_t rf_code_l = 0;
uint8_t rf_code_h = 0;

struct ARILUX {
  int rf_timings[ARILUX_RF_MAX_CHANGES];

  unsigned long rf_received_value = 0;
  unsigned long rf_last_received_value = 0;
  unsigned long rf_last_time = 0;
  unsigned long rf_lasttime = 0;

  unsigned int rf_change_count = 0;
  unsigned int rf_repeat_count = 0;

  uint8_t rf_toggle = 0;
} Arilux;

ICACHE_RAM_ATTR void AriluxRfInterrupt(void) {
  unsigned long time = micros();
  int duration = time - Arilux.rf_lasttime;

  if (duration > ARILUX_RF_SEPARATION_LIMIT) {
    if (abs(duration - Arilux.rf_timings[0]) < 200) {
      Arilux.rf_repeat_count++;
      if (Arilux.rf_repeat_count == 2) {
        unsigned long code = 0;
        const int delay = Arilux.rf_timings[0] / 31;
        const int delayTolerance = delay * ARILUX_RF_RECEIVE_TOLERANCE / 100;
        for (unsigned int i = 1; i < Arilux.rf_change_count - 1; i += 2) {
          code <<= 1;
          if (abs(Arilux.rf_timings[i] - (delay * 3)) < delayTolerance && abs(Arilux.rf_timings[i + 1] - delay) < delayTolerance) {
            code |= 1;
          }
        }
        if (Arilux.rf_change_count > 49) {  // Need 1 sync bit and 24 data bits
          Arilux.rf_received_value = code;
        }
        Arilux.rf_repeat_count = 0;
      }
    }
    Arilux.rf_change_count = 0;
  }
  if (Arilux.rf_change_count >= ARILUX_RF_MAX_CHANGES) {
    Arilux.rf_change_count = 0;
    Arilux.rf_repeat_count = 0;
  }
  Arilux.rf_timings[Arilux.rf_change_count++] = duration;
  Arilux.rf_lasttime = time;
}

void AriluxRfHandler(void) {
  unsigned long now = millis();
  if (Arilux.rf_received_value && !((Arilux.rf_received_value == Arilux.rf_last_received_value) && (now - Arilux.rf_last_time < ARILUX_RF_TIME_AVOID_DUPLICATE))) {
    Arilux.rf_last_received_value = Arilux.rf_received_value;
    Arilux.rf_last_time = now;

    uint16_t hostcode = Arilux.rf_received_value >> 8 & 0xFFFF;
    if (rf_code_l == rf_code_h) {
      rf_code_l = hostcode >> 8 & 0xFF;
      rf_code_h = hostcode & 0xFF;
      detachInterrupt(RF_IN_PIN);
      EEPROM.write(290, rf_code_l);
      EEPROM.write(292, rf_code_h);
      EEPROM.commit();
      Serial.print("store new rf id: ");
      Serial.print(rf_code_l, HEX);
      Serial.println(rf_code_h, HEX);
      attachInterrupt(RF_IN_PIN, AriluxRfInterrupt, CHANGE);
    }
    uint16_t stored_hostcode = rf_code_l << 8 | rf_code_h;

    Serial.print("proces rf code: ");
    Serial.println(Arilux.rf_received_value, HEX);
    if (hostcode == stored_hostcode) {
      uint8_t  keycode = Arilux.rf_received_value & 0xFF;
      switch (keycode) {
        case 1:  // Power On
          rgb->handleAction(0, Supla::TOGGLE);
          break;
        case 3:  // Power Off
          dim2->handleAction(0, Supla::TOGGLE);
          break;
        case 2:  // Toggle
          dim1->handleAction(0, Supla::TOGGLE);
          break;
        case 4:  // Speed +
          rgb->handleAction(0, Supla::BRIGHTEN_RGB);
          break;
        case 7:  // Speed -
          rgb->handleAction(0, Supla::DIM_RGB);
          break;
        case 5:  // Scheme +
          dim1->handleAction(0, Supla::BRIGHTEN_W);
          break;
        case 8:  // Scheme -
          dim1->handleAction(0, Supla::DIM_W);
          break;
        case 6:  // Dimmer +
          dim2->handleAction(0, Supla::BRIGHTEN_W);
          break;
        case 9:  // Dimmer -
          dim2->handleAction(0, Supla::DIM_W);
          break;
        case 10:  // RED
          rgb->setRGBW(255, 0, 0, 100, 0);
          break;
        case 11:  // GREEN
          rgb->setRGBW(0, 255, 0, 100, 0);
          break;
        case 12:  // BLUE
          rgb->setRGBW(0, 0, 255, 100, 0);
          break;
        case 13:  // ORANGE
          rgb->setRGBW(255, 165, 0, 100, 0);
          break;
        case 14:  // LT GRN
          rgb->setRGBW(144, 238, 144, 100, 0);
          break;
        case 15:  // LT BLUE
          rgb->setRGBW(135, 206, 235, 100, 0);
          break;
        case 16:  // AMBER
          rgb->setRGBW(255, 191, 0, 100, 0);
          break;
        case 17:  // CYAN
          rgb->setRGBW(0, 255, 255, 100, 0);
          break;
        case 18:  // PURPLE
          rgb->setRGBW(128, 0, 128, 100, 0);
          break;
        case 19:  // YELLOW
          rgb->setRGBW(255, 255, 0, 100, 0);
          break;
        case 20:  // PINK
          rgb->setRGBW(255, 105, 180, 100, 0);
          break;
        case 21:  // WHITE
          rgb->setRGBW(255, 255, 255, 100, 0);
          break;
      }
    }
  }
  Arilux.rf_received_value = 0;
}
/*
   RF remote functions:
   +--------+--------+--------+
   |RGB 1/0 | WW 1/0 | CW 1/0 |   TOGGLE on/off
   +--------+--------+--------+
   | RGB +  |  WW +  |  CW +  |   BRIGHTEN
   +--------+--------+--------+
   | RGB -  |  WW -  |  CW -  |   DIM
   +--------+--------+--------+
   |  RED   | GREEN  |  BLUE  |
   +--------+--------+--------+
   | ORANGE | LT GRN | LT BLUE|
   +--------+--------+--------+
   | AMBER  |  CYAN  | PURPLE |
   +--------+--------+--------+
   | YELLOW |  PINK  | WHITE  |
   +--------+--------+--------+
*/

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void ondemandwifiCallback () {

  detachInterrupt(RF_IN_PIN);

  //wifiManager.setDebugOutput(false); // Debug off
  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 81, "required");
  WiFiManagerParameter custom_Email("email", "Email", Email, 81, "required");
  WiFiManagerParameter custom_Supla_name("name", "Supla Device Name", Supla_name, 81, "required");
  WiFiManagerParameter custom_html_id21("<div><h4> - Supla State -   ");
  WiFiManagerParameter custom_html_id22( Supla_status);
  WiFiManagerParameter custom_html_id23( "</h4></div>");

  wifiManager.setBreakAfterConfig(true);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setConfigPortalBlocking(true);

  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_Email);
  wifiManager.addParameter(&custom_Supla_name);
  wifiManager.addParameter(&custom_html_id21);
  wifiManager.addParameter(&custom_html_id22);
  wifiManager.addParameter(&custom_html_id23);

  wifiManager.setCustomHeadElement(logo);
  wifiManager.setMinimumSignalQuality(8);
  wifiManager.setConfigPortalTimeout(300);
  std::vector<const char *> menu = {"wifi", "wifinoscan", "sep", "update", "sep", "info", "restart", "exit"};
  wifiManager.setMenu(menu);
  wifiManager.setTitle("By..ElMaya");

  if (!wifiManager.startConfigPortal("SuplaLC11")) {
    Serial.println("Not connected to WiFi but continuing anyway.");
  } else {
    Serial.println("connected...yeey :)");
  }
  strcpy(Supla_server, custom_Supla_server.getValue());
  strcpy(Email, custom_Email.getValue());
  strcpy(Supla_name, custom_Supla_name.getValue());
  wifiManager.getWiFiSSID().toCharArray(Router_SSID, 33);
  wifiManager.getWiFiPass().toCharArray(Router_Pass, 65);
  if (strcmp(Supla_server, "get_new_guid_and_authkey") == 0) {
    Serial.println("new guid & authkey.");
    EEPROM.write(300, 0);
    EEPROM.commit();
    delay(100);
    ESP.reset();
  }
  if (strcmp(Supla_server, "get_new_rf") == 0) {
    Serial.println("new rf.");
    rf_code_l = 0;
    rf_code_h = 0;
    EEPROM.write(290, rf_code_l);
    EEPROM.write(292, rf_code_h);
    EEPROM.commit();
    delay(100);
    ESP.reset();
  }
  if (shouldSaveConfig == true) {
    Serial.println(" config...");
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
  Arilux.rf_received_value = 0;
  attachInterrupt(RF_IN_PIN, AriluxRfInterrupt, CHANGE);

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

  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY, 1);
  delay(10);
  Serial.println(" ");
  Serial.println(" ");
  pinMode(RF_ENABLE_PIN, OUTPUT);
  digitalWrite(RF_ENABLE_PIN, LOW);  // RF On
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  EEPROM.begin(1024);
  if (WiFi.SSID() == "") {
    initialConfig = true;
  }
  if (EEPROM.read(300) != 60) {
    initialConfig = true;
  }

  guid_authkey();

  rf_code_l = (EEPROM.read(290));
  rf_code_h = (EEPROM.read(292));
  Serial.print("rf id: ");
  Serial.print(rf_code_l, HEX);
  Serial.println(rf_code_h, HEX);

  if (LittleFS.begin()) {
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
          if (json.containsKey("Router_SSID")) strcpy(Router_SSID, json["Router_SSID"]);
          if (json.containsKey("Router_Pass")) strcpy(Router_Pass, json["Router_Pass"]);
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

  if (initialConfig == true) {
    ondemandwifiCallback();
  }

  rgb = new Supla::Control::RGBLeds(RED_PIN, GREEN_PIN, BLUE_PIN);
  dim1 = new Supla::Control::DimmerLeds(BRIGHTNESS_1_PIN);
  dim2 = new Supla::Control::DimmerLeds(BRIGHTNESS_2_PIN);

  SuplaDevice.setName(Supla_name);
  SuplaDevice.setStatusFuncImpl(&status_func);
  wifi.enableSSL(false);
  wifi.setSsid(Router_SSID);
  wifi.setPassword(Router_Pass);
  SuplaDevice.begin(GUID, Supla_server, Email, AUTHKEY);

  buttonA.setClickHandler(click);
  buttonA.setLongClickHandler(longClick);
  buttonA.setDoubleClickHandler(doubleClick);
  buttonA.setTripleClickHandler(tripleClick);
  buttonA.setQuatleClickHandler(quatleClick);
  buttonA.setQuintleClickHandler(quintleClick);

  Arilux.rf_received_value = 0;
  attachInterrupt(RF_IN_PIN, AriluxRfInterrupt, CHANGE);
}

void loop() {

  buttonA.loop();
  SuplaDevice.iterate();

  delay(15);

  AriluxRfHandler();

  int C_W_read = digitalRead(BUTTON_PIN);
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
void click(Button3& btn) {
  rgb->handleAction(0, Supla::TOGGLE);
  Serial.println("click TOGGLE_RGB");
}
void doubleClick(Button3& btn) {
  dim1->handleAction(0, Supla::TOGGLE);
  Serial.println("doubleClick TOGGLE_CW");
}
void tripleClick(Button3& btn) {
  dim2->handleAction(0, Supla::TOGGLE);
  Serial.println("3 x Click TOGGLE_WW");
}
void quatleClick(Button3& btn) {
  Serial.println("4 x click");
}
void quintleClick(Button3& btn) {
  Serial.println("5 x click");
}
void longClick(Button3& btn) {

  Serial.println("longClick");
}
