#include <FS.h>       // ---- esp board manager 2.4.2 --- iwip Variant V2 higher Bandwidth
#include <Wire.h>
#include "paj7620.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
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
char Supla_server[20];
char linkn_1[11];
char D_Link_1[60];
char D_Link_2[60];
char linkn_2[11];
char D_Link_3[50];
char D_Link_4[60];
char linkn_3[11];
char D_Link_5[60];
char D_Link_6[60];
char linkn_4[11];
char D_Link_7[60];
char D_Link_8[60];
char linkn_5[11];
char D_Link_9[60];
char D_Link_10[60];
char linkn_6[11];
char D_Link_11[60];
char D_Link_12[60];
char Link_T[60];
char Link_T_H[60];
byte mac[6];
String url = "/direct";
bool shouldSaveConfig = false;
bool shouldSendLink = false;
bool initialConfig = false;
#define onboard_led 2 //D4  
#define Config_PIN 0 //D3     // triger config
#define GES_REACTION_TIME    300       // You can adjust the reaction time according to the actual circumstance.
#define GES_ENTRY_TIME      200       // When you want to recognize the Forward/Backward gestures, your gestures' reaction time must less than GES_ENTRY_TIME(0.8s). 
#define GES_QUIT_TIME     200
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
int Temp_mtbs = 60000;                //Temperature update interval 60 seconds
unsigned long Temp_lasttime; 
int TempDisp = 1;
double Temp = 0.0;
double Temp2 = 0.0;
int Humi = 0;
int TempDisp_mtbs = 3500;                //Temperature display interval 3.5 seconds
unsigned long TempDisp_lasttime; 
int isr_flag = 0;
int LinkN = 1;
int C_W_state = HIGH;            
int last_C_W_state = HIGH;       
unsigned long time_last_C_W_change = 0;   
long C_W_delay = 5000;               // config delay 5 seconds  ------------ opóźnienie konfiguracji 5 sekund
int timeout           = 120; // seconds to run for

void tick(){
  //toggle state
  int state = digitalRead(onboard_led);  // get the current state of D4 pin
  digitalWrite(onboard_led, !state);     // set pin to the opposite state
}
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void ondemandwifiCallback () {
  ticker.attach(0.2, tick);
          display.clearDisplay();
          display.setTextColor(1,0);
          display.setTextSize(1);         
          display.display(); 
          display.setCursor(0,16);
          display.println("wifi config connect"); 
          display.setCursor(0,24);
          display.println("  to wifi hotspot"); 
          display.setCursor(0,32);
          display.println(""); 
          display.setCursor(0,44);
          display.println("  GestureLink12t"); 
          display.display();
  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 20);
  WiFiManagerParameter custom_linkn_1("linkn_1", "location 1", linkn_1, 11);
  WiFiManagerParameter custom_D_Link_1("Link 1", "D Link 1 UP", D_Link_1, 60);
  WiFiManagerParameter custom_D_Link_2("Link 2", "D Link 1 DOWN", D_Link_2, 60);
  WiFiManagerParameter custom_linkn_2("linkn_2", "location 2", linkn_2, 11);
  WiFiManagerParameter custom_D_Link_3("Link 3", "D Link 2 UP", D_Link_3, 60);
  WiFiManagerParameter custom_D_Link_4("Link 4", "D Link 2 DOWN", D_Link_4, 60);
  WiFiManagerParameter custom_linkn_3("linkn_3", "location 3", linkn_3, 11);
  WiFiManagerParameter custom_D_Link_5("Link 5", "D Link 3 UP", D_Link_5, 60);
  WiFiManagerParameter custom_D_Link_6("Link 6", "D Link 3 DOWN", D_Link_6, 60);
  WiFiManagerParameter custom_linkn_4("linkn_4", "location 4", linkn_4, 11);
  WiFiManagerParameter custom_D_Link_7("Link 7", "D Link 4 UP", D_Link_7, 60);
  WiFiManagerParameter custom_D_Link_8("Link 8", "D Link 4 DOWN", D_Link_8, 60);
  WiFiManagerParameter custom_linkn_5("linkn_5", "location 5", linkn_5, 11);
  WiFiManagerParameter custom_D_Link_9("Link 9", "D Link 5 UP", D_Link_9, 60);
  WiFiManagerParameter custom_D_Link_10("Link 10", "D Link 5 DOWN", D_Link_10, 60);
  WiFiManagerParameter custom_linkn_6("linkn_6", "location 6", linkn_6, 11);
  WiFiManagerParameter custom_D_Link_11("Link 11", "D Link 6 UP", D_Link_11, 60);
  WiFiManagerParameter custom_D_Link_12("Link 12", "D Link 6 DOWN", D_Link_12, 60);
  WiFiManagerParameter custom_Link_T("Link T", "Direct Link T", Link_T, 60);
  WiFiManagerParameter custom_Link_T_H("Link T H", "Direct Link T+H", Link_T_H, 60);

  WiFiManager wifiManager;

  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_linkn_1);
  wifiManager.addParameter(&custom_D_Link_1);
  wifiManager.addParameter(&custom_D_Link_2);
  wifiManager.addParameter(&custom_linkn_2);
  wifiManager.addParameter(&custom_D_Link_3);
  wifiManager.addParameter(&custom_D_Link_4);
  wifiManager.addParameter(&custom_linkn_3);
  wifiManager.addParameter(&custom_D_Link_5);
  wifiManager.addParameter(&custom_D_Link_6);
  wifiManager.addParameter(&custom_linkn_4);
  wifiManager.addParameter(&custom_D_Link_7);
  wifiManager.addParameter(&custom_D_Link_8);
  wifiManager.addParameter(&custom_linkn_5);
  wifiManager.addParameter(&custom_D_Link_9);
  wifiManager.addParameter(&custom_D_Link_10);
  wifiManager.addParameter(&custom_linkn_6);
  wifiManager.addParameter(&custom_D_Link_11);
  wifiManager.addParameter(&custom_D_Link_12);
  wifiManager.addParameter(&custom_Link_T);
  wifiManager.addParameter(&custom_Link_T_H);

  wifiManager.setCustomHeadElement("<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>");

  wifiManager.setMinimumSignalQuality(10);

    wifiManager.setConfigPortalTimeout(timeout);

    if (!wifiManager.startConfigPortal("GestureLink12t")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      ESP.restart();
      delay(5000);
    }
    Serial.println("connected...yeey :)");
    
    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(linkn_1, custom_linkn_1.getValue());
    strcpy(D_Link_1, custom_D_Link_1.getValue());
    strcpy(D_Link_2, custom_D_Link_2.getValue());
    strcpy(linkn_2, custom_linkn_2.getValue());
    strcpy(D_Link_3, custom_D_Link_3.getValue());
    strcpy(D_Link_4, custom_D_Link_4.getValue());
    strcpy(linkn_3, custom_linkn_3.getValue());
    strcpy(D_Link_5, custom_D_Link_5.getValue());
    strcpy(D_Link_6, custom_D_Link_6.getValue());
    strcpy(linkn_4, custom_linkn_4.getValue());
    strcpy(D_Link_7, custom_D_Link_7.getValue());
    strcpy(D_Link_8, custom_D_Link_8.getValue()); 
    strcpy(linkn_5, custom_linkn_5.getValue());
    strcpy(D_Link_9, custom_D_Link_9.getValue());
    strcpy(D_Link_10, custom_D_Link_10.getValue());
    strcpy(linkn_6, custom_linkn_6.getValue());
    strcpy(D_Link_11, custom_D_Link_11.getValue());
    strcpy(D_Link_12, custom_D_Link_12.getValue()); 
    strcpy(Link_T, custom_Link_T.getValue()); 
    strcpy(Link_T_H, custom_Link_T_H.getValue());
}

void setup() {  //------------------------------------------------ Setup ----------------------------------------------
 
  wifi_set_sleep_type(NONE_SLEEP_T);

  Serial.begin(115200);

  pinMode(Config_PIN, INPUT);
  pinMode(onboard_led, OUTPUT);
 
  ticker.attach(0.8, tick);
    
  uint8_t error = 0;
  error = paj7620Init();      // initialize Paj7620 registers
  if (error) 
  {
    Serial.print("paj7620 INIT ERROR,CODE:");
    Serial.println(error);
  }
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false)) { 
    Serial.println("SSD1306 allocation failed");
    for(;;); // Don't proceed, loop forever
  }
  // Clear the buffer
  display.clearDisplay();
  display.setFont();             //Fuente por defecto -si no la hemos cambiado no es necesario seleccionarla
   
  if (WiFi.SSID()==""){   
    initialConfig = true;
  }
  //read configuration from FS json
  Serial.println("mounting FS...");
  display.setTextColor(1,0);
  display.setTextSize(1);
  display.setCursor(0,16);
  display.println("mounting FS...");
  display.display();
  
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
          strcpy(linkn_1, json["linkn_1"]);
          strcpy(D_Link_1, json["D_Link_1"]);
          strcpy(D_Link_2, json["D_Link_2"]);
          strcpy(linkn_2, json["linkn_2"]);
          strcpy(D_Link_3, json["D_Link_3"]);
          strcpy(D_Link_4, json["D_Link_4"]);
          strcpy(linkn_3, json["linkn_3"]);
          strcpy(D_Link_5, json["D_Link_5"]);
          strcpy(D_Link_6, json["D_Link_6"]);
          strcpy(linkn_4, json["linkn_4"]);
          strcpy(D_Link_7, json["D_Link_7"]);
          strcpy(D_Link_8, json["D_Link_8"]);
          strcpy(linkn_5, json["linkn_5"]);
          strcpy(D_Link_9, json["D_Link_9"]);
          strcpy(D_Link_10, json["D_Link_10"]);
          strcpy(linkn_6, json["linkn_6"]);
          strcpy(D_Link_11, json["D_Link_11"]);
          strcpy(D_Link_12, json["D_Link_12"]);
          strcpy(Link_T, json["Link_T"]);
          strcpy(Link_T_H, json["Link_T_H"]);

        } else {
          Serial.println("failed to load json config");
          display.setTextColor(1,0);
          display.setTextSize(1);
          display.setCursor(0,16);
          display.println("                     ");
          display.display(); 
          display.setCursor(0,16);
          display.println("failed to load config");
          display.display();          
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
          display.setTextColor(1,0);
          display.setTextSize(1);
          display.setCursor(0,16);
          display.println("                     ");
          display.display(); 
          display.setCursor(0,16);
          display.println("failed to mount FS");
          display.display();     
  } 
  host = Supla_server;
  WiFi.mode(WIFI_STA); 
  Serial.println (linkn_1);
  disp1(); 

}

void loop() { 

  if  (initialConfig)  {
    ondemandwifiCallback () ;
    initialConfig = false; 
  }

  yield(); 
  
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
   
    if (millis() > Temp_lasttime + Temp_mtbs)  {    //-------------Temp_lasttime--------------------

     update_temp();
         
    Temp_lasttime = millis();
  }
  if (millis() > TempDisp_lasttime + TempDisp_mtbs)  {    //-------------Temp_lasttime--------------------

     Disp_temp();
         
    TempDisp_lasttime = millis();
  }
   
   uint8_t data = 0, data1 = 0, error;
 
  error = paj7620ReadReg(0x43, 1, &data);       // Read Bank_0_Reg_0x43/0x44 for gesture result.
  if (!error) 
  {
    switch (data)                   // When different gestures be detected, the variable 'data' will be set to different values by paj7620ReadReg(0x43, 1, &data).
    {
      case GES_RIGHT_FLAG:
        delay(GES_ENTRY_TIME);
        paj7620ReadReg(0x43, 1, &data);
        if(data == GES_FORWARD_FLAG) 
        {
          Serial.println("Forward");
          delay(GES_QUIT_TIME);
        }
        else if(data == GES_BACKWARD_FLAG) 
        {
          Serial.println("Backward");
          delay(GES_QUIT_TIME);
        }
        else
        {
          Serial.print("RIGHT ");
          display.setTextColor(1,0);
          display.setTextSize(1);
          display.setCursor(0,24);
          display.println("                     ");
          display.display(); 
          display.setCursor(0,24);
          display.println("   >>>>>>>>>>>>>>");  
          display.display();
          TempDisp_lasttime = millis();
        if (LinkN < 7) {
          LinkN = LinkN +1;
        }
        if (LinkN == 7) {
          LinkN = 1;
        }
        if (LinkN == 1){
          Serial.println(LinkN);
          Serial.println (linkn_1); 
          disp1(); 
        }
        if (LinkN == 2){
          Serial.println(LinkN);
          Serial.println (linkn_2);
          disp2();   
        }
        if (LinkN == 3){
          Serial.println(LinkN);
          Serial.println (linkn_3);
          disp3();   
        }
        if (LinkN == 4){
          Serial.println(LinkN);
          Serial.println (linkn_4);
          disp4();   
        }
        if (LinkN == 5){
          Serial.println(LinkN);
          Serial.println (linkn_5);
          disp5();   
        }
        if (LinkN == 6){
          Serial.println(LinkN);
          Serial.println (linkn_6);
          disp6();   
        }
        }
        yield();           
        break;
      case GES_LEFT_FLAG: 
        delay(GES_ENTRY_TIME);
        paj7620ReadReg(0x43, 1, &data);
        if(data == GES_FORWARD_FLAG) 
        {
          Serial.println("Forward");
          delay(GES_QUIT_TIME);
        }
        else if(data == GES_BACKWARD_FLAG) 
        {
          Serial.println("Backward");
          delay(GES_QUIT_TIME);
        }
        else
        {
          Serial.print("LEFT ");
          display.setTextColor(1,0);
          display.setTextSize(1);
          display.setCursor(0,24);
          display.println("                     ");
          display.display(); 
          display.setCursor(0,24);
          display.println("   <<<<<<<<<<<<<<");  
          display.display();
          TempDisp_lasttime = millis();
        if (LinkN > 0) {
          LinkN = LinkN -1;
        }
        if (LinkN == 0) {
          LinkN = 6;
        }
        if (LinkN == 1){
          Serial.println(LinkN);
          Serial.println (linkn_1); 
          disp1();  
        }
        if (LinkN == 2){
          Serial.println(LinkN);
          Serial.println (linkn_2); 
          disp2();  
        }
        if (LinkN == 3){
          Serial.println(LinkN);
          Serial.println (linkn_3); 
          disp3();  
        }
        if (LinkN == 4){
          Serial.println(LinkN);
          Serial.println (linkn_4); 
          disp4();  
        }
        if (LinkN == 5){
          Serial.println(LinkN);
          Serial.println (linkn_5);
          disp5();   
        }
        if (LinkN == 6){
          Serial.println(LinkN);
          Serial.println (linkn_6);
          disp6();   
        }
        } 
        yield();          
        break;
      case GES_UP_FLAG:
        delay(GES_ENTRY_TIME);
        paj7620ReadReg(0x43, 1, &data);
        if(data == GES_FORWARD_FLAG) 
        {
          Serial.println("Forward");
          delay(GES_QUIT_TIME);
        }
        else if(data == GES_BACKWARD_FLAG) 
        {
          Serial.println("Backward");
          delay(GES_QUIT_TIME);
        }
        else
        {
          Serial.println("UP");
          display.setTextColor(1,0);
          display.setTextSize(1);
          display.setCursor(0,24);
          display.println("                     ");
          display.display(); 
          display.setCursor(0,24);
          display.println("   ^^^^^^^^^^^^^^");  
          display.display();
          TempDisp_lasttime = millis();
        if (LinkN == 1){
        Serial.println("send Link 1 "); 
        url = D_Link_1;
        direct_Link() ; 
        }
        if (LinkN == 2){
        Serial.println("send Link 3 ");
        url = D_Link_3;
        direct_Link() ;  
        }
        if (LinkN == 3){
        Serial.println("send Link 5 ");
        url = D_Link_5;
        direct_Link() ;  
        }
        if (LinkN == 4){
        Serial.println("send Link 7 ");
        url = D_Link_7;
        direct_Link() ;  
        }
        if (LinkN == 5){
        Serial.println("send Link 9 ");
        url = D_Link_9;
        direct_Link() ;  
        }
        if (LinkN == 6){
        Serial.println("send Link 11 ");
        url = D_Link_11;
        direct_Link() ;  
        }
        }
        yield();           
        break;
      case GES_DOWN_FLAG:
        delay(GES_ENTRY_TIME);
        paj7620ReadReg(0x43, 1, &data);
        if(data == GES_FORWARD_FLAG) 
        {
          Serial.println("Forward");
          delay(GES_QUIT_TIME);
        }
        else if(data == GES_BACKWARD_FLAG) 
        {
          Serial.println("Backward");
          delay(GES_QUIT_TIME);
        }
        else
        {
          Serial.println("DOWN");
          display.setTextColor(1,0);
          display.setTextSize(1);
          display.setCursor(0,24);
          display.println("                     ");
          display.display(); 
          display.setCursor(0,24);
          display.println("   vvvvvvvvvvvvvv");    
          display.display();
          TempDisp_lasttime = millis();
        if (LinkN == 1){
        Serial.println("send Link 2 ");
        url = D_Link_2;
        direct_Link() ;  
        }
        if (LinkN == 2){
        Serial.println("send Link 4 ");
        url = D_Link_4;
        direct_Link() ;  
        }
        if (LinkN == 3){
        Serial.println("send Link 6 ");
        url = D_Link_6;
        direct_Link() ;  
        }
        if (LinkN == 4){
        Serial.println("send Link 8 ");
        url = D_Link_8;
        direct_Link() ;  
        }
        if (LinkN == 5){
        Serial.println("send Link 10 ");
        url = D_Link_10;
        direct_Link() ;  
        }
        if (LinkN == 6){
        Serial.println("send Link 12 ");
        url = D_Link_12;
        direct_Link() ;  
        }
        }
        yield();           
        break;
      case GES_FORWARD_FLAG:
        Serial.println("Forward");
        delay(GES_QUIT_TIME);
        yield(); 
        break;
      case GES_BACKWARD_FLAG:     
        Serial.println("Backward");
        delay(GES_QUIT_TIME);
        yield(); 
        break;
      case GES_CLOCKWISE_FLAG:
        Serial.println("Clockwise");    
        break;
      case GES_COUNT_CLOCKWISE_FLAG:
        Serial.println("anti-clockwise");
        break;  
        default:
        paj7620ReadReg(0x44, 1, &data1);
        if (data1 == GES_WAVE_FLAG) 
        {
          Serial.println("wave");
        }
        break;
    }
  }
  delay(100);
  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
          display.setTextColor(1,0);
          display.setTextSize(1);
          display.setCursor(0,16);
          display.println("                     ");
          display.display(); 
          display.setCursor(0,16);
          display.println("    saving config"); 
          display.display();    
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Supla_server"] = Supla_server;
    json["linkn_1"] = linkn_1;
    json["D_Link_1"] = D_Link_1;
    json["D_Link_2"] = D_Link_2;
    json["linkn_2"] = linkn_2;
    json["D_Link_3"] = D_Link_3;
    json["D_Link_4"] = D_Link_4;
    json["linkn_3"] = linkn_3;
    json["D_Link_5"] = D_Link_5;
    json["D_Link_6"] = D_Link_6;
    json["linkn_4"] = linkn_4;
    json["D_Link_7"] = D_Link_7;
    json["D_Link_8"] = D_Link_8;
    json["linkn_5"] = linkn_5;
    json["D_Link_9"] = D_Link_9;
    json["D_Link_10"] = D_Link_10;
    json["linkn_6"] = linkn_6;
    json["D_Link_11"] = D_Link_11;
    json["D_Link_12"] = D_Link_12;
    json["Link_T"] = Link_T;
    json["Link_T_H"] = Link_T_H;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    Serial.println("config saved");
          display.setTextColor(1,0);
          display.setTextSize(1);
          display.setCursor(0,16);
          display.println("                     ");
          display.display(); 
          display.setCursor(0,16);
          display.println("    config saved"); 
          display.display();    
    shouldSaveConfig = false;
    WiFi.mode(WIFI_STA);
    ticker.detach();
    digitalWrite(onboard_led, HIGH);
    ESP.restart();
      delay(5000); 
    }
  
  if (WiFi.status() != WL_CONNECTED) 
   {
    ticker.attach(0.8, tick);
          display.setTextColor(1,0);
          display.setTextSize(1);
          display.setCursor(0,16);
          display.println("                     ");
          display.display(); 
          display.setCursor(0,16);
          display.println("  conecting wifi"); 
          display.display();    
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
          display.setTextColor(1,0);
          display.setTextSize(1);
          display.setCursor(0,16);
          display.println("                     ");
          display.display(); 
          display.setCursor(0,16);
          display.println("   WIFI Connected"); 
          display.display();
       update_temp();
       Disp_temp();    
    digitalWrite(onboard_led, HIGH);
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
          display.setTextColor(1,0);
          display.setTextSize(1);
          display.setCursor(0,16);
          display.println("                     ");
          display.display(); 
          display.setCursor(0,16);
          display.println("  connection failed"); 
          display.display();    
  }
}
void direct_Link() {
  
  WiFiClientSecure client;
  Serial.print("connecting to ");  
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
          display.setTextColor(1,0);
          display.setTextSize(1);
          display.setCursor(0,24);
          display.println("                     ");
          display.display(); 
          display.setCursor(0,24);
          display.println("  connection failed");  
          display.display();
          TempDisp_lasttime = millis(); 
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
    display.invertDisplay(true);
    delay(300);
    display.invertDisplay(false); 
    Serial.println("successfull!");
    Disp_temp();    
  } else {
    Serial.println("failed");
          display.setTextColor(1,0);
          display.setTextSize(1);
          display.setCursor(0,24);
          display.println("                     ");
          display.display(); 
          display.setCursor(0,24);
          display.println("      failed!"); 
          display.display(); 
          TempDisp_lasttime = millis();
  }
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
          display.setTextColor(1,0);
          display.setTextSize(1);
          display.setCursor(0,24);
          display.println("                     ");
          display.display();        
 }
void Temp_Hum_D_Link() {

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
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  DynamicJsonBuffer  jsonBuffer(200);
    JsonObject& root = jsonBuffer.parseObject(line);

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  Humi = root["humidity"];
  Temp2 = root["temperature"];
  
  Serial.print("temperature ");
  Serial.println(Temp2, 2);
  Serial.print("humidity ");
  Serial.println(Humi);
  Serial.println("closing connection");
}
void Temp_D_Link() {

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
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  DynamicJsonBuffer  jsonBuffer(200);
    JsonObject& root = jsonBuffer.parseObject(line);

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  Temp = root["temperature"];
  
  Serial.print("temperature ");
  Serial.println(Temp, 2);
  Serial.println("closing connection");
}
void disp1(){
  display.setTextColor(1,0);  
  display.setCursor(0,0);
  display.setTextSize(2);
  display.print("          ");
  display.display();
  display.setCursor(0,0);
  display.print(linkn_1);
  display.setTextSize(3);
  display.setTextColor(0,1);     //Color invertido
  display.setCursor(0,32);
  display.print("1");
  display.setTextColor(1,0);  
  display.setCursor(20,32);
  display.print("2");
  display.setTextColor(1,0);  
  display.setCursor(40,32);
  display.print("3");
  display.setTextColor(1,0);  
  display.setCursor(60,32);
  display.print("4");
  display.setTextColor(1,0);  
  display.setCursor(80,32);
  display.print("5");
  display.setTextColor(1,0);  
  display.setCursor(100,32);
  display.print("6");
  display.display(); 
}
void disp2(){
  display.setTextColor(1,0);  
  display.setCursor(0,0);
  display.setTextSize(2);
  display.print("          ");
  display.display();
  display.setCursor(0,0);
  display.print(linkn_2);
  display.setTextSize(3);
  display.setTextColor(1,0); 
  display.setCursor(0,32);
  display.print("1");
  display.setTextColor(0,1);     //Color invertido
  display.setCursor(20,32);
  display.print("2");
  display.setTextColor(1,0);  
  display.setCursor(40,32);
  display.print("3");
  display.setTextColor(1,0);  
  display.setCursor(60,32);
  display.print("4");
  display.setTextColor(1,0);  
  display.setCursor(80,32);
  display.print("5");
  display.setTextColor(1,0);  
  display.setCursor(100,32);
  display.print("6");
  display.display(); 
}
void disp3(){
  display.setTextColor(1,0);  
  display.setCursor(0,0);
  display.setTextSize(2);
  display.print("          ");
  display.display();
  display.setCursor(0,0);
  display.print(linkn_3);
  display.setTextSize(3);
  display.setTextColor(1,0); 
  display.setCursor(0,32);
  display.print("1");
  display.setTextColor(1,0);  
  display.setCursor(20,32);
  display.print("2");
  display.setTextColor(0,1);     //Color invertido
  display.setCursor(40,32);
  display.print("3");
  display.setTextColor(1,0);  
  display.setCursor(60,32);
  display.print("4");
  display.setTextColor(1,0);  
  display.setCursor(80,32);
  display.print("5");
  display.setTextColor(1,0);  
  display.setCursor(100,32);
  display.print("6");
  display.display(); 
}
void disp4(){
  display.setTextColor(1,0);  
  display.setCursor(0,0);
  display.setTextSize(2);
  display.print("          ");
  display.display();
  display.setCursor(0,0);
  display.print(linkn_4);
  display.setTextSize(3);
  display.setTextColor(1,0); 
  display.setCursor(0,32);
  display.print("1");
  display.setTextColor(1,0);  
  display.setCursor(20,32);
  display.print("2");
  display.setTextColor(1,0);  
  display.setCursor(40,32);
  display.print("3");
  display.setTextColor(0,1);     //Color invertido
  display.setCursor(60,32);
  display.print("4");
  display.setTextColor(1,0);  
  display.setCursor(80,32);
  display.print("5");
  display.setTextColor(1,0);  
  display.setCursor(100,32);
  display.print("6");
  display.display(); 
}
void disp5(){
  display.setTextColor(1,0);  
  display.setCursor(0,0);
  display.setTextSize(2);
  display.print("          ");
  display.display();
  display.setCursor(0,0);
  display.print(linkn_5);
  display.setTextSize(3);
  display.setTextColor(1,0); 
  display.setCursor(0,32);
  display.print("1");
  display.setTextColor(1,0);  
  display.setCursor(20,32);
  display.print("2");
  display.setTextColor(1,0);  
  display.setCursor(40,32);
  display.print("3");
  display.setTextColor(1,0);  
  display.setCursor(60,32);
  display.print("4");
  display.setTextColor(0,1);     //Color invertido
  display.setCursor(80,32);
  display.print("5");
  display.setTextColor(1,0);  
  display.setCursor(100,32);
  display.print("6");
  display.display(); 
}
void disp6(){
  display.setTextColor(1,0);  
  display.setCursor(0,0);
  display.setTextSize(2);
  display.print("          ");
  display.display();
  display.setCursor(0,0);
  display.print(linkn_6);
  display.setTextSize(3);
  display.setTextColor(1,0); 
  display.setCursor(0,32);
  display.print("1");
  display.setTextColor(1,0);  
  display.setCursor(20,32);
  display.print("2");
  display.setTextColor(1,0);  
  display.setCursor(40,32);
  display.print("3");
  display.setTextColor(1,0);  
  display.setCursor(60,32);
  display.print("4");
  display.setTextColor(1,0);  
  display.setCursor(80,32);
  display.print("5");
  display.setTextColor(0,1);     //Color invertido
  display.setCursor(100,32);
  display.print("6");
  display.display(); 
}
void Disp_temp(){
  if (TempDisp == 4){
    TempDisp = 1;
  }
  if (TempDisp == 1){
          display.setTextColor(1,0);
          display.setTextSize(1);
          display.setCursor(0,24);
          display.println("                     ");
          display.display(); 
          display.setCursor(0,24);
          display.print("Temp.interior ");
          display.print(Temp, 2);
          display.print(" C");  
          display.display(); 
    
  }
  if (TempDisp == 2){
          display.setTextColor(1,0);
          display.setTextSize(1);
          display.setCursor(0,24);
          display.println("                     ");
          display.display(); 
          display.setCursor(0,24);
          display.print("Temp.exterior ");
          display.print(Temp2, 2);
          display.print(" C");  
          display.display(); 
    
    
  }
  if (TempDisp == 3){
          display.setTextColor(1,0);
          display.setTextSize(1);
          display.setCursor(0,24);
          display.println("                     ");
          display.display(); 
          display.setCursor(0,24);
          display.print("   Hunidity ");
          display.print(Humi); 
          display.print(" %"); 
          display.display(); 
    
    
  }
  TempDisp = TempDisp + 1;
}
void update_temp(){
     url = Link_T; 
     Temp_D_Link();

     url = Link_T_H; 
     Temp_Hum_D_Link();
}

