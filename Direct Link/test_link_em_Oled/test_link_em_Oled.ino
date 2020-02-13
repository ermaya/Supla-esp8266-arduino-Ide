//#define D0 16  //no internal pullup resistor
//#define D1  5
//#define D2  4
//#define D3  0  //must not be pulled low during power on/reset, toggles value during boot
//#define D4  2  //must not be pulled low during power on/reset, toggles value during boot
//#define D5 14
//#define D6 12
//#define D7 13
//#define D8 15  //must not be pulled high during power on/reset

#include <FS.h>       // ---- esp board manager 2.5.2 
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>     //--------- https://github.com/tzapu/WiFiManager/tree/0.14 -------------
#include <ArduinoJson.h>     //--------- https://github.com/bblanchon/ArduinoJson/tree/v5.13.2 ------
#include <Ticker.h>      //for LED status
#include "SSD1306Wire.h" //----- 0.96 Oled --- https://github.com/ThingPulse/esp8266-oled-ssd1306
SSD1306Wire  display(0x3c, 4, 5);  // D3-SDA  D4-SCL ----- 0.96 Oled ---
//#include "SH1106Wire.h" //----- 1.3 Oled ---
//SH1106Wire display(0x3c, 4, 5);  // D3-SDA  D4-SCL ----- 1.3 Oled ---
Ticker ticker;

const char* host = "svr.supla.org";
const int httpsPort = 443;

char Supla_server[41];
char D_Link_1[61];
byte mac[6];
String url = "/direct/xx/xxxxxxxxx/xxx";
bool dimm = false;         
unsigned long dimm_milis ;
int C_W_state = HIGH;            
int last_C_W_state = HIGH;       
unsigned long time_last_C_W_change = 0;   
long C_W_delay = 10000;               // config delay 10 seconds  ------------ opóźnienie konfiguracji 10 sekund

bool shouldSaveConfig = false;
bool initialConfig = false;
unsigned long link_delay = 5000; 
#define onboard_led 2    //D4  
#define Config_PIN 0     //D3    // wifi config

int timeout           = 120; // seconds to run for wifi Manager

const uint8_t logo32_glcd_bmp[] PROGMEM =  //logo supla 32
{
  0x00,0x00,0x00,0x00,0xC0,0x1F,0x00,0x00,0xE0,0x7F,0x00,0x00,
  0xF0,0x70,0x00,0x00,0x30,0xE0,0x00,0x00,0x38,0xC0,0x00,0x00,
  0x18,0xC0,0x01,0x00,0x18,0xC0,0x01,0x00,0x38,0xC0,0x00,0x00,
  0x38,0xE0,0x01,0x00,0x70,0xF0,0x07,0x00,0xE0,0x7F,0x0E,0x00,
  0xC0,0x3F,0x38,0x00,0x00,0x1F,0xE0,0x0F,0x00,0x18,0xC0,0x1F,
  0x00,0x18,0xC0,0x30,0x00,0x18,0xC0,0x30,0x00,0x30,0xC0,0x30,
  0x00,0x30,0x80,0x1F,0x00,0x30,0xC0,0x0F,0x00,0x20,0x60,0x00,
  0x00,0x60,0x20,0x00,0x00,0x60,0x30,0x00,0x00,0x40,0x18,0x00,
  0x00,0xC0,0x0D,0x00,0x00,0xC0,0x07,0x00,0x00,0x60,0x04,0x00,
  0x00,0x20,0x0C,0x00,0x00,0x20,0x0C,0x00,0x00,0x60,0x06,0x00,
  0x00,0xC0,0x03,0x00,0x00,0x00,0x00,0x00
};

void tick() {
  int state = digitalRead(onboard_led);  
  digitalWrite(onboard_led, !state);     
}
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void ondemandwifiCallback () {
          display.clear();
          display.setContrast(100, 241, 64);
          display.setTextAlignment(TEXT_ALIGN_CENTER);
          display.setFont(ArialMT_Plain_10);
          display.drawString(64, 16, "wifi config connect");
          display.drawString(64, 28, "to wifi hotspot");
          display.setFont(ArialMT_Plain_16);
          display.drawString(64, 44, "DirectEM"); 
          display.display(); 

  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 41);
  WiFiManagerParameter custom_D_Link_1("Linka", "D_Link On", D_Link_1, 61);

  WiFiManager wifiManager;
  wifiManager.setBreakAfterConfig(true);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_D_Link_1);


  wifiManager.setCustomHeadElement("<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>");

  wifiManager.setMinimumSignalQuality();
    wifiManager.setConfigPortalTimeout(timeout);

    if (!wifiManager.startConfigPortal("DirectEM")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      ESP.restart();
      delay(5000);
    }
    Serial.println("connected...yeey :)");

    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(D_Link_1, custom_D_Link_1.getValue());  
}
ICACHE_RAM_ATTR void no_dimm(){
            dimm_milis = millis() + 30000 ;
            display.setContrast(200, 241, 60);
            dimm = false; 
}
void setup() {  //------------------------------------------------ Setup ----------------------------------------------

  Serial.begin(115200);

  pinMode(Config_PIN, INPUT_PULLUP);
  pinMode(onboard_led, OUTPUT);

 display.init();
  display.flipScreenVertically();
   display.clear();
     display.setTextAlignment(TEXT_ALIGN_CENTER);
      display.setFont(ArialMT_Plain_24);
       display.drawString(64, 40, "SUPLA");
         display.drawXbm(0, 16, 32, 32,logo32_glcd_bmp );
          display.display();
           delay(1);
  
  if (WiFi.SSID()==""){   
    initialConfig = true;
 }

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
            host = Supla_server;
              url = D_Link_1;
        } else {
          Serial.println("failed to load json config");          
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
 } 

  WiFi.mode(WIFI_STA);
  dimm_milis = millis() + 30000 ;
  attachInterrupt(Config_PIN, no_dimm, FALLING); 
}

void loop() { 

   if (initialConfig){ondemandwifiCallback();initialConfig = false;}
    
    


   int C_W_read = digitalRead(Config_PIN);{    
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

  if (millis() > dimm_milis){ 
    if (dimm == false){
    display.setContrast(50, 40, 30);
    display.display();
    dimm = true ;     
    }    
    dimm_milis = millis() + 30000 ;
     } 
       
    if ((WiFi.status() == WL_CONNECTED) && (millis() > link_delay)){
      direct_Link();
      link_delay = link_delay + 10000;
    }
  
  if (shouldSaveConfig) {
    Serial.println("saving config");
          display.clear();
          display.setTextAlignment(TEXT_ALIGN_CENTER);
          display.setFont(ArialMT_Plain_10);
          display.drawString(64, 0, "saving config");
          display.display(); 
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Supla_server"] = Supla_server;
    json["D_Link_1"] = D_Link_1;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    Serial.println("config saved");
          display.clear();
          display.setTextAlignment(TEXT_ALIGN_CENTER);
          display.setFont(ArialMT_Plain_10);
          display.drawString(64, 0, "config saved");
          display.display();   
    shouldSaveConfig = false;
    WiFi.mode(WIFI_STA);
    ticker.detach();
    digitalWrite(onboard_led, HIGH);
    ESP.restart();
      delay(5000); 
 }
  
  if (WiFi.status() != WL_CONNECTED) {
               display.clear();
               display.setFont(ArialMT_Plain_24);
               display.drawString(64, 40, "SUPLA");
               display.drawXbm(0, 16, 32, 32,logo32_glcd_bmp );
               display.setTextAlignment(TEXT_ALIGN_CENTER);
               display.setFont(ArialMT_Plain_10);
               display.drawString(64, 0, "  conecting wifi  ");               
               display.display(); 
    WiFi_up();
 }
 
}

void WiFi_up() {
  WiFi.begin(); 
  
  for (int x = 20; x > 0; x--) 
  {
    if (WiFi.status() == WL_CONNECTED) 
    {
          display.clear();
          display.setTextAlignment(TEXT_ALIGN_CENTER);
          display.setFont(ArialMT_Plain_24);
          display.drawString(64, 40, "SUPLA");
          display.drawXbm(0, 16, 32, 32,logo32_glcd_bmp );
          display.setFont(ArialMT_Plain_10);
          display.drawString(64, 0, "  WIFI Connected  ");
          display.display();        
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
          display.clear();
          display.setTextAlignment(TEXT_ALIGN_CENTER);
          display.setFont(ArialMT_Plain_24);
          display.drawString(64, 40, "SUPLA");
          display.drawXbm(0, 16, 32, 32,logo32_glcd_bmp );
          display.setFont(ArialMT_Plain_10);
          display.drawString(64, 0, "  WIFI Connected  ");
          display.display();  
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
          display.clear();
          display.setTextAlignment(TEXT_ALIGN_CENTER);
          display.setFont(ArialMT_Plain_24);
          display.drawString(64, 40, "SUPLA");
          display.drawXbm(0, 16, 32, 32,logo32_glcd_bmp );
          display.setFont(ArialMT_Plain_10);
          display.drawString(64, 0, "connection failed");
          display.display();     
  }
}
void direct_Link() {
  WiFiClientSecure client;
  client.setInsecure();
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed -------------------------------------------");
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
   String line = client.readStringUntil('\n');
  if (line.indexOf("true") >0) {
    yield();
  Serial.println("reply was:");Serial.println("==========");  
  Serial.println(line);
  Serial.println("==========");Serial.println("closing connection");
   
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(line);

 // bool connected = root["connected"]; // true
 // int support = root["support"]; // 382
  const char* currency = root["currency"]; // "EUR"
 // float pricePerUnit = root["pricePerUnit"]; // 0.12
  float totalCost = root["totalCost"]; // 380.25

  JsonArray& phases = root["phases"];

  JsonObject& phases_0 = phases[0];
  //int phases_0_number = phases_0["number"]; // 1
  float phases_0_voltage = phases_0["voltage"]; // 234.6
  float phases_0_current = phases_0["current"]; // 7.65
  float phases_0_powerActive = phases_0["powerActive"]; // 1786
 // float phases_0_powerReactive = phases_0["powerReactive"]; // 17.63985
 // float phases_0_powerApparent = phases_0["powerApparent"]; // 1794.69006
 // float phases_0_powerFactor = phases_0["powerFactor"]; // 0.995
  float phases_0_totalForwardActiveEnergy = phases_0["totalForwardActiveEnergy"]; // 3168.79008 

         display.clear(); 
         display.setFont(ArialMT_Plain_16);
         display.setTextAlignment(TEXT_ALIGN_CENTER);
         display.drawString(64, 0, "V " + String(phases_0_voltage, 1) + "  " + "A " + String(phases_0_current, 2));
         display.drawString(64, 16, "W " + String(phases_0_powerActive, 0));
         display.drawString(64, 32, "Kwh " + String(phases_0_totalForwardActiveEnergy, 3));
         display.drawString(64, 48,  String(totalCost, 2) + " " + String(currency));         
         display.display();    

 /*
  JsonObject& phases_1 = phases[1];
  int phases_1_number = phases_1["number"]; // 2
  int phases_1_voltage = phases_1["voltage"]; // 0
  int phases_1_current = phases_1["current"]; // 0
  int phases_1_powerActive = phases_1["powerActive"]; // 0
  int phases_1_powerReactive = phases_1["powerReactive"]; // 0
  int phases_1_powerApparent = phases_1["powerApparent"]; // 0
  int phases_1_powerFactor = phases_1["powerFactor"]; // 0
  int phases_1_totalForwardActiveEnergy = phases_1["totalForwardActiveEnergy"]; // 0

  JsonObject& phases_2 = phases[2];
  int phases_2_number = phases_2["number"]; // 3
  int phases_2_voltage = phases_2["voltage"]; // 0
  int phases_2_current = phases_2["current"]; // 0
  int phases_2_powerActive = phases_2["powerActive"]; // 0
  int phases_2_powerReactive = phases_2["powerReactive"]; // 0
  int phases_2_powerApparent = phases_2["powerApparent"]; // 0
  int phases_2_powerFactor = phases_2["powerFactor"]; // 0
  int phases_2_totalForwardActiveEnergy = phases_2["totalForwardActiveEnergy"]; // 0
 */ 
  } else {Serial.println("failed ------------------------------------------------------");
       display.clear();
        display.setFont(ArialMT_Plain_24);
         display.setTextAlignment(TEXT_ALIGN_CENTER);
          display.drawString(64, 32, "OFFLINE");
            display.display();
  } 
}
