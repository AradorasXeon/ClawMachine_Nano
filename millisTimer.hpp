//Implementing own timer function, because I suspect that I have freezes because i2c wants to communicate at some delay
#ifndef MILLIS_TIMER_H
#define MILLIS_TIMER_H

class MillisTimer
{
    public:
    MillisTimer(unsigned long delayTime);
    void doDelay();
    static void delayThisMuch(unsigned long delayTime);

    private:
    unsigned long delayAmount = 0U;
    unsigned long startTime = 0U;
    unsigned long finishTime = 0U;
};


/// @brief creates a timer
/// @param delayTime in milliseconds
MillisTimer::MillisTimer(unsigned long delayTime)
{
    delayAmount = delayTime;
}

/// @brief Call this function when you need the specified delay
void MillisTimer::doDelay()
{
    startTime = millis();
    finishTime = startTime + delayAmount;

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



#endif // !MILLIS_TIMER_H