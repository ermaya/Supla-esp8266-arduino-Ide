first tests.
channel 0 = main switch "relay 1"
channel 1 = solar panel pump "relay 2"
channel 2 = auxiliary 1 "relay 3"
channel 3 = auxiliary 2 "relay 4"
channel 4 =  Auto / Manual mode for channel 1
channel 5,6,7,8 = DS18B20

channel 0 - 3 work as a bistable "light switch, power switch" as well as monostable "stair timer, door opening, etc. ..." from both the App and the local buttons.

simply set the channel mode in Supla cloud, perform an on-off cycle from the App and the module will store the duration of the ignition or bistable mode.

button 2 toggle the Auto / Manual mode and not "relay 2"
if "relay 2" is active "solar panel pump" and it is changed to Manual mode the relay turns off

for WiFiConfig button 1 pressed for 10 seconds. automatic during the first start "clean falsh"

sonoff4ch:
S6: 1
K5: all 1
K6: all 0
Gpio3 DS18B20