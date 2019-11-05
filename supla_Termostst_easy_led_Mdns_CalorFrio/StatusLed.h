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
'''''''''''''''''''

Supla Status List:
  1 _REGISTER_IN_PROGRESS  
  2 _ALREADY_INITIALIZED     
  3 _CB_NOT_ASSIGNED         
  4 _INVALID_GUID  --------  "Incorrect device GUID!"--"Incorrect AuthKey!"--"Registration disabled!"--"No location available!"--"User conflict!"        
  5 _UNKNOWN_SERVER_ADDRESS  
  6 _UNKNOWN_LOCATION_ID     
  7 _INITIALIZED             
  8 _CHANNEL_LIMIT_EXCEEDED  
  9 _DISCONNECTED              
 11 _ITERATE_FAIL            
 12 _PROTOCOL_VERSION_ERROR  
 13 _BAD_CREDENTIALS         
 14 _TEMPORARILY_UNAVAILABLE 
 15 _LOCATION_CONFLICT       
 16 _CHANNEL_CONFLICT        
 17 _REGISTERED_AND_READY ------ connection ok Led Off -----
 18 _DEVICE_IS_DISABLED      
 19 _LOCATION_IS_DISABLED    
 20 _DEVICE_LIMIT_EXCEEDED   
*/


#include "EasyLed.h"
EasyLed led(status_led, COMMON_POSITIVE);
bool ledRunnin = false;

void finished() {
  ledRunnin = false;
  led.update();
}
void StatusLedOn(){
   led.on();
   led.update();
}
void StatusLedOff(){
   led.off();
   led.update();
}
void StatusLed(int Lstate){
     led.update();                 
        if ((Lstate == 17) && (WiFi.status() == WL_CONNECTED) && (ledRunnin == true)){  //---- check if Wi-Fi is connected, if Supla is (REGISTERED_AND_READY) and if the Status Led is running 
        Serial.println("Status Supla OK "); 
        ledRunnin = false;
        led.off();
        }
    if ((ledRunnin == false) && (WiFi.status() == WL_CONNECTED) && (Lstate != 17)){ //----- check if Wi-Fi is connected, if Supla is not (REGISTERED_AND_READY) and if enough time has elapsed since the last pass.
        if (Lstate == 10){Lstate = 1;}
        Serial.print("Status Led ");
        Serial.println(Lstate);
        ledRunnin = true;
        led.blink(300  /* time on */,
            200  /* time off */,
            (int(Lstate))    /* cycles */,
            3000 /* pause between sequences */,
            1    /* sequences */,
            finished /* function to call when finished */
            ); /*----Flashing X times depending on the connection state */            
        }
      if ((ledRunnin == false) && (WiFi.status() != WL_CONNECTED)){  //---- check if WiFi is disconnected and if the Status Led is not running.
        Serial.println("Status No Wifi ");
        ledRunnin = true; 
        led.blink(1000  /* time on */,
            1000  /* time off */,
            1    /* cycles */,
            1000 /* pause between sequences */,
            1    /* sequences */,
            finished /* function to call when finished */
            );/*----Flashing every second*/             
        }   
}
