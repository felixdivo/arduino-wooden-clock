#include "Arduino.h"

#include "Time.hpp"
#include "TimeProvider.hpp"

/** 
* interrupt number of the pin where the DCF antenna result is connected 
* see in general: https://www.arduino.cc/en/Reference/AttachInterrupt 
* On the Mega 2560 these pins can be used: 2, 3, 18, 19, 20, 21
*/
#define DCF_INTERRUPT_PIN 2

/** 
 * must be the same as in the connected computers serial monitor
 */
#define SERIAL_BITRATE 19200 // bits per second

/**
 * Gets called once when the board starts.
 */
void setup() {
  
  // set the transmission bitrate
  Serial.begin(SERIAL_BITRATE);
  
  // setup the DCF77 receiver
  globalTimeProvider = TimeProvider(DCF_INTERRUPT_PIN, onTimeProviderResult);

  // start listening indefinitely
  globalTimeProvider.start();
}

/**
* Gets called in a loop.
*/
void loop() {
  // simply wait for some milliseconds
  delay(10);
}

/**
* Callback for the TimeProvider.
*/

void onTimeProviderResult(Time now) {
  Serial.println("Now is: " + now.toString());
  // TODO use http://playground.arduino.cc/Code/Time
}
