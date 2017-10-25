#pragma once

#include "Arduino.h"

#include "Time.hpp"

// Config:
#define LOGGING_DCF77 true // if true, logs basic information to serial out
#define LOGGING_DCF77_LED true // if true, displays the receiver date on LOGGING_DCF77_LED_PIN
#define LOGGING_DCF77_DATA false // if true, logs the binary representation of the signal
#define LOGGING_DCF77_DATA_RAW false // if true, logs the elapsed milliseconds each time the signal changes

#define DCF77_DISCARD_ON_PARITY_PROBLEM true // if true, discards a transmission if the parity bit was incorrect

#define LOGGING_DCF77_LED_PIN LED_BUILTIN // the pin to send the LED signal blinking to

typedef void (*callback_t)(Time); 

/**
* Provides asynchronous time information using a DCF77 receiver.
* It's algorithm ignores leap seconds. 
* 
* Do not create more than one instance!
*/
class TimeProvider
{

public:
    
    /**
    * Creates a new instance. May only be called once.
    */
    TimeProvider(int interrupt_pin, callback_t callback);

    /**
     * Do not use!
     */
    TimeProvider() {};

    /**
    * Starts listening for new time signals. 
    * It might take a while for the first one to arrive.
    * When a new time signal was decoded, the callback gets fired.
    */
    void start();
    
    /**
    * Stops listening for new time signals. 
    */
    void stop();

    /**
    * Handles the actual interrupt.
    * Is public so it can be called from the ISR.
    */
    void interruptHandler();    
    
private:
    
    int interrupt_pin; // the pin number of the time signal receiver
    callback_t callback; // the callback to fire on a newly decoded time signal
    
    unsigned long previous_time = 0; // used by calculateTimeDelta()
    uint8_t buffer[59]; // buffer for currently receiving data
    volatile int next_index = -1; // next index to be written to, -1 = paused

    /**
    * Calculate the time delta between now and the previous 
    * invocation in milliseconds.
    */
    unsigned int calculateTimeDelta();

    /**
    * Called at the start of a new minute/transmission.
    */
    void startNewMinute();
    
    /**
    * Gets called at the end of minute/transmission. Parses the received data 
    * and fires the callback.
    */
    void finishMinute();

    /**
    * Writes a bit to the buffer and determinis if it should get parsed.
    */
    void writeNextBit(uint8_t bit);

    /**
    * Returns true if the given value lies between lower_bound (inclusive) 
    * and upper_bound (inclusive) and else returns false.
    */
    static bool isWithinRange(unsigned int value, unsigned int lower_bound, unsigned int upper_bound);

    /**
    * Returns true if the given range (each inclusive) has an even parity.
    * Assumes that index_start and index_end are valid zero-based indices of input.
    */
    static bool isEvenParity(uint8_t input[], int index_start, int index_end);

};

/**
 * The one TimeProvider for all methods.
 */
extern TimeProvider globalTimeProvider;
