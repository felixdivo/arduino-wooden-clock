#include "Time.h" // custom Time struct

const bool LOGGING = true;

// pin where the DCF antenna result is connected
const unsigned int DCF_DATA = 50;

void setup() {

  // set the transmission rate,
  // must be the same as in the PC's/IDE's serial monitor
  Serial.begin(19200);

  setupDCF77(DCF_DATA, LOGGING, LOGGING);
}


void loop() {
  
  loopDCF77();
  
  // wait for X ms
  delay(5);
}

