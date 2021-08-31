-------------------------------------------------------------
new software written from scratch.
using the current SuplaDevice library.
RF decoding based on the Tasmota code.

RF remote control functions are:
   +--------+--------+--------+
   |   ON   | Toggle |   OFF  |
   +--------+--------+--------+
   |BRI_ALL |BRI_RGB | BRI_ W |
   +--------+--------+--------+
   | DIM_ALL| DIM_RGB| DIM_W  |
   +--------+--------+--------+
   |  RED   | GREEN  |  BLUE  |
   +--------+--------+--------+
   | ORANGE | LT GRN | LT BLUE|
   +--------+--------+--------+
   | AMBER  |  CYAN  | PURPLE |
   +--------+--------+--------+
   | YELLOW |  PINK  | WHITE  |
   +--------+--------+--------+


RF remote controls have different identification codes so it is necessary to pair them with the LC10.
this is done automatically with the first received RF code.
if later it is necessary to reactivate the pairing:
access WiFiConfig by pressing the button for at least 10 seconds.
in the "suppla server" field write "get_new_rf" and press Save.
WiFiConfig data does not change, the LC10 reboots and RF pairing is activated again.

you have to write "get_new_rf" as shown below, without blank spaces.

get_new_rf


the button "Gpio0 to GND" has the following functions:
click = TOGGLE_RGB.
doubleClick = TOGGLE_W.
10 seconds = WiFiConfig.

