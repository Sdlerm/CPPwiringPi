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

void detection(double msm)
{
    for (int i=0;i<5;i++)
    {
        std::cout << "Flashing delay: " << msm << " ms\n";
        flash(msm);
        delay(1000);
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

    //for (int i=5;i>0;i--) 
   // {
   //     sos();
   // }

   //Test for different delay times 
   /*detection(1000);
   delay(1000);
   detection(500);
   delay(1000);
   detection(200);
   delay(1000);
   detection(100);
   delay(1000);
   detection(50);
   delay(1000);
   detection(25);
   delay(1000);
   detection(10);
   delay(1000);
   detection(5);
   delay(1000);
   detection(1);
   delay(1000); 
   detection(1);
   delay(1000);
   detection(0.8);
   delay(1000); 
   detection(0.6);
   delay(1000);
   detection(0.1);
   delay(1000);
    */

    
    while (true)
    {
        digitalWrite(LED,HIGH);
        digitalWrite(LED,LOW);
    }
        
    return 0;
}