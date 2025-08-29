#ifndef SOIL_HUMIDITY_SENSOR_H
#define SOIL_HUMIDITY_SENSOR_H

#include <Arduino.h>
#include "sensors/ISensor.h"
#include "config.h"
#include "debug.h"

class SoilHumiditySensor : public ISensor {
public:
    explicit SoilHumiditySensor(const std::string& id);

    bool begin() override;
    SensorReading read() override;
    const std::string& getId() const override { return _id; }
    SensorType getType() const override { return _type; }
    CommunicationProtocol getProtocol() const override { return CommunicationProtocol::ANALOG_ADC; }
    PowerRequirement getPowerRequirement() const override { return PowerRequirement::POWER_3V3_SWITCHED; }
};

#endif // SOIL_HUMIDITY_SENSOR_H