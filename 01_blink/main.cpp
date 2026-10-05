#include <wiringPi.h>
#include <iostream>

constexpr int LED = 24;
void flash(int ms)
{
    digitalWrite(LED,HIGH);
    delay(ms);
    digitalWrite(LED,LOW);
    delay(200);
}

void detection(int msm)
{
    for (int i=0;i<5;i++)
    {
        std::cout << "Flashing delay: " << msm << " ms\n";
        flash(msm);;
    }
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
        // detection(500);
        // detection(100);
        // detection(10);
        detection(1);
       sos();
    }

        
    return 0;
}