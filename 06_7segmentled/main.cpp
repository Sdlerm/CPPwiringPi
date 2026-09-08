#include <wiringPi.h>
#include <array>
#include <cstdint>
#include <iostream>
using namespace std;

//segment order: a b c d e f g (0=lit for common anode)
const array<uint8_t,10> DIGIT = {
    0b0111111,//0
    0b0110000,//1
    0b1011011,//2
    0b1111001,//3
    0b1110100,//4
    0b1101101,//5
    0b1101111,//6
    0b0111000,//7
    0b1111111,//8
    0b1111101,//9
};

const int SEG[7] = {16,25,19,26,21,20,17};

void write_digit(uint8_t pattern) 
{
    for (int s=0;s<7;s++)
    {
        bool on = (pattern >> s) & 1;
        digitalWrite(SEG[s],on ? LOW:HIGH);
    }
}

int main()
{
    if (wiringPiSetupGpio() == -1)
    {
        cerr << "WiringPi failed to initialize!\n";
        return 1;
    }

    for (int s=0;s<7;s++)
    {
        pinMode(SEG[s],OUTPUT);
    }

    for (int i=0;i<=10;i++)
    {
        write_digit(DIGIT[i]); 
        delay(1000);
    }
}