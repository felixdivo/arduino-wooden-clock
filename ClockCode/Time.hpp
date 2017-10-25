#pragma once

#include "Arduino.h" // for the String class

/**
* A simple struct for storing time and date information.
*/
struct Time {
    
    uint8_t second = 0;
    uint8_t minute = 0;
    uint8_t hour = 0;
    
    uint8_t day_of_week = 0;
    
    uint8_t day = 0;
    uint8_t month = 0;
    uint16_t year = 2000;
    
    /**
    * Converts a Time struct into a human readable String.
    */
    String toString();
    
    /**
    * Returns the English name of a day of the week.  Monday is 1, Sunday is 7, 
    * everything outside of that returns a placeholder.
    */
    static String getDayOfWeek(uint8_t day_of_week);
    
};
