
// Config:
const bool LOGGING_DCF77 = true; // if true, logs basic information to serial out
const bool LOGGING_DCF77_LED = true; // if true, displays the receiver date on LOGGING_DCF77_LED_PIN
const bool LOGGING_DCF77_DATA = false; // if true, logs the binary representation of the signal
const bool LOGGING_DCF77_DATA_RAW = false; // if true, logs the elapsed milliseconds each time the signal changes

const bool DISCARD_ON_PARITY_PROBLEM = true; // if true, discards a transmission if the parity bit was incorrect

const unsigned int LOGGING_DCF77_LED_PIN = LED_BUILTIN; // the pin to send the LED signal blinking to
volatile unsigned int INTERRUPT_PIN; // the interrupt pin of the time signal receiver

/**
 * Sets up the DCF77 receiver and start listening for time signals. 
 * If a signal is found, the (global) function onDCF77Result(Time now) is called. 
 * This algorithm ignores leap seconds.
 */
void setupDCF77(int interrupt_pin) {

  // save the interrupt pin
  INTERRUPT_PIN = interrupt_pin;
  pinMode(INTERRUPT_PIN, INPUT);

  // init time
  calculateTimeDelta();

  if (LOGGING_DCF77) Serial.println("registering the Interrupt Service Routine");

  // enable interrupts
  interrupts();

  /* Inside the attached function, delay() won't work and the value returned by
     millis() will not increment. Serial data received while in the function may
     be lost. You should declare as volatile any variables that you modify within
     the attached function. See the section on ISRs below for more information.
     See: https://www.arduino.cc/en/Reference/AttachInterrupt */
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), interruptReceiver, CHANGE);
}

/**
 * Stops listening for DCF77 time signals. 
 */
void stopDCF77() {
  detachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN));
  next_index = -1;
}

/**
 * Is called on a CHANGE event on the INTERRUPT_PIN.
 */
void interruptReceiver() {

  // read the HIGH or LOW state
  byte current_data = digitalRead(INTERRUPT_PIN);
  unsigned int time_delta = calculateTimeDelta();

  // logging
  if (LOGGING_DCF77) {
    if (current_data == HIGH) {
      if (LOGGING_DCF77_DATA_RAW) Serial.println("HIGH: " + String(time_delta, DEC));
      if (LOGGING_DCF77_LED) digitalWrite(LOGGING_DCF77_LED_PIN, HIGH);
    } else { /* c... == LOW */
      if (LOGGING_DCF77_DATA_RAW) Serial.println("LOW: "  + String(time_delta, DEC));
      if (LOGGING_DCF77_LED) digitalWrite(LOGGING_DCF77_LED_PIN, LOW);
    }
  }

  // skip junk signals
  if (isWithinRange(time_delta, 0, 50)) return;

  // extract information
  if (current_data == HIGH) {
    // check if this was the one second pause; the start of a new transmission
    if      (isWithinRange(time_delta, 1000, 2000)) startNewMinute();
  } else { /* current_data == LOW */
    if      (isWithinRange(time_delta, 80,   195))  writeNextBit(0);
    else if (isWithinRange(time_delta, 196,  320))  writeNextBit(1);
  }

}

// used by calculateTimeDelta()
volatile unsigned long previous_time = 0;

/**
 * Calculate the time delta between now and the previous 
 * invocation in milliseconds.
 */
unsigned int calculateTimeDelta() {

  /* use micros() since millis() wont't work inside interrupts
     See: https://www.arduino.cc/en/Reference/AttachInterrupt */
  unsigned long current_time = micros() / 1000; // convert to milliseconds
  unsigned int time_delta = current_time - previous_time;

  // save new values for the next time
  previous_time = current_time;

  return time_delta;
}

// buffer for currently receiving data
const byte buffer[59];
volatile int next_index = -1; // -1 = paused

/**
 * Called at the start of a new minute/transmission.
 */
void startNewMinute() {
  if (LOGGING_DCF77) Serial.println("witnessed the start of a new minute, waiting ...");
  
  // set to start
  next_index = 0;
}

/**
 * Gets called at the end of minute/transmission. Parses the received data 
 * and fires the callback.
 */
void finishMinute() {

  // make sure it is only evaluated once
  next_index = -1;

  // see https://de.wikipedia.org/wiki/DCF77#Zeitinformation
  // or  https://en.wikipedia.org/wiki/DCF77#Time_code_interpretation

  if (LOGGING_DCF77) Serial.println("witnessed complete time signal, now parsing");

  // check parity bits
  if (!isEvenParity(buffer, 21, 28)) {
    if (LOGGING_DCF77) Serial.println("parity problem (minutes: bit index 21-28)");
    if (DISCARD_ON_PARITY_PROBLEM) return;
  }
  if (!isEvenParity(buffer, 29, 35)) {
    if (LOGGING_DCF77) Serial.println("parity problem (hours: bit index 29-35)");
    if (DISCARD_ON_PARITY_PROBLEM) return;
  }
  if (!isEvenParity(buffer, 36, 58)) {
    if (LOGGING_DCF77) Serial.println("parity problem (date: bit index 36-58)");
    if (DISCARD_ON_PARITY_PROBLEM) return;
  }

  // add up the received data
  const Time now;

  // set the seconds field to zero, because the signal
  // only has a resolution of one minute
  now.second = 0;

  if (buffer[21]) now.minute += 1;
  if (buffer[22]) now.minute += 2;
  if (buffer[23]) now.minute += 4;
  if (buffer[24]) now.minute += 8;
  if (buffer[25]) now.minute += 10;
  if (buffer[26]) now.minute += 20;
  if (buffer[27]) now.minute += 40;

  // 28 == parity minute, checked previously

  if (buffer[29]) now.hour += 1;
  if (buffer[30]) now.hour += 2;
  if (buffer[31]) now.hour += 4;
  if (buffer[32]) now.hour += 8;
  if (buffer[33]) now.hour += 10;
  if (buffer[34]) now.hour += 20;

  // 35 == parity hour, checked previously

  if (buffer[36]) now.day += 1;
  if (buffer[37]) now.day += 2;
  if (buffer[38]) now.day += 4;
  if (buffer[39]) now.day += 8;
  if (buffer[40]) now.day += 10;
  if (buffer[41]) now.day += 20;

  if (buffer[42]) now.day_of_week += 1;
  if (buffer[43]) now.day_of_week += 2;
  if (buffer[44]) now.day_of_week += 4;

  if (buffer[45]) now.month += 1;
  if (buffer[46]) now.month += 2;
  if (buffer[47]) now.month += 4;
  if (buffer[48]) now.month += 8;
  if (buffer[49]) now.month += 10;

  if (buffer[50]) now.year += 1;
  if (buffer[51]) now.year += 2;
  if (buffer[52]) now.year += 4;
  if (buffer[53]) now.year += 8;
  if (buffer[54]) now.year += 10;
  if (buffer[55]) now.year += 20;
  if (buffer[56]) now.year += 40;
  if (buffer[57]) now.year += 80;

  // 58 == parity date, checked previously

  if (LOGGING_DCF77) Serial.println("parsed new time: " + printTime(now));

  // send result to callback
  onDCF77Result(now);
}

/**
 * Writes a bit to the buffer and determinis if it should get parsed.
 */
void writeNextBit(byte bit) {
  // ignore while in wait state
  if (next_index == -1) return;

  if (LOGGING_DCF77_DATA) Serial.println(
    "data[" + String(next_index, DEC) + "] <- " + String(bit, DEC));

  // write data
  if (next_index <= 58) buffer[next_index++] = bit;

  // check if this is the end of the current transmission
  if (next_index > 58) finishMinute();
}

/**
 * Returns true if the given value lies between lower_bound (inclusive) 
 * and upper_bound (inclusive) and else returns false.
 */
 bool isWithinRange(unsigned int value, unsigned int lower_bound, unsigned int upper_bound) {
  return value >= lower_bound && value <= upper_bound;
}

/**
 * Returns true if the given range (each inclusive) has an even parity. 
 */
bool isEvenParity(byte input, int index_start, int index_end) {
  int sum = 0;
  for (int i = index_start; i <= index_end; i++)
    if (input[i]) sum++;
  return (sum % 2) == 0;
}
