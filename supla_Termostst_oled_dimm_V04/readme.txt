
thermostat with Oled display, control from the Supla App and buttons for local control.

Gpio:
#define ONE_WIRE_BUS 13  //D7 --------------------oneWire Dallas Temperature -------requires 4k7 resistance
int relay_1 = 15;        //D8 ------------------- thermometer Auto/Man Led ----- active high ---optional
int relay_2 = 12;        //D6 ------------------- relay output ----- active high
int button_1 = 14;       //D5 ------------------- automatic / manual thermostat change  ---- WiFi config 10 seconds  --------
int button_2 = 16;       //D0 ------------------- manual on / off ---- requires 10k pullup resistance due to lack of internal
int button_3 = 5;        //D1 ------------------- thermostat temperature setting - 0.5º----
int button_4 = 4;        //D2 ------------------- thermostat temperature setting + 0.5º----

the screen automatically dims after 15 seconds and the first press on one of the buttons makes it bright again.

if any button is pressed while the screen is dimmed, it will glow and the following presses will acruate according to the button pressed.

After 15 seconds without pressing any button the screen dims.

After changing the temperature of the thermostat, the display reverses for a moment indicating that the new value has been stored.

relay off when switching from auto to manual mode to allow schedule.

in case of a connection error with the cloud, it shows Exx instead of the supla logo, with xx being the error number.

STATUS_ALREADY_INITIALIZED     2
STATUS_CB_NOT_ASSIGNED         3
STATUS_INVALID_GUID            4
STATUS_UNKNOWN_SERVER_ADDRESS  5
STATUS_UNKNOWN_LOCATION_ID     6
STATUS_INITIALIZED             7
STATUS_CHANNEL_LIMIT_EXCEEDED  8
STATUS_DISCONNECTED            9
STATUS_REGISTER_IN_PROGRESS    10
STATUS_ITERATE_FAIL            11
STATUS_PROTOCOL_VERSION_ERROR  12
STATUS_BAD_CREDENTIALS         13
STATUS_TEMPORARILY_UNAVAILABLE 14
STATUS_LOCATION_CONFLICT       15
STATUS_CHANNEL_CONFLICT        16
STATUS_REGISTERED_AND_READY    17  ------ connection ok display Supla logo -----
STATUS_DEVICE_IS_DISABLED      18
STATUS_LOCATION_IS_DISABLED    19
STATUS_DEVICE_LIMIT_EXCEEDED   20