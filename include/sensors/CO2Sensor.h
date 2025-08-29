#ifndef CO2_SENSOR_H
#define CO2_SENSOR_H

#include <Arduino.h>
#include "sensors/ISensor.h"
#include "config.h"
#include "debug.h"
#include "SparkFun_SCD4x_Arduino_Library.h"

// Declaración externa del objeto del sensor (se definirá en src/main.cpp)
extern SCD4x scd4x;

class CO2Sensor : public ISensor {
public:    explicit CO2Sensor(const std::string& id);

    bool begin() override;
    SensorReading read() override;
    const std::string& getId() const override { return _id; }
    SensorType getType() const override { return _type; }
    CommunicationProtocol getProtocol() const override { return CommunicationProtocol::I2C; }
    PowerRequirement getPowerRequirement() const override { return PowerRequirement::POWER_3V3_MAIN; }
};

#endif // CO2_SENSOR_H