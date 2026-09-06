#include <wiringPi.h>
#include <iostream>
#include <random>
using namespace std;

constexpr int BTN1 = 22;
constexpr int BTN2 = 21;
constexpr int LED = 25;

enum class gameState
{
    waitingToStart,
    waitingForLight,
    waitingForPress,
    winnerShown
};

int main() 
{   
    int winner = 0; //0 means tie, 1 and 2 mean players1/2 win
    if (wiringPiSetupGpio() == -1) 
    {
        return  1;
    }

    //setup each pin as input/output
    pinMode(LED,OUTPUT);
    pinMode(BTN1,INPUT);
    pinMode(BTN2,INPUT);

    //enable input's internal pull-up resistor
    pullUpDnControl(BTN1,PUD_UP);
    pullUpDnControl(BTN2,PUD_UP);

    //setup initial state of LED (off)
    digitalWrite(LED,LOW);

    //declaration of initial state
    gameState state = gameState::waitingToStart;

//setup random timing variables
//waitTime(rng) will give random delay from 1-5 secs
//lightTime stores the future time for the LED to turn on
    mt19937 rng(random_device{}());
    uniform_int_distribution<int> waitTime(1000,5000);

    unsigned int lightTime = 0;

    while (true)
    {
        switch (state)
        {
        // Calculate the exact future time (in milliseconds) the LED should turn on
        case gameState::waitingToStart:
            if (digitalRead(BTN1) == HIGH && digitalRead(BTN2) == HIGH)
            {
                lightTime = millis() +  waitTime(rng);
                state = gameState::waitingForLight; 
            }
            break;
        
         // Check if the current time has surpassed the target time
         //If it has, turn LED on and switch to waitingForPress
         case gameState::waitingForLight:
             if (digitalRead(BTN1) == LOW || digitalRead(BTN2) == LOW)
            {
                cout << "Too early, cheating cunt! Versuchen Sie spater!\n";
                state = gameState::waitingToStart;
            }
            else if (millis() >= lightTime)
            {
                digitalWrite(LED,HIGH);
                state = gameState::waitingForPress;
            }
            break;

        case gameState::waitingForPress:
        {
            const bool player1Pressed = digitalRead(BTN1) == LOW;
            const bool player2Pressed = digitalRead(BTN2) == LOW;
            if (player1Pressed && player2Pressed)
            {
                winner = 0;
                cout << "Tie cocksuckers!\n";
                state = gameState::winnerShown;
            }
            else if (player1Pressed)
            {
                winner = 1;
                cout << "Player1 wins mother fucker!\n";
                state = gameState::winnerShown;

            }
            else if(player2Pressed)
            {
                winner = 2;
                cout << "Player2 wins mother fucker!\n";
                state = gameState::winnerShown;
            }

            if (state == gameState::winnerShown)
            {
                digitalWrite(LED,LOW);
            }
    
            break;
        }
        case gameState::winnerShown:
            digitalWrite(LED,LOW);
            delay(2000);
            state = gameState::waitingToStart;
            break;

        //prevents the program from maxing out a CPU core and gives basic button debounce
        delay(5);

        }
    }
}