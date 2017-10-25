#include "Time.hpp"

String Time::toString() {
    return 
      String(hour, DEC) + ":"
    + String(minute, DEC) + ":"
    + String(second, DEC) + " at "
    + Time::getDayOfWeek(day_of_week) + ", "
    + String(year, DEC) + "-"
    + String(month, DEC) + "-"
    + String(day, DEC);
}

String Time::getDayOfWeek(uint8_t day_of_week) {
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
