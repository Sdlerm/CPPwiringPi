#include <wiringPi.h>
#include <softPwm.h>
#include <iostream>
using namespace std;

constexpr int R = 20, G = 18, B =19;
// constexpr int pins[] = {R,G,B};
// const char* names[] = {"R (GPIO20)","G (GPIO18)","B (GPIO19)"};

//each 0...100 duty
struct Color
{
    int r, g, b;
};

void show(const Color& c) 
{
    softPwmWrite(R,100 -  c.r);
    softPwmWrite(G,100 - c.g);
    softPwmWrite(B,100 - c.b);
}


int main()
{
    if (wiringPiSetupGpio() == -1) 
    {
        cerr << "WiringPi failed to initialize!\n";
        return 1;
    }

     pinMode(R,OUTPUT);
     digitalWrite(R,HIGH);
     pinMode(G,OUTPUT);
     digitalWrite(G,HIGH);
     pinMode(B,OUTPUT);
     digitalWrite(B,HIGH);

    softPwmCreate(R,100,100);
    softPwmCreate(G,100,100);
    softPwmCreate(B,100,100);

    Color magenta{100,0,100};
    Color cyan{0,100,100};
    Color yellow{100,100,0};
    Color white {100,100,100};

    //Red:off -->  full brightness
    // for (int i=0;i<100;i++) 
    //     {
    //         show({100-i,0,0});
    //         delay(20);
    //     }

    while (true) 
    {
        show(magenta);
        delay(1000);
        show(cyan);
        delay(1000);
        show(yellow);
        delay(1000);
        show(white);
        delay(1000);

    }
}