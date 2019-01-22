

PORTS SETTINGS:

GPIO5 /D1 	Switching_Relay_Button Channel 0- On the port must be pull-up. Active - connect to GND

GPIO4 /D2 	Switching_Relay_Button Channel 1- On the port must be pull-up. Active - connect to GND

GPIO2 /D4 	Switching_Relay_Button Channel 2- On the port must be pull-up. Active - connect to GND

GPI14 /D5 	Switching_Relay_Button Channel 3- On the port must be pull-up. Active - connect to GND

GPIO12/D6	Switching_Relay_Button Channel 4- On the port must be pull-up. Active - connect to GND

GPIO13/D7	Switching_Relay_Button Channel 5- On the port must be pull-up. Active - connect to GND

GPIO3 /RX	Switching_Relay_Button Channel 6- On the port must be pull-up. Active - connect to GND

GPIO1 /TX	Switching_Relay_Button Channel 7- On the port must be pull-up. Active - connect to GND

GPIO15/D8	shift registers, data pin 14 on 74595

GPIO16/D0	shift registers, clock pin 11 on 74595

GPIO0 /D3	shift registers, latch pin 12 on 74595


Initial parameters for "ESP Falsh Download Tool":

CreystalFreq 	26M
SPI SPEED 	40 MHz
SPI MODE 	QIO
BAUDRATE 	11520
FLASH SIZE 	32Mbit (4MByte)

supla_595_8x8_10K-ntc.bin ------------> 0x00000





// CFG MODE - SWITCH

To bring the device into configuration mode, in this case press Reset_Button  2 times
When in configuration mode, the device goes into Access Point mode.

In order to enter or change the settings, you need to:

- Sign in at https://cloud.supla.org (registration is free of charge)
- Connect to WiFi called (Supla 595 x8) from any computer with a wireless network card and Internet browser.
- Open access page: http://192.168.4.1
- Enter user name and password to the WiFi through which the device will get Internet access.
- Enter Server address, Location ID and Location Password, which will be provided once you sign in at cloud.supla.org
