#ifndef RTD_SENSOR_H
#define RTD_SENSOR_H

#include <Arduino.h>
#include "sensors/ISensor.h"
#include "config.h"
#include <Adafruit_MAX31865.h>

// Variable externa del sensor RTD
extern Adafruit_MAX31865 rtdSensor;

class RTDSensor : public ISensor {
public:
    explicit RTDSensor(const std::string& id);

    bool begin() override;
    SensorReading read() override;
    const std::string& getId() const override { return _id; }
    SensorType getType() const override { return _type; }
    CommunicationProtocol getProtocol() const override { return CommunicationProtocol::SPI; }
    PowerRequirement getPowerRequirement() const override { return PowerRequirement::POWER_3V3_SWITCHED; }

private:
    // Parámetros del RTD
    static constexpr float RREF = 430.0f;  // Resistencia de referencia
    static constexpr float RNOMINAL = 100.0f;  // Resistencia nominal PT100
};

#endif // RTD_SENSOR_H