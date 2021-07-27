
in the Supla App:
the first dimmer set the level of the outputs for when they are on between 50% and 100%.
the second dimmer set the level of the outputs for when they are off between 0% and 50%.

two stair timer channels, one for On / Off from 0 to 15 and the other for 15 to 0.
set the power-on time in the Supla cloud.

One light switch channel to turn all the channels On / Off at the same time.
allows to leave On.


For testing you can connect Led diodes directly to the outputs and GND.

Gpio4  "D2" = SDA -- PCA9685
Gpio5  "D1" = SCL -- PCA9685
Gpio2  "D4" = LED_PIN 
Gpio0  "D3" = button_0  
Gpio14 "D5" = button_1  
Gpio16 "D0" = relay_0 
Gpio15 "D8" = relay_1 
Gpio13 "D7" = relay_2

ESP Buildin Led:
winking 0.5 seconds = not connected / connecting.
on = WiFiConfig.
off = connected / Ok.

button 0:
short press = toggle On / Off from 0 to 15 "climb stairs".
long press = togle all On / OFF.
hold 10 seconds = WiFiConfig.

button 1:
short press = toggle On / Off from 15 to 0 "go down stairs".
long press = togle all On / OFF.

the three relay outputs "relay_0, relay_1, relay_2":
they show the status of the three channels as in the Supla App.
Just for testing, I plan to move to virtual channels and release those three Gpios.

in WiFiConfig there is an additional parameter "Fade Time".
this is the time it takes to turn on each channel in milliseconds, 500 is a good value to start with.
500 * 16 = 8 seconds.  

