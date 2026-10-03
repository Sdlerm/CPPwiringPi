/**********************************************************************
* Filename    : ADC.cpp
* Description : Use ADC module to read the voltage value of potentiometer.
**********************************************************************/
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <cstdio>
#include <memory>
#include "ADCDevice.h"

// ADS7830: 8-bit, 8-channel I2C ADC
class ADS7830 : public ADCDevice {
public:
    explicit ADS7830(int address = 0x4b) : ADCDevice(8, 3.3f), m_address(address) {}
    ~ADS7830() override = default;

    bool init() override {
        m_fd = wiringPiI2CSetup(m_address);
        return m_fd >= 0;
    }

    uint32_t readRaw(uint8_t channel) override {
        // Single-ended input select bits, internal reference off, ADC on
        uint8_t cmd = 0x84 | (((channel << 2 | channel >> 1) & 0x07) << 4);
        wiringPiI2CReadReg8(m_fd, cmd);        // dummy read to start conversion
        return wiringPiI2CReadReg8(m_fd, cmd);
    }

private:
    int m_address;
    int m_fd = -1;
};

int main(void){
    printf("Program is starting ... \n");
    std::unique_ptr<ADCDevice> adc = std::make_unique<ADS7830>(0x4b);
    if(!adc->init()){
        printf("No correct I2C address found, \n"
        "Please use command 'i2cdetect -y 1' to check the I2C address! \n"
        "Program Exit. \n");
        return -1;
    }

    while(1){
        uint32_t adcValue = adc->readRaw(0);
        float voltage = adc->readVoltage(0);
        printf("ADC value : %u  ,\tVoltage : %.2fV\n", adcValue, voltage);
        delay(100);
    }
    return 0;
}
