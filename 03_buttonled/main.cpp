#include <wiringPi.h>
#include <iostream>
using namespace std;

constexpr int BTN = 26;
constexpr int LED =24;

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
    int raw_state = HIGH;
    int stable_state = HIGH;
    unsigned int last_change = millis();
    
    while (true)
    {
        int now  = digitalRead(BTN);
        if (now != raw_state)
        {
            raw_state = now;
            last_change = millis();
        }

        if (now != stable_state && millis() - last_change >= 20)
        {
            stable_state = now;
            led_on = (stable_state == LOW);
            digitalWrite(LED, led_on ? HIGH : LOW);
            cout << "Time: " << last_change << " msec\n";
            cout << "LED " << (led_on ? "on" : "off") << "\n";
        }
        delay(5);
    }
    return 0; }
