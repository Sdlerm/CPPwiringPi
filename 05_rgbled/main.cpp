#include <wiringPi.h>
#include <softPwm.h>
#include <iostream>
using namespace std;

constexpr int R = 17, G = 27, B =22;

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

    //Creat software PWM threads for each pin: softPwmCreate(pin, initial_value, range)
    //Initial value = 100 = fully off for common-anode
    softPwmCreate(R,100,100);
    softPwmCreate(G,100,100);
    softPwmCreate(B,100,100);

    Color magenta{100,0,100};
    Color cyan{0,100,100};
    Color yellow{100,100,0};
    Color white {100,100,100}

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
