// ============================================================================
// File: ldr_rc_timing.cpp
// Target: Raspberry Pi (WiringPi C++)
// Description: Measures LDR light level without an ADC using a capacitor and 
//              the RC step-response timing method.
// Build: g++ -O2 ldr_rc_timing.cpp -o ldr_rc_timing -lwiringPi
// Usage: sudo ./ldr_rc_timing
// ============================================================================

#include <wiringPi.h>
#include <iostream>
#include <cmath>

// Define WiringPi Pin 0 (which corresponds to BCM GPIO 17 / Physical Pin 11)
const int LDR_PIN = 0; 

// Component Values for Resistance / Lux Estimation
const double CAPACITANCE_UF = 0.1;           // 0.1 uF (100 nF)
const double V_IN = 3.3;                      // Raspberry Pi 3.3V Rail
const double V_TH = 1.8;                      // Digital HIGH threshold (~1.8V)

int main() {
    // Initialize WiringPi library (uses WiringPi pin numbering scheme)
    if (wiringPiSetup() == -1) {
        std::cerr << "[ERROR] WiringPi setup failed! Make sure to run with root privileges." << std::endl;
        return 1;
    }

    std::cout << "======================================================" << std::endl;
    std::cout << "  Raspberry Pi LDR RC Timing Logger (No ADC Required) " << std::endl;
    std::cout << "======================================================" << std::endl;

    while (true) {
        // --- Step 1: Discharge Phase ---
        // Set pin as OUTPUT and drive LOW to drain stored charge from the capacitor
        pinMode(LDR_PIN, OUTPUT);
        digitalWrite(LDR_PIN, LOW);
        delay(100); // Wait 100 ms to guarantee capacitor is fully discharged to 0V

        // --- Step 2: Charge & Timing Phase ---
        // Reconfigure pin as INPUT (high impedance) and record start time in microseconds
        pinMode(LDR_PIN, INPUT);
        unsigned long startTime = micros();

        // Loop until pin reads HIGH (capacitor charges up to digital threshold ~1.8V)
        // Includes a safety timeout of 1 second (1000000 us) for total darkness
        while (digitalRead(LDR_PIN) == LOW) {
            if ((micros() - startTime) > 1000000) {
                break; // Timeout if pitch black
            }
        }

        unsigned long durationUs = micros() - startTime;

        // --- Step 3: Resistance Calculation ---
        // Formula: t = R * C * ln(V_in / (V_in - V_th))
        // So R = t / (C * ln(V_in / (V_in - V_th)))
        double lnTerm = std::log(V_IN / (V_IN - V_TH)); // ln(3.3 / 1.5) approx 0.788
        double capFarads = CAPACITANCE_UF * 1e-6;
        double rLdrOhms = (durationUs * 1e-6) / (capFarads * lnTerm);

        // Display results
        std::cout << "[Reading] Charging Time: " << durationUs << " us"
                  << " | Estimated LDR Resistance: " << (rLdrOhms / 1000.0) << " kOhms";

        if (durationUs < 500) {
            std::cout << " (Bright Light)" << std::endl;
        } else if (durationUs < 5000) {
            std::cout << " (Ambient Room Light)" << std::endl;
        } else {
            std::cout << " (Dark / Dim)" << std::endl;
        }

        delay(500); // Take a reading twice per second
    }

    return 0;
}
