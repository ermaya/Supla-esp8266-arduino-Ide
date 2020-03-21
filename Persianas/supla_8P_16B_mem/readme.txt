With 8 blinds there will be no temperature (cannot waste time reading the temperature).

does not need external EEPROM memory (changed the memory algorithm so that it store the position only once per movement of each blind)

esp8266
Gpio16(D0) --Status Led (conect to +3.3v whit 220ohm)
Gpio 5(D1) --SCL  Mcp23017
Gpio 4(D2) --SDA  Mcp23017
Gpio 2(D4) --Mcp23017-Reset

the first Mcp23017 "address 0x20" (A0 + A1 + A2 to Gnd) controls the relays.

Mcp23017 (0x20)
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

the second Mcp23017 "address 0x24" (A0 + A1 to Gnd)(A2 to + 3.3v) is connected to the buttons.

Mcp23017 (0x24)
VCC - - +3.3
Gnd - - GND
SCL - - esp Gpio 5(D1)
SDA - - esp Gpio 4(D2)
RESET - esp Gpio 2(D4)
Gpio A0 --Button blind 1 Reveal
Gpio B0 --Button blind 1 Shut
Gpio A1 --Button blind 2 Reveal
Gpio B1 --Button blind 2 Shut
Gpio A2 --Button blind 3 Reveal
Gpio B2 --Button blind 3 Shut
Gpio A3 --Button blind 4 Reveal
Gpio B3 --Button blind 4 Shut
Gpio A4 --Button blind 5 Reveal
Gpio B4 --Button blind 5 Shut
Gpio A5 --Button blind 6 Reveal
Gpio B5 --Button blind 6 Shut
Gpio A6 --Button blind 7 Reveal
Gpio B6 --Button blind 7 Shut
Gpio A7 --Button blind 8 Reveal
Gpio B7 --Button blind 8 Shut