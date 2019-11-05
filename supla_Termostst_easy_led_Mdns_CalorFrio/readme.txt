added the selection between heating or cooling in wificonfig

some stability improvements

added mDNS  HTTPUpdateServer ready! Open http://XXXX.local:81/update
where XXXX is what your set in config portal "xxxx (DHCP name)"
example in the config portal "xxxx (DHCP name)" you type Term1, 
in the browser you have to look for the page
  http: //Term1.local: 81 / update

This version of the Supla thermostat (Without Oled) flashes the Led every second if it is not connected to the Wifi.
The Led will be on during WifiConfig.
Led off in normal state.
and blinks x times according to the Supla cloud connection error with a 3 second pause between each error code.
For example, if status is BAD_CREDENTIALS, the Led flashes 13 times.

According to the following list:
STATUS_REGISTER_IN_PROGRESS    1
STATUS_ALREADY_INITIALIZED     2
STATUS_CB_NOT_ASSIGNED         3
STATUS_INVALID_GUID            4
STATUS_UNKNOWN_SERVER_ADDRESS  5
STATUS_UNKNOWN_LOCATION_ID     6
STATUS_INITIALIZED             7
STATUS_CHANNEL_LIMIT_EXCEEDED  8
STATUS_DISCONNECTED            9
STATUS_ITERATE_FAIL            11
STATUS_PROTOCOL_VERSION_ERROR  12
STATUS_BAD_CREDENTIALS         13
STATUS_TEMPORARILY_UNAVAILABLE 14
STATUS_LOCATION_CONFLICT       15
STATUS_CHANNEL_CONFLICT        16
STATUS_REGISTERED_AND_READY   ------ connection ok Led Off -----
STATUS_DEVICE_IS_DISABLED      18
STATUS_LOCATION_IS_DISABLED    19
STATUS_DEVICE_LIMIT_EXCEEDED   20

in total 5 channels
channel 0 = automatic / manual thermostat
channel 1 = manual relay on / off
channel 2 = to the left the temperature decreases by 0.5º the right increases by 0.5º
channel 3 = temperature DS18B20
channel 4 = thermostat temperature

temperature and hysterysis are adjusted in wificonfig
temperature in x10 format (22º is written 220 in the configuration)
It can be changed from the app in 0.5º steps and will be stored
hysterisis in x10 format (+/- 1 is written 10 in the configuration)

Gpio
#define ONE_WIRE_BUS 2   //--------------------oneWire Dallas Temperature ----------
int relay_1 = 12;        //------------------- automatic / manual Led ----- active high
int relay_2 = 13;        //------------------- relay output ----- active high
int relay_3 = 120;       //------------------- no Gpio -----
int button_1 = 4;        //------------------- automatic / manual thermostat change  ---- WiFi config 10 seconds  --------
int button_2 = 5;        //------------------- manual on / off ----
#define status_led 16    //------------------- status Led -----