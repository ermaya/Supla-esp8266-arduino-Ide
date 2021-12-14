/*
  Copyright (C) AC SOFTWARE SP. Z O.O.

  This program is free software; you can istribute it and/or
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
#include <FS.h>
#include <WiFiManager.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h> //--------- https://github.com/bblanchon/ArduinoJson/tree/v5.13.2 ------
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266TrueRandom.h>
#include <RCSwitch.h>
#include <Ticker.h>
#include <SuplaDevice.h>
#include <supla/control/action_trigger.h>
#include <supla/control/button.h>
#include <supla/network/esp_wifi.h>
Supla::ESPWifi wifi("SSID", "PASS");  //------ Do not change----wifimanager takes care------
extern "C"
{
#include "user_interface.h"
}

Supla::Control::ActionTrigger *at0 = nullptr;
Supla::Control::ActionTrigger *at1 = nullptr;

#define CONFIG_PIN           0  // WiFiconfig 
#define ledpin               2  // Buildin Led
#define pinrf               14  // RF Reciver
#define button0_pin          5  // AT 0 Button
#define button1_pin          4  // AT 1 Button

//const unsigned int captureBufSize = 100;
DynamicJsonBuffer jsonBuffer;
JsonObject& deviceState = jsonBuffer.createObject();
WiFiClient client;
ESP8266WebServer *server = NULL;
Ticker ticker;
RCSwitch mySwitch = RCSwitch();
File fsUploadFile;
int s;
int web_port = 80;
char GUID[SUPLA_GUID_SIZE];
char AUTHKEY[SUPLA_AUTHKEY_SIZE];
byte uuidNumber[16];
char Supla_server[81] = ("Set server address");
char Email[81] = ("set email address");
char Supla_name[51] = ("RF-AT");
char Router_SSID[32];
char Router_Pass[64];
char Supla_status[51];
int C_W_state = HIGH;
int last_C_W_state = HIGH;
unsigned long time_last_C_W_change = 0;
long C_W_delay = 5000;                      // config delay 5 seconds
bool shouldSaveConfig = false;
bool initialConfig = false;
bool starting = true;
bool pr_wifi = true;
char limpio[40];
char at_0_hold[40];
char at_0_short_1[40];
char at_0_short_2[40];
char at_0_short_3[40];
char at_0_short_4[40];
char at_0_short_5[40];
char at_1_hold[40];
char at_1_short_1[40];
char at_1_short_2[40];
char at_1_short_3[40];
char at_1_short_4[40];
char at_1_short_5[40];

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
}; returnMessage rtnMessage;

void saveConfigCallback () {
  Serial.println(F("Should save config"));
  shouldSaveConfig = true;
}
void tick() {
  int state = digitalRead(ledpin);
  digitalWrite(ledpin, !state);
}
void disableLed() {
  digitalWrite(ledpin, HIGH);
  ticker.detach();
}
void ondemandwifiCallback () {

  ticker.attach(0.5, tick);
  initialConfig = false;
  if (server != NULL) {
    delete server;
  }
  WiFiManager wifiManager;

  wifiManager.setConfigPortalBlocking(true);
  wifiManager.setCleanConnect(true);
  wifiManager.setBreakAfterConfig(true);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setCustomHeadElement("<style>html{ background-color: #01DF3A;}</style>");
  wifiManager.setMinimumSignalQuality(8);
  wifiManager.setConfigPortalTimeout(300);
  std::vector<const char *> menu = {"wifi", "wifinoscan", "sep", "update", "sep", "info", "restart"};
  wifiManager.setMenu(menu);
  wifiManager.setTitle("&#66&#121&#46&#46&#69&#108&#77&#97&#121&#97");

  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 81, "required");
  WiFiManagerParameter custom_Email("email", "Email", Email, 81, "required");
  WiFiManagerParameter custom_Supla_name("name", "Supla Device Name", Supla_name, 51, "required");
  WiFiManagerParameter custom_html_id21("<div><h4> - Supla State -   ");
  WiFiManagerParameter custom_html_id22( Supla_status);
  WiFiManagerParameter custom_html_id23( "</h4></div>");

  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_Email);
  wifiManager.addParameter(&custom_Supla_name);
  wifiManager.addParameter(&custom_html_id21);
  wifiManager.addParameter(&custom_html_id22);
  wifiManager.addParameter(&custom_html_id23);

  if (!wifiManager.startConfigPortal("AT_RF")) {
    Serial.println(F("Not connected to WiFi but continuing anyway."));
  } else {
    Serial.println(F("connected...yeey :)"));
  }
  strcpy(Supla_server, custom_Supla_server.getValue());
  strcpy(Email, custom_Email.getValue());
  strcpy(Supla_name, custom_Supla_name.getValue());
  wifiManager.getWiFiSSID().toCharArray(Router_SSID, 33);
  wifiManager.getWiFiPass().toCharArray(Router_Pass, 65);
  if (strcmp(Supla_server, "get_new_guid_and_authkey") == 0) {
    Serial.println(F("new guid & authkey."));
    EEPROM.write(300, 0);
    EEPROM.commit();
    ESP.reset();
  }
  if (shouldSaveConfig) {
    Serial.println(F(" config..."));
    DynamicJsonBuffer jsonWifiBuffer;
    JsonObject& json = jsonWifiBuffer.createObject();
    json["Supla_server"] = Supla_server;
    json["Email"] = Email;
    json["Supla_name"] = Supla_name;
    json["Router_SSID"] = Router_SSID;
    json["Router_Pass"] = Router_Pass;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println(F("failed to open config file for writing"));
    }
    json.printTo(Serial);
    Serial.println(F(" "));
    Serial.println(F("Writing config file"));
    json.printTo(configFile);
    configFile.close();
    jsonWifiBuffer.clear();
    Serial.println(F("Config written successfully"));
    shouldSaveConfig = false;
    //initialConfig = false;
    WiFi.mode(WIFI_STA);
    delay(5000);
    ESP.restart();
  }
  ticker.detach();
  WiFi.softAPdisconnect(true);
  digitalWrite(ledpin, LOW);
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
  }
  read_guid();
  read_authkey();
  Serial.print(F("GUID : ")); Serial.println(read_guid());
  Serial.print(F("AUTHKEY : ")); Serial.println(read_authkey());
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
  if (s != status) {
    s = status;
    Serial.print(F("status: "));
    Serial.println(s);
    if (status != 10) {
      strcpy(Supla_status, msg);
    }
  }
}
void setup() { // ========================================================================== setup ================================================
  wifi_set_sleep_type(NONE_SLEEP_T);
  WiFi.mode(WIFI_STA);
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY, 1);
  delay(200);
  Serial.println(F(" "));
  Serial.println(F(" "));
  EEPROM.begin(1024);
  pinMode(CONFIG_PIN, INPUT_PULLUP);
  pinMode(ledpin, OUTPUT);

  guid_authkey();

  if (WiFi.SSID() == "") {
    initialConfig = true;
  }
  if (EEPROM.read(300) != 60) {
    initialConfig = true;
  }

  if (SPIFFS.begin()) {  // ------------------------- wificonfig read -----------------
    Serial.println(F("mounted file system"));
    if (SPIFFS.exists("/config.json")) {
      Serial.println(F("reading config file"));
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println(F("opened config file"));
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);   //print config data to serial on startup
        if (json.success()) {
          Serial.println(F("\nparsed json"));
          if (json.containsKey("Supla_server")) strcpy(Supla_server, json["Supla_server"]);
          if (json.containsKey("Email")) strcpy(Email, json["Email"]);
          if (json.containsKey("Supla_name")) strcpy(Supla_name, json["Supla_name"]);
          if (json.containsKey("Router_SSID")) strcpy(Router_SSID, json["Router_SSID"]);
          if (json.containsKey("Router_Pass")) strcpy(Router_Pass, json["Router_Pass"]);
        } else {
          Serial.println(F("failed to load json config"));
          initialConfig = true;
        }
        configFile.close();
      }
    }
  } else {
    Serial.println(F("failed to mount FS"));
  }

  read_stored_command();
  wifi_station_set_hostname(Supla_name);

  WiFi.mode(WIFI_STA);

  auto button0 = new Supla::Control::Button(button0_pin, true, true);
  auto button1 = new Supla::Control::Button(button1_pin, true, true);
  button0->setMulticlickTime(800, false);
  button1->setMulticlickTime(800, false);
  at0 = new Supla::Control::ActionTrigger();
  at1 = new Supla::Control::ActionTrigger();
  at0->attach(button0);
  at1->attach(button1);

  wifi.enableSSL(false);
  wifi.setSsid(Router_SSID);
  wifi.setPassword(Router_Pass);
  SuplaDevice.setName(Supla_name);
  SuplaDevice.setStatusFuncImpl(&status_func);
  SuplaDevice.begin(GUID, Supla_server, Email, AUTHKEY);

  digitalWrite(ledpin, LOW);
  ticker.attach(2, disableLed);
  // irrecv.enableIRIn();
  mySwitch.enableReceive(pinrf);
  Serial.println(F("Ready to receive RF signals"));

}
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
  server->sendContent("    <meta name='viewport' content='width=device-width, initial-scale=1.0' />\n");
  server->sendContent("    <link rel='stylesheet' href='https://stackpath.bootstrapcdn.com/bootstrap/3.4.1/css/bootstrap.min.css' />\n");
  server->sendContent("    <script src='https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js'></script>\n");
  server->sendContent("    <script src='https://stackpath.bootstrapcdn.com/bootstrap/3.4.1/js/bootstrap.min.js'></script>\n");
  server->sendContent("    <style>@media (max-width: 991px) {.nav-pills>li {float: none; margin-left: 0; margin-top: 5px; text-align: center;}}</style>\n");
  server->sendContent("    <title>Supla RF AT (" + String(Supla_name) + ")</title>\n");
  server->sendContent("  <link rel='icon' href='data:;base64,AAABAAEAEBAQAAEABAAoAQAAFgAAACgAAAAQAAAAIAAAAAEABAAAAAAAgAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAA////AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABERAAEREQAAEREAEREQAAAREQEREQAAABEREREQAAAAEREREQAAAAAREREQAAAAABEREQAAAAAAERERAAAAAAAREREQAAAAABEREREAAAAAERERERAAAAAREQEREQAAABERABEREAAAEREAARERAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA' type='image/x-icon' />\n");  server->sendContent("  </head>\n");
  server->sendContent("  <body>\n");
  server->sendContent("   <nav class='navbar navbar-inverse'>\n");
  server->sendContent("      <a class='navbar-brand' href='/'>Supla RF AT</a>\n");
  server->sendContent("      <ul class='nav navbar-nav'>\n");
  server->sendContent("       <li class='dropdown'><a class='dropdown-toggle' data-toggle='dropdown' href='#'>Tools <span class='caret'></span></a>\n");
  server->sendContent("        <ul class='dropdown-menu'>\n");
  // server->sendContent("         <li><a href='/update'>ESP8266 firmware update</a></li>\n");
  server->sendContent("         <li><a href='#' data-toggle='modal' data-target='#myModal'>Upload RF remote code file</a></li>\n");
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
  server->sendContent("        input.runAction('fileselect', [numFiles, label]);\n");
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
  server->sendContent("              <a href='http://" + WiFi.localIP().toString() + ":" + int(web_port) + "'>Local<span class='badge'>" + WiFi.localIP().toString() + ":" + String(web_port) + "</span></a></li>\n");
  server->sendContent("            <li class='active'>\n");
  server->sendContent("              <a href='http://" + WiFi.localIP().toString() + ":"  + int(web_port) + "/listcodes"  + "'>Stored Codes<span class='badge'>" + WiFi.localIP().toString() + ":"  + String(web_port) + "/listcodes"  + "</span></a></li>\n");
  server->sendContent("            <li class='active'>\n");
  server->sendContent("              <a href='https://cloud.supla.org/login'>Supla state<span class='badge'>" + String(Supla_status) + "</span></a></li>\n");
  server->sendContent("            <li class='active'>\n");
  server->sendContent("              <a href='http://" + String(Supla_name) + ".local" + ":" + String(web_port) + "'>mDNS<span class='badge'>" + String(Supla_name) + ".local" + ":" + String(web_port) + "</span></a></li>\n");
  server->sendContent("          </ul>\n");
  server->sendContent("        </div>\n");
  server->sendContent("      </div><hr />\n");
}

void sendFooter() {

  server->sendContent("      <hr />\n");
  server->sendContent("      <div class='row'><div class='col-md-12'><em>By ElMaya ---- heap: " + String(ESP.getFreeHeap()) + "</em></div></div>");
  server->sendContent("     <hr />\n");
  server->sendContent("    <hr />\n");
  server->sendContent("   </div>\n");
  server->sendContent("  </body>\n");
  server->sendContent("  </div>\n");
  server->sendContent("</html>\n");
  server->sendContent("");
  server->client().stop();
  Serial.println(F("client stop"));
}

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
  server->sendContent("       <p></p>\n");
  server->sendContent("        </div>\n");
  server->sendContent("      </div>\n");
  sendFooter();
}

void storeCodePage(Code selCode, int id) {
  storeCodePage(selCode, id, 200);
}
void storeCodePage(Code selCode, int ids, int httpcode) {
  sendHeader(httpcode);
  server->sendContent("      <div class='row'>\n");
  server->sendContent("        <div class='col-md-12'>\n");
  server->sendContent("          <h2><span class='label label-success'>" + String(selCode.data) + ":" + String(selCode.encoding) + ":" + String(selCode.bits) + "</span></h2>\n");
  server->sendContent("        </div></div>\n");
  server->sendContent("          <form action='/store' method='POST'>\n");
  server->sendContent("          <div class='form-group row'>\n");
  server->sendContent("              <label for='name_input' class='col-sm-2 col-form-label col-form-label-sm'>Select the function:</label>\n");
  server->sendContent("              <div class='col-sm-10'>\n");
  server->sendContent("              <select class='form-control form-control-sm' name='name_input' name='id'>\n");
  server->sendContent("               <option>at_0__hold</option>\n");
  server->sendContent("               <option>at_0_press_x1</option>\n");
  server->sendContent("               <option>at_0_press_x2</option>\n");
  server->sendContent("               <option>at_0_press_x3</option>\n");
  server->sendContent("               <option>at_0_press_x4</option>\n");
  server->sendContent("               <option>at_0_press_x5</option>\n");
  server->sendContent("               <option>at_1__hold</option>\n");
  server->sendContent("               <option>at_1_press_x1</option>\n");
  server->sendContent("               <option>at_1_press_x2</option>\n");
  server->sendContent("               <option>at_1_press_x3</option>\n");
  server->sendContent("               <option>at_1_press_x4</option>\n");
  server->sendContent("               <option>at_1_press_x5</option>\n");
  server->sendContent("               </select>\n");
  server->sendContent("              </div>\n");
  server->sendContent("          </div>\n");
  server->sendContent("          <div class='form-group row'>\n");
  server->sendContent("              <label for='btn_name' class='col-sm-2 col-form-label col-form-label-sm'>Specify remote button name:</label>\n");
  server->sendContent("              <div class='col-sm-10'>\n");
  server->sendContent("               <input type='text' class='form-control form-control-sm' name='btn_name' placeholder='set button name'>\n");
  server->sendContent("              </div>\n");
  server->sendContent("          </div>\n");
  server->sendContent("          <div class='form-group row'>\n");
  server->sendContent("              <div class='col-sm-10'>\n");
  server->sendContent("              <button type='submit' formaction='/store?id=" + String(ids) + "' class='btn btn-danger'>Store</button>\n");
  server->sendContent("              </div>\n");
  server->sendContent("          </div>\n");
  server->sendContent("          </form>\n");
  server->sendContent("      <hr />\n");
  sendFooter();
}

void listStoredCodes() {
  sendHeader(200);
  server->sendContent("      <div class='row'>\n");
  server->sendContent("        <div class='col-md-12'>\n");
  server->sendContent("          <h3>Codes Stored</h3>\n");
  server->sendContent("          <table class='table table-striped' style='table-layout: fixed;'>\n");
  server->sendContent("            <thead><tr><th>Function</th><th>Data</th><th>Button Name</th><th>Action</th></tr></thead>\n"); //Title
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
            String jsonStringbtn_name = filebtn["btn_name"];
            jsonReadCodeFileBuffer.clear();
            if (filebtn.containsKey("data") && filebtn.containsKey("name") && filebtn.containsKey("btn_name")) server->sendContent("<tr class='text-uppercase'><td>" + jsonStringbtn + "</td><td>" + jsonStringdata + "</td><td>" + jsonStringbtn_name + "</td><td> <a class='btn btn-primary btn-xs' href='" + vtxt + "' role='button'>Download</a> <a class='btn btn-danger btn-xs' href='/listcodes?item=" + jsonStringbtn + "' role='button'>Delete</a></td></tr>\n");
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

static void send_redirect(const String &redirect) {
  String html;
  html += F("HTTP/1.1 301 OK\r\n");
  html += F("Location: ");
  html += redirect;
  html += F("\r\n");
  html += F("Cache-Control: no-cache\r\n\r\n");
  server->sendContent(html);
}

void read_stored_command() {
  DynamicJsonBuffer jsonCodeBtnBuffer;
  JsonArray& rootbtn = jsonCodeBtnBuffer.parseArray(listcodefiles("/codes/"));
  if (rootbtn.size() != 0) {
    if (SPIFFS.begin()) {
      if (SPIFFS.exists("/codes/at_0_press_x1.json")) {
        File codeFileRead = SPIFFS.open("/codes/at_0_press_x1.json", "r"); //reads the json file
        if (codeFileRead) {
          DynamicJsonBuffer jsonReadCodeFileBuffer;
          JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);
          codeFileRead.close();
          if (filebtn.containsKey("data")) strcpy(at_0_short_1 , filebtn["data"]);
          Serial.print(F("at_0_short_1: ")); Serial.println(at_0_short_1);
          jsonReadCodeFileBuffer.clear();
        }
      } else {
        strcpy(at_0_short_1 , limpio);
      }
      if (SPIFFS.exists("/codes/at_0_press_x2.json")) {
        File codeFileRead = SPIFFS.open("/codes/at_0_press_x2.json", "r"); //reads the json file
        if (codeFileRead) {
          DynamicJsonBuffer jsonReadCodeFileBuffer;
          JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);
          codeFileRead.close();
          if (filebtn.containsKey("data")) strcpy(at_0_short_2 , filebtn["data"]);
          Serial.print(F("at_0_short_2: ")); Serial.println(at_0_short_2);
          jsonReadCodeFileBuffer.clear();
        }
      } else {
        strcpy(at_0_short_2 , limpio);
      }
      if (SPIFFS.exists("/codes/at_0_press_x3.json")) {
        File codeFileRead = SPIFFS.open("/codes/at_0_press_x3.json", "r"); //reads the json file
        if (codeFileRead) {
          DynamicJsonBuffer jsonReadCodeFileBuffer;
          JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);
          codeFileRead.close();
          if (filebtn.containsKey("data")) strcpy(at_0_short_3 , filebtn["data"]);
          Serial.print(F("at_0_short_3: ")); Serial.println(at_0_short_3);
          jsonReadCodeFileBuffer.clear();
        }
      } else {
        strcpy(at_0_short_3 , limpio);
      }
      if (SPIFFS.exists("/codes/at_0_press_x4.json")) {
        File codeFileRead = SPIFFS.open("/codes/at_0_press_x4.json", "r"); //reads the json file
        if (codeFileRead) {
          DynamicJsonBuffer jsonReadCodeFileBuffer;
          JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);
          codeFileRead.close();
          if (filebtn.containsKey("data")) strcpy(at_0_short_4 , filebtn["data"]);
          Serial.print(F("at_0_short_4: ")); Serial.println(at_0_short_4);
          jsonReadCodeFileBuffer.clear();
        }
      } else {
        strcpy(at_0_short_4 , limpio);
      }
      if (SPIFFS.exists("/codes/at_0_press_x5.json")) {
        File codeFileRead = SPIFFS.open("/codes/at_0_press_x5.json", "r"); //reads the json file
        if (codeFileRead) {
          DynamicJsonBuffer jsonReadCodeFileBuffer;
          JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);
          codeFileRead.close();
          if (filebtn.containsKey("data")) strcpy(at_0_short_5 , filebtn["data"]);
          Serial.print(F("at_0_short_5: ")); Serial.println(at_0_short_5);
          jsonReadCodeFileBuffer.clear();
        }
      } else {
        strcpy(at_0_short_5 , limpio);
      }
      if (SPIFFS.exists("/codes/at_0__hold.json")) {
        File codeFileRead = SPIFFS.open("/codes/at_0__hold.json", "r"); //reads the json file
        if (codeFileRead) {
          DynamicJsonBuffer jsonReadCodeFileBuffer;
          JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);
          codeFileRead.close();
          if (filebtn.containsKey("data")) strcpy(at_0_hold , filebtn["data"]);
          Serial.print(F("at_0_hold: ")); Serial.println(at_0_hold);
          jsonReadCodeFileBuffer.clear();
        }
      } else {
        strcpy(at_0_hold , limpio);
      }
      if (SPIFFS.exists("/codes/at_1_press_x1.json")) {
        File codeFileRead = SPIFFS.open("/codes/at_1_press_x1.json", "r"); //reads the json file
        if (codeFileRead) {
          DynamicJsonBuffer jsonReadCodeFileBuffer;
          JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);
          codeFileRead.close();
          if (filebtn.containsKey("data")) strcpy(at_1_short_1 , filebtn["data"]);
          Serial.print(F("at_1_short_1: ")); Serial.println(at_1_short_1);
          jsonReadCodeFileBuffer.clear();
        }
      } else {
        strcpy(at_1_short_1 , limpio);
      }
      if (SPIFFS.exists("/codes/at_1_press_x2.json")) {
        File codeFileRead = SPIFFS.open("/codes/at_1_press_x2.json", "r"); //reads the json file
        if (codeFileRead) {
          DynamicJsonBuffer jsonReadCodeFileBuffer;
          JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);
          codeFileRead.close();
          if (filebtn.containsKey("data")) strcpy(at_1_short_2 , filebtn["data"]);
          Serial.print(F("at_1_short_2: ")); Serial.println(at_1_short_2);
          jsonReadCodeFileBuffer.clear();
        }
      } else {
        strcpy(at_1_short_2 , limpio);
      }
      if (SPIFFS.exists("/codes/at_1_press_x3.json")) {
        File codeFileRead = SPIFFS.open("/codes/at_1_press_x3.json", "r"); //reads the json file
        if (codeFileRead) {
          DynamicJsonBuffer jsonReadCodeFileBuffer;
          JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);
          codeFileRead.close();
          if (filebtn.containsKey("data")) strcpy(at_1_short_3 , filebtn["data"]);
          Serial.print(F("at_1_short_3: ")); Serial.println(at_1_short_3);
          jsonReadCodeFileBuffer.clear();
        }
      } else {
        strcpy(at_1_short_3 , limpio);
      }
      if (SPIFFS.exists("/codes/at_1_press_x4.json")) {
        File codeFileRead = SPIFFS.open("/codes/at_1_press_x4.json", "r"); //reads the json file
        if (codeFileRead) {
          DynamicJsonBuffer jsonReadCodeFileBuffer;
          JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);
          codeFileRead.close();
          if (filebtn.containsKey("data")) strcpy(at_1_short_4 , filebtn["data"]);
          Serial.print(F("at_1_short_4: ")); Serial.println(at_1_short_4);
          jsonReadCodeFileBuffer.clear();
        }
      } else {
        strcpy(at_1_short_4 , limpio);
      }
      if (SPIFFS.exists("/codes/at_1_press_x5.json")) {
        File codeFileRead = SPIFFS.open("/codes/at_1_press_x5.json", "r"); //reads the json file
        if (codeFileRead) {
          DynamicJsonBuffer jsonReadCodeFileBuffer;
          JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);
          codeFileRead.close();
          if (filebtn.containsKey("data")) strcpy(at_1_short_5 , filebtn["data"]);
          Serial.print(F("at_1_short_5: ")); Serial.println(at_1_short_5);
          jsonReadCodeFileBuffer.clear();
        }
      } else {
        strcpy(at_1_short_5 , limpio);
      }
      if (SPIFFS.exists("/codes/at_1__hold.json")) {
        File codeFileRead = SPIFFS.open("/codes/at_1__hold.json", "r"); //reads the json file
        if (codeFileRead) {
          DynamicJsonBuffer jsonReadCodeFileBuffer;
          JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);
          codeFileRead.close();
          if (filebtn.containsKey("data")) strcpy(at_1_hold , filebtn["data"]);
          Serial.print(F("at_1_hold: ")); Serial.println(at_1_hold);
          jsonReadCodeFileBuffer.clear();
        }
      } else {
        strcpy(at_1_hold , limpio);
      }
    }
  } else {
    Serial.println(F("No Stored Data"));
  }
  jsonCodeBtnBuffer.clear();
}

/*String getValue(String data, char separator, int index) {
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
}*/

void storeReceivedCode (Code code, String btn, String btn_name) {
  DynamicJsonBuffer jsonWriteBuffer;
  JsonObject& jsonadd = jsonWriteBuffer.createObject();
  btn.toLowerCase();
  jsonadd["name"] = btn;
  jsonadd["btn_name"] = btn_name;
  jsonadd["type"] = code.encoding;
  jsonadd["data"] = code.data;
  jsonadd["length"] = code.bits;
  if (saveJSON(jsonadd, btn)) sendHomePage("Code stored to " + btn, "Alert", 2, 200);
  jsonWriteBuffer.clear();
}

bool DeleteJSONitem (String btn) {
  btn.toLowerCase();
  if (SPIFFS.begin()) {
    SPIFFS.remove("/codes/" + btn + ".json");
    read_stored_command();
    return true;
  } else {
    return false;
  }
}

bool deletecodefiles(String path) {
  if (!path) path = "/codes/";
  path.toLowerCase();
  Serial.println("deleteFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  while (dir.next()) {
    SPIFFS.remove(dir.fileName());
  }
  read_stored_command();
  return true;
}

bool saveJSON (JsonObject& json, String btn) {
  btn.toLowerCase();
  if (SPIFFS.begin()) {
    if (SPIFFS.exists("/codes/" + btn + ".json")) DeleteJSONitem(btn);
    File codeFileWrite = SPIFFS.open("/codes/" + btn + ".json", "w");
    if (codeFileWrite) {
      json.printTo(codeFileWrite);
      json.prettyPrintTo(Serial);
      codeFileWrite.close();
      Serial.println(F("Codes written successfully"));
      read_stored_command();
      return true;
    }
  }
}

String listcodefiles(String path) {
  path.toLowerCase();
  Dir dir = SPIFFS.openDir(path);
  path = String();
  String output = "[";
  while (dir.next()) {
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"name\":\"";
    output += String(dir.fileName());
    output += "\"}";
  }
  output += "]";
  return output;
}

void handleFileUpload() {
  if (server->uri() != "/listcodes") return;
  HTTPUpload& upload = server->upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/codes/")) filename = "/codes/" + filename;
    filename.toLowerCase();
    Serial.print(F("handleFileUpload Name: ")); Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      File codeFileCheck = SPIFFS.open(fsUploadFile.name(), "r");
      DynamicJsonBuffer jsonCheckBuffer;
      JsonObject& filebtn = jsonCheckBuffer.parseObject(codeFileCheck);
      codeFileCheck.close();
      String jsonStringname = filebtn["name"];
      jsonStringname = "/codes/" + jsonStringname + ".json";
      jsonStringname.toLowerCase();
      jsonCheckBuffer.clear();
      Serial.println(jsonStringname + " vs. " + fsUploadFile.name());
      if (jsonStringname != fsUploadFile.name()) {
        SPIFFS.remove(fsUploadFile.name());
        Serial.println(F("Object doesnt match, file deleted"));
        send_redirect("/?message=Code file mismatch&type=2&header=Alert&httpcode=400");
      } else {
        send_redirect("/listcodes");
      }
      fsUploadFile.close();
    } else {
      Serial.print(F("Error uploading "));
      send_redirect("/?message=Error uploading file&type=2&header=Alert&httpcode=200");
    }
  }
}

bool handleFileRead(String path) {
  path.toLowerCase();
  Serial.println("handleFileRead: " + path);
  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    size_t sent = server->streamFile(file, "application/octet-stream");
    file.close();
    return true;
  }
  return false;
}

String uint64ToString(uint64_t input, uint8_t base) {
  String result = "";
  if (base < 2) base = 10;
  if (base > 36) base = 10;
  result.reserve(16);
  do {
    char c = input % base;
    input /= base;
    if (c < 10)
      c += '0';
    else
      c += 'A' - 10;
    result = c + result;
  } while (input);
  return result;
}

void copyCode (Code& c1, Code& c2) {
  strncpy(c2.data, c1.data, 40);
  strncpy(c2.encoding, c1.encoding, 14);
  c2.bits = c1.bits;
  c2.valid = c1.valid;
}
void loop() { // ============================================================================= LOOP ===========================================================

  if (initialConfig == true) {
    ondemandwifiCallback();
  }

  SuplaDevice.iterate();
  delay(22);

  int C_W_read = digitalRead(CONFIG_PIN);
  if (C_W_read != last_C_W_state) {
    time_last_C_W_change = millis();
  }
  if ((millis() - time_last_C_W_change) > C_W_delay) {
    if (C_W_read != C_W_state) {
      Serial.println(F("Triger sate changed"));
      C_W_state = C_W_read;
      if (C_W_state == LOW) {
        ondemandwifiCallback () ;
      }
    }
  }
  last_C_W_state = C_W_read;

  if (mySwitch.available()) {
    Serial.print(F("RF received: "));
    Serial.println( mySwitch.getReceivedValue() );
    if (digitalRead(ledpin)) {
      copyCode(last_recv_4, last_recv_5);
      copyCode(last_recv_3, last_recv_4);
      copyCode(last_recv_2, last_recv_3);
      copyCode(last_recv, last_recv_2);
      String protostring = (String)mySwitch.getReceivedProtocol();
      strncpy(last_recv.data, uint64ToString(mySwitch.getReceivedValue(), 16).c_str(), 40);
      strncpy(last_recv.encoding, protostring.c_str(), 14);
      last_recv.bits = mySwitch.getReceivedBitlength();

      last_recv.valid = true;
      Serial.print(F("rf code: ")); Serial.println(last_recv.data);
      if (strcmp(last_recv.data, at_0_short_1) == 0) {
        at0->handleAction(0, Supla::SEND_AT_SHORT_PRESS_x1);
        Serial.println(F("AT 0 SHORT 1"));
      }
      else if (strcmp(last_recv.data, at_0_short_2) == 0) {
        at0->handleAction(0, Supla::SEND_AT_SHORT_PRESS_x2);
        Serial.println(F("AT 0 SHORT 2"));
      }
      else if (strcmp(last_recv.data, at_0_short_3) == 0) {
        at0->handleAction(0, Supla::SEND_AT_SHORT_PRESS_x3);
        Serial.println(F("AT 0 SHORT 3"));
      }
      else if (strcmp(last_recv.data, at_0_short_4) == 0) {
        at0->handleAction(0, Supla::SEND_AT_SHORT_PRESS_x4);
        Serial.println(F("AT 0 SHORT 4"));
      }
      else if (strcmp(last_recv.data, at_0_short_5) == 0) {
        at0->handleAction(0, Supla::SEND_AT_SHORT_PRESS_x5);
        Serial.println(F("AT 0 SHORT 5"));
      }
      else if (strcmp(last_recv.data, at_0_hold) == 0) {
        at0->handleAction(0, Supla::SEND_AT_HOLD);
        Serial.println(F("AT 0 HOLD"));
      }
      else if (strcmp(last_recv.data, at_1_short_1) == 0) {
        at1->handleAction(0, Supla::SEND_AT_SHORT_PRESS_x1);
        Serial.println(F("AT 1 SHORT 1"));
      }
      else if (strcmp(last_recv.data, at_1_short_2) == 0) {
        at1->handleAction(0, Supla::SEND_AT_SHORT_PRESS_x2);
        Serial.println(F("AT 1 SHORT 2"));
      }
      else if (strcmp(last_recv.data, at_1_short_3) == 0) {
        at1->handleAction(0, Supla::SEND_AT_SHORT_PRESS_x3);
        Serial.println(F("AT 1 SHORT 3"));
      }
      else if (strcmp(last_recv.data, at_1_short_4) == 0) {
        at1->handleAction(0, Supla::SEND_AT_SHORT_PRESS_x4);
        Serial.println(F("AT 1 SHORT 4"));
      }
      else if (strcmp(last_recv.data, at_1_short_5) == 0) {
        at1->handleAction(0, Supla::SEND_AT_SHORT_PRESS_x5);
        Serial.println(F("AT 1 SHORT 5"));
      }
      else if (strcmp(last_recv.data, at_1_hold) == 0) {
        at1->handleAction(0, Supla::SEND_AT_HOLD);
        Serial.println(F("AT 1 HOLD"));
      }
    }
    mySwitch.resetAvailable();
    digitalWrite(ledpin, LOW);
    ticker.attach(0.6, disableLed);
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (pr_wifi == true) {
      Serial.println(F(" "));
      Serial.println(F("CONNECTED"));
      pr_wifi = false;
      if (server != NULL) {
        delete server;
      }
      server = new ESP8266WebServer(web_port);
      if (!MDNS.begin(Supla_name)) {
        Serial.println(F("Error setting up MDNS responder!"));
      }
      MDNS.addService("http", "tcp", web_port);
      Serial.println("MDNS http service added. Hostname is set to http://" + String(Supla_name) + ".local:" + String(web_port));
      server->on("/store", []() {
        Serial.println(F("Connection received to store code page"));
        int id = server->arg("id").toInt();
        Serial.print(F("store id: ")); Serial.println(id);
        String btn = server->arg("name_input");
        String btn_name = server->arg("btn_name");
        btn.toLowerCase();
        String output;
        if (id == 1 && last_recv.valid) {
          if (btn != "") {
            storeReceivedCode(last_recv, btn, btn_name);
          } else {
            storeCodePage(last_recv, 1, 200);
          }
        } else if (id == 2 && last_recv_2.valid) {
          if (btn != "") {
            storeReceivedCode(last_recv_2, btn, btn_name);
          } else {
            storeCodePage(last_recv_2, 2, 200);
          }
        } else if (id == 3 && last_recv_3.valid) {
          if (btn != "") {
            storeReceivedCode(last_recv_3, btn, btn_name);
          } else {
            storeCodePage(last_recv_3, 3, 200);
          }
        } else if (id == 4 && last_recv_4.valid) {
          if (btn != "") {
            storeReceivedCode(last_recv_4, btn, btn_name);
          } else {
            storeCodePage(last_recv_4, 4, 200);
          }
        } else if (id == 5 && last_recv_5.valid) {
          if (btn != "") {
            storeReceivedCode(last_recv_5, btn, btn_name);
          } else {
            storeCodePage(last_recv_5, 5, 200);
          }
        } else {
          sendHomePage("Code does not exist", "Alert", 2, 404);
        }
      });

      server->on("/listcodes", HTTP_POST, []() {
        server->send(200, "text/plain", "");
      }, {handleFileUpload});

      server->on("/listcodes", []() {
        String item = server->arg("item");
        if (server->hasArg("item")) {
          Serial.println("Connection received - delete item " + item);
          if (DeleteJSONitem(item)) listStoredCodes();
        }
        listStoredCodes();
      });

      server->on("/deleteall", []() {
        if (deletecodefiles("/codes/")) sendHomePage("All Stored Codes DELETED", "Alert", 2, 404);
      });

      server->on("/", []() {
        if (server->hasArg("message")) {
          rtnMessage.message = server->arg("message");
          rtnMessage.type = server->arg("type").toInt();
          rtnMessage.title = server->arg("header");
          rtnMessage.httpcode = server->arg("httpcode").toInt();
          sendHomePage(rtnMessage.message, rtnMessage.title, rtnMessage.type, rtnMessage.httpcode);
        }
        Serial.println(F("Connection received"));
        sendHomePage();
      });

      server->onNotFound( []() {
        if (!handleFileRead(server->uri()))
          sendHomePage("Resource not found", "Alert", 2, 404);
      });

      server->begin();
      Serial.println("HTTP Server started on port " + String(web_port));
    }
    MDNS.update();
    server->handleClient();
  }
}
