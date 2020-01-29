Supla Pir Pushover Mini Alarm
It consists of a Pir motion sensor which is activated / deactivated from the Supla App,
If the sensor is activated and detects movement, it sends the message that we have defined in WiFiconfig to the Pushover service.
Connections:
Gpio 0 - D3 WiFiconfig
Gpio 2 - D4 Led
Gpio12 - D6 Pir Sensor

To enter or change the settings:
To enter the WiFi configuration mode, press and hold the button for at least 5 seconds.
When in Wi-Fi configuration mode, the device enters Wi-Fi access point mode and the status LED turns on.
- Log in to https://cloud.supla.org (registration is free) and Activate registration for new device.
- Connect to the WiFi called "Supla_Pir_Push" from any device with wireless network and an Internet browser.
- Open the Page: http://192.168.4.1
- Tap Configure WiFi.

- On the configuration page
- Select the Wi-Fi network at the top by pressing the appropriate one and then enter the password.
- enter the data to:
suplaServer (svrX.supla.org),
Email (registration email in supla),
Supla Device Name (name with which it will be seen in the cloud),
App Token (Pushover Api Token)
User Token (Pushover User Token)
Alarm Message (we write the text of the message) 
select Sound (Pushover notification sound)
- To finish click on Save to save the configuration data.

Firmware update through the OTA web browser - http: // XX: 81 / update
xx = Device IP. For example http://192.168.1.22:81/update
User: admin
Password: pass