#include <ESP8266WiFi.h>
#include "EspPush.h"

void setup() {
  Serial.begin(115200);
  WiFi.begin("myNetwork", "secureWPAKii");
  while (WiFi.status() != WL_CONNECTED) delay(50);
  Serial.println("Connected");
  
  EspPush po = EspPush("SecureAppToken","UserToken");
  po.setDevice("Device1");
  po.setMessage("Testnotification");
  po.setSound("bike");
  Serial.println(po.send()); //should return 1 on success
}

void loop() {
}
