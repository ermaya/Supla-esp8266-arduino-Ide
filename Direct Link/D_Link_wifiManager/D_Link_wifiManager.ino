#include <FS.h>  
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h> 
#include <ArduinoJson.h>


const char* host = "svr.supla.org";
const int httpsPort = 443;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* fingerprint = "CF 05 98 89 CA FF 8E D8 5E 5C E0 C2 E4 F7 E6 C3 C7 50 DD 5C";

//define your default values here, if there are different values in config.json, they are overwritten.
//length should be max size + 1 
char Supla_server[20];
char D_Link_1[40];
char D_Link_2[40];
byte mac[6];
String url = "/direct/74/ZTVzdjkNhNjM/read";

//flag for saving data
bool shouldSaveConfig = false;
bool initialConfig = false;

#define TRIGGER_PIN D3
#define Link_1_PIN D5
#define Link_2_PIN D6

int timeout           = 120; // seconds to run for

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void ondemandwifiCallback () {
// The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 20);
  WiFiManagerParameter custom_D_Link_1("Link 1", "D_Link_1", D_Link_1, 40);
  WiFiManagerParameter custom_D_Link_2("Link 2", "D_Link_2", D_Link_2, 40);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  //add all your parameters here
  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_D_Link_1);
  wifiManager.addParameter(&custom_D_Link_2);

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
    strcpy(D_Link_1, custom_D_Link_1.getValue());
    strcpy(D_Link_2, custom_D_Link_2.getValue());
  
}

void setup() {  //------------------------------------------------ Setup ----------------------------------------------

  Serial.begin(115200);
  //sensors.begin();

  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  pinMode(Link_1_PIN, INPUT_PULLUP);
  pinMode(Link_2_PIN, INPUT_PULLUP);
  
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
        Serial.println(jsonBuffer.size());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(Supla_server, json["Supla_server"]);
          strcpy(D_Link_1, json["D_Link_1"]);
          strcpy(D_Link_2, json["D_Link_2"]);

        } else {
          Serial.println("failed to load json config");
          
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  } 
  host = Supla_server;
  WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.
}

void loop() { 

 if ( digitalRead(Link_1_PIN) == LOW )  {
    url = D_Link_1;
    direct_Link() ;
    
  }
   if ( digitalRead(Link_2_PIN) == LOW)  {
    url = D_Link_2;
    direct_Link() ;
    
  }
 
    // is configuration portal requested?
  if ( digitalRead(TRIGGER_PIN) == LOW|| (initialConfig))  {
    ondemandwifiCallback () ;
    initialConfig = false; 
  }
  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Supla_server"] = Supla_server;
    json["D_Link_1"] = D_Link_1;
    json["D_Link_2"] = D_Link_2;

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
    }
  
  if (WiFi.status() != WL_CONNECTED) 
   {
    WiFi_up();
   }
  

}

void WiFi_up() // Procedimiento de conexión para redes WiFi
{
  Serial.print("Conexión a la red ");
 // Serial.println(ssid);

  WiFi.begin(); // Intentar conectarse a la red


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
    Serial.println("Conexión hecha");
    Serial.println("Adres IP: ");
    Serial.print(WiFi.localIP());
    Serial.print(" / ");
    Serial.println(WiFi.subnetMask());
    Serial.print("puerta: ");
    Serial.println(WiFi.gatewayIP());
    long rssi = WiFi.RSSI();
    Serial.print("Fuerza de la señal (RSSI): ");
    Serial.print(rssi);
    Serial.println(" dBm");
  }
  else    
  {
    Serial.println("");
    Serial.println("La conexión no pudo hacerse");
  }
}
void direct_Link() {
  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  //String url = "/direct/74/ZTVzdjkNhNjM/turn-off";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  /*
  String line = client.readStringUntil('\n');
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
  */
}
