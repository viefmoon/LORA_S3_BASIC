#ifndef ISENSOR_H
#define ISENSOR_H

#include "sensor_types.h"
#include <string>

enum class CommunicationProtocol {
    NONE,
    I2C,
    ONE_WIRE,
    MODBUS,
    ANALOG_ADC,
    SPI
};

enum class PowerRequirement {
    POWER_3V3_MAIN,
    POWER_3V3_SWITCHED,
    POWER_12V,
    POWER_NONE
};

class ISensor {
public:
    virtual ~ISensor() = default;

    virtual bool begin() = 0;
    virtual SensorReading read() = 0;
    virtual const std::string& getId() const = 0;
    virtual SensorType getType() const = 0;
    virtual CommunicationProtocol getProtocol() const = 0;
    virtual PowerRequirement getPowerRequirement() const = 0;
    
    bool isInitialized() const {
        return _initialized;
    }

protected:
    std::string _id;
    SensorType _type;
    bool _initialized = false;
};

#endif // ISENSOR_H