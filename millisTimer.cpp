#include "millisTimer.hpp"

/// @brief Inline delay if the time is not known at compile time
/// @param delayTime in ms
void MillisTimer::delayThisMuch(unsigned long delayTime)
{
    unsigned long startTime = millis();
    unsigned long finishTime = startTime + delayTime;

    if(finishTime < startTime) //if this is true then it overflowed
    {
        return; //most likely the machine will never be powered on long enough to overflow with millis
    }

    while(millis() < finishTime)
    {
        //Wasting time here
        // This loop ensures that the function doesn't get optimized away
        asm volatile ("nop");  // No operation assembly instruction
    }
}