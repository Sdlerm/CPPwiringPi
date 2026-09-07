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
    waitingForRelease,
    waitingForLight,
    waitingForPress,
    winnerShown
};

int main()
{
    // stdout is a pipe (not a tty) when launched from the VS Code debugger,
    // so force a flush after every << or no messages appear until exit
    cout << unitbuf;

    if (wiringPiSetupGpio() == -1)
    {
        cerr << "wiringPiSetupGpio failed\n";
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

    cout << "Press both buttons to start.\n";

    while (true)
    {
        switch (state)
        {
        case gameState::waitingToStart:
            if (digitalRead(BTN1) == LOW && digitalRead(BTN2) == LOW)
            {
                cout << "Release both buttons...\n";
                state = gameState::waitingForRelease;
            }
            break;

        // The random timer only starts once both buttons are let go,
        // otherwise the still-held buttons would trigger "Too early"
        case gameState::waitingForRelease:
            if (digitalRead(BTN1) == HIGH && digitalRead(BTN2) == HIGH)
            {
                lightTime = millis() +  waitTime(rng);
                cout << "Get ready... wait for the light!\n";
                state = gameState::waitingForLight;
            }
            break;

        // Turn the LED on once the random delay has elapsed
        case gameState::waitingForLight:
            if (digitalRead(BTN1) == LOW || digitalRead(BTN2) == LOW)
            {
                cout << "Too early, cheating cunt! Versuchen Sie spater!\n";
                cout << "Press both buttons to start.\n";
                state = gameState::waitingToStart;
            }
            else if (millis() >= lightTime)
            {
                digitalWrite(LED,HIGH);
                cout << "GO!\n";
                state = gameState::waitingForPress;
            }
            break;

        case gameState::waitingForPress:
        {
            const bool player1Pressed = digitalRead(BTN1) == LOW;
            const bool player2Pressed = digitalRead(BTN2) == LOW;
            if (player1Pressed && player2Pressed)
            {
                cout << "Tie cocksuckers!\n";
                state = gameState::winnerShown;
            }
            else if (player1Pressed)
            {
                cout << "Player1 wins mother fucker!\n";
                state = gameState::winnerShown;
            }
            else if (player2Pressed)
            {
                cout << "Player2 wins mother fucker!\n";
                state = gameState::winnerShown;
            }
            break;
        }

        case gameState::winnerShown:
            digitalWrite(LED,LOW);
            delay(2000);
            cout << "Press both buttons to start.\n";
            state = gameState::waitingToStart;
            break;
        }

        // prevents the program from maxing out a CPU core and gives basic button debounce
        delay(5);
    }
}