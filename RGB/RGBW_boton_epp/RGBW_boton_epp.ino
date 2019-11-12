#include <FS.h>       // ---- esp board manager 2.4.2 --- iwip Variant V2 higher Bandwidth
#include <ESP8266WiFi.h>
#define SUPLADEVICE_CPP
#include <SuplaDevice.h>
#include <DNSServer.h>
#include <WiFiManager.h> 
#include <ArduinoJson.h> //--------- https://github.com/bblanchon/ArduinoJson/tree/v5.13.2 ------
#include <EEPROM.h>
#include <ESP8266WebServer.h>
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

#define CONFIG_PIN 0  // WiFiconfig 
#define RED_PIN 14
#define GREEN_PIN 12
#define BLUE_PIN 13
#define BRIGHTNESS_PIN  5
//#define COLOR_BRIGHTNESS_PIN 101  //no Gpio

char Supla_server[80];
char Location_id[15];
char Location_Pass[34];
char Supla_name[51];
byte mac[6];
int C_W_state = HIGH; 
int last_C_W_state = HIGH;
unsigned long time_last_C_W_change = 0; 
long C_W_delay = 10000;                      // config delay 10 seconds           ----------        opóźnienie konfiguracji 10 sekund
int C_W_state2 = HIGH; 
long C_W_delay2 = 100;                      // button debounce
bool shouldSaveConfig = false;
bool initialConfig = false;
int timeout = 180;  
bool pr_wifi = true;
bool wificonfig = false;
unsigned long wifi_checkDelay = 20000;
unsigned long wifimilis;
unsigned long mem_milis;
unsigned char _red = 0;
unsigned char _green = 255;
unsigned char _blue = 0;
unsigned char _color_brightness = 0;
unsigned char _brightness = 0;
int memBr;
int memCBr;
int memRed;
int memGreen;
int memBlue;
int Power_Off;

WiFiClient client;
WiFiManager wifiManager;

void saveConfigCallback () {         
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void ondemandwifiCallback () {
   wificonfig = true;
  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 80);
  WiFiManagerParameter custom_Location_id("ID", "Location id", Location_id, 15);
  WiFiManagerParameter custom_Location_Pass("Password", "Location Pass", Location_Pass, 34);
  WiFiManagerParameter custom_Supla_name("name", "Supla Device Name", Supla_name, 51,"required");

  wifiManager.setBreakAfterConfig(true);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_Location_id);
  wifiManager.addParameter(&custom_Location_Pass);
  wifiManager.addParameter(&custom_Supla_name);

  wifiManager.setCustomHeadElement("<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>");
  wifiManager.setMinimumSignalQuality(8);
  wifiManager.setConfigPortalTimeout(timeout);
   //wifiManager.setShowStaticFields(true); // force show static ip fields
   //wifiManager.setShowDnsFields(true);    // force show dns field always
    if (!wifiManager.startConfigPortal("Supla_RGB")) {
      Serial.println("Not connected to WiFi but continuing anyway.");
    } else {      
      Serial.println("connected...yeey :)");    //if you get here you have connected to the WiFi
    }
    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(Location_id, custom_Location_id.getValue());
    strcpy(Location_Pass, custom_Location_Pass.getValue());
    strcpy(Supla_name, custom_Supla_name.getValue());
   
  WiFi.softAPdisconnect(true);   //  close AP
}
void get_rgbw_value(int channelNumber, unsigned char *red, unsigned char *green, unsigned char *blue, unsigned char *color_brightness, unsigned char *brightness) {
   *brightness = _brightness;
   *color_brightness= _color_brightness;
   *red = _red;
   *green = _green;
   *blue = _blue;
 
}
void set_rgbw() {

  if (Power_Off == 0){    
    analogWrite(BRIGHTNESS_PIN, (_brightness * 1023) / 100);
    int out_br =((_color_brightness * 1023) / 100);
    int out_red = map(_red, 0, 255, 0,out_br);
    analogWrite(RED_PIN, (out_red));
    int out_green = map(_green, 0, 255, 0,out_br);
    analogWrite(GREEN_PIN, (out_green));
    int out_blue = map(_blue, 0, 255, 0,out_br);
    analogWrite(BLUE_PIN, (out_blue));
  }else{
    analogWrite(BRIGHTNESS_PIN, 0);
    analogWrite(RED_PIN, 0);
    analogWrite(GREEN_PIN, 0);
    analogWrite(BLUE_PIN, 0); 
  }
  mem_milis = (millis()+3000);      
}
void set_rgbw_value(int channelNumber, unsigned char red, unsigned char green, unsigned char blue, unsigned char color_brightness, unsigned char brightness) {

  if(((String(brightness).toInt()) == 0) && ((String(color_brightness).toInt()) == 0)){Power_Off = 1;}else{Power_Off = 0;}
    _brightness = brightness;
    _color_brightness= color_brightness;  
    _red = red;
    _green = green;
    _blue = blue;  
    
    set_rgbw(); 
}
void save_epp(){
  EEPROM.write(0,memRed);EEPROM.write(1,memGreen);EEPROM.write(2,memBlue);EEPROM.write(3,memBr);EEPROM.write(4,memCBr);EEPROM.write(5,Power_Off);
  EEPROM.commit();
  //Serial.print("epp save ");
}
void read_epp(){
  memRed = EEPROM.read(0);memGreen = EEPROM.read(1);memBlue = EEPROM.read(2);memBr = EEPROM.read(3);memCBr = EEPROM.read(4);Power_Off = EEPROM.read(5);
  //Serial.print("epp read ");
}
// ------------ SETUP -----------------
void setup () {

    wifi_set_sleep_type(NONE_SLEEP_T);  
    
  Serial.begin (115200);
  delay (10);
  EEPROM.begin(64);
  pinMode(CONFIG_PIN, INPUT_PULLUP);  

  if (WiFi.SSID()==""){   
    initialConfig = true;
  } 
   
  WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.

  Serial.println("mounting FS...");//read configuration from FS json
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;         
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);   //print config data to serial on startup
        if (json.success()) {
          Serial.println("\nparsed json");
          strcpy(Supla_server, json["Supla_server"]);
          strcpy(Location_id, json["Location_id"]);
          strcpy(Location_Pass, json["Location_Pass"]);
          strcpy(Supla_name, json["Supla_name"]);     
        } else {
          Serial.println("failed to load json config");
          initialConfig = true;
     }  } }    
  } else {
    Serial.println("failed to mount FS");
  }
  read_epp();
  if (Power_Off == 0){_brightness = memBr;_color_brightness= memCBr; _red = memRed;_green = memGreen;_blue = memBlue;}  
  set_rgbw();  
  SuplaDevice.setRGBWCallbacks(&get_rgbw_value, &set_rgbw_value);
  
   uint8_t mac[WL_MAC_ADDR_LENGTH];
   WiFi.macAddress(mac);
   char GUID[SUPLA_GUID_SIZE] = {mac[WL_MAC_ADDR_LENGTH - 6], mac[WL_MAC_ADDR_LENGTH - 5], mac[WL_MAC_ADDR_LENGTH - 4], mac[WL_MAC_ADDR_LENGTH - 3],                                
                                 mac[WL_MAC_ADDR_LENGTH - 2], mac[WL_MAC_ADDR_LENGTH - 1], mac[WL_MAC_ADDR_LENGTH - 1], mac[WL_MAC_ADDR_LENGTH - 2], 
                                 mac[WL_MAC_ADDR_LENGTH - 3], mac[WL_MAC_ADDR_LENGTH - 4], mac[WL_MAC_ADDR_LENGTH - 5], mac[WL_MAC_ADDR_LENGTH - 6]};

     SuplaDevice.addRgbControllerAndDimmer ();
     SuplaDevice.setName(Supla_name);

  int LocationID = atoi(Location_id);
  SuplaDevice.begin(GUID,              
                    mac,               
                    Supla_server,      
                    LocationID,        
                    Location_Pass);    

}

// ---------------- LOOP ------------------------------ -----------------------------
void loop () {
  
  if (initialConfig == true){
      Serial.println("initial config triger");
      ondemandwifiCallback () ;
  }
  if (shouldSaveConfig == true) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Supla_server"] = Supla_server;
    json["Location_id"] = Location_id;
    json["Location_Pass"] = Location_Pass;
    json["Supla_name"] = Supla_name;    
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
    Serial.println("failed to open config file for writing");
    }
    json.prettyPrintTo(Serial); //print config data to serial
    json.printTo(configFile);
    configFile.close();
    Serial.println("config saved");    
    shouldSaveConfig = false;
    initialConfig = false;
    if (wificonfig == true){ 
    WiFi.mode(WIFI_STA);   
    delay(5000);
    ESP.restart(); 
  }}
  
  if (millis() > mem_milis){
    if (EEPROM.read(5) != Power_Off){
      save_epp();
    }
    if (Power_Off == 0){
     if (((String(_brightness).toInt()) != EEPROM.read(3)) || ((String(_color_brightness).toInt()) != EEPROM.read(4)) || ((String(_red).toInt()) !=EEPROM.read(0)) || ((String(_green).toInt()) != EEPROM.read(1)) || ((String(_blue).toInt()) != EEPROM.read(2))){      
        memBr = (String(_brightness).toInt());memCBr =(String(_color_brightness).toInt());memRed =(String(_red).toInt());memGreen = (String(_green).toInt());memBlue = (String(_blue).toInt()); 
        save_epp();
     }}
    mem_milis = (millis()+3000); 
  }
  
   if (WiFi.status() != WL_CONNECTED) { 
    WiFi_up();
  }

 int C_W_read = digitalRead(CONFIG_PIN);{  
   if (C_W_read != last_C_W_state) {            
     time_last_C_W_change = millis();
   }
   if ((millis() - time_last_C_W_change) > C_W_delay2) {     
     if (C_W_read != C_W_state2) {     
       Serial.println("short press");
       C_W_state2 = C_W_read;       
       if (C_W_state2 == LOW) {
        if (Power_Off == 0){
         Power_Off = 1;
         _brightness = 0;
         _color_brightness = 0;
         SuplaDevice.channelValueChanged(0, 0); // off _brightness
     }else{
         Power_Off = 0; 
        _brightness = memBr;
        _color_brightness = memCBr;
        SuplaDevice.channelSetRGBWvalues(0,memRed,memGreen,memBlue,memCBr,memBr); // restore _color_brightness & _brightness
     }
     if (Power_Off){
      Serial.println("Power sate Off");
     }else{
      Serial.println("Power sate On");
     }
    set_rgbw();
       }
     }
    }
   if ((millis() - time_last_C_W_change) > C_W_delay) {     
     if (C_W_read != C_W_state) {     
       Serial.println("long press");
       C_W_state = C_W_read;       
       if (C_W_state == LOW) {
        ondemandwifiCallback () ;
       }
     }
    }
   last_C_W_state = C_W_read;            
   }

  if (WiFi.status() == WL_CONNECTED){ 
      SuplaDevice.iterate();
      delay(100);      
    if (pr_wifi == true){
     Serial.println("");
     Serial.println("CONNECTED");
     Serial.print("local IP: ");
     Serial.println(WiFi.localIP());
     Serial.print("subnetMask: ");
     Serial.println(WiFi.subnetMask());
     Serial.print("gatewayIP: ");
     Serial.println(WiFi.gatewayIP());
     long rssi = WiFi.RSSI();
     Serial.print("Signal Strength (RSSI): ");
     Serial.print(rssi);
     Serial.println(" dBm");
     pr_wifi = false;               
  }}
}
void WiFi_up(){ // conect to wifi 
  if (millis() > wifimilis)  {
  WiFi.begin();
  pr_wifi = true;
  Serial.println("CONNECTING WIFI"); 
  wifimilis = (millis() + wifi_checkDelay) ;
  }
}

//----------------Supla.org ethernet layer --------------------------- -------------
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
    bool supla_arduino_svr_connect(const char *server, int port) {
          return client.connect(server, 2015);
    }    
    bool supla_arduino_svr_connected(void) {
          return client.connected();
    }    
    void supla_arduino_svr_disconnect(void) {
         client.stop();
    }    
    void supla_arduino_eth_setup(uint8_t mac[6], IPAddress *ip) {
    }

SuplaDeviceCallbacks supla_arduino_get_callbacks (void) {
SuplaDeviceCallbacks cb;

cb.tcp_read = & supla_arduino_tcp_read;
cb.tcp_write = & supla_arduino_tcp_write;
cb.eth_setup = & supla_arduino_eth_setup;
cb.svr_connected = & supla_arduino_svr_connected;
cb.svr_connect = & supla_arduino_svr_connect;
cb.svr_disconnect = & supla_arduino_svr_disconnect;
cb.get_temperature = NULL;
cb.get_temperature_and_humidity = NULL;
cb.get_rgbw_value = & get_rgbw_value;
cb.set_rgbw_value = & set_rgbw_value;
return cb;
}
