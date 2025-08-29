#ifndef SHT40_SENSOR_H
#define SHT40_SENSOR_H

#include <Arduino.h>
#include "sensors/ISensor.h"
#include "config.h"
#include "debug.h"
#include <SensirionI2cSht4x.h>

// Variable externa del sensor SHT40 que será declarada en otro lugar
extern SensirionI2cSht4x sht40Sensor;

class SHT40Sensor : public ISensor {
public:    explicit SHT40Sensor(const std::string& id);

    bool begin() override;
    SensorReading read() override;
    const std::string& getId() const override { return _id; }
    SensorType getType() const override { return _type; }
    CommunicationProtocol getProtocol() const override { return CommunicationProtocol::I2C; }
    PowerRequirement getPowerRequirement() const override { return PowerRequirement::POWER_3V3_MAIN; }
};

#endif // SHT40_SENSOR_H