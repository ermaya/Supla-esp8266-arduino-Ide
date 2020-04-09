soft compatible with Sonoff Touch T1 (1 Gang),
(2 Gang) and (3 Gang) + DS18B20 on Gpio 3.
status memory (on / off) in case of power failure.
configurable as bistable (light switch) or as monostable (stairs timer), if we select light switch in the cloud the touch works as on / off, if we select stairs timer in the cloud (after cycling on / off from the App ) the touchpad turns on for the same time as the App (it learns in time), to return to bistable mode (light switch) we only have to change in the cloud and cycle on / off from the App.

WiFiConfig = Gpio0 10 seconds (the first Touch, channel 0)
Led on = not connected-connecting.
led blinks = WiFiConfig.
Led off = OK

WiFi update = http://"IP":81/update
username and password = defined in WiFiConfig