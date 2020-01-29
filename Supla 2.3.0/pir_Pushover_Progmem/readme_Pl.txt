Supla Pir Pushover Mini Alarm
Składa się z czujnika ruchu Pir, który jest aktywowany / dezaktywowany z aplikacji Supla,
Jeśli czujnik jest aktywowany i wykrywa ruch, wysyła komunikat zdefiniowany w WiFiconfig do usługi Pushover.
Connections:
Gpio 0 - D3 WiFiconfig
Gpio 2 - D4 Led
Gpio12 - D6 Pir Sensor



Aby wprowadzić lub zmienić ustawienia:
Aby przejść do trybu konfiguracji Wi-Fi, naciśnij i przytrzymaj przycisk przez co najmniej 5 sekund.
W trybie konfiguracji Wi-Fi urządzenie przechodzi w tryb punktu dostępu Wi-Fi i włącza się dioda LED stanu.
- Zaloguj się na https://cloud.supla.org (rejestracja jest bezpłatna) i Aktywuj rejestrację dla nowego urządzenia.
- Połącz się z Wi-Fi o nazwie „Supla_Pir_Push” z dowolnego urządzenia z siecią bezprzewodową i przeglądarką internetową.
- Otwórz stronę: http://192.168.4.1
- Stuknij Skonfiguruj WiFi.

[img]https://raw.githubusercontent.com/ermaya/Supla-esp8266-arduino-Ide/master/Supla%202.3.0/pir_Pushover_2/wificonfig3.png[/img]

- Na stronie konfiguracji
- Wybierz sieć Wi-Fi u góry, naciskając odpowiednią, a następnie wprowadź hasło.
- wprowadź dane do:
suplaServer (svrX.supla.org),
E-mail (e-mail rejestracyjny w supla),
Nazwa urządzenia Supla (nazwa, pod którą będzie widoczna w chmurze),
App Token  (Pushover Api Token)
User Token (token użytkownika Pushover)
Komunikat alarmowy (piszemy tekst wiadomości)
select Sound (dźwięk powiadomienia pushover)
- Aby zakończyć, kliknij Save, aby zapisać dane konfiguracji.


Firmware update through the OTA web browser - http: // XX: 81 / update
xx = Device IP. For example http://192.168.1.22:81/update
User: admin
Password: pass