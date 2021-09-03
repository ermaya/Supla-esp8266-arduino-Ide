#define supla_lib_config_h_  // silences debug messages
#include "LittleFS.h"
#include <WiFiManager.h>
#include <EEPROM.h>
#include <ESP8266TrueRandom.h>
#include <ArduinoJson.h>
#include <DoubleResetDetector.h>
#include <ShiftRegister74HC595.h>
#include <SuplaDevice.h>
#include <supla/control/relay.h>
#include <supla/control/button.h>
#include <supla/sensor/NTC10K.h>
#include <supla/network/esp_wifi.h>
Supla::ESPWifi wifi("SSID", "PASS");  //------ Do not change----wifimanager takes care------
#define STORAGE_OFFSET 512
#include <supla/storage/eeprom.h>
Supla::Eeprom eeprom(STORAGE_OFFSET);

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



char Supla_server[81] = ("Set server address");
char Email[81] = ("set email address");
char Supla_name[81] = ("Supla595");
char Router_SSID[32];
char Router_Pass[64];
char Supla_status[51];
bool shouldSaveConfig = false;
bool initialConfig = false;
int s;
char GUID[SUPLA_GUID_SIZE];
char AUTHKEY[SUPLA_AUTHKEY_SIZE];
byte uuidNumber[16];
int hc595outState[] = {0, 0, 0, 0, 0, 0, 0, 0};

static const char logo[] PROGMEM = "<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>";
WiFiManager wifiManager;

// create shift register object (number of shift registers, data pin 14 on 74595, clock pin 11 on 74595, latch pin 12 on 74595)
ShiftRegister74HC595 sr (1, 15, 16, 0);

#define DRD_TIMEOUT 10 // Number of seconds after reset during which a  subseqent reset will be considered a double reset.
#define DRD_ADDRESS 0 // RTC Memory Address for the DoubleResetDetector to use
DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);

class CustomControl : public Supla::Io {
  public:
    void customDigitalWrite(int channelNumber, uint8_t pin, uint8_t val) {

      if (pin > 99) {
        sr.set(pin - 100, val);
        hc595outState[pin - 100] = val;
      }
      else {
        return ::digitalWrite(pin, val);
      }
    }

    int customDigitalRead(int channelNumber, uint8_t pin) {
      if (pin > 99) {
        return hc595outState[pin - 100];
      } else {
        return ::digitalRead(pin);
      }
    }
} CustomControl;

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void ondemandwifiCallback () {

  wifiManager.setDebugOutput(false); // Debug off
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

  if (!wifiManager.startConfigPortal("Supla595")) {
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

  EEPROM.begin(1024);

  if (WiFi.SSID() == "") {
    initialConfig = true;
  }
  if (EEPROM.read(300) != 60) {
    initialConfig = true;
  }

  guid_authkey();

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

  if (drd.detectDoubleReset()) {
    Serial.println("Double Reset Detected");
    ondemandwifiCallback ();
  }

  if (initialConfig == true) {
    ondemandwifiCallback();
  }

  auto relay_0 = new Supla::Control::Relay(100, true, 96);
  relay_0->getChannel()->setDefault(SUPLA_CHANNELFNC_POWERSWITCH);
  //relay_0->keepTurnOnDuration();  //relay_0->setDefaultStateOff(); //relay_0->setDefaultStateOn(); //relay_0->setDefaultStateRestore();
  auto relay_1 = new Supla::Control::Relay(101, true, 96);
  relay_1->getChannel()->setDefault(SUPLA_CHANNELFNC_POWERSWITCH);
  auto relay_2 = new Supla::Control::Relay(102, true, 96);
  relay_2->getChannel()->setDefault(SUPLA_CHANNELFNC_POWERSWITCH);
  auto relay_3 = new Supla::Control::Relay(103, true, 96);
  relay_3->getChannel()->setDefault(SUPLA_CHANNELFNC_POWERSWITCH);
  auto relay_4 = new Supla::Control::Relay(104, true, 96);
  relay_4->getChannel()->setDefault(SUPLA_CHANNELFNC_POWERSWITCH);
  auto relay_5 = new Supla::Control::Relay(105, true, 96);
  relay_5->getChannel()->setDefault(SUPLA_CHANNELFNC_POWERSWITCH);
  auto relay_6 = new Supla::Control::Relay(106, true, 96);
  relay_6->getChannel()->setDefault(SUPLA_CHANNELFNC_POWERSWITCH);
  auto relay_7 = new Supla::Control::Relay(107, true, 96);
  relay_7->getChannel()->setDefault(SUPLA_CHANNELFNC_POWERSWITCH);

  auto Button_0 = new Supla::Control::Button(5, true, true); // Button on pin XX with internal pullUp // and LOW is considered as "pressed" state
  auto Button_1 = new Supla::Control::Button(4, true, true); 
  auto Button_2 = new Supla::Control::Button(2, true, true); 
  auto Button_3 = new Supla::Control::Button(14, true, true); 
  auto Button_4 = new Supla::Control::Button(12, true, true); 
  auto Button_5 = new Supla::Control::Button(13, true, true); 
  auto Button_6 = new Supla::Control::Button(3, true, true); 
  auto Button_7 = new Supla::Control::Button(1, true, true); 
  Button_0->addAction(Supla::TOGGLE, relay_0, Supla::ON_CHANGE);  // Biestable
  Button_1->addAction(Supla::TOGGLE, relay_1, Supla::ON_CHANGE);
  Button_2->addAction(Supla::TOGGLE, relay_2, Supla::ON_PRESS);   // Monoestable
  Button_3->addAction(Supla::TOGGLE, relay_3, Supla::ON_CHANGE);
  Button_4->addAction(Supla::TOGGLE, relay_4, Supla::ON_CHANGE);
  Button_5->addAction(Supla::TOGGLE, relay_5, Supla::ON_CHANGE);
  Button_6->addAction(Supla::TOGGLE, relay_6, Supla::ON_CHANGE);
  Button_7->addAction(Supla::TOGGLE, relay_7, Supla::ON_PRESS);
  new Supla::Sensor::NTC10K(A0);

  SuplaDevice.setName(Supla_name);
  SuplaDevice.setStatusFuncImpl(&status_func);
  wifi.enableSSL(false);
  wifi.setSsid(Router_SSID);
  wifi.setPassword(Router_Pass);
  SuplaDevice.begin(GUID, Supla_server, Email, AUTHKEY);


}

void loop() {

  drd.loop();

  SuplaDevice.iterate();

  delay(25);

}
