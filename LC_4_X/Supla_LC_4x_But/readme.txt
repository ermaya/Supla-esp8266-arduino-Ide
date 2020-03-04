Supla relay x4 lc technology
"compatible with x2 and x1 on models that have the 2 buttons S1 and S2"

Added push button for local control on Gpio 0
Connect the button between Gpio 0 and Gnd
It allows toggle states of the 4 relays as follows:
click - relay 1
double click - relay 2
triple click - relay 3
long click - relay 4

4-channel relay module that we can control from the Supla App
And a DS18B20 temperature channel in Gpio2 (needs pullup 4,7kohm)
The module must always be in mode2 (Red LED D7 on) and we shut not press S1 mode1 (Blue LED D5 on).
If it is in mode1 (Blue LED D5 on) we have to press and hold S2 while connecting it to the power supply (mode2 Red LED D7 on).
green LED D6 status: 
flashing = not connected or connecting 
on = connected (normal operation) 
off = WiFiConfig.
The S2 button to access WiFiConfig.
Status memory for power outages: if there is an interuption in the module power when restarting it will be in the state before the cut.

To access the WiFi configuration mode, press S2.
When in WiFi configuration mode, the device enters WiFi access point mode and the green LED D6 turns off.
- Log in to https://cloud.supla.org (registration is free) and Activate registration for new device.
- Connect to the WiFi called "Supla_LC4" from any device with wireless network and an Internet browser.
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