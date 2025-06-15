#include <G1OJS_Tiny_Si5351_CLK0.h>

#include <Wire.h>
#include "Arduino.h"

void setup()
{
  Wire.begin();
  delay(1000);
  G1OJS_Tiny_Si5351_CLK0 DDS;
  DDS.set_freq_Hz((uint32_t)100000000);
}


void loop() {
  
  
}



