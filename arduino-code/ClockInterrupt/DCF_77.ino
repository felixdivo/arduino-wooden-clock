const bool LOGGING_DCF77 = true;
const bool LOGGING_DCF77_LED = true;
const bool LOGGING_DCF77_DATA = true;
const bool LOGGING_DCF77_DATA_RAW = false;

const bool DISCARD_ON_PARITY_PROBLEM = true;

volatile unsigned int INTERRUPT_PIN;

void startDCF77(int interrupt_pin) {

  INTERRUPT_PIN = interrupt_pin;
  pinMode(INTERRUPT_PIN, INPUT);

  /* LOGGING_DCF77 = logging;
     LOGGING_DCF77_LED = LOGGING_DCF77_LED && logging;
     LOGGING_DCF77_DATA = LOGGING_DCF77_DATA && logging;
     LOGGING_DCF77_DATA_RAW = LOGGING_DCF77_DATA_RAW && logging; */

  // init time
  calculateTimeDelta();

  if (LOGGING_DCF77) Serial.println("Register the \"Interrupt Service Routine\"");

  // enable interrupts
  interrupts();

  /* Inside the attached function, delay() won't work and the value returned by
     millis() will not increment. Serial data received while in the function may
     be lost. You should declare as volatile any variables that you modify within
     the attached function. See the section on ISRs below for more information.
     See: https://www.arduino.cc/en/Reference/AttachInterrupt */
  attachInterrupt(digitalPinToInterrupt(interrupt_pin), interruptReceiver, CHANGE);
}

void stopDCF77() {
  detachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN));
}

/* is called on a CHANGE event */
void interruptReceiver() {

  byte current_data = digitalRead(INTERRUPT_PIN);
  unsigned int time_delta = calculateTimeDelta();

  // logging
  if (LOGGING_DCF77) {
    if (current_data == HIGH) {
      if (LOGGING_DCF77_DATA_RAW) Serial.println("HIGH: " + String(time_delta, DEC));
      if (LOGGING_DCF77_LED) digitalWrite(LED_BUILTIN, HIGH);
    } else { /* c... == LOW */
      if (LOGGING_DCF77_DATA_RAW) Serial.println("LOW: "  + String(time_delta, DEC));
      if (LOGGING_DCF77_LED) digitalWrite(LED_BUILTIN, LOW);
    }
  }

  // skip junk signals
  if (isWithin(time_delta, 0, 50)) return;

  // extract information
  if (current_data == HIGH) {
    if      (isWithin(time_delta, 1000, 2000)) startNewMinute();
  } else { /* c... == LOW */
    if      (isWithin(time_delta, 80,   195))  writeNextBitZero();
    else if (isWithin(time_delta, 196,  320))  writeNextBitOne();
  }

}

volatile unsigned long previous_time = 0;

/* calculate the time delta */
unsigned int calculateTimeDelta() {

  /* use micros() since millis() wont't work inside interrupts
     See: https://www.arduino.cc/en/Reference/AttachInterrupt */
  unsigned long current_time = micros() / 1000;
  unsigned int time_delta = current_time - previous_time;

  // save new values for nex time
  previous_time = current_time;

  return time_delta;
}

/* including the boundaries  */
bool isWithin(unsigned int value, unsigned int min_val, unsigned int max_val) {
  return value >= min_val && value <= max_val;
}

// ignoring leap seconds
// true is one, false is zero
bool data[59];
volatile int next_index = -1; // -1 is paused

void startNewMinute() {
  if (LOGGING_DCF77) Serial.println("witnessed the start of a new minute, waiting");
  next_index = 0;
}

void finishMinute() {
  if (next_index != -1) {

    // make sure it is only evaluated once
    next_index = -1;

    // see https://de.wikipedia.org/wiki/DCF77#Zeitinformation
    // or  https://en.wikipedia.org/wiki/DCF77#Time_code_interpretation

    if (LOGGING_DCF77) Serial.println("witnessed complete time signal, processing");

    // check parity
    if (!isEvenParity(data, 21, 28)) {
      if (LOGGING_DCF77) Serial.println("parity problem (minutes: bit index 21-28)");
      if (DISCARD_ON_PARITY_PROBLEM) return;
    }
    if (!isEvenParity(data, 29, 35)) {
      if (LOGGING_DCF77) Serial.println("parity problem (hours: bit index 29-35)");
      if (DISCARD_ON_PARITY_PROBLEM) return;
    }
    if (!isEvenParity(data, 36, 58)) {
      if (LOGGING_DCF77) Serial.println("parity problem (date: bit index 36-58)");
      if (DISCARD_ON_PARITY_PROBLEM) return;
    }

    // evaluate the received data
    Time now;

    // set it to zero, because the signal only has a resolution of 1 minute
    now.second = 0;

    if (data[21]) now.minute += 1;
    if (data[22]) now.minute += 2;
    if (data[23]) now.minute += 4;
    if (data[24]) now.minute += 8;
    if (data[25]) now.minute += 10;
    if (data[26]) now.minute += 20;
    if (data[27]) now.minute += 40;

    // 28 == parity minute

    if (data[29]) now.hour += 1;
    if (data[30]) now.hour += 2;
    if (data[31]) now.hour += 4;
    if (data[32]) now.hour += 8;
    if (data[33]) now.hour += 10;
    if (data[34]) now.hour += 20;

    // 35 == parity hour

    if (data[36]) now.day += 1;
    if (data[37]) now.day += 2;
    if (data[38]) now.day += 4;
    if (data[39]) now.day += 8;
    if (data[40]) now.day += 10;
    if (data[41]) now.day += 20;

    if (data[42]) now.day_of_week += 1;
    if (data[43]) now.day_of_week += 2;
    if (data[44]) now.day_of_week += 4;

    if (data[45]) now.month += 1;
    if (data[46]) now.month += 2;
    if (data[47]) now.month += 4;
    if (data[48]) now.month += 8;
    if (data[49]) now.month += 10;

    if (data[50]) now.year += 1;
    if (data[51]) now.year += 2;
    if (data[52]) now.year += 4;
    if (data[53]) now.year += 8;
    if (data[54]) now.year += 10;
    if (data[55]) now.year += 20;
    if (data[56]) now.year += 40;
    if (data[57]) now.year += 80;

    // 58 == parity date

    // logging
    if (LOGGING_DCF77) Serial.println("Calculated new time: " + printTime(now));

    // send result
    onDCF77Result(now);

  }
}

void writeNextBitZero() {
  if (next_index == -1) return;

  if (LOGGING_DCF77_DATA) Serial.println("#" + String(next_index, DEC) + " => 0");
  if (next_index <= 58) data[next_index++] = false;

  if (next_index > 58) finishMinute();
}

void writeNextBitOne() {
  if (next_index == -1) return;

  if (LOGGING_DCF77_DATA) Serial.println("#" + String(next_index, DEC) + " => 1");
  if (next_index <= 58) data[next_index++] = true;

  if (next_index > 58) finishMinute();
}

bool isEvenParity(bool input, int index_start, int index_end) {
  int sum = 0;
  for (int i = index_start; i <= index_end; i++)
    if (data[i]) sum++;
  return (sum % 2) == 0;
}

