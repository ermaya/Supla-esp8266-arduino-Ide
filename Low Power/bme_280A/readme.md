supla esp8266 sensor with deep sleep

esp8266 connects to Supla to send the the status of the
 sensor"channel no 0 ",
  supply voltage as temperature channel"channel no 1 ",
   atmospheric pressure"channel no 2 ",
    temperature"channel no 3 " and
     humidity"channel no 4 " (humidity as a temperature channel since the temperature and humidity channel at the first reading sends the same value for both)
	  with an interval of 220 (SLEEP_TIME) seconds.

Pinout:
Gpio  5 // D1 scl
Gpio  4 // D2 sda
Gpio  2 // D4 status led  ---on when in wificonfig---blinks every SLEEP_TIME seconds in normal operation
Gpio 14 // D5 wificonfig pin ---hold Pressed and restart to access wificonfig ---will enter wificonfig automatically on first start.
Gpio 12 // D6 sensor pin

to make this possible it is necessary to edit SuplaDevice.h.
edit this line:
#define ACTIVITY_TIMEOUT 30 // max 240 -------------------
to this:
#define ACTIVITY_TIMEOUT 240 // max 240 -------------------
Remember to restore to the original state when you finish so as not to affect other compilations!

The result is a huge reduction in the power consumption of esp8266.