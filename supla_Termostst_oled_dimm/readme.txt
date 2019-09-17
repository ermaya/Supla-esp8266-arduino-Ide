


the screen automatically dims after 15 seconds and the first press on one of the buttons makes it bright again.

Gpio:
#define ONE_WIRE_BUS 13  //D7 --------------------oneWire Dallas Temperature -------requires 4k7 resistance
int relay_1 = 15;        //D8 ------------------- thermometer Auto/Man Led ----- active high ---optional
int relay_2 = 12;        //D6 ------------------- relay output ----- active high
int button_1 = 14;       //D5 ------------------- automatic / manual thermostat change  ---- WiFi config 10 seconds  --------
int button_2 = 16;       //D0 ------------------- manual on / off ---- requires 10k pullup resistance due to lack of internal
int button_3 = 5;        //D1 ------------------- thermostat temperature setting - 0.5º----
int button_4 = 4;        //D2 ------------------- thermostat temperature setting + 0.5º----

if any button is pressed while the screen is dimmed, it will glow and the following presses will acruate according to the button pressed.

After 15 seconds without pressing any button the screen dims.

After changing the temperature of the thermostat, the display reverses for a moment indicating that the new value has been stored.