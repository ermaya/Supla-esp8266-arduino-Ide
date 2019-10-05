/*
## Example

#define status_led 16    //------------------- status Led -----define led gpio before #include "StatusLed.h" ------------------
#include "StatusLed.h"

void status_func(int status, const char *msg) {     //    --------------- Status ----------------------
 s=status;                                          //    -- to check if we are registered and ready --
}

void setup() { 

SuplaDevice.setStatusFuncImpl(&status_func);    //   ----------------------------- Status -----------------------------
}

void loop() {
  StatusLed(s); 
  
}
```
other functions:
StatusLedOn()  // to turn on the Status Led (we use it to indicate wifi config).
StatusLedOff() // to turn off the Status Led

Supla Status List:
STATUS_ALREADY_INITIALIZED     2
STATUS_CB_NOT_ASSIGNED         3
STATUS_INVALID_GUID            4
STATUS_UNKNOWN_SERVER_ADDRESS  5
STATUS_UNKNOWN_LOCATION_ID     6
STATUS_INITIALIZED             7
STATUS_CHANNEL_LIMIT_EXCEEDED  8
STATUS_DISCONNECTED            9
STATUS_REGISTER_IN_PROGRESS    10
STATUS_ITERATE_FAIL            11
STATUS_PROTOCOL_VERSION_ERROR  12
STATUS_BAD_CREDENTIALS         13
STATUS_TEMPORARILY_UNAVAILABLE 14
STATUS_LOCATION_CONFLICT       15
STATUS_CHANNEL_CONFLICT        16
STATUS_REGISTERED_AND_READY    17  ------ connection ok Led Off -----
STATUS_DEVICE_IS_DISABLED      18
STATUS_LOCATION_IS_DISABLED    19
STATUS_DEVICE_LIMIT_EXCEEDED   20
*/
#include <jled.h>  //--https://github.com/jandelgado/jled--- 
constexpr auto PIN_LED = status_led;
auto led = JLed(PIN_LED).LowActive().Blink(1000, 1000).Forever();
unsigned long statusmilis;


void StatusLedOn(){
   led.Reset();
   JLed(PIN_LED).LowActive().On().Update();
}
void StatusLedOff(){
   led.Reset();
   JLed(PIN_LED).LowActive().Off().Update();
}
void StatusLed(int Lstate){
        led.Update();
      if ((WiFi.status() == WL_CONNECTED) && (millis() > statusmilis) && (Lstate != 17)){ //-----We check if Wi-Fi is connected, if Supla is not (REGISTERED_AND_READY) and if enough time has elapsed since the last pass.
        Serial.print("Status Led ");
        Serial.println(Lstate);
        led.Reset().Update();
        JLed(PIN_LED).LowActive().Off().Update();
        led = JLed(PIN_LED).LowActive().Blink(300, 250).Repeat(uint16_t(Lstate));   //----Flashing X times depending on the connection state
        statusmilis = (millis() + 15000) ;
    }
      if ((WiFi.status() != WL_CONNECTED) && (led.IsRunning()== false)){  //----We check if WiFi is disconnected and if the Status Led is not running.
        Serial.println("Status No Wifi "); 
        led.Reset().Update();
        led = JLed(PIN_LED).LowActive().Blink(1000, 1000).Forever();  //----Flashing every second
        statusmilis = (millis() + 5000) ;
    }
      if ((Lstate == 17) && (led.IsRunning()== true) && (WiFi.status() == WL_CONNECTED)){  //----We check if Wi-Fi is connected, if Supla is (REGISTERED_AND_READY) and if the Status Led is running 
        Serial.println("Status Supla OK "); 
        led.Stop();
        JLed(PIN_LED).LowActive().Off().Update();  //----Led Off
    }       
}
