#include <wiringPi.h>
#include <iostream>
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
        //Toggling to turn LED  on/off via use of boolled_on/!led_on
        /*
        if (now == LOW && prev ==HIGH)
        {
            led_on = !led_on; //TOGGLE
            digitalWrite(LED,(led_on ? HIGH:LOW));
            cout << "LED " << (led_on ? "on" : "off") << "\n";
        }
            */
        //Holding & releasing button (edge detection level-follow) to control LED
        if  (now == LOW && prev == HIGH)
        {
            digitalWrite(LED,HIGH);
            cout << "LED on\n";
        }
        else if (now == HIGH && prev == LOW) 
        {
            digitalWrite(LED,LOW);
            cout << "LED off\n";
        }
        prev = now;
        delay(5);
    }
    return 0;
}