esp8266 + 2 x mcp23017

esp8266
Gpio16(D0) -- Led (turn on when on WiFiConfig)
Gpio 5(D1) -- SCL
Gpio 4(D2) -- SDA
Gpio 0(D3) -- WiFiConfig
Gpio 2(D4) -- Mcp23017 RESET


Mcp23017 -- Relay
in VCC -- +3.3
in Gnd-a0-a1-a2 --GND
SCL -- esp Gpio 5(D1)
SDA -- esp Gpio 4(D2)
RESET -- esp Gpio 2(D4)
Gpio A0 -- Out  1
Gpio A1 -- Out  2
Gpio A2 -- Out  3
Gpio A3 -- Out  4
Gpio A4 -- Out  5
Gpio A5 -- Out  6
Gpio A6 -- Out  7
Gpio A7 -- Out  8
Gpio B0 -- Out  9
Gpio B1 -- Out 10
Gpio B2 -- Out 11
Gpio B3 -- Out 12
Gpio B4 -- Out 13
Gpio B5 -- Out 14
Gpio B6 -- Out 15
Gpio B7 -- Out 16

Mcp23017 -- button
in VCC-a0 -- +3.3
in Gnd-a1-a2 --GND
SCL -- esp Gpio 5(D1)
SDA -- esp Gpio 4(D2)
RESET -- esp Gpio 2(D4)
Gpio A0 -- button  1
Gpio A1 -- button  2
Gpio A2 -- button  3
Gpio A3 -- button  4
Gpio A4 -- button  5
Gpio A5 -- button  6
Gpio A6 -- button  7
Gpio A7 -- button  8
Gpio B0 -- button  9
Gpio B1 -- button 10
Gpio B2 -- button 11
Gpio B3 -- button 12
Gpio B4 -- button 13
Gpio B5 -- button 14
Gpio B6 -- button 15
Gpio B7 -- button 16

added automatic delay memory for monostable channel from local push buttons.
at the moment that one of the channels is configured in the cloud as monostable (staircase timer, garage opening, etc.)
In the first activation from the App the module store the delay that has been set in the cloud.
from that moment the local button corresponding to that channel will act with the same delay.
for example:
In the Supla cloud we set channel 0 as a staircase timer with a time of 60 seconds.
Then we activate that channel once from the SuplaApp (or Supla cloud).
This saves the 60 second delay for channel 0 in the module.
if we press local button 0, channel 0 is activated for 60 seconds.


To enter or change the settings:
To enter the WiFi configuration mode, press and hold the button for at least 5 seconds.
When in Wi-Fi configuration mode, the device enters Wi-Fi access point mode and the status LED turns on.
- Log in to https://cloud.supla.org (registration is free) and Activate registration for new device.
- Connect to the WiFi called "Supla_16X16" from any device with wireless network and an Internet browser.
- Open the Page: http://192.168.4.1
- Tap Configure WiFi.
- On the configuration page
- Select the Wi-Fi network at the top by pressing the appropriate one and then enter the password.
- enter the data to:
suplaServer (svrX.supla.org),
Email (registration email in supla),
Supla Device Name (name with which it will be seen in the cloud),
- To finish click on Save to save the configuration data.

Firmware update through the OTA web browser - http: // XX: 81 / update
xx = Device IP. For example http://192.168.1.22:81/update
User: admin
Password: pass
