#include <wiringPi.h>
#include <iostream>
using namespace std;

constexpr int LED = 18;

int main()
{
    if (wiringPiSetupGpio() == -1)
    {
        cerr << "WiringPi failed to start!\n";  
        return 1;
    }

    //configure BCM18 for hardware pwm output
    pinMode(LED,PWM_OUTPUT);

    //set the mode to mark-space (constant frequency, variable duty cycle)
    pwmSetMode(PWM_MODE_MS);

    //configure resolution range of the duty cycle
    //set to 1024 ticks/cycle
    //brightness settings will scale from 0 to 1024
    const int pwmRange = 1024;
    pwmSetRange(pwmRange);

    //compute and set the hardware clock divisor to target a 100Hz frequency
    // divisor = (base clock)/((range)*(target_frequency))
    // (19,200,000 Hz)/((1024)*(100 Hz)) = 187.5 --> 188
    pwmSetClock(188);

    

    while (true)
    {
        //ramp up from dark (0) to  full brightnesss (1024)
        for (int duty = 0; duty <= pwmRange; duty++) 
        {
            cout  << "Ramping Up: " << duty << "/1024\n";
            pwmWrite(LED,duty);
            delay(2); //small 2 ms delay to slow sweep to be detectable human eye
        }
        //ramp down from full  bright (1024) to dark (0)
        for (int duty = 1024; duty >= 0; duty--)
        {
            cout  << "Ramping  DOWN: " << duty << "/1024\n";
            pwmWrite(LED, duty);
            delay(2);
        }
    }
    return 0;
}