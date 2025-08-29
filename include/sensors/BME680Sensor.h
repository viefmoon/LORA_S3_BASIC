#ifndef BME680_SENSOR_H
#define BME680_SENSOR_H

#include <Arduino.h>
#include "sensors/ISensor.h"
#include <Adafruit_BME680.h>

// Variable externa del sensor
extern Adafruit_BME680 bme680Sensor;

class BME680Sensor : public ISensor {
public:
    explicit BME680Sensor(const std::string& id);

    bool begin() override;
    SensorReading read() override;
    const std::string& getId() const override { return _id; }
    SensorType getType() const override { return _type; }
    CommunicationProtocol getProtocol() const override { return CommunicationProtocol::I2C; }
    PowerRequirement getPowerRequirement() const override { return PowerRequirement::POWER_3V3_SWITCHED; }
};

#endif // BME680_SENSOR_H