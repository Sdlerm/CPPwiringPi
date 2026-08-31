#include <wiringPi.h>
using namespace std;

constexpr int BTN = 2;
constexpr int LED =17;

int main()
{
    if   (wiringPiSetupGpio() == -1)
    {
        return 1;
    }

    pinMode(BTN,INPUT);
    pullUpDnControl(BTN,PUD_UP);
    pinMode(LED,OUTPUT);

    bool led_on = false;
    int prev  = HIGH;
    
    while (true)
    {
        int  now  = digitalRead(BTN);
        if (now == LOW && prev ==HIGH)
        {
            led_on = !led_on; //TOGGLE
            digitalWrite(LED,(led_on ? HIGH:LOW));
        }
        prev = now;
        delay(5);
    }
    return 0;
}