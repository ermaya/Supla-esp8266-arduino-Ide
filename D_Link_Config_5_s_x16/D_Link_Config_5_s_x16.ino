#include <FS.h>       // ---- esp board manager 2.4.2 --- iwip Variant V2 higher Bandwidth
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>     //--------- https://github.com/tzapu/WiFiManager/tree/0.14 -------------
#include <ArduinoJson.h>     //--------- https://github.com/bblanchon/ArduinoJson/tree/v5.13.2 ------
#include <Ticker.h>      //for LED status
extern "C"
{
#include "user_interface.h"
}
Ticker ticker;

const char* host = "org";
const int httpsPort = 443;
const char* fingerprint = "CF 05 98 89 CA FF 8E D8 5E 5C E0 C2 E4 F7 E6 C3 C7 50 DD 5C";
char Supla_server[20] = "svrX.supla.org";
char D_Link_1[40] = "/direct/XX/XXXXXXX/turn-on";
char D_Link_2[40];
char D_Link_3[40];
char D_Link_4[40];
char D_Link_5[40];
char D_Link_6[40];
char D_Link_7[40];
char D_Link_8[40];
char D_Link_9[40];
char D_Link_10[40];
char D_Link_11[40];
char D_Link_12[40];
char D_Link_13[40];
char D_Link_14[40];
char D_Link_15[40];
char D_Link_16[40];
byte mac[6];
String url = "/direct";
bool shouldSaveConfig = false;
bool shouldSendLink = false;
bool initialConfig = false;
#define status_led D8  
#define Link_1_PIN D0     // triger Link 1
#define Link_2_PIN D1     // triger Link 2
#define Link_3_PIN D2     // triger Link 3
#define Link_4_PIN D3     // triger Link 4  and config
#define Link_5_PIN D4     // triger Link 5
#define Link_6_PIN D5     // triger Link 6
#define Link_7_PIN D6     // triger Link 7
#define Link_8_PIN D7     // triger Link 8
int buttonValue; //Stores analog value when button is pressed
int C_W_state = HIGH;            
int last_C_W_state = HIGH;       
unsigned long time_last_C_W_change = 0;   
long C_W_delay = 5000;               // config delay 5 seconds  ------------ opóźnienie konfiguracji 5 sekund
int timeout           = 120; // seconds to run for

void tick(){
  //toggle state
  int state = digitalRead(status_led);  // get the current state of D4 pin
  digitalWrite(status_led, !state);     // set pin to the opposite state
}
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void ondemandwifiCallback () {
  ticker.attach(0.2, tick);
  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 20);
  WiFiManagerParameter custom_D_Link_1("Link 1", "DirectLink 1", D_Link_1, 40);
  WiFiManagerParameter custom_D_Link_2("Link 2", "DirectLink 2", D_Link_2, 40);
  WiFiManagerParameter custom_D_Link_3("Link 3", "DirectLink 3", D_Link_3, 40);
  WiFiManagerParameter custom_D_Link_4("Link 4", "DirectLink 4", D_Link_4, 40);
  WiFiManagerParameter custom_D_Link_5("Link 5", "DirectLink 5", D_Link_5, 40);
  WiFiManagerParameter custom_D_Link_6("Link 6", "DirectLink 6", D_Link_6, 40);
  WiFiManagerParameter custom_D_Link_7("Link 7", "DirectLink 7", D_Link_8, 40);
  WiFiManagerParameter custom_D_Link_8("Link 8", "DirectLink 8", D_Link_7, 40);
  WiFiManagerParameter custom_D_Link_9("Link 9", "DirectLink 9", D_Link_9, 40);
  WiFiManagerParameter custom_D_Link_10("Link 10", "DirectLink 10", D_Link_10, 40);
  WiFiManagerParameter custom_D_Link_11("Link 11", "DirectLink 11", D_Link_11, 40);
  WiFiManagerParameter custom_D_Link_12("Link 12", "DirectLink 12", D_Link_12, 40);
  WiFiManagerParameter custom_D_Link_13("Link 13", "DirectLink 13", D_Link_13, 40);
  WiFiManagerParameter custom_D_Link_14("Link 14", "DirectLink 14", D_Link_14, 40);
  WiFiManagerParameter custom_D_Link_15("Link 15", "DirectLink 15", D_Link_15, 40);
  WiFiManagerParameter custom_D_Link_16("Link 16", "DirectLink 16", D_Link_16, 40);

  WiFiManager wifiManager;

  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_D_Link_1);
  wifiManager.addParameter(&custom_D_Link_2);
  wifiManager.addParameter(&custom_D_Link_3);
  wifiManager.addParameter(&custom_D_Link_4);
  wifiManager.addParameter(&custom_D_Link_5);
  wifiManager.addParameter(&custom_D_Link_6);
  wifiManager.addParameter(&custom_D_Link_7);
  wifiManager.addParameter(&custom_D_Link_8);
  wifiManager.addParameter(&custom_D_Link_9);
  wifiManager.addParameter(&custom_D_Link_10);
  wifiManager.addParameter(&custom_D_Link_11);
  wifiManager.addParameter(&custom_D_Link_12);
  wifiManager.addParameter(&custom_D_Link_13);
  wifiManager.addParameter(&custom_D_Link_14);
  wifiManager.addParameter(&custom_D_Link_15);
  wifiManager.addParameter(&custom_D_Link_16);

  wifiManager.setCustomHeadElement("<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>");

  wifiManager.setMinimumSignalQuality(10);

    wifiManager.setConfigPortalTimeout(timeout);

    if (!wifiManager.startConfigPortal("DirectLink16")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      ESP.restart();
      delay(5000);
    }
    Serial.println("connected...yeey :)");
    
    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(D_Link_1, custom_D_Link_1.getValue());
    strcpy(D_Link_2, custom_D_Link_2.getValue());
    strcpy(D_Link_3, custom_D_Link_3.getValue());
    strcpy(D_Link_4, custom_D_Link_4.getValue());
    strcpy(D_Link_5, custom_D_Link_5.getValue());
    strcpy(D_Link_6, custom_D_Link_6.getValue());
    strcpy(D_Link_7, custom_D_Link_7.getValue());
    strcpy(D_Link_8, custom_D_Link_8.getValue());
    strcpy(D_Link_9, custom_D_Link_9.getValue());
    strcpy(D_Link_10, custom_D_Link_10.getValue());
    strcpy(D_Link_11, custom_D_Link_11.getValue());
    strcpy(D_Link_12, custom_D_Link_12.getValue());
    strcpy(D_Link_13, custom_D_Link_13.getValue());
    strcpy(D_Link_14, custom_D_Link_14.getValue());
    strcpy(D_Link_15, custom_D_Link_15.getValue());
    strcpy(D_Link_16, custom_D_Link_16.getValue()); 
}

void setup() {  //------------------------------------------------ Setup ----------------------------------------------
 
  wifi_set_sleep_type(NONE_SLEEP_T);

  Serial.begin(115200);

  pinMode(Link_1_PIN, INPUT);
  pinMode(Link_2_PIN, INPUT_PULLUP);
  pinMode(Link_3_PIN, INPUT_PULLUP);
  pinMode(Link_4_PIN, INPUT_PULLUP);
  pinMode(Link_5_PIN, INPUT_PULLUP);
  pinMode(Link_6_PIN, INPUT_PULLUP);
  pinMode(Link_7_PIN, INPUT_PULLUP);
  pinMode(Link_8_PIN, INPUT_PULLUP);
  pinMode(status_led, OUTPUT);
 
  ticker.attach(0.8, tick);
  
  if (WiFi.SSID()==""){   
    initialConfig = true;
  }
  //read configuration from FS json
  Serial.println("mounting FS...");
  
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;         
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        Serial.println(jsonBuffer.size());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(Supla_server, json["Supla_server"]);
          strcpy(D_Link_1, json["D_Link_1"]);
          strcpy(D_Link_2, json["D_Link_2"]);
          strcpy(D_Link_3, json["D_Link_3"]);
          strcpy(D_Link_4, json["D_Link_4"]);
          strcpy(D_Link_5, json["D_Link_5"]);
          strcpy(D_Link_6, json["D_Link_6"]);
          strcpy(D_Link_7, json["D_Link_7"]);
          strcpy(D_Link_8, json["D_Link_8"]);
          strcpy(D_Link_9, json["D_Link_9"]);
          strcpy(D_Link_10, json["D_Link_10"]);
          strcpy(D_Link_11, json["D_Link_11"]);
          strcpy(D_Link_12, json["D_Link_12"]);
          strcpy(D_Link_13, json["D_Link_13"]);
          strcpy(D_Link_14, json["D_Link_14"]);
          strcpy(D_Link_15, json["D_Link_15"]);
          strcpy(D_Link_16, json["D_Link_16"]);

        } else {
          Serial.println("failed to load json config");          
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  } 
  host = Supla_server;
  WiFi.mode(WIFI_STA); 
}

void loop() { 

    analog_switch();   // ------------------------ 

  if ( digitalRead(Link_1_PIN) == LOW )  {
    url = D_Link_1;
    direct_Link() ;    
  }
   if ( digitalRead(Link_2_PIN) == LOW)  {
    url = D_Link_2;
    direct_Link() ;    
  }
  if ( digitalRead(Link_3_PIN) == LOW )  {
    url = D_Link_3;
    direct_Link() ;   
  }
   if ( digitalRead(Link_4_PIN) == LOW)  {
    url = D_Link_4;
    direct_Link() ;    
  }
  if ( digitalRead(Link_5_PIN) == LOW )  {
    url = D_Link_5;
    direct_Link() ;    
  }
   if ( digitalRead(Link_6_PIN) == LOW)  {
    url = D_Link_6;
    direct_Link() ;    
  }
  if ( digitalRead(Link_7_PIN) == LOW )  {
    url = D_Link_7;
    direct_Link() ;    
  }
   if ( digitalRead(Link_8_PIN) == LOW)  {
    url = D_Link_8;
    direct_Link() ;    
  }
  
  if  (initialConfig)  {
    ondemandwifiCallback () ;
    initialConfig = false; 
  }
   int C_W_read = digitalRead(Link_4_PIN );{    
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
   
  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Supla_server"] = Supla_server;
    json["D_Link_1"] = D_Link_1;
    json["D_Link_2"] = D_Link_2;
    json["D_Link_3"] = D_Link_3;
    json["D_Link_4"] = D_Link_4;
    json["D_Link_5"] = D_Link_5;
    json["D_Link_6"] = D_Link_6;
    json["D_Link_7"] = D_Link_7;
    json["D_Link_8"] = D_Link_8;
    json["D_Link_9"] = D_Link_9;
    json["D_Link_10"] = D_Link_10;
    json["D_Link_11"] = D_Link_11;
    json["D_Link_12"] = D_Link_12;
    json["D_Link_13"] = D_Link_13;
    json["D_Link_14"] = D_Link_14;
    json["D_Link_15"] = D_Link_15;
    json["D_Link_16"] = D_Link_16;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    Serial.println("config saved");
    shouldSaveConfig = false;
    WiFi.mode(WIFI_STA);
    ticker.detach();
    digitalWrite(status_led, LOW);
    ESP.restart();
      delay(5000); 
    }
  
  if (WiFi.status() != WL_CONNECTED) 
   {
    ticker.attach(0.8, tick);
    WiFi_up();
   }
}

void WiFi_up() 
{
  WiFi.begin(); 
  for (int x = 60; x > 0; x--) 
  {
    if (WiFi.status() == WL_CONNECTED) 
    {
      break;                           
    }
    else                                 
    {
      Serial.print(".");                
      delay(500);                      
    }
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    ticker.detach();
    digitalWrite(status_led, LOW);
    Serial.println("");
    Serial.println("Connected");
    Serial.println("Adres IP: ");
    Serial.print(WiFi.localIP());
    Serial.print(" / ");
    Serial.print(WiFi.subnetMask());
    Serial.print(" / ");
    Serial.println(WiFi.gatewayIP());
    long rssi = WiFi.RSSI();
    Serial.print("(RSSI): ");
    Serial.print(rssi);
    Serial.println(" dBm");
  }
  else    
  {
    Serial.println("");
    Serial.println("connection failed");
  }
}
void direct_Link() {
  
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  Serial.print("requesting URL: ");
  Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: ESP8266\r\n" +
               "Connection: close\r\n\r\n");
  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('}');
  line = line + "}";
  if (line.indexOf("true") >0) {
    digitalWrite(status_led, HIGH);  
    delay(200);
    digitalWrite(status_led, LOW);
    delay(200);
    digitalWrite(status_led, HIGH);  
    delay(200);
    digitalWrite(status_led, LOW); 
    Serial.println("successfull!");
  } else {
    Serial.println("failed");
  }
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
 }
 void analog_switch(){
  
  buttonValue = analogRead(A0); //Read analog value from A0 pin
    
  //For no button:
  if (buttonValue<=50){
  delay (100);
  return;
  }
  //For 1st button:
  else if (buttonValue>=113 && buttonValue<=153){
    Serial.println("read: 9 ");
    url = D_Link_9;
    direct_Link() ;
    return;
  }
  //For 2nd button:
  else if (buttonValue>=256 && buttonValue<=296){
    Serial.println("read: 10 ");
    url = D_Link_10;
    direct_Link() ;
    return;
  }
  //For 3rd button:
  else if (buttonValue>=389  && buttonValue<=429){
    Serial.println("read: 11 ");
    url = D_Link_11;
    direct_Link() ;
    return;
  }
  //For 4th button:
  else if (buttonValue>=525  && buttonValue<=565){
    Serial.println("read: 12 ");
    url = D_Link_12;
    direct_Link() ;
    return;
  }  
  //For 5rd button:
  else if (buttonValue>=654  && buttonValue<=694){
    Serial.println("read: 13 ");
    url = D_Link_13;
    direct_Link() ;
    return;
  }
  //For 6th button:
  else if (buttonValue>=796  && buttonValue<=836){
    Serial.println("read: 14 ");
    url = D_Link_14;
    direct_Link() ;
    return;
  }
  //For 7rd button:
  else if (buttonValue>=930  && buttonValue<=970){
    Serial.println("read: 15 ");
    url = D_Link_15;
    direct_Link() ;
    return;
  }
  //For 8th button:
  else if (buttonValue>=1000 ){
    Serial.println("read: 16 ");
    url = D_Link_16;
    direct_Link() ;
    return;
  } 
}
