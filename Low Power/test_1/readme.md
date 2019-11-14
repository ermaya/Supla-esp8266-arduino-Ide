supla esp8266 sensor with deep sleep

esp8266 connects to Supla to send the sensor state and the supply voltage (as temperature) with an interval of 120 seconds.

Pinout:
Gpio  2 // D4 status led  ---on when in wificonfig---blinks every 2 minutes in normal operation
Gpio 14 // wificonfig pin ---hold Pressed and restart to access wificonfig ---will enter wificonfig automatically on first start.
Gpio 12 // D6 sensor pin

to make this possible it is necessary to edit SuplaDevice.h.
edit this line:
#define ACTIVITY_TIMEOUT 30 // max 240 -------------------
to this:
#define ACTIVITY_TIMEOUT 240 // max 240 -------------------
Remember to restore to the original state when you finish so as not to affect other compilations!
With this in theory we should be able to do sleep of up to 240 seconds but for some reason with more than 120 seconds it appears as disconnected.

The result is a huge reduction in the power consumption of esp8266.