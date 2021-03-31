wificonfig_pin  =  D3 -- Gpio 0  "5 seconds"
DS18B20         =  TX --  Gpio 1
button auto/Man =  RX --  Gpio 3     
relay_pin       =  D6 -- Gpio 12  
led_pin         =  D7 -- Gpio 13 

channel 0 "power switch" = automatic/manual mode
channel 1 "power switch" = manual on/off "shows the state of the relay"
channel 2 "temperature" = shows the temperature read by DS18B20
channel 3 "temperature" = shows the temperature set for the thermostat
channel 4 "power switch" = increases or decreases the temperature set for the thermostat by 0.5ºC

in automatic mode "channel 0 on":
if the DS18B20 temperature falls below the temperature set for the thermostat the relay is activated. 
if the DS18B20 temperature exceeds the temperature set for the thermostat by +0.5ºC the relay is deactivated.

when toggle from automatic to manual mode "channel 0" the relay is always deactivated, this allows setting schedules in the Supla cloud.

in manual mode "channel 0 off":
manual on/off "channel 1" controls the relay regardless of temperatures.

remember that the "DS18B20" temperature is updated every 10 seconds so it takes up to 10 seconds for the thermostat to react, insignificant in real life but it can confuse you during testing.