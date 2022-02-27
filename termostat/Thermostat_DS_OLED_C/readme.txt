Pinout:
Gpio 0 -- D3 = SDA "Oled".
Gpio 2 -- D4 = SCL "Oled".
Gpio 4 -- D2 = DS18B20 "4.7 Kohm to +3.3V"
Gpio 5 -- D1 = Relay "Set Relay HIGH/LOW level in (relayOnLevel)".
Gpio 14 -- D5 = Button "Rotary encoder SW".
Gpio 12 -- D6 = Rotary encoder A.
Gpio 13 -- D7 = Rotary encoder B.

Local control:
Botton ON_CLICK_1 = toggle auto/manual mode.
Botton ON_CLICK_2 = toggle Relay on/off "disabled if (Protected = true)".
Botton Hold 10 seconds = WiFiConfig.
rotary encoder = increases/decreases Thermostat preset temperature in steps set in WiFiConfig "0.1 to 10.0".

Supla cloud channels:
channel 1 = automatic/manual mode "ON = automatic, Off = manual".
channel 2 = relay state, manual control.
channel 3 = room temperature "DS18B20".
channel 4 = thermostat temperature.
channel 5 = increases/decreases thermostat temperature.


