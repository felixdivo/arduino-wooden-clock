typedef struct {

  byte second = 0;
  byte minute = 0;
  byte hour = 0;

  byte day_of_week = 0;

  byte day = 0;
  byte month = 0;
  short year = 2000;

} Time;

String getDayOfWeek(byte day_of_week) {
  switch (day_of_week) {
    case 1: return "Monday";
    case 2: return "Tuesday";
    case 3: return "Wednesday";
    case 4: return "Thursday";
    case 5: return "Friday";
    case 6: return "Saturday";
    case 7: return "Sunday";
    default: return "*Day of week*";
  }
}

String printTime(Time t) {
  return String(t.hour, DEC) + ":"
         + String(t.minute, DEC) + ":"
         + String(t.second, DEC) + " at "
         + getDayOfWeek(t.day_of_week) + ", "
         + String(t.year, DEC) + "-"
         + String(t.month, DEC) + "-"
         + String(t.day, DEC);
}

