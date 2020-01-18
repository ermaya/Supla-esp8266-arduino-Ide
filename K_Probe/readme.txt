PORT SETTINGS:

  CFG BUTTON = GPIO  0 // D3
  status_led = GPIO  2 // D4
  thermo DO =  GPIO 14 // D5
  thermo CS =  GPIO 12 // D6
  thermo CLK = GPIO 13 // D7	

To bring the device into configuration mode, press and hold button for at least 10 sec.
When in configuration mode, the device goes into Access Point mode and status led goes On.

In order to enter or change the settings, you need to:

- Connect to WiFi called „K_Probe” from any computer with a wireless network card and Internet browser.
- Open access page: http://192.168.4.1
on the configuration page enter the data for "suplaServer, Email and Supla Device Name (name with which it will be seen in the cloud)"


OTA update http://XX:81/update -- xx = device ip. for example http://192.168.1.22:81/update
user: admin
password: pass