here you have something for test:

compatible with MAX6675 & MAX31655 

D1 Gpio 5 = relay
D3 Gpio 0 = botton -- toggle automatic/manual mode, hold down for at least 10 seconds "WiFiConfig"
D4 Gpio 2 = Led "Buildin"
D5 Gpio14 = Do "max6675"
D6 Gpio12 = Cs "max6675"
D7 Gpio13 = Clk "max6675"

WiFiConfig:
hold botton down for at least 10 seconds. "D2 -- Gpio0 to ground"
the Led turns on.
connect to WiFi AP name "MAX_Thermostat".
Open 192.168.4.1 in your browser.
click on configure WiFi.
select your WiFi network by clicking on the one that corresponds in the list of WiFi networks.
enter your WiFi password.
enter your Supla server.
enter your Email.
enter Supla device name "the name you want the module to have in your Supla server"
enter Step +/- "value for steps +/- for the thermostat temperature setting" allowed from 0.1 to 50.0
enter Hysteresis "allowed from 0.1 to 50.0"
to separate the decimals point is used, do not use the comma.
hit Save.
After a few seconds the module restarts with the established configuration.

Supla cloud seting:
channel 0 "power switch" = automatic/manual mode
channel 1 "power switch" = manual on/off "shows the state of the relay"
channel 2 "temperature" = shows the temperature read by max6675
channel 3 "temperature" = shows the temperature set for the thermostat
channel 4 "power switch" = increases or decreases the temperature set for the thermostat

in automatic mode "channel 0 on":
if the max6675 temperature falls below the "temperature set for the thermostat - hysteresis" the relay is activated. 
if the max6675 temperature exceeds the temperature set for the thermostat the relay is deactivated.

when toggle from automatic to manual mode "channel 0" the relay is always deactivated, this allows setting schedules in the Supla cloud.

in manual mode "channel 0 off":
manual on/off "channel 1" controls the relay regardless of temperatures.
