/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef _max6675_k_h
#define _max6675_k_h

#if defined(ESP8266)
#include <pgmspace.h>
//#define _delay_ms(ms) delayMicroseconds((ms) * 1000)
#endif

//#include <util/delay.h>
#ifdef avr
#include <util/delay.h>
#endif

#include <stdlib.h>
#include "supla/channel.h"
#include "supla/sensor/thermometer.h"


namespace Supla {
namespace Sensor {
class MAX6675_K : public Thermometer {
 public:
  MAX6675_K(uint8_t pin_CLK,uint8_t pin_CS,uint8_t pin_DO) {
    sclk = pin_CLK;
    cs = pin_CS;
	miso = pin_DO;
  }

  double getValue() {
	  
	    double v;

      digitalWrite(cs, LOW);
      delay(1);

      v = spiread();
      v <<= 8;
      v |= spiread();

      digitalWrite(cs, HIGH);

      if (v & 0x4) {
      	// uh oh, no thermocouple attached!
      	//return NAN; 
      	return -275;
      }

      v >>= 3;

      return v*0.25;	  
	  
  }


  void onInit() {
    pinMode(cs, OUTPUT);
    pinMode(sclk, OUTPUT); 
    pinMode(miso, INPUT);

    digitalWrite(cs, HIGH);  
  
    channel.setNewValue(getValue());
  }

  byte spiread(void) { 
    int i;
    byte d = 0;

    for (i=7; i>=0; i--)
    {
    	digitalWrite(sclk, LOW);
    	delay(1);
    	if (digitalRead(miso)) {
      	d |= (1 << i);
    	}

    digitalWrite(sclk, HIGH);
    delay(1);
  }

  return d;
}

 protected:
  int8_t sclk;
  int8_t cs;
  int8_t miso;

};
};  // namespace Sensor
};  // namespace Supla

#endif
