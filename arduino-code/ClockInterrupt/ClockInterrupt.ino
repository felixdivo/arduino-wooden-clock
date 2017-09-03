
#include "Time.h" // custom Time struct
// #include DCF_77 is done automatically

/** 
 * interrupt number of the pin where the DCF antenna result is connected
 * see in general: https://www.arduino.cc/en/Reference/AttachInterrupt
 * Interrupt #0 equals pin #2 on the Arduino Mega 2560, see this link: 
 * https://goo.gl/LP6X1W 
 */
const unsigned int DCF_INTERRUPT_PIN = 2;

/** 
 * must be the same as in the connected compiters serial monitor
 */
const unsigned int SERIAL_BITRATE = 19200; // bits per second

/**
 * Gets called once when the board starts.
 */
void setup() {

  // set the transmission bitrate
  Serial.begin(SERIAL_BITRATE);

  // setup the DCF77 receiver
  setupDCF77(DCF_INTERRUPT_PIN);
}

/**
 * Gets called in a loop.
 */
void loop() {
  // simply wait for some milliseconds
  delay(10);
}

/**
 * Callback for DCF_77.ino
 */
void onDCF77Result(Time now) {
  Serial.println("Now is: " + printTime(now));
}
