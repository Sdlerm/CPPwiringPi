#ifndef ADC_DEVICE_H
#define ADC_DEVICE_H

#include <cstdint>

/**
 * @class ADCDevice
 * @brief Abstract base class representing a generic Analog-to-Digital Converter (ADC).
 */
class ADCDevice {
protected:
    uint8_t m_resolutionBits;
    float m_referenceVoltage;

    // Protected constructor to prevent direct instantiation of the base class
    ADCDevice(uint8_t resolutionBits = 12, float refVoltage = 3.3f) 
        : m_resolutionBits(resolutionBits), m_referenceVoltage(refVoltage) {}

public:
    virtual ~ADCDevice() = default;

    /**
     * @brief Initializes the ADC hardware.
     * @return true if initialization was successful, false otherwise.
     */
    virtual bool init() = 0;

    /**
     * @brief Reads the raw digital value from the specified ADC channel.
     * @param channel The hardware ADC channel to read from.
     * @return The raw digital value (0 to 2^resolution - 1).
     */
    virtual uint32_t readRaw(uint8_t channel) = 0;

    /**
     * @brief Reads the converted voltage from the specified ADC channel.
     * @param channel The hardware ADC channel to read from.
     * @return The measured voltage in Volts.
     */
    virtual float readVoltage(uint8_t channel) {
        uint32_t rawValue = readRaw(channel);
        uint32_t maxValue = (1 << m_resolutionBits) - 1;
        return (static_cast<float>(rawValue) / maxValue) * m_referenceVoltage;
    }

    /**
     * @brief Sets the ADC resolution in bits (e.g., 10, 12, 16).
     * @param bits The number of bits for the resolution.
     */
    virtual void setResolution(uint8_t bits) {
        m_resolutionBits = bits;
    }

    /**
     * @brief Sets the reference voltage for the ADC.
     * @param refVoltage The reference voltage in Volts (e.g., 3.3, 5.0).
     */
    virtual void setReferenceVoltage(float refVoltage) {
        m_referenceVoltage = refVoltage;
    }
    
    /**
     * @brief Gets the current reference voltage.
     * @return The reference voltage in Volts.
     */
    float getReferenceVoltage() const {
        return m_referenceVoltage;
    }
};

#endif // ADC_DEVICE_H