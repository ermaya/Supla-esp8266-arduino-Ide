#include <FS.h>                                               // This needs to be first, or it all crashes and burns

#include <IRremoteESP8266.h>                                  // ---- esp board manager 2.4.2 ---
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>                                      // https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ESP8266mDNS.h>                                      // Useful to access to ESP by hostname.local
#define CUSTOMSUPLA_CPP
#include <CustomSupla.h>                                      // 1.6.1 Custom Supla lib.
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <Ticker.h>                                           // For LED status
extern "C"
{
#include "user_interface.h"
}

const unsigned int captureBufSize = 512;                      // Size of the IR capture buffer.

//#define D0 16  //no internal pullup resistor
//#define D1  5
//#define D2  4
//#define D3  0  //must not be pulled low during power on/reset, toggles value during boot
//#define D4  2  //must not be pulled low during power on/reset, toggles value during boot
//#define D5 14
//#define D6 12
//#define D7 13
//#define D8 15  //must not be pulled high during power on/reset

byte mac[6];
const int pinr1 = 14;                                         // Receiving pin
const int pins1 = 4;                                          // Transmitting pin
const int configpin = 0;                                     // Reset Pin

int C_W_state = HIGH; 
int last_C_W_state = HIGH;
unsigned long time_last_C_W_change = 0; 
long C_W_delay = 10000;               // ---------------------- config delay 10 seconds ---------------------------
int canal = 0;
int web_port = 80;
char Supla_server[40]= "";
char Location_id[15]= "";
char Location_Pass[20]= "";
char Supla_name[51]= "Supla IR";
char port_str[6] = "80";
char passcode[2] = "";
char user_id[2] = "";
char Supla_status[51];
int s;   

// ========= User settings are above here =========

const int ledpin = BUILTIN_LED;                               // Built in LED defined for WEMOS people
const char *wifi_config_name = "IR WiFiComfig";

DynamicJsonBuffer jsonBuffer;
JsonObject& deviceState = jsonBuffer.createObject();
WiFiClient client;
ESP8266WebServer *server = NULL;
Ticker ticker;

bool shouldSaveConfig = false;                                // Flag for saving data
bool holdReceive;                                             // Flag to prevent IR receiving while transmitting

IRrecv irrecv(pinr1, captureBufSize);
IRsend irsend1(pins1);

WiFiUDP ntpUDP;

char _ip[16] = "";

class Code {
  public:
    char encoding[14] = "";
    char address[20] = "";
    char command[40] = "";
    char data[40] = ""; 
    String raw = "";
    int bits = 0;
    time_t timestamp = 0;
    bool valid = false;
};


Code last_recv;
Code last_recv_2;
Code last_recv_3;
Code last_recv_4;
Code last_recv_5;
Code last_send;
Code last_send_2;
Code last_send_3;
Code last_send_4;
Code last_send_5;

class returnMessage {
  public:
    String message = "";
    String title = "";
    int httpcode = 0;
    int type = 1;
};
returnMessage rtnMessage;

File fsUploadFile;              // a File object to temporarily store the received file

void suplaDigitalWrite(int channelNumber, uint8_t pin, uint8_t val) {  
 if (val == HIGH){ 
   send_ir_btn(channelNumber);
 }
  return;        
}
//+=============================================================================
// Callback notifying us of the need to save config
//
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}


//+=============================================================================
// Reenable IR receiving
//
void resetReceive() {
  if (holdReceive) {
    Serial.println("Reenabling receiving");
    irrecv.resume();
    holdReceive = false;
  }
}


//+=============================================================================
// Valid user_id formatting
//
bool validUID(char* user_id) {
    return true;
}


//+=============================================================================
// Valid EPOCH time retrieval
//
bool validEPOCH(time_t timenow) {
  return true;
}

//+=============================================================================
//
bool validateHMAC(String epid, String mid, String timestamp, String signature) {
    return true;
}

//+=============================================================================
// Toggle state
//
void tick()
{
  int state = digitalRead(ledpin);  // get the current state of GPIO1 pin
  digitalWrite(ledpin, !state);     // set pin to the opposite state
}


//+=============================================================================
// Get External IP Address
//
String externalIP()
{
    return "0.0.0.0"; 
}


//+=============================================================================
// Turn off the Led after timeout
//
void disableLed()
{
  Serial.println("Turning off the LED to save power.");
  digitalWrite(ledpin, HIGH);                           // Shut down the LED
  ticker.detach();                                      // Stopping the ticker
}


//+=============================================================================
// Gets called when WiFiManager enters configuration mode
//
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}


//+=============================================================================
// Gets called when device loses connection to the accesspoint
//
void lostWifiCallback (const WiFiEventStationModeDisconnected& evt) {
  Serial.println("Lost Wifi");
  // reset and try again, or maybe put it to deep sleep
  ESP.reset();
  delay(1000);
}


//+=============================================================================
// First setup of the Wifi.
// If return true, the Wifi is well connected.
// Should not return false if Wifi cannot be connected, it will loop
//
bool setupWifi(bool resetConf) {
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.5, tick);

  WiFiManager wifiManager;

    if (resetConf){  
    wifiManager.resetSettings();
    delay(2000);
    ESP.reset();
    delay(1000);
   }

  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setBreakAfterConfig(true);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setConfigPortalTimeout(180);

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
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }

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


  if (!wifiManager.autoConnect(wifi_config_name)) {
    Serial.println("Failed to connect and hit timeout");
    ESP.reset();
    delay(1000);
  }

    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(Location_id, custom_Location_id.getValue());
    strcpy(Location_Pass, custom_Location_Pass.getValue());
    strcpy(Supla_name, custom_Supla_name.getValue());
    strncpy(port_str, custom_port.getValue(), 6);
    web_port = atoi(port_str);

  if (server != NULL) {
    delete server;
  }
  server = new ESP8266WebServer(web_port);

  // Reset device if lost wifi Connection
  WiFi.onStationModeDisconnected(&lostWifiCallback);

  Serial.println("WiFi connected! User chose hostname '" + String(Supla_name) + "' and port '" + String(port_str) + "'");

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

  // keep LED on
  digitalWrite(ledpin, LOW);
  return true;
}
void status_func(int status, const char *msg) { 
  if (s != status){
     s=status; 
     Serial.print("status: "); 
     Serial.println(s); 
         if (status != 10){
      // std::string Supla_status = msg;
      strcpy(Supla_status, msg);
   }  
  }                                        
}

//+=============================================================================
// Setup web server and IR receiver/blaster
//
void setup() {
  // Initialize serial
  Serial.begin(115200);
  pinMode(ledpin, OUTPUT);
  Serial.println("");
  pinMode(configpin, INPUT_PULLUP);
  if (!setupWifi(digitalRead(configpin) == LOW))
    return;

   uint8_t mac[WL_MAC_ADDR_LENGTH];
   WiFi.macAddress(mac);
   char GUID[SUPLA_GUID_SIZE] = {mac[WL_MAC_ADDR_LENGTH - 0], mac[WL_MAC_ADDR_LENGTH - 5], mac[WL_MAC_ADDR_LENGTH - 4], mac[WL_MAC_ADDR_LENGTH - 3],                                
                                 mac[WL_MAC_ADDR_LENGTH - 2], mac[WL_MAC_ADDR_LENGTH - 1], mac[WL_MAC_ADDR_LENGTH - 1], mac[WL_MAC_ADDR_LENGTH - 2], 
                                 mac[WL_MAC_ADDR_LENGTH - 3], mac[WL_MAC_ADDR_LENGTH - 4], mac[WL_MAC_ADDR_LENGTH - 5], mac[WL_MAC_ADDR_LENGTH - 0]};
          
  CustomSupla.addRelay(101, false); 
  CustomSupla.addRelay(102, false); 
  CustomSupla.addRelay(103, false); 
  CustomSupla.addRelay(104, false); 
  CustomSupla.addRelay(105, false); 
  CustomSupla.addRelay(106, false);
  CustomSupla.addRelay(107, false); 
  CustomSupla.addRelay(108, false);    
   
  CustomSupla.setDigitalWriteFuncImpl(&suplaDigitalWrite);  
  CustomSupla.setName(Supla_name);
  CustomSupla.setStatusFuncImpl(&status_func);
  wifi_station_set_hostname(Supla_name);
  int LocationID = atoi(Location_id);
  CustomSupla.begin(GUID,              // Global Unique Identifier 
                    mac,               // Ethernet MAC address
                    Supla_server,      // SUPLA server address
                    LocationID,        // Location ID 
                    Location_Pass);    // Location Password     

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  wifi_set_sleep_type(LIGHT_SLEEP_T);
  digitalWrite(ledpin, LOW);
  // Turn off the led in 2s
  ticker.attach(2, disableLed);

  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP().toString());

      // Configure mDNS
  if (!MDNS.begin(Supla_name)) {Serial.println("Error setting up MDNS responder!");}
    MDNS.addService("http", "tcp", web_port); // Announce the ESP as an HTTP service
    Serial.println("MDNS http service added. Hostname is set to http://" + String(Supla_name) + ".local:" + String(web_port));

  // Configure the server
  server->on("/json", []() { // JSON handler for more complicated IR blaster routines
    
    Serial.println("Connection received - JSON");
    //int simple = 0;
    int simple = server->arg("simple").toInt() | 0;
    String signature = server->arg("auth");
    String epid = server->arg("epid");
    String mid = server->arg("mid");
    String timestamp = server->arg("time");
    int out = (server->hasArg("out")) ? server->arg("out").toInt() : 1;
    String jsonError;
    String device = server->arg("device");
    int state = (server->hasArg("state")) ? server->arg("state").toInt() : 0;
    int storedState = (deviceState.containsKey(device)) ? deviceState[device] : 888888;
    DynamicJsonBuffer jsonStringBuffer;
    Serial.println("device:" + device + " state:" + state + " storedState:" + storedState);
    String jsonString;
   
    // Handle device state limitations for the global JSON command request
     if ((device != "" && storedState != state) || device == "") { //url sent a device name, and its state doesnt match the stored state, or there isnt a stored state
      if (strlen(passcode) == 0 || server->arg("pass") == passcode) { //passcode good
        if (strlen(user_id) == 0 || validateHMAC(epid, mid, timestamp, signature)) {        
          if (device != "") deviceState[device] = state;
          if (server->hasArg("btn")) {
            String btn = server->arg("btn"); 
            jsonString; 
            btn.toLowerCase(); 
            DynamicJsonBuffer jsonBtnBuffer;
            JsonArray& rootbtn = jsonBtnBuffer.parseArray(btn);
            rootbtn.prettyPrintTo(Serial);
            Serial.println(rootbtn.size());      
            for (int v = 0; v < rootbtn.size(); v++) {
              if (!rootbtn[v].is<JsonObject>()) {
                String vtxt; rootbtn[v].printTo(vtxt); vtxt = "/codes/" + vtxt + ".json";
                vtxt.toLowerCase();
                Serial.println("reading codes file:" + vtxt);
                if (SPIFFS.exists(vtxt)) {
                  File codeFileRead = SPIFFS.open(vtxt, "r"); //reades the json file
                  if (codeFileRead) {
                    DynamicJsonBuffer jsonReadBuffer;
                    JsonObject& filebtn = jsonReadBuffer.parseObject(codeFileRead);                
                    JsonObject& fingers =  rootbtn[v+1];
                    for (auto finger : fingers){
                      if (String(finger.key) == "state") {
                        filebtn["state"] = finger.value;
                        filebtn["device"] = filebtn["name"];
                      }
                    }
                    String jsonStringtmp;
                    filebtn.printTo(jsonStringtmp);  
                    jsonReadBuffer.clear();                  
                    if (jsonStringtmp != "") jsonString = jsonString + jsonStringtmp + ",";            
                  } else {
                    jsonError = jsonError + " " + vtxt;
                  }
                } else {
                  jsonError = jsonError + " " + vtxt;              
                }
              }
            }
            if (jsonString != "") {
              if (jsonString.endsWith(",")) jsonString = jsonString.substring(0, jsonString.length() -1);
              jsonString = "[" + jsonString + "]";
            }
            jsonBtnBuffer.clear();            
          } else if (server->hasArg("plain")){           
            jsonString = server->arg("plain");
          } else {
            rtnMessage.message = "No command received"; rtnMessage.title = "Information"; rtnMessage.type = 1, rtnMessage.httpcode = 200;
          }
          JsonArray& root = jsonStringBuffer.parseArray(jsonString);
          if (root.success()) {
            root.prettyPrintTo(Serial);
            Serial.println("");  
            Serial.println("");                        
            Serial.println("Received code qty: " + root.size());
            if (root.size() > 0) {
              for (int x = 0; x < root.size(); x++) {
                // Handle device state limitations on a per JSON object basis
                String device = root[x]["device"];
                Serial.println("Code read " + device);                  
                int state = root[x]["state"];
                int storedState = (deviceState.containsKey(device)) ? deviceState[device] : 888888;
                Serial.println("device:" + device + " state:" + state + " storedstate:" + storedState);
                if ((device != "" && storedState != state) || device == "") {
                  if (device != "") deviceState[device] = state;  
                  String type = root[x]["type"];
                  String ip = root[x]["ip"];
                  int rdelay = (root[x]["rdelay"] > 0) ? root[x]["rdelay"] : 1000;
                  int pulse = (root[x]["pulse"] > 0) ? root[x]["pulse"] : 1;
                  int pdelay = (root[x]["pdelay"] > 0) ? root[x]["pdelay"] : 100;
                  int repeat = (root[x]["repeat"] > 0) ? root[x]["repeat"] : 1;
                  int xout = root[x]["out"];
                  if (xout == 0) {
                    xout = out;
                  }
                  int duty = (root[x]["duty"] > 0) ? root[x]["duty"] : 50;                  
                  Serial.println("type:" + type + " repeat:" + repeat + " rdelay:" + rdelay + " pulse:" + pulse + " pdelay:" + pdelay + " duty:" + duty);
                  if (type == "delay") {
                    delay(rdelay);
                  } else if (type == "raw") {
                    JsonArray& raw = root[x]["data"]; // Array of unsigned int values for the raw signal
                    int khz = root[x]["khz"];
                    if (khz <= 0) khz = 38; // Default to 38khz if not set
                    rawblast(raw, khz, rdelay, pulse, pdelay, repeat, pickIRsend(xout),duty);
                    rtnMessage.message = "RAW code sent"; rtnMessage.title = "Sucess"; rtnMessage.type = 1, rtnMessage.httpcode = 200;
                  } else if (type == "roku") {
                    String data = root[x]["data"];
                    rokuCommand(ip, data);
                    rtnMessage.message = "ROKU code sent"; rtnMessage.title = "Sucess"; rtnMessage.type = 1, rtnMessage.httpcode = 200;
                  } else if (type != "") {
                    String data = root[x]["data"];
                    String addressString = root[x]["address"];
                    long address = strtoul(addressString.c_str(), 0, 0);
                    int len = root[x]["length"];
                    Serial.println("Sending code: " + type);
                    irblast(type, data, len, rdelay, pulse, pdelay, repeat, address, pickIRsend(xout));
                    rtnMessage.message = type + " code sent"; rtnMessage.title = "Sucess"; rtnMessage.type = 1, rtnMessage.httpcode = 200;
                  } else {
                    rtnMessage.message = "The format of the recevied code is not recorgnised, no code transmitted"; rtnMessage.title = "Error"; rtnMessage.type = 2, rtnMessage.httpcode = 422;
                  }
                } else {
                  Serial.println("Not sending command to " + device + ", already in state " + state);
                  rtnMessage.message = "Some components of the code were held because device was already in the appropriate state"; rtnMessage.title = "Information"; rtnMessage.type = 1, rtnMessage.httpcode = 200; //message = "Code sent. Some components of the code were held because device was already in appropriate state";
                }
              }
            } else {
              rtnMessage.message = "Error with JSON codes passed"; rtnMessage.title = "Error"; rtnMessage.type = 3, rtnMessage.httpcode = 400;
            }
          } else {
          rtnMessage.message = "Error with JSON codes passed"; rtnMessage.title = "Error"; rtnMessage.type = 3, rtnMessage.httpcode = 400;
          }
        } else {
        rtnMessage.message = "HMAC security authentication failed"; rtnMessage.title = "Unauthorized"; rtnMessage.type = 3, rtnMessage.httpcode = 401;
        }
      } else {
      rtnMessage.message = "Passcode wrong"; rtnMessage.title = "Alert"; rtnMessage.type = 3, rtnMessage.httpcode = 401;
      }
    } else {
    rtnMessage.message = "The device is in the same state, not re-transmitting"; rtnMessage.title = "Information"; rtnMessage.type = 1, rtnMessage.httpcode = 200;
    }
    jsonStringBuffer.clear();
    Serial.println(jsonError);
    if (simple && jsonError == "") {
      server->send(rtnMessage.httpcode, "text/plain", rtnMessage.title + ", " + rtnMessage.message);
    } else if (simple && jsonError != "") {
       server->send(rtnMessage.httpcode, "text/plain", "Warning, some or all codes not found - " + jsonError);
    } else if (!simple && jsonError == ""){
      sendHomePage(rtnMessage.message, rtnMessage.title, rtnMessage.type, rtnMessage.httpcode);
    } else if (!simple && jsonError != ""){
      sendHomePage("Some or all codes not found - " + jsonError, "Warning", 2, rtnMessage.httpcode);
    }

  });

  // Setup simple msg server to mirror version 1.0 functionality
  server->on("/msg", []() {
    Serial.println("Connection received - MSG");

    int simple = 0;
    if (server->hasArg("simple")) simple = server->arg("simple").toInt();
    String signature = server->arg("auth");
    String epid = server->arg("epid");
    String mid = server->arg("mid");
    String timestamp = server->arg("time");

    if (strlen(passcode) != 0 && server->arg("pass") != passcode) {
      Serial.println("Unauthorized access");
      if (simple) {
        server->send(401, "text/plain", "Unauthorized, invalid passcode");
      } else {
        sendHomePage("Invalid passcode", "Unauthorized", 3, 401); // 401
      }
    } else if (strlen(user_id) != 0 && !validateHMAC(epid, mid, timestamp, signature)) {
      server->send(401, "text/plain", "Unauthorized, HMAC security authentication");
    } else {
      digitalWrite(ledpin, LOW);
      ticker.attach(0.5, disableLed);
      String type = server->arg("type");
      String data = server->arg("data");
      String ip = server->arg("ip");

      // Handle device state limitations
      if (server->hasArg("device")) {
        String device = server->arg("device");
        Serial.println("Device name detected " + device);
        int state = (server->hasArg("state")) ? server->arg("state").toInt() : 0;
        if (deviceState.containsKey(device)) {
          Serial.println("Contains the key!");
          Serial.println(state);
          int storedState = deviceState[device];
          Serial.println(storedState);
          if (state == storedState) {
            if (simple) {
              server->send(200, "text/html", "Not sending command to " + device + ", already in state " + state);
            } else {
              sendHomePage("Not sending command to " + device + ", already in state " + state, "Warning", 2); // 200
            }
            Serial.println("Not sending command to " + device + ", already in state " + state);
            return;
          } else {
            Serial.println("Setting device " + device + " to state " + state);
            deviceState[device] = state;
          }
        } else {
          Serial.println("Setting device " + device + " to state " + state);
          deviceState[device] = state;
        }
      }

      int len = server->arg("length").toInt();
      long address = 0;
      if (server->hasArg("address")) {
        String addressString = server->arg("address");
        address = strtoul(addressString.c_str(), 0, 0);
      }

      int rdelay = (server->hasArg("rdelay")) ? server->arg("rdelay").toInt() : 1000;
      int pulse = (server->hasArg("pulse")) ? server->arg("pulse").toInt() : 1;
      int pdelay = (server->hasArg("pdelay")) ? server->arg("pdelay").toInt() : 100;
      int repeat = (server->hasArg("repeat")) ? server->arg("repeat").toInt() : 1;
      int out = (server->hasArg("out")) ? server->arg("out").toInt() : 1;
      if (server->hasArg("code")) {
        String code = server->arg("code");
        char separator = ':';
        data = getValue(code, separator, 0);
        type = getValue(code, separator, 1);
        len = getValue(code, separator, 2).toInt();
      }

      if (type == "roku") {
        rokuCommand(ip, data);
      } else {
        irblast(type, data, len, rdelay, pulse, pdelay, repeat, address, pickIRsend(out));
      }

      if (simple) {
        server->send(200, "text/html", "Success, code sent");
      } else {
        sendHomePage("Code Sent", "Success", 1); // 200
      }
    }
  });

  server->on("/received", []() {
    Serial.println("Connection received to received detail page");
    int id = server->arg("id").toInt();
    String output;
    if (id == 1 && last_recv.valid) {
      sendCodePage(last_recv);
    } else if (id == 2 && last_recv_2.valid) {
      sendCodePage(last_recv_2);
    } else if (id == 3 && last_recv_3.valid) {
      sendCodePage(last_recv_3);
    } else if (id == 4 && last_recv_4.valid) {
      sendCodePage(last_recv_4);
    } else if (id == 5 && last_recv_5.valid) {
      sendCodePage(last_recv_5);
    } else {
      sendHomePage("Code does not exist", "Alert", 2, 404); // 404
    }      

  });

  // Loads the store code page with the selected code, if btn parameter passed store the code to JSON file
  server->on("/store", []() {
    Serial.println("Connection received to store code page");
    int id = server->arg("id").toInt();
    String btn = server->arg("name_input");
    btn.toLowerCase();
    Serial.print("btn name: ");Serial.println(btn);
    String pulse = server->arg("pulse");
    String pulse_delay = server->arg("pulse_delay");    
    String repeat = server->arg("repeat");
    String repeat_delay = server->arg("repeat_delay");          
    String output;

      if (btn != "") {
        storeReceivedCode(last_recv, btn, pulse, pulse_delay, repeat, repeat_delay);
      } else {
        storeCodePage(last_recv, 1);
      }
  });

  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server->on("/listcodes", HTTP_POST, []() {server->send(200, "text/plain", "");}, {handleFileUpload});
    
  // list the JSON codes contents
  server->on("/listcodes", []() {
    String item = server->arg("item");
    if (server->hasArg("item")) {
      Serial.println("Connection received - delete item " + item);
      if (DeleteJSONitem(item)) listStoredCodes(); //send_redirect("/listcodes"); //listStoredCodes("Code removed", "", 1, 404);    
    }
    listStoredCodes();
  });

  server->on("/deleteall", []() {
    if(deletecodefiles("/codes/")) sendHomePage("Done", "Alert", 2, 404); //server->send(404, "text/plain", "FileNotFound");
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

  //called when the url is not defined here
  //use it to load content from SPIFFS
  server->onNotFound( []() {
    if(!handleFileRead(server->uri()))
      sendHomePage("Resource not found", "Alert", 2, 404); //server->send(404, "text/plain", "FileNotFound");
  });
  
  server->begin();
  Serial.println("HTTP Server started on port " + String(web_port));

  externalIP();

  irsend1.begin();
  irrecv.enableIRIn();
  Serial.println("Ready to send and receive IR signals");
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
  server->sendContent("  <head>\n");
  server->sendContent("    <meta name='viewport' content='width=device-width, initial-scale=.75' />\n");
  server->sendContent("    <link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css'>\n");
  server->sendContent("    <script src='https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js'></script>\n");  
  server->sendContent("    <script src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js'></script>\n");
  server->sendContent("    <style>@media (max-width: 991px) {.nav-pills>li {float: none; margin-left: 0; margin-top: 5px; text-align: center;}}</style>\n");
  server->sendContent("    <title>ESP8266 IR Controller (" + String(Supla_name) + ")</title>\n");
  server->sendContent("  </head>\n");
  server->sendContent("  <body>\n");
  server->sendContent("   <nav class='navbar navbar-inverse'>\n");
  server->sendContent("      <a class='navbar-brand' href='/'>ESP8266 IR</a>\n");
  server->sendContent("      <ul class='nav navbar-nav'>\n");
  server->sendContent("       <li class='dropdown'><a class='dropdown-toggle' data-toggle='dropdown' href='#'>Tools <span class='caret'></span></a>\n");
  server->sendContent("        <ul class='dropdown-menu'>\n");
  server->sendContent("         <li><a href='/listcodes'>Stored Codes</a></li>\n");
  server->sendContent("         <li><a href='#' data-toggle='modal' data-target='#myModal'>Upload button code file</a></li>\n");
  server->sendContent("         <li class='divider'></li>\n");
  server->sendContent("         <li><a target='_blank' href='https://forum.supla.org/''>forum.supla.org</a></li>\n");  
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
  server->sendContent("      </form>\n");
  server->sendContent("          <button type='button' class='btn btn-default' data-dismiss='modal'>Close</button>\n");
  server->sendContent("        </div>\n");
  server->sendContent("     </div>\n");
  server->sendContent("    </div>\n");
  server->sendContent("   </div>\n");  
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
  server->sendContent("      </div><hr />\n");
}

//+=============================================================================
// Send footer HTML
//
void sendFooter() {

  server->sendContent("      <hr />\n");
  /* 
  server->sendContent("      <div class='row'><div class='col-md-12'><em>" + epochToString(millis() / 1000) + " uptime; EPOCH " + String(timeClient.getUnixTime() - timeOffset) + "</em> / <em id='jepoch'></em> ( <em id='jdiff'></em> )</div></div>\n");
  server->sendContent("      <script>document.getElementById('jepoch').innerHTML = Math.round((new Date()).getTime() / 1000)</script>");
  server->sendContent("      <script>document.getElementById('jdiff').innerHTML = Math.abs(Math.round((new Date()).getTime() / 1000) - " + String(timeClient.getUnixTime() - timeOffset) + ")</script>");  
  if (strlen(user_id) != 0)
  server->sendContent("      <div class='row'><div class='col-md-12'><em>Device secured with SHA256 authentication. Only commands sent and verified with Amazon Alexa and the IR Controller Skill will be processed</em></div></div>");
  if (authError)
  server->sendContent("      <div class='row'><div class='col-md-12'><em>Error - last authentication failed because HMAC signatures did not match, see serial output for debugging details</em></div></div>");
  if (timeAuthError > 0)
  server->sendContent("      <div class='row'><div class='col-md-12'><em>Error - last authentication failed because your timestamps are out of sync, see serial output for debugging details. Timediff: " + String(timeAuthError) + "</em></div></div>");
  if (externalIPError)
  server->sendContent("      <div class='row'><div class='col-md-12'><em>Error - unable to retrieve external IP address, this may be due to bad network settings. There is currently a bug with the latest versions of ESP8266 for Arduino, please use version 2.4.0 along with lwIP v1.4 Prebuilt to resolve this</em></div></div>");
  time_t timenow = timeClient.getUnixTime() - timeOffset;
  if (!validEPOCH(timenow))
  server->sendContent("      <div class='row'><div class='col-md-12'><em>Error - EPOCH time is inappropraitely low, likely connection to external time server has failed, check your network settings</em></div></div>");
  if (userIDError)*/
 // server->sendContent("      <div class='row'><div class='col-md-12'><em>Supla status:" + String(Supla_status) + "</em></div></div>");
  server->sendContent("      <div class='row'><div class='col-md-12'><em>memory heap: " + String(ESP.getFreeHeap()) + "</em></div></div>");
  server->sendContent("   </div>\n");
  server->sendContent("  </body>\n");
  server->sendContent("</html>\n");
  server->client().stop();
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
  server->sendContent("          <h3>Codes Transmitted</h3>\n");
  server->sendContent("          <table class='table table-striped' style='table-layout: fixed;'>\n");
  server->sendContent("            <thead><tr><th>Command</th><th>Type</th><th>Length</th><th>Address</th></tr></thead>\n"); //Title
  server->sendContent("            <tbody>\n");
  if (last_send.valid)
  server->sendContent("              <tr class='text-uppercase'><td><code>" + String(last_send.data) + "</code></td><td><code>" + String(last_send.encoding) + "</code></td><td><code>" + String(last_send.bits) + "</code></td><td><code>" + String(last_send.address) + "</code></td></tr>\n");
  if (last_send_2.valid)
  server->sendContent("              <tr class='text-uppercase'><td><code>" + String(last_send_2.data) + "</code></td><td><code>" + String(last_send_2.encoding) + "</code></td><td><code>" + String(last_send_2.bits) + "</code></td><td><code>" + String(last_send_2.address) + "</code></td></tr>\n");
  if (last_send_3.valid)
  server->sendContent("              <tr class='text-uppercase'><td><code>" + String(last_send_3.data) + "</code></td><td><code>" + String(last_send_3.encoding) + "</code></td><td><code>" + String(last_send_3.bits) + "</code></td><td><code>" + String(last_send_3.address) + "</code></td></tr>\n");
  if (last_send_4.valid)
  server->sendContent("              <tr class='text-uppercase'><td><code>" + String(last_send_4.data) + "</code></td><td><code>" + String(last_send_4.encoding) + "</code></td><td><code>" + String(last_send_4.bits) + "</code></td><td><code>" + String(last_send_4.address) + "</code></td></tr>\n");
  if (last_send_5.valid)
  server->sendContent("              <tr class='text-uppercase'><td><code>" + String(last_send_5.data) + "</code></td><td><code>" + String(last_send_5.encoding) + "</code></td><td><code>" + String(last_send_5.bits) + "</code></td><td><code>" + String(last_send_5.address) + "</code></td></tr>\n");
  if (!last_send.valid && !last_send_2.valid && !last_send_3.valid && !last_send_4.valid && !last_send_5.valid)
  server->sendContent("              <tr><td colspan='5' class='text-center'><em>No codes sent</em></td></tr>");
  server->sendContent("            </tbody></table>\n");
  server->sendContent("          </div></div>\n");
  server->sendContent("      <div class='row'>\n");
  server->sendContent("        <div class='col-md-12'>\n");
  server->sendContent("          <h3>Codes Received</h3>\n");
  server->sendContent("          <table class='table table-striped' style='table-layout: fixed;'>\n");
  server->sendContent("            <thead><tr><th>Command</th><th>Type</th><th>Length</th><th>Address</th><th>Action</th></tr></thead>\n"); //Title
  server->sendContent("            <tbody>\n");
  if (last_recv.valid) {
    if (String(last_recv.encoding) == "UNKNOWN") {
      jsonTest = "[{data:[" + String(last_recv.raw) + "],type:raw,khz:38}]";
    } else if (String(last_recv.encoding) == "PANASONIC") {
      jsonTest = "[{data:" + String(last_recv.data) + ",type:" + String(last_recv.encoding) + ",length:" + String(last_recv.bits) + ",address:" + String(last_recv.address) + "}]";
    } else {
      jsonTest = "[{data:" + String(last_recv.data) + ",type:" + String(last_recv.encoding) + ",length:" + String(last_recv.bits) + "}]";
    }
    server->sendContent("              <form style='display:inline-block; margin:0; padding:0' action='/json' method='post'><input type='hidden' name='pass' value='" + String(passcode) + "'><tr class='text-uppercase'><td><code>" + String(last_recv.data) + "</code></td><td><code>" + String(last_recv.encoding) + "</code></td><td><code>" + String(last_recv.bits) + "</code></td><td><code>" + String(last_recv.address) + "</code></td><td><button name='plain' value='" + jsonTest + "' class='btn btn-success btn-xs' type='submit'>TEST</button> <a class='btn btn-warning btn-xs' href='/store' role='button'>Store</a></td></tr></form>\n");
  }  
  if (last_recv_2.valid) {
    if (String(last_recv_2.encoding) == "UNKNOWN") {
      jsonTest = "[{data:[" + String(last_recv_2.raw) + "],type:raw,khz:38}]";
    } else if (String(last_recv_2.encoding) == "PANASONIC") {
      jsonTest = "[{data:" + String(last_recv_2.data) + ",type:" + String(last_recv_2.encoding) + ",length:" + String(last_recv_2.bits) + ",address:" + String(last_recv_2.address) + "}]";
    } else {
      jsonTest = "[{data:" + String(last_recv_2.data) + ",type:" + String(last_recv_2.encoding) + ",length:" + String(last_recv_2.bits) + "}]";
    }
    server->sendContent("              <form style='display:inline-block; margin:0; padding:0' action='/json' method='post'><input type='hidden' name='pass' value='" + String(passcode) + "'><tr class='text-uppercase'><td><code>" + String(last_recv_2.data) + "</code></td><td><code>" + String(last_recv_2.encoding) + "</code></td><td><code>" + String(last_recv_2.bits) + "</code></td><td><code>" + String(last_recv_2.address) + "</code></td><td><button name='plain' value='" + jsonTest + "' class='btn btn-success btn-xs' type='submit'>TEST</button></td></tr></form>\n");
    }
    if (last_recv_3.valid) {
    if (String(last_recv_3.encoding) == "UNKNOWN") {
      jsonTest = "[{data:[" + String(last_recv_3.raw) + "],type:raw,khz:38}]";
    } else if (String(last_recv_3.encoding) == "PANASONIC") {
      jsonTest = "[{data:" + String(last_recv_3.data) + ",type:" + String(last_recv_3.encoding) + ",length:" + String(last_recv_3.bits) + ",address:" + String(last_recv_3.address) + "}]";
    } else {
      jsonTest = "[{data:" + String(last_recv_3.data) + ",type:" + String(last_recv_3.encoding) + ",length:" + String(last_recv_3.bits) + "}]";
    }
    server->sendContent("              <form style='display:inline-block; margin:0; padding:0' action='/json' method='post'><input type='hidden' name='pass' value='" + String(passcode) + "'><tr class='text-uppercase'><td><code>" + String(last_recv_3.data) + "</code></td><td><code>" + String(last_recv_3.encoding) + "</code></td><td><code>" + String(last_recv_3.bits) + "</code></td><td><code>" + String(last_recv_3.address) + "</code></td><td><button name='plain' value='" + jsonTest + "' class='btn btn-success btn-xs' type='submit'>TEST</button></td></tr></form>\n");
    }
    if (last_recv_4.valid) {
    if (String(last_recv_4.encoding) == "UNKNOWN") {
      jsonTest = "[{data:[" + String(last_recv_4.raw) + "],type:raw,khz:38}]";
    } else if (String(last_recv_4.encoding) == "PANASONIC") {
      jsonTest = "[{data:" + String(last_recv_4.data) + ",type:" + String(last_recv_4.encoding) + ",length:" + String(last_recv_4.bits) + ",address:" + String(last_recv_4.address) + "}]";
    } else {
      jsonTest = "[{data:" + String(last_recv_4.data) + ",type:" + String(last_recv_4.encoding) + ",length:" + String(last_recv_4.bits) + "}]";
    }
    server->sendContent("              <form style='display:inline-block; margin:0; padding:0' action='/json' method='post'><input type='hidden' name='pass' value='" + String(passcode) + "'><tr class='text-uppercase'><td><code>" + String(last_recv_4.data) + "</code></td><td><code>" + String(last_recv_4.encoding) + "</code></td><td><code>" + String(last_recv_4.bits) + "</code></td><td><code>" + String(last_recv_4.address) + "</code></td><td><button name='plain' value='" + jsonTest + "' class='btn btn-success btn-xs' type='submit'>TEST</button></td></tr></form>\n");
    }
    if (last_recv_5.valid) {
    if (String(last_recv_5.encoding) == "UNKNOWN") {
      jsonTest = "[{data:[" + String(last_recv_5.raw) + "],type:raw,khz:38}]";
    } else if (String(last_recv_5.encoding) == "PANASONIC") {
      jsonTest = "[{data:" + String(last_recv_5.data) + ",type:" + String(last_recv_5.encoding) + ",length:" + String(last_recv_5.bits) + ",address:" + String(last_recv_5.address) + "}]";
    } else {
      jsonTest = "[{data:" + String(last_recv_5.data) + ",type:" + String(last_recv_5.encoding) + ",length:" + String(last_recv_5.bits) + "}]";
    }
    server->sendContent("              <form style='display:inline-block; margin:0; padding:0' action='/json' method='post'><input type='hidden' name='pass' value='" + String(passcode) + "'><tr class='text-uppercase'><td><code>" + String(last_recv_5.data) + "</code></td><td><code>" + String(last_recv_5.encoding) + "</code></td><td><code>" + String(last_recv_5.bits) + "</code></td><td><code>" + String(last_recv_5.address) + "</code></td><td><button name='plain' value='" + jsonTest + "' class='btn btn-success btn-xs' type='submit'>TEST</button></td></tr></form>\n");
    }
  if (!last_recv.valid && !last_recv_2.valid && !last_recv_3.valid && !last_recv_4.valid && !last_recv_5.valid)
  server->sendContent("              <tr><td colspan='6' class='text-center'><em>No codes received</em></td></tr>");
  server->sendContent("            </tbody></table>\n");
  server->sendContent("          </div></div>\n");
  server->sendContent("      <div class='row'>\n");
  server->sendContent("        <div class='col-md-12'>\n");
  server->sendContent("          <ul class='list-unstyled'>\n");
  server->sendContent("            <li><span class='badge'>GPIO " + String(pinr1) + "</span> Receiving </li>\n");
  server->sendContent("            <li><span class='badge'>GPIO " + String(pins1) + "</span> Transmitter 1 </li></ul>\n");
  server->sendContent("        </div>\n");
  server->sendContent("      </div>\n");
  sendFooter();
}

//+=============================================================================
// Stream code page HTML
//
void sendCodePage(Code selCode) {
  sendCodePage(selCode, 200);
}
void sendCodePage(Code selCode, int httpcode){
  sendHeader(httpcode);
  server->sendContent("      <div class='row'>\n");
  server->sendContent("        <div class='col-md-12'>\n");
  server->sendContent("          <h2><span class='label label-success'>" + String(selCode.data) + ":" + String(selCode.encoding) + ":" + String(selCode.bits) + "</span></h2><br/>\n");
  server->sendContent("          <dl class='dl-horizontal'>\n");
  server->sendContent("            <dt>Data</dt>\n");
  server->sendContent("            <dd><code>" + String(selCode.data)  + "</code></dd></dl>\n");
  server->sendContent("          <dl class='dl-horizontal'>\n");
  server->sendContent("            <dt>Type</dt>\n");
  server->sendContent("            <dd><code>" + String(selCode.encoding)  + "</code></dd></dl>\n");
  server->sendContent("          <dl class='dl-horizontal'>\n");
  server->sendContent("            <dt>Length</dt>\n");
  server->sendContent("            <dd><code>" + String(selCode.bits)  + "</code></dd></dl>\n");
  server->sendContent("          <dl class='dl-horizontal'>\n");
  server->sendContent("            <dt>Address</dt>\n");
  server->sendContent("            <dd><code>" + String(selCode.address)  + "</code></dd></dl>\n");
  server->sendContent("          <dl class='dl-horizontal'>\n");
  server->sendContent("            <dt>Raw</dt>\n");
  server->sendContent("            <dd><code>" + String(selCode.raw)  + "</code></dd></dl>\n");
  server->sendContent("        </div></div>\n");
  server->sendContent("      <div class='row'>\n");
  server->sendContent("        <div class='col-md-12'>\n");
  server->sendContent("          <div class='alert alert-warning'>Don't forget to add your passcode to the URLs below if you set one</div>\n");
  server->sendContent("      </div></div>\n");
  if (String(selCode.encoding) == "UNKNOWN") {
  server->sendContent("      <div class='row'>\n");
  server->sendContent("        <div class='col-md-12'>\n");
  server->sendContent("          <ul class='list-unstyled'>\n");
  server->sendContent("            <li>Hostname <span class='label label-default'>JSON</span></li>\n");
  server->sendContent("            <li><pre>http://" + String(Supla_name) + ".local:" + String(web_port) + "/json?plain=[{'data':[" + String(selCode.raw) + "],'type':'raw','khz':38}]</pre></li>\n");
  server->sendContent("            <li>Local IP <span class='label label-default'>JSON</span></li>\n");
  server->sendContent("            <li><pre>http://" + WiFi.localIP().toString() + ":" + String(web_port) + "/json?plain=[{'data':[" + String(selCode.raw) + "],'type':'raw','khz':38}]</pre></li>\n");
  server->sendContent("            <li>External IP <span class='label label-default'>JSON</span></li>\n");
  server->sendContent("            <li><pre>http://" + externalIP() + ":" + String(web_port) + "/json?plain=[{'data':[" + String(selCode.raw) + "],'type':'raw','khz':38}]</pre></li></ul>\n");
  } else if (String(selCode.encoding) == "PANASONIC") {
  //} else if (strtoul(selCode.address, 0, 0) > 0) {
  server->sendContent("      <div class='row'>\n");
  server->sendContent("        <div class='col-md-12'>\n");
  server->sendContent("          <ul class='list-unstyled'>\n");
  server->sendContent("            <li>Hostname <span class='label label-default'>MSG</span></li>\n");
  server->sendContent("            <li><pre>http://" + String(Supla_name) + ".local:" + String(web_port) + "/msg?code=" + String(selCode.data) + ":" + String(selCode.encoding) + ":" + String(selCode.bits) + "&address=" + String(selCode.address) + "</pre></li>\n");
  server->sendContent("            <li>Local IP <span class='label label-default'>MSG</span></li>\n");
  server->sendContent("            <li><pre>http://" + WiFi.localIP().toString() + ":" + String(web_port) + "/msg?code=" + String(selCode.data) + ":" + String(selCode.encoding) + ":" + String(selCode.bits) + "&address=" + String(selCode.address) + "</pre></li>\n");
  server->sendContent("            <li>External IP <span class='label label-default'>MSG</span></li>\n");
  server->sendContent("            <li><pre>http://" + externalIP() + ":" + String(web_port) + "/msg?code=" + selCode.data + ":" + String(selCode.encoding) + ":" + String(selCode.bits) + "&address=" + String(selCode.address) + "</pre></li></ul>\n");
  server->sendContent("          <ul class='list-unstyled'>\n");
  server->sendContent("            <li>Hostname <span class='label label-default'>JSON</span></li>\n");
  server->sendContent("            <li><pre>http://" + String(Supla_name) + ".local:" + String(web_port) + "/json?plain=[{'data':'" + String(selCode.data) + "','type':'" + String(selCode.encoding) + "','length':" + String(selCode.bits) + ",'address':'" + String(selCode.address) + "'}]</pre></li>\n");
  server->sendContent("            <li>Local IP <span class='label label-default'>JSON</span></li>\n");
  server->sendContent("            <li><pre>http://" + WiFi.localIP().toString() + ":" + String(web_port) + "/json?plain=[{'data':'" + String(selCode.data) + "','type':'" + String(selCode.encoding) + "','length':" + String(selCode.bits) + ",'address':'" + String(selCode.address) + "'}]</pre></li>\n");
  server->sendContent("            <li>External IP <span class='label label-default'>JSON</span></li>\n");
  server->sendContent("            <li><pre>http://" + externalIP() + ":" + String(web_port) + "/json?plain=[{'data':'" + String(selCode.data) + "','type':'" + String(selCode.encoding) + "','length':" + String(selCode.bits) + ",'address':'" + String(selCode.address) + "'}]</pre></li></ul>\n");
  } else {
  server->sendContent("      <div class='row'>\n");
  server->sendContent("        <div class='col-md-12'>\n");
  server->sendContent("          <ul class='list-unstyled'>\n");
  server->sendContent("            <li>Hostname <span class='label label-default'>MSG</span></li>\n");
  server->sendContent("            <li><pre>http://" + String(Supla_name) + ".local:" + String(web_port) + "/msg?code=" + String(selCode.data) + ":" + String(selCode.encoding) + ":" + String(selCode.bits) + "</pre></li>\n");
  server->sendContent("            <li>Local IP <span class='label label-default'>MSG</span></li>\n");
  server->sendContent("            <li><pre>http://" + WiFi.localIP().toString() + ":" + String(web_port) + "/msg?code=" + String(selCode.data) + ":" + String(selCode.encoding) + ":" + String(selCode.bits) + "</pre></li>\n");
  server->sendContent("            <li>External IP <span class='label label-default'>MSG</span></li>\n");
  server->sendContent("            <li><pre>http://" + externalIP() + ":" + String(web_port) + "/msg?code=" + selCode.data + ":" + String(selCode.encoding) + ":" + String(selCode.bits) + "</pre></li></ul>\n");
  server->sendContent("          <ul class='list-unstyled'>\n");
  server->sendContent("            <li>Hostname <span class='label label-default'>JSON</span></li>\n");
  server->sendContent("            <li><pre>http://" + String(Supla_name) + ".local:" + String(web_port) + "/json?plain=[{'data':'" + String(selCode.data) + "','type':'" + String(selCode.encoding) + "','length':" + String(selCode.bits) + "}]</pre></li>\n");
  server->sendContent("            <li>Local IP <span class='label label-default'>JSON</span></li>\n");
  server->sendContent("            <li><pre>http://" + WiFi.localIP().toString() + ":" + String(web_port) + "/json?plain=[{'data':'" + String(selCode.data) + "','type':'" + String(selCode.encoding) + "','length':" + String(selCode.bits) + "}]</pre></li>\n");
  server->sendContent("            <li>External IP <span class='label label-default'>JSON</span></li>\n");
  server->sendContent("            <li><pre>http://" + externalIP() + ":" + String(web_port) + "/json?plain=[{'data':'" + String(selCode.data) + "','type':'" + String(selCode.encoding) + "','length':" + String(selCode.bits) + "}]</pre></li></ul>\n");
  }
  server->sendContent("        </div>\n");
  server->sendContent("     </div>\n");
  sendFooter();
}


//+=============================================================================
// Stream store code page HTML
//
void storeCodePage(Code selCode, int id) {
  storeCodePage(selCode, id, 200);
}
void storeCodePage(Code selCode, int id, int httpcode){
  
  sendHeader(httpcode);
  server->sendContent("      <div class='row'>\n");
  server->sendContent("        <div class='col-md-12'>\n");  
  server->sendContent("          <h2><span class='label label-success'>" + String(selCode.data) + ":" + String(selCode.encoding) + ":" + String(selCode.bits) + "</span></h2>\n");
  server->sendContent("        </div></div>\n");
  server->sendContent("          <form action='/store' method='POST'>\n");
  server->sendContent("          <div class='form-group row'>\n");
  server->sendContent("              <label for='name_input' class='col-sm-2 col-form-label col-form-label-sm'>Select the remote channel number:</label>\n");
  server->sendContent("              <div class='col-sm-10'>\n");
  server->sendContent("              <select class='form-control form-control-sm' name='name_input' name='id'>\n");
  server->sendContent("               <option>supla1</option>\n");
  server->sendContent("               <option>supla2</option>\n");
  server->sendContent("               <option>supla3</option>\n");
  server->sendContent("               <option>supla4</option>\n");
  server->sendContent("               <option>supla5</option>\n");
  server->sendContent("               <option>supla6</option>\n");
  server->sendContent("               <option>supla7</option>\n");
  server->sendContent("               <option>supla8</option>\n");
  server->sendContent("               </select>\n");
  server->sendContent("              </div>\n");
  server->sendContent("          </div>\n");
  server->sendContent("          <div class='form-group row'>\n");
  server->sendContent("              <label for='pulse' class='col-sm-2 col-form-label col-form-label-sm'>Specify the number of pulses:</label>\n");
  server->sendContent("              <div class='col-sm-10'>\n");
  server->sendContent("               <input type='text' class='form-control form-control-sm' name='pulse' placeholder='Pulses (default - single pulse)'>\n");
  server->sendContent("              </div>\n");
  server->sendContent("          </div>\n");
  server->sendContent("          <div class='form-group row'>\n");
  server->sendContent("              <label for='pulse_delay' class='col-sm-2 col-form-label col-form-label-sm'>Specify the pulse delay:</label>\n");
  server->sendContent("              <div class='col-sm-10'>\n");
  server->sendContent("               <input type='text' class='form-control form-control-sm' name='pulse_delay' placeholder='Pulse delay (milliseconds - default 100)'>\n");
  server->sendContent("              </div>\n");
  server->sendContent("          </div>\n");
  server->sendContent("          <div class='form-group row'>\n");
  server->sendContent("              <label for='repeat' class='col-sm-2 col-form-label col-form-label-sm'>Specify the number of repeats:</label>\n");
  server->sendContent("              <div class='col-sm-10'>\n");
  server->sendContent("               <input type='text' class='form-control form-control-sm' name='repeat' placeholder='Repeat (default - no repeat)'>\n");
  server->sendContent("              </div>\n");
  server->sendContent("          </div>\n");
  server->sendContent("          <div class='form-group row'>\n");
  server->sendContent("              <label for='repeat_delay' class='col-sm-2 col-form-label col-form-label-sm'>Specify the repeat delay:</label>\n");
  server->sendContent("              <div class='col-sm-10'>\n");
  server->sendContent("               <input type='text' class='form-control form-control-sm' name='repeat_delay' placeholder='Repeat delay (milliseconds - default 1000)'>\n");
  server->sendContent("              </div>\n");
  server->sendContent("          </div>\n");
  server->sendContent("          <div class='form-group row'>\n");
  server->sendContent("              <div class='col-sm-10'>\n");
  server->sendContent("              <button type='submit' formaction='/store' class='btn btn-danger'>Store</button>\n");    
  server->sendContent("              </div>\n");
  server->sendContent("          </div>\n");     
  server->sendContent("          </form>\n");
  server->sendContent("      <hr />\n");    
  server->sendContent("      <div class='row'><div class='col-md-12'><em>set the name for the code as follow: <h4>supla1</h4> for the code that is transmitted with the first channel in Supla App </em></div></div>");
  server->sendContent("      <div class='row'><div class='col-md-12'><em><h4>supla2</h4> for the second channel in Supla App <h4>supla8</h4>  and so on until last channel in Supla App</em></div></div>");
  
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
  server->sendContent("            <thead><tr><th>Button Name</th><th>Type</th><th>Action</th></tr></thead>\n"); //Title
  server->sendContent("            <tbody>\n");

  DynamicJsonBuffer jsonCodeBtnBuffer;
  JsonArray& rootbtn = jsonCodeBtnBuffer.parseArray(listcodefiles("/codes/"));
  //rootbtn.prettyPrintTo(Serial); 
  if (rootbtn.size() != 0) {
    if (SPIFFS.begin()) {
      for (auto v : rootbtn) {
        String vtxt = v["name"];
        //Serial.println(vtxt);
        vtxt.toLowerCase();
        if (SPIFFS.exists(vtxt)) {
          File codeFileRead = SPIFFS.open(vtxt, "r"); //reads the json file
          if (codeFileRead) {
            DynamicJsonBuffer jsonReadCodeFileBuffer;
            JsonObject& filebtn = jsonReadCodeFileBuffer.parseObject(codeFileRead);   
            codeFileRead.close();             
            String jsonStringtype = filebtn["type"];
            String jsonStringbtn = filebtn["name"];
            //Serial.println(jsonStringbtn + " " + jsonStringtype);
            jsonReadCodeFileBuffer.clear();
            if(filebtn.containsKey("type") && filebtn.containsKey("name")) server->sendContent("<tr class='text-uppercase'><td>" + jsonStringbtn + "</td><td>" + jsonStringtype + "</td><td><a class='btn btn-success btn-xs' href='/json?btn=[" + jsonStringbtn + "]' role='button'>Test</a> <a class='btn btn-primary btn-xs' href='" + vtxt + "' role='button'>Download</a> <a class='btn btn-danger btn-xs' href='/listcodes?item=" + jsonStringbtn + "' role='button'>Delete</a></td></tr>\n");
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
// Return which IRsend object to act on
//
IRsend pickIRsend (int out) {
  switch (out) {
    case 1: return irsend1;
    case 2: return irsend1;
    case 3: return irsend1;
    case 4: return irsend1;
    default: return irsend1;
  }
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
  }
  return output;
}


//+=============================================================================
// Stores the selected code to JSON file
//
void storeReceivedCode (Code code, String btn, String pulse, String pulse_delay, String repeat, String repeat_delay) {
  //generate the nested json from the code
  DynamicJsonBuffer jsonWriteBuffer;  
  JsonObject& jsonadd = jsonWriteBuffer.createObject();
  btn.toLowerCase();
  jsonadd["name"] = btn;       
  if (String(code.encoding) == "UNKNOWN") {
    jsonadd["data"] = RawJson("[" + code.raw + "]");
    jsonadd["type"] = "raw";
    jsonadd["rawdata"] = code.data;    
    jsonadd["khz"] = 38;
  } else if (String(code.encoding) == "PANASONIC") {
    jsonadd["raw"] = RawJson("[" + code.raw + "]");    
    jsonadd["type"] = code.encoding;
    jsonadd["address"] = code.address;
    jsonadd["data"] = code.data;
    jsonadd["length"] = code.bits;
  } else {
    jsonadd["raw"] = RawJson("[" + code.raw + "]");    
    jsonadd["type"] = code.encoding;
    jsonadd["data"] = code.data;
    jsonadd["length"] = code.bits;
  }
  if (pulse != "") jsonadd["pulse"] = pulse;
  if (pulse_delay != "") jsonadd["pdelay"] =  pulse_delay;
  if (repeat != "") jsonadd["repeat"] =  repeat;
  if (repeat_delay != "") jsonadd["rdelay"] =  repeat_delay;
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
  //SPIFFS.format();
  return true;
}


//+=============================================================================
// Saves the codes JSON file with code (called storeReceivedCode)
//
bool saveJSON (JsonObject& json, String btn){
  btn.toLowerCase();
  if (SPIFFS.begin()) {     //read current codes file
    //Serial.println("  mounted file system");
    if (SPIFFS.exists("/codes/" + btn +".json")) DeleteJSONitem(btn);
    File codeFileWrite = SPIFFS.open("/codes/" + btn +".json", "w"); //writes the updated json to code file
    if (codeFileWrite) {
      json.printTo(codeFileWrite);
      json.prettyPrintTo(Serial);
      codeFileWrite.close();      
      Serial.println("Codes written successfully");
      return true;
    }
  }
}

//+=============================================================================
// lists all stored codes for display in storedcodepage
//
String listcodefiles(String path) {
  path.toLowerCase();
  Serial.println("handleFileList: " + path);
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
  Serial.println(output);  
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
    //Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
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
  String r = "";
      for (uint16_t i = 1; i < results->rawlen; i++) {
      r += results->rawbuf[i] * RAWTICK;
      if (i < results->rawlen - 1)
        r += ",";                           // ',' not needed on last one
      //if (!(i & 1)) r += " ";
    }
  codeData.raw = r;
  if (results->decode_type != UNKNOWN) {
    strncpy(codeData.address, ("0x" + String(results->address, HEX)).c_str(), 20);
    strncpy(codeData.command, ("0x" + String(results->command, HEX)).c_str(), 40);
  } else {
    strncpy(codeData.address, "0x0", 20);
    strncpy(codeData.command, "0x0", 40);
  }

}


//+=============================================================================
// Send IR codes to variety of sources
//
void irblast(String type, String dataStr, unsigned int len, int rdelay, int pulse, int pdelay, int repeat, long address, IRsend irsend) {
  Serial.println("Code transmit");
  type.toLowerCase();
  uint64_t data = strtoull(("0x" + dataStr).c_str(), 0, 0);
  holdReceive = true;
  Serial.println("Blocking incoming IR signals");
  // Repeat Loop
  for (int r = 0; r < repeat; r++) {
    // Pulse Loop
    for (int p = 0; p < pulse; p++) {
      serialPrintUint64(data, HEX);
      Serial.print(":");
      Serial.print(type);
      Serial.print(":");
      Serial.println(len);
      if (type == "nec") {
        irsend.sendNEC(data, len);
      } else if (type == "sony") {
        irsend.sendSony(data, len);
      } else if (type == "coolix") {
        irsend.sendCOOLIX(data, len);
      } else if (type == "whynter") {
        irsend.sendWhynter(data, len);
      } else if (type == "panasonic") {
        Serial.print("Address: ");
        Serial.println(address);
        irsend.sendPanasonic(address, data);
      } else if (type == "jvc") {
        irsend.sendJVC(data, len, 0);
      } else if (type == "samsung") {
        irsend.sendSAMSUNG(data, len);
      } else if (type == "sharpraw") {
        irsend.sendSharpRaw(data, len);
      } else if (type == "dish") {
        irsend.sendDISH(data, len);
      } else if (type == "rc5") {
        irsend.sendRC5(data, len);
      } else if (type == "rc6") {
        irsend.sendRC6(data, len);
      } else if (type == "denon") {
        irsend.sendDenon(data, len);
      } else if (type == "lg") {
        irsend.sendLG(data, len);
      } else if (type == "sharp") {
        irsend.sendSharpRaw(data, len);
      } else if (type == "rcmm") {
        irsend.sendRCMM(data, len);
      } else if (type == "gree") {
        irsend.sendGree(data, len);
      } else if (type == "roomba") {
        roomba_send(atoi(dataStr.c_str()), pulse, pdelay, irsend);
      }
      if (p + 1 < pulse) delay(pdelay);
    }
    if (r + 1 < repeat) delay(rdelay);
  }

  Serial.println("Transmission complete");

  copyCode(last_send_4, last_send_5);
  copyCode(last_send_3, last_send_4);
  copyCode(last_send_2, last_send_3);
  copyCode(last_send, last_send_2);

  strncpy(last_send.data, dataStr.c_str(), 40);
  last_send.bits = len;
  strncpy(last_send.encoding, type.c_str(), 14);
  strncpy(last_send.address, ("0x" + String(address, HEX)).c_str(), 20);
  last_send.timestamp = 0; //timeClient.getUnixTime();
  last_send.valid = true;

  resetReceive();
}

void rawblast(JsonArray &raw, int khz, int rdelay, int pulse, int pdelay, int repeat, IRsend irsend,int duty) {
  Serial.println("Raw transmit");
  holdReceive = true;
  Serial.println("Blocking incoming IR signals");
  // Repeat Loop
  for (int r = 0; r < repeat; r++) {
    // Pulse Loop
    for (int p = 0; p < pulse; p++) {
      Serial.println("Sending code");
      irsend.enableIROut(khz,duty);
      for (unsigned int i = 0; i < raw.size(); i++) {
        int val = raw[i];
        if (i & 1) irsend.space(std::max(0, val));
        else       irsend.mark(val);
      }
      irsend.space(0);
      if (p + 1 < pulse) delay(pdelay);
    }
    if (r + 1 < repeat) delay(rdelay);
  }

  Serial.println("Transmission complete");

  copyCode(last_send_4, last_send_5);
  copyCode(last_send_3, last_send_4);
  copyCode(last_send_2, last_send_3);
  copyCode(last_send, last_send_2);

  strncpy(last_send.data, "", 40);
  last_send.bits = raw.size();
  strncpy(last_send.encoding, "RAW", 14);
  strncpy(last_send.address, "0x0", 20);
  last_send.timestamp = 0; //timeClient.getUnixTime();
  last_send.valid = true;

  resetReceive();
}

void roomba_send(int code, int pulse, int pdelay, IRsend irsend)
{
  Serial.print("Sending Roomba code ");
  Serial.println(code);
  holdReceive = true;
  Serial.println("Blocking incoming IR signals");

  int length = 8;
  uint16_t raw[length * 2];
  unsigned int one_pulse = 3000;
  unsigned int one_break = 1000;
  unsigned int zero_pulse = one_break;
  unsigned int zero_break = one_pulse;
  uint16_t len = 15;
  uint16_t hz = 38;

  int arrayposition = 0;
  for (int counter = length - 1; counter >= 0; --counter) {
    if (code & (1 << counter)) {
      raw[arrayposition] = one_pulse;
      raw[arrayposition + 1] = one_break;
    }
    else {
      raw[arrayposition] = zero_pulse;
      raw[arrayposition + 1] = zero_break;
    }
    arrayposition = arrayposition + 2;
  }
  for (int i = 0; i < pulse; i++) {
    irsend.sendRaw(raw, len, hz);
    delay(pdelay);
  }

  resetReceive();
}

int rokuCommand(String ip, String data) {
  String url = "http://" + ip + ":8060/" + data;
  HTTPClient http;
  http.begin(url);
  Serial.println(url);
  Serial.println("Sending roku command");

  copyCode(last_send_4, last_send_5);
  copyCode(last_send_3, last_send_4);
  copyCode(last_send_2, last_send_3);
  copyCode(last_send, last_send_2);

  strncpy(last_send.data, data.c_str(), 40);
  last_send.bits = 1;
  strncpy(last_send.encoding, "roku", 14);
  strncpy(last_send.address, ip.c_str(), 20);
  last_send.timestamp = 0; //timeClient.getUnixTime();
  last_send.valid = true;

  int output = http.POST("");
  http.end();
  return output;
}

// TO SEE RECEIVED AND SENT CODES IN SERIAL MONITOR UNCOMMENT THESE LINES AND THE FULLCODE AND DUMPCODE COMMANDS IN THE LOOP()
// THESE SECTIONS COMMENTED OUT TO SAVE SPACE ONLY

//+=============================================================================
// Code to string
//
void fullCode (decode_results *results) {
  Serial.print("One line: ");
  serialPrintUint64(results->value, 16);
  Serial.print(":");
  Serial.print(encoding(results));
  Serial.print(":");
  Serial.print(results->bits, DEC);
  if (results->repeat) Serial.print(" (Repeat)");
  Serial.println("");
  if (results->overflow)
    Serial.println("WARNING: IR code too long. "
                   "Edit IRController.ino and increase captureBufSize");
}


//+=============================================================================
// Dump out the decode_results structure.
//
void dumpInfo(decode_results *results) {
  if (results->overflow)
    Serial.println("WARNING: IR code too long. "
                   "Edit IRrecv.h and increase RAWBUF");

  // Show Encoding standard
  Serial.print("Encoding  : ");
  Serial.print(encoding(results));
  Serial.println("");

  // Show Code & length
  Serial.print("Code      : ");
  serialPrintUint64(results->value, 16);
  Serial.print(" (");
  Serial.print(results->bits, DEC);
  Serial.println(" bits)");
}


//+=============================================================================
// Dump out the decode_results structure.
//
void dumpRaw(decode_results *results) {
  // Print Raw data
  Serial.print("Timing[");
  Serial.print(results->rawlen - 1, DEC);
  Serial.println("]: ");

  for (uint16_t i = 1;  i < results->rawlen;  i++) {
    if (i % 100 == 0)
      yield();  // Preemptive yield every 100th entry to feed the WDT.
    uint32_t x = results->rawbuf[i] * RAWTICK;
    if (!(i & 1)) {  // even
      Serial.print("-");
      if (x < 1000) Serial.print(" ");
      if (x < 100) Serial.print(" ");
      Serial.print(x, DEC);
    } else {  // odd
      Serial.print("     ");
      Serial.print("+");
      if (x < 1000) Serial.print(" ");
      if (x < 100) Serial.print(" ");
      Serial.print(x, DEC);
      if (i < results->rawlen - 1)
        Serial.print(", ");  // ',' not needed for last one
    }
    if (!(i % 8)) Serial.println("");
  }
  Serial.println("");  // Newline
}


//+=============================================================================
// Dump out the decode_results structure.
//
void dumpCode(decode_results *results) {
  // Start declaration
  Serial.print("uint16_t  ");              // variable type
  Serial.print("rawData[");                // array name
  Serial.print(results->rawlen - 1, DEC);  // array size
  Serial.print("] = {");                   // Start declaration

  // Dump data
  for (uint16_t i = 1; i < results->rawlen; i++) {
    Serial.print(results->rawbuf[i] * RAWTICK, DEC);
    if (i < results->rawlen - 1)
      Serial.print(",");  // ',' not needed on last one
    if (!(i & 1)) Serial.print(" ");
  }

  // End declaration
  Serial.print("};");  //

  // Comment
  Serial.print("  // ");
  Serial.print(encoding(results));
  Serial.print(" ");
  serialPrintUint64(results->value, 16);

  // Newline
  Serial.println("");

  // Now dump "known" codes
  if (results->decode_type != UNKNOWN) {
    // Some protocols have an address &/or command.
    // NOTE: It will ignore the atypical case when a message has been decoded
    // but the address & the command are both 0.
    if (results->address > 0 || results->command > 0) {
      Serial.print("uint32_t  address = 0x");
      Serial.print(results->address, HEX);
      Serial.println(";");
      Serial.print("uint32_t  command = 0x");
      Serial.print(results->command, HEX);
      Serial.println(";");
    }

    // All protocols have data
    Serial.print("uint64_t  data = 0x");
    serialPrintUint64(results->value, 16);
    Serial.println(";");
  }
}


//+=============================================================================
// Binary value to hex
//
String bin2hex(const uint8_t* bin, const int length) {
  String hex = "";

  for (int i = 0; i < length; i++) {
    if (bin[i] < 16) {
      hex += "0";
    }
    hex += String(bin[i], HEX);
  }

  return hex;
}

void copyCode (Code& c1, Code& c2) {
  strncpy(c2.data, c1.data, 40);
  strncpy(c2.encoding, c1.encoding, 14);
  //strncpy(c2.timestamp, c1.timestamp, 40);
  strncpy(c2.address, c1.address, 20);
  strncpy(c2.command, c1.command, 40);
  c2.bits = c1.bits;
  c2.raw = c1.raw;
  // c2.timestamp = c1.timestamp;
  c2.valid = c1.valid;
}
  // Configure the server
void send_ir_btn(int remote_send) { // btn IR blaster routines

    String jsonError;
    DynamicJsonBuffer jsonStringBuffer;
    String btn;
    String jsonString;
   switch (remote_send){
    case 0:
    btn = "[supla1]";
    break;
    case 1:
    btn = "[supla2]";
    break;
    case 2:
    btn = "[supla3]";
    break;
    case 3:
    btn = "[supla4]";
    break;
    case 4:
    btn = "[supla5]";
    break;
    case 5:
    btn = "[supla6]";
    break;
    case 6:
    btn = "[supla7]";
    break;
    case 7:
    btn = "[supla8]";
    break; 
   }
            jsonString; 
            btn.toLowerCase(); 
            DynamicJsonBuffer jsonBtnBuffer;
            JsonArray& rootbtn = jsonBtnBuffer.parseArray(btn);
            rootbtn.prettyPrintTo(Serial);
            Serial.println(rootbtn.size());      
            for (int v = 0; v < rootbtn.size(); v++) {
              if (!rootbtn[v].is<JsonObject>()) {
                String vtxt; rootbtn[v].printTo(vtxt); vtxt = "/codes/" + vtxt + ".json";
                vtxt.toLowerCase();
                Serial.println("reading codes file:" + vtxt);
                if (SPIFFS.exists(vtxt)) {
                  File codeFileRead = SPIFFS.open(vtxt, "r"); //reades the json file
                  if (codeFileRead) {
                    DynamicJsonBuffer jsonReadBuffer;
                    JsonObject& filebtn = jsonReadBuffer.parseObject(codeFileRead);                
                    JsonObject& fingers =  rootbtn[v+1];
                    for (auto finger : fingers){
                      if (String(finger.key) == "state") {
                        filebtn["state"] = finger.value;
                        filebtn["device"] = filebtn["name"];
                      }
                    }
                    String jsonStringtmp;
                    filebtn.printTo(jsonStringtmp);  
                    jsonReadBuffer.clear();                  
                    if (jsonStringtmp != "") jsonString = jsonString + jsonStringtmp + ",";            
                  } else {
                    jsonError = jsonError + " " + vtxt;
                  }
                } else {
                  jsonError = jsonError + " " + vtxt;              
                }
              }
            }
            if (jsonString != "") {
              if (jsonString.endsWith(",")) jsonString = jsonString.substring(0, jsonString.length() -1);
              jsonString = "[" + jsonString + "]";
            }
            jsonBtnBuffer.clear();           
          
          JsonArray& root = jsonStringBuffer.parseArray(jsonString);
          if (root.success()) {
            //root.prettyPrintTo(Serial);
            Serial.println("");            
            Serial.println("Received code qty: " + root.size());
            if (root.size() > 0) {
              for (int x = 0; x < root.size(); x++) {
                // Handle device state limitations on a per JSON object basis
                String device = root[x]["device"];
                Serial.println("Code read " + device);                  
                int state = root[x]["state"];
                int storedState = (deviceState.containsKey(device)) ? deviceState[device] : 888888;
                Serial.println("device:" + device + " state:" + state + " storedstate:" + storedState);
                if ((device != "" && storedState != state) || device == "") {
                  if (device != "") deviceState[device] = state;  
                  String type = root[x]["type"];
                  String ip = root[x]["ip"];
                  int rdelay = (root[x]["rdelay"] > 0) ? root[x]["rdelay"] : 1000;
                  int pulse = (root[x]["pulse"] > 0) ? root[x]["pulse"] : 1;
                  int pdelay = (root[x]["pdelay"] > 0) ? root[x]["pdelay"] : 100;
                  int repeat = (root[x]["repeat"] > 0) ? root[x]["repeat"] : 1;
                  int xout = root[x]["out"];
                  if (xout == 0) {
                    xout = 1;
                  }
                  int duty = (root[x]["duty"] > 0) ? root[x]["duty"] : 50;                  
                  Serial.println("type:" + type + " repeat:" + repeat + " rdelay:" + rdelay + " pulse:" + pulse + " pdelay:" + pdelay + " duty:" + duty);
                  if (type == "delay") {
                    delay(rdelay);
                  } else if (type == "raw") {
                    JsonArray& raw = root[x]["data"]; // Array of unsigned int values for the raw signal
                    int khz = root[x]["khz"];
                    if (khz <= 0) khz = 38; // Default to 38khz if not set
                    rawblast(raw, khz, rdelay, pulse, pdelay, repeat, pickIRsend(xout),duty);
                    rtnMessage.message = "RAW code sent"; rtnMessage.title = "Sucess"; rtnMessage.type = 1, rtnMessage.httpcode = 200;
                  } else if (type == "roku") {
                    String data = root[x]["data"];
                    rokuCommand(ip, data);
                    rtnMessage.message = "ROKU code sent"; rtnMessage.title = "Sucess"; rtnMessage.type = 1, rtnMessage.httpcode = 200;
                  } else if (type != "") {
                    String data = root[x]["data"];
                    String addressString = root[x]["address"];
                    long address = strtoul(addressString.c_str(), 0, 0);
                    int len = root[x]["length"];

                    
                    Serial.println("Sending code: " + type);
                    irblast(type, data, len, rdelay, pulse, pdelay, repeat, address, pickIRsend(xout));
                    rtnMessage.message = type + " code sent"; rtnMessage.title = "Sucess"; rtnMessage.type = 1, rtnMessage.httpcode = 200;
                  } else {
                    rtnMessage.message = "The format of the recevied code is not recorgnised, no code transmitted"; rtnMessage.title = "Error"; rtnMessage.type = 2, rtnMessage.httpcode = 422;
                  }
                } else {
                  Serial.println("Not sending command to " + device + ", already in state " + state);
                  rtnMessage.message = "Some components of the code were held because device was already in the appropriate state"; rtnMessage.title = "Information"; rtnMessage.type = 1, rtnMessage.httpcode = 200; //message = "Code sent. Some components of the code were held because device was already in appropriate state";
                }
              }
            } else {
              rtnMessage.message = "Error with JSON codes passed"; rtnMessage.title = "Error"; rtnMessage.type = 3, rtnMessage.httpcode = 400;
            }
          } else {
          rtnMessage.message = "Error with JSON codes passed"; rtnMessage.title = "Error"; rtnMessage.type = 3, rtnMessage.httpcode = 400;
          }
}

void loop() {
  
  int C_W_read = digitalRead(configpin);  // ---------------------let's read the status of the Gpio to start wificonfig ---------------------
   if (C_W_read != last_C_W_state) {  time_last_C_W_change = millis();}    
    if ((millis() - time_last_C_W_change) > C_W_delay) {     
     if (C_W_read != C_W_state) {     
       C_W_state = C_W_read;       
        if (C_W_state == LOW) {
         Serial.println("WiFiConfig");
          setupWifi(true);} } }         
           last_C_W_state = C_W_read;            
  
  server->handleClient();
  
  CustomSupla.iterate();
    
  decode_results  results;                                        // Somewhere to store the results

  if (irrecv.decode(&results) && !holdReceive) {                  // Grab an IR code
    Serial.println("Signal received:");
    //fullCode(&results);                                         // Print the singleline value
    //dumpCode(&results);                                         // Output the results as source code
    copyCode(last_recv_4, last_recv_5);                           // Pass
    copyCode(last_recv_3, last_recv_4);                           // Pass
    copyCode(last_recv_2, last_recv_3);                           // Pass
    copyCode(last_recv, last_recv_2);                             // Pass
    cvrtCode(last_recv, &results);                                // Store the results
    last_recv.timestamp = 0; //timeClient.getUnixTime();               // Set the new update time
    last_recv.valid = true;
    Serial.println("");                                           // Blank line between entries
    irrecv.resume();                                              // Prepare for the next value
    digitalWrite(ledpin, LOW);                                    // Turn on the LED for 0.5 seconds
    ticker.attach(0.5, disableLed);
  }
  delay(50);
  
  MDNS.update();
  
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
