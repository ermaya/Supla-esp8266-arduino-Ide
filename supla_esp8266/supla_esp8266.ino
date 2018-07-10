#include <FS.h>  

#include <ESP8266WiFi.h>
#define SUPLADEVICE_CPP
#include <SuplaDevice.h>


#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h> 
#include <ArduinoJson.h>

#define BTN_COUNT 3

WiFiClient client;
//define your default values here, if there are different values in config.json, they are overwritten.
//length should be max size + 1 
char Supla_server[40];
char Location_id[15];
char Location_Pass[20];
byte mac[6];

//flag for saving data
bool shouldSaveConfig = false;
bool initialConfig = false;

#define TRIGGER_PIN D3       // wifi config trigger gpio pin
int timeout           = 120; // wifi config seconds to run for
int config_button_state = HIGH;                
int last_config_button_state = HIGH;            
unsigned long time_last_Cambio_boton_C_W = 0;    
long config_delay = 10000;               // 10000 = 10 second delay

unsigned int Rs = 100000;
double Vcc = 3.3;

int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;

float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void ondemandwifiCallback () {
// The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 40);
  WiFiManagerParameter custom_Location_id("ID", "Location_id", Location_id, 15);
  WiFiManagerParameter custom_Location_Pass("Password", "Location_Pass", Location_Pass, 20);

  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  //add all your parameters here
  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_Location_id);
  wifiManager.addParameter(&custom_Location_Pass);

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  wifiManager.setMinimumSignalQuality();

  // set configportal timeout
    wifiManager.setConfigPortalTimeout(timeout);

    if (!wifiManager.startConfigPortal("OnDemandAP")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.restart();
      delay(5000);
    }
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    
    //read updated parameters
    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(Location_id, custom_Location_id.getValue());
    strcpy(Location_Pass, custom_Location_Pass.getValue());
   
}

// ntc Sensor read implementation---------------------------------- Temp -------------------------------------------
double get_temperature(int channelNumber, double last_val) {  // 10k Ntc Adc to Gnd and 100k Resistor Adc to +3,3v
    
      int val = 0;
      for(int i = 0; i < 20; i++) {
      val += analogRead(A0);
      delay(1);
      }
      val = val / 20;
      double V_NTC = (double)val / 1024;
      double R_NTC = (Rs * V_NTC) / (Vcc - V_NTC);
      R_NTC = log(R_NTC);
      double t = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * R_NTC * R_NTC ))* R_NTC );
      t = t - 273.15;

    
      return t;  
}
typedef struct {  //------------------------------------------- BTN ----------------------------------------------------
  int pin;
  int relay_pin;
  int channel;
  char last_val;
  unsigned long last_time;
} _btn_t;

_btn_t btn[BTN_COUNT];

void supla_timer() {
  char v;
  unsigned long now = millis();
  
  for(int a=0;a<BTN_COUNT;a++)
    if (btn[a].pin > 0) {
        v = digitalRead(btn[a].pin);
        if (v != btn[a].last_val && now - btn[a].last_time ) {
           btn[a].last_val = v;
           btn[a].last_time = now;
           if (v==0)
             {
                 if ( digitalRead(btn[a].relay_pin) == 0 ) {
                  SuplaDevice.relayOff(btn[a].channel);
                  Serial.print("BTN Switsh off relay ");
                  Serial.println(btn[a].relay_pin);
                 } else {
                  SuplaDevice.relayOn(btn[a].channel, 0);
                  Serial.print("BTN Switsh on relay ");
                  Serial.println(btn[a].relay_pin);
                 }        
             }
        }
  
    }
}
void supla_btn_init() {
  for(int a=0;a<BTN_COUNT;a++)
    if (btn[a].pin > 0) {
        pinMode(btn[a].pin, INPUT_PULLUP);
        btn[a].last_val = digitalRead(btn[a].pin);
        btn[a].last_time = millis();
    }
}

void setup() {  //------------------------------------------------ Setup ----------------------------------------------

  Serial.begin(115200);
  //sensors.begin();

  pinMode(TRIGGER_PIN, INPUT);
  
  if (WiFi.SSID()==""){
    //Serial.println("We haven't got any access point credentials, so get them now");   
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
        //json.printTo(Serial);   //print config data to serial on startup
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(Supla_server, json["Supla_server"]);
          strcpy(Location_id, json["Location_id"]);
          strcpy(Location_Pass, json["Location_Pass"]);

        } else {
          Serial.println("failed to load json config");
          
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.

   char GUID[SUPLA_GUID_SIZE] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};  // ﻿with GUID that you can retrieve from https://www.supla.org/arduino/get-guid.
   
   WiFi.macAddress(mac);  // Get mac
   
   
  //CHANNEL0 - RELAY                 
  SuplaDevice.addRelay(5, true); //D1
  //CHANNEL1 - RELAY              
  SuplaDevice.addRelay(4,true);  //D2
  //CHANNEL2 - RELAY              
  SuplaDevice.addRelay(2,true);  //D4 
  // CHANNEL3 - Thermometer DS18B20
  SuplaDevice.addDS18B20Thermometer();

  memset(btn, 0, sizeof(btn));
  btn[0].pin =14;  // pin gpio  button D5
  btn[0].relay_pin =5;  // pin gpio on which is relay  D1
  btn[0].channel =0;     // supla channel
  btn[1].pin =12;  // pin gpio  button D6
  btn[1].relay_pin =4;  // pin gpio on which is relay  D2
  btn[1].channel =1;     // supla channel
  btn[2].pin =13;  // pin gpio  button D7 
  btn[2].relay_pin =2;  // pin gpio on which is relay  D4 
  btn[2].channel =2;     // supla channel
  supla_btn_init();
  SuplaDevice.setTimerFuncImpl(&supla_timer);
  SuplaDevice.setName("elmaya esp8266");

  int LocationID = atoi(Location_id);
  SuplaDevice.begin(GUID,              // Global Unique Identifier 
                    mac,               // Ethernet MAC address
                    Supla_server,  // SUPLA server address
                    LocationID,                 // Location ID 
                    Location_Pass);               // Location Password
    
}

void loop() {

    // is configuration portal requested?
  int read_config_button = digitalRead(TRIGGER_PIN);{    
   if (read_config_button != last_config_button_state) {       
     
     time_last_Cambio_boton_C_W = millis();
   }
   if ((millis() - time_last_Cambio_boton_C_W) > config_delay) {
     
     if (read_config_button != config_button_state) {     
       config_button_state = read_config_button;      
       if (config_button_state == LOW) {
        ondemandwifiCallback ();
       }
     }
    }
   last_config_button_state = read_config_button;            
   }
  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Supla_server"] = Supla_server;
    json["Location_id"] = Location_id;
    json["Location_Pass"] = Location_Pass;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    Serial.println("config saved");
    shouldSaveConfig = false;
    //end save
    WiFi.softAPdisconnect(true);   //  close AP 
  }
  
  if (WiFi.status() != WL_CONNECTED) 
  {
    WiFi_up();
  }
  
  SuplaDevice.iterate();
  SuplaDevice.setTemperatureCallback(&get_temperature);
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

       WiFi_up();
    }

SuplaDeviceCallbacks supla_arduino_get_callbacks(void) {
          SuplaDeviceCallbacks cb;
          
          cb.tcp_read = &supla_arduino_tcp_read;
          cb.tcp_write = &supla_arduino_tcp_write;
          cb.eth_setup = &supla_arduino_eth_setup;
          cb.svr_connected = &supla_arduino_svr_connected;
          cb.svr_connect = &supla_arduino_svr_connect;
          cb.svr_disconnect = &supla_arduino_svr_disconnect;
          cb.get_temperature = &get_temperature;
           cb.get_temperature_and_humidity = NULL;
          cb.get_rgbw_value = NULL;
          cb.set_rgbw_value = NULL;
          
          return cb;
}

void WiFi_up() // WiFi connection
{  
  WiFi.begin(); // wifi start


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
    Serial.println("");
    Serial.println("Connected to");
    Serial.println("IP: ");
    Serial.print(WiFi.localIP());
    Serial.print(" / ");
    Serial.println(WiFi.subnetMask());
    Serial.print("geteway: ");
    Serial.println(WiFi.gatewayIP());
    long rssi = WiFi.RSSI();
    Serial.print("signal strengt (RSSI): ");
    Serial.print(rssi);
    Serial.println(" dBm");
  }
  else    
  {
    Serial.println("");
    Serial.println("connection fail");
  }
}
