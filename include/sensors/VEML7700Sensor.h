#ifndef VEML7700_SENSOR_H
#define VEML7700_SENSOR_H

#include <Arduino.h>
#include "sensors/ISensor.h"
#include <Adafruit_VEML7700.h>


class VEML7700Sensor : public ISensor {
public:
    explicit VEML7700Sensor(const std::string& id);

    bool begin() override;
    SensorReading read() override;
    const std::string& getId() const override { return _id; }
    SensorType getType() const override { return _type; }
    CommunicationProtocol getProtocol() const override { return CommunicationProtocol::I2C; }
    PowerRequirement getPowerRequirement() const override { return PowerRequirement::POWER_3V3_SWITCHED; }
};

#endif