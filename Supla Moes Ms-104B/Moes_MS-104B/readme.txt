soft for Moes MS-104B with the inputs configured as a counter (correct reading of the A/C pulse inputs).

GPIO #  Component
GPIO02  Button1
GPIO04  Buzzer
GPIO12  Counter2
GPIO13  Counter1
GPIO14  Relay1
GPIO15  Relay2

configurable as bistable (light switch) or as monostable (stairs timer),
if we select light switch in the cloud the input works as on/off,
if we select stairs timer in the cloud (after cycling on/off from the App ) the input turns on for the same time as the App (it learns in time),
to return to bistable mode (light switch) we only have to change in the cloud and cycle on/off from the App.
full offline functionality.

WiFiConfig = Gpio2 Button 10 seconds 


To bring the device into configuration mode,hold the Button for at least 10 seconds
When in configuration mode, the device goes into Access Point mode.

In order to enter or change the settings, you need to:

- Sign in at https://cloud.supla.org (registration is free of charge)
- Connect to WiFi called „Moes_MS-104B” from any computer with a wireless network card and Internet browser.
- access: http://192.168.4.1
- Enter user name and password to the WiFi through which the device will get Internet access.
- Enter Server address, Location ID and Location Password, which will be provided once you sign in at cloud.supla.org
- Supla Device Name (name with which it will be seen in the cloud)
- username and password for OTA Firmware update.
read-only field for Supla Last State.
- To finish click on Save to save the configuration data.


WiFi Firmware update (OTA) = http://"IP":81/update
username and password = defined in WiFiConfig