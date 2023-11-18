//Implementing own timer function, because I suspect that I have freezes because i2c wants to communicate at some delay

#ifndef MICROS_TIMER_H
#define MICROS_TIMER_H
class Timer
{
    public:
    Timer(unsigned long delayTime);
    void doDelay();

    private:
    unsigned long delayAmount = 0U;
    unsigned long startTime = 0U;
    unsigned long finishTime = 0U;
};


/// @brief creates a timer
/// @param delayTime in microseconds
Timer::Timer(unsigned long delayTime)
{
    delayAmount = delayTime;
}

/// @brief Call this function when you need the specified delay
void Timer::doDelay()
{
    startTime = micros();
    finishTime = startTime + delayAmount;

    if(finishTime < startTime) //if this is true then it overflowed
    {
        return; //micros() is overflowing about every 70 minutes, I can live with that if we skip one delay every 70 minutes and the function returns sooner
    }

    while(micros() < finishTime)
    {
        //Wasting time here
        // This loop ensures that the function doesn't get optimized away
        asm volatile ("nop");  // No operation assembly instruction
    }

}

#endif // !MICROS_TIMER_H