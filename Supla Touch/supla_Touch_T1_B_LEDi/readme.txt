soft compatible with Sonoff Touch T1 (1 Gang),
(2 Gang) and (3 Gang) + DS18B20 on Gpio 3 (RX)
state memory (on/off) in case of power failure.
configurable as bistable (light switch) or as monostable (stairs timer), if we select light switch in the cloud the touch works as on/off, if we select stairs timer in the cloud (after cycling on/off from the App ) the touchpad turns on for the same time as the App (it learns in time), to return to bistable mode (light switch) we only have to change in the cloud and cycle on/off from the App.
full offline functionality.

WiFiConfig = Gpio0 10 seconds (the first Touch, channel 0)
Led on = not connected-connecting.
led blinks = WiFiConfig.
Led off = OK

To bring the device into configuration mode,hold the first touch for at least 10 seconds
When in configuration mode, the device goes into Access Point mode.

In order to enter or change the settings, you need to:

- Sign in at https://cloud.supla.org (registration is free of charge)
- Connect to WiFi called „Supla_Touch” from any computer with a wireless network card and Internet browser.
- access: http://192.168.4.1
- Enter user name and password to the WiFi through which the device will get Internet access.
- Enter Server address, Location ID and Location Password, which will be provided once you sign in at cloud.supla.org
- Supla Device Name (name with which it will be seen in the cloud)
- username and password for OTA Firmware update.
read-only field for Supla Last State.
- To finish click on Save to save the configuration data.

WiFi Firmware update (OTA) = http://"IP":81/update
username and password = defined in WiFiConfig

