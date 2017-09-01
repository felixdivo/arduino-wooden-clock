#include "Time.h" // custom Time struct
// #include DCF_77 is done automatically

// interrupt number of the pin where the DCF antenna result is connected
// see in general: https://www.arduino.cc/en/Reference/AttachInterrupt
// Interrupt #0 equals pin #2 on the mega, see the shematics of the official MEGA: https://goo.gl/LP6X1W
const int DCF_INTERRUPT_PIN = 2;

void setup() {

  // set the transmission rate, in bit/s
  // must be the same as in the PC's/IDE's serial monitor
  Serial.begin(19200);

  startDCF77(DCF_INTERRUPT_PIN);
}


void loop() {
  // wait for X ms
  delay(5);
}

void onDCF77Result(Time now) {
  Serial.println("Now = " + printTime(now));
}

void testMethod(void* param){
  param();
}

void testParam(){
  Serial.println("####################################### testParam() called!");
}

