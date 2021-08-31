RF remote controls have different identification codes so it is necessary to pair them with the LC11.
this is done automatically with the first received RF code.
Press a couple of buttons on the RF remote control after uploading the firmware to the LC11, if you wait a long time depending on where you live it is possible that it receives some signal from a "433Mhz" automatic door and saves it as the reference with which you would have to perform the next operation.

if later it is necessary to reactivate the pairing:
access WiFiConfig by pressing the button for at least 10 seconds.
in the "suppla server" field write "get_new_rf" and press Save.
WiFiConfig data does not change, the LC11 reboots and RF pairing is activated again.

you have to write "get_new_rf" as shown below, without blank spaces.

get_new_rf


the button "Gpio 0 to GND" currently has the following functions:
click = TOGGLE_RGB.
doubleClick = TOGGLE_WW.
tripleClick = TOGGLE_CW.
10 seconds = WiFiConfig.