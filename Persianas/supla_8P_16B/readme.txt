8 blinds 16 buttons

Manual control (push buttons) fully operational offline.

saves opening time, closing time and position, it does not need recalibration after an interruption of the electrical supply.

if the blinds are moved during disconnection from the Supla cloud (lack of internet), the position is updated in the App (cloud) when reconnecting Supla cloud.

To bring the device into configuration mode, Gpio 0(D3) -- WiFiConfig (to ground for at least 5 seconds)
When in configuration mode, the device goes into Access Point mode.

In order to enter or change the settings, you need to:

- Sign in at https://cloud.supla.org (registration is free of charge)
- Connect to WiFi called „Supla 8P” from any computer with a wireless network card and Internet browser.
- Open access page: http://192.168.4.1
- Enter user name and password to the WiFi through which the device will get Internet access.
- Enter Server address, Location ID and Location Password, which will be provided once you sign in at cloud.supla.org
- Supla Device Name (name with which it will be seen in the cloud)
- username and password for OTA Firmware update.
read-only field for Supla Last State.
- To finish click on Save to save the configuration data.

Firmware update through the OTA web browser - http: // XX: 81 / update
xx = Device IP. For example http://192.168.1.22:81/update
User & Password -- those defined in WiFiConfig.

esp8266
Gpio16(D0) --Status Led (conect to +3.3v whit 100ohm)
Gpio 0(D3) -- WiFiConfig (to ground for at least 5 seconds)
Gpio 5(D1) --SCL  Mcp23017
Gpio 4(D2) --SDA  Mcp23017
Gpio 2(D4) --Mcp23017-Reset

the first Mcp23017 "address 0x20" (A0 + A1 + A2 to Gnd) controls the relays.

Mcp23017 (0x20)
A0 - - GND
A1 - - GND
A2 - - GND
VCC - - +3.3
Gnd - - GND
SCL - - esp Gpio 5(D1)
SDA - - esp Gpio 4(D2)
RESET - esp Gpio 2(D4)

Gpio A0 --Relay blind 1 Shut
Gpio B0 --Relay blind 1 Reveal
Gpio A1 --Relay blind 2 Shut
Gpio B1 --Relay blind 2 Reveal
Gpio A2 --Relay blind 3 Shut
Gpio B2 --Relay blind 3 Reveal
Gpio A3 --Relay blind 4 Shut
Gpio B3 --Relay blind 4 Reveal
Gpio A4 --Relay blind 5 Shut
Gpio B4 --Relay blind 5 Reveal
Gpio A5 --Relay blind 6 Shut
Gpio B5 --Relay blind 6 Reveal
Gpio A6 --Relay blind 7 Shut
Gpio B6 --Relay blind 7 Reveal
Gpio A7 --Relay blind 8 Shut
Gpio B7 --Relay blind 8 Reveal

the second Mcp23017 "address 0x21" (A0 to + 3.3v)(A1 + A2 to Gnd) is connected to the buttons.

Mcp23017 (0x21)
A0 - - +3.3
A1 - - GND
A2 - - GND
VCC - - +3.3
Gnd - - GND
SCL - - esp Gpio 5(D1)
SDA - - esp Gpio 4(D2)
RESET - esp Gpio 2(D4)

Gpio A0 --Button blind 1 Shut
Gpio B0 --Button blind 1 Reveal
Gpio A1 --Button blind 2 Shut
Gpio B1 --Button blind 2 Reveal
Gpio A2 --Button blind 3 Shut
Gpio B2 --Button blind 3 Reveal
Gpio A3 --Button blind 4 Shut
Gpio B3 --Button blind 4 Reveal
Gpio A4 --Button blind 5 Shut
Gpio B4 --Button blind 5 Reveal
Gpio A5 --Button blind 6 Shut
Gpio B5 --Button blind 6 Reveal
Gpio A6 --Button blind 7 Shut
Gpio B6 --Button blind 7 Reveal
Gpio A7 --Button blind 8 Shut
Gpio B7 --Button blind 8 Reveal