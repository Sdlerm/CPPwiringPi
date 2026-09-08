# CPPwiringPi

A modern C++ wrapper library for WiringPi, providing object-oriented access to GPIO, PWM, and other Raspberry Pi hardware features.

## Overview

CPPwiringPi is a comprehensive C++ library that wraps the popular WiringPi library, offering an easy-to-use, type-safe interface for Raspberry Pi hardware control. Whether you're building robotics projects, IoT applications, or hardware prototypes, CPPwiringPi simplifies GPIO, PWM, and other peripheral interactions.

## Features

- 🔌 **GPIO Control** - Easy digital input/output operations
- ⚡ **PWM Support** - Pulse-width modulation for speed and brightness control
- 🎯 **Object-Oriented API** - Clean, modern C++ interface
- 🛡️ **Type-Safe** - Leverages C++ type system for safer hardware operations
- 🚀 **Efficient** - Minimal overhead over WiringPi
- 🔧 **CMake Build System** - Easy integration and cross-platform building

## Requirements

- Raspberry Pi (any model)
- Raspbian or compatible Linux distribution
- WiringPi library installed
- C++11 or later
- CMake 3.10 or later

## Installation

### Prerequisites

First, install WiringPi:

```bash
sudo apt-get update
sudo apt-get install wiringpi
```

Verify WiringPi installation:

```bash
gpio -v
```

### Building CPPwiringPi

```bash
# Clone the repository
git clone https://github.com/Sdlerm/CPPwiringPi.git
cd CPPwiringPi

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
make

# Optional: Install the library
sudo make install
```

## Quick Start

### Basic GPIO Example

```cpp
#include <cppwiringpi/gpio.h>

int main() {
    // Initialize WiringPi
    wiringPiSetup();
    
    // Create a GPIO pin object
    GPIOPin led(17);  // GPIO pin 17
    led.setMode(OUTPUT);
    
    // Blink the LED
    for (int i = 0; i < 10; i++) {
        led.digitalWrite(HIGH);
        delay(500);
        led.digitalWrite(LOW);
        delay(500);
    }
    
    return 0;
}
```

### PWM Example

```cpp
#include <cppwiringpi/pwm.h>

int main() {
    wiringPiSetup();
    
    // Create PWM pin object
    PWMPin pwm(18);
    pwm.setMode(PWM_OUTPUT);
    pwm.setPWMRange(1024);
    
    // Gradually increase brightness
    for (int brightness = 0; brightness < 1024; brightness += 10) {
        pwm.pwmWrite(brightness);
        delay(10);
    }
    
    return 0;
}
```

## Project Structure

```
CPPwiringPi/
├── CMakeLists.txt          # Main CMake configuration
├── README.md               # This file
├── src/                    # Source files
│   ├── gpio.cpp
│   ├── pwm.cpp
│   └── ...
├── include/                # Header files
│   └── cppwiringpi/
│       ├── gpio.h
│       ├── pwm.h
│       └── ...
├── examples/               # Example programs
└── tests/                  # Unit tests
```

## API Documentation

### GPIOPin Class

- `setMode(mode)` - Set pin as INPUT or OUTPUT
- `digitalWrite(value)` - Write HIGH or LOW to pin
- `digitalRead()` - Read pin state
- `getPinNumber()` - Get the GPIO pin number

### PWMPin Class

- `setMode(mode)` - Set pin to PWM_OUTPUT mode
- `setPWMRange(range)` - Set PWM range/frequency
- `pwmWrite(value)` - Write PWM value
- `getPWMRange()` - Get current PWM range

## Building and Running Examples

```bash
cd build
cmake .. -DBUILD_EXAMPLES=ON
make

# Run an example (e.g., blink)
./examples/blink
```

## Troubleshooting

### Permission Denied Errors

Most GPIO operations require elevated privileges:

```bash
sudo ./your_program
```

Or add your user to the gpio group:

```bash
sudo usermod -aG gpio $USER
```

### WiringPi Not Found

Ensure WiringPi is installed and in your system path:

```bash
which gpio
gpio -v
```

### Build Errors

Clean and rebuild:

```bash
cd build
rm -rf *
cmake ..
make
```

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- [WiringPi](http://wiringpi.com/) - Gordon Henderson's excellent GPIO library
- Raspberry Pi Foundation for the wonderful Raspberry Pi platform

## Support

For issues, questions, or suggestions, please open an issue on the [GitHub repository](https://github.com/Sdlerm/CPPwiringPi/issues).

---

**Happy Pi Programming!** 🥧
