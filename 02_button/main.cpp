#include <wiringPi.h>
#include <iostream>

constexpr int BTN=2;

int main() {
    if (wiringPiSetupGpio() == -1) {
        std::cerr << "WiringPi setup failed!" << std::endl;
        return 1;
    }

    pinMode(BTN, INPUT);
    pullUpDnControl(BTN,PUD_UP);

    int prev = HIGH; //resting level with a pull-up

    // Main loop to detect button presses
    while (true) {
        int now = digitalRead(BTN); // read the current state of the button
        
        if (now == LOW && prev == HIGH) //detect exactly one edge (= change in state?); now is the current state and prev is the previous state
        {
            std::cout << "PUSHED MOTHERFUCKER!\n";
        }
        //Message for when button is released
        
        if (now == HIGH && prev == LOW)
        {
            std::cout << "RELEASED COCKSUCKER!\n";
        }
        prev = now; // update the previous state to the current state; high --(button pushed)--> low; prev is now assigned to the current state of the button
        delay(10); //debounce-ish poll interval; prevents detecting multiple presses too quickly

    }



    return 0;
}