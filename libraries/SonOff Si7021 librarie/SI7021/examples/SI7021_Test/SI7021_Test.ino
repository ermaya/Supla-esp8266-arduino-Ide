#include "SI7021.h"

DHT si7021;

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)");

  si7021.setup(5); // data pin 2
}

void loop()
{
  delay(3000);

  float humidity = si7021.getHumidity();
  float temperature = si7021.getTemperature();

  Serial.println(si7021.getModel());

  Serial.print(si7021.getStatusString());
  Serial.print("\t");
  Serial.print(humidity, 1);
  Serial.print("\t\t");
  Serial.print(temperature, 1);
  Serial.print("\t\t");
  Serial.println(si7021.toFahrenheit(temperature), 1);
}


