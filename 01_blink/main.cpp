#include <wiringPi.h>
#include <iostream>

constexpr int LED = 17;

void flash(int ms)
{
    digitalWrite(LED,HIGH);
    delay(ms);
    digitalWrite(LED,LOW);
    delay(200);
}

void dot()  
{
    flash(200);
}

void dash()
{
    flash(600);
}

void sos()
{
    dot(); dot(); dot();
    delay(400);
    dash();dash(); dash();
    delay(400);
    dot(); dot(); dot();
    delay(1500);
}

int main()
{
    if (wiringPiSetupGpio() == -1) 
    {
        std::cerr << "Failed to initialize wiringPi" << std::endl;
        return 1;
    }

    pinMode(LED,OUTPUT);

    for (int i=5;i>0;i--) 
    {
        sos();
    }
    

    /*
    for (int i=0;i<10;i++)
    {
        // LED pin stays HIGH unit you change it;nothing auto-releases it at scope end
        digitalWrite(LED,HIGH);
        delay(500);
        digitalWrite(LED,LOW);
        delay(500);
    }
        */
    return 0;
}