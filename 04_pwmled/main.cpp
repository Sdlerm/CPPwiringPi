#include <wiringPi.h>
#include <softPwm.h>

constexpr int LED = 18;

int main() 
{
    if (wiringPiSetupGpio() == -1) return 1;

    //softPwmCreate(pin,initial,range)
    softPwmCreate(LED,0,100);
    while (true)
    {
        for (int duty=0;duty<=100;duty++) 
        {
            //set brightness
            softPwmWrite(LED,duty);
            delay(15);
        }

        for (int duty=100;duty>=0;duty--)
        {
            softPwmWrite(LED,duty);
            delay(15);
        }
    }
}