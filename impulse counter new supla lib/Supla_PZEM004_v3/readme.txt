Pulse counter based on SoftVer 2.3.0. (Supla Arduino develop) By klew.

/* Use software serial for the PZEM
 * Pin 5 Rx (Connects to the Tx pin on the PZEM)
 * Pin 4 Tx (Connects to the Rx pin on the PZEM)
*/
PZEM004Tv30 pzem(5, 4); //change based on your hardware configuration

wisiconfig Gpio 0 //D3

To access the Wi-Fi configuration, press Gpio 0 for at least 5 seconds.
in the configuration page enter the data for "suplaServer, Email and Supla Device Name (name with which it will be seen in the cloud)"
added clear counter in wificonfig.
Added read-only field with the latest Supla connection state.

https://raw.githubusercontent.com/ermaya/Supla-esp8266-arduino-Ide/master/impulse%20counter%20new%20supla%20lib/Supla_impulse_Th16/wificonfig.png

[img]https://raw.githubusercontent.com/ermaya/Supla-esp8266-arduino-Ide/master/impulse%20counter%20new%20supla%20lib/Supla_PZEM004_s_s/PZEM2.jpg[/img]

[img]https://raw.githubusercontent.com/ermaya/Supla-esp8266-arduino-Ide/master/impulse%20counter%20new%20supla%20lib/Supla_PZEM004_s_s/PZEM1.jpg[/img]