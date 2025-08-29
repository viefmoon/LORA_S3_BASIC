#ifndef HDS10_SENSOR_H
#define HDS10_SENSOR_H

#include <Arduino.h>
#include "sensors/ISensor.h"
#include "config.h"
#include "debug.h"

class HDS10Sensor : public ISensor {
public:
    explicit HDS10Sensor(const std::string& id);

    bool begin() override;
    SensorReading read() override;
    const std::string& getId() const override { return _id; }
    SensorType getType() const override { return _type; }
    CommunicationProtocol getProtocol() const override { return CommunicationProtocol::ANALOG_ADC; }
    PowerRequirement getPowerRequirement() const override { return PowerRequirement::POWER_3V3_SWITCHED; }

private:
    /**
     * @brief Convierte la resistencia del sensor HDS10 a porcentaje de humedad
     *
     * @param resistance Resistencia del sensor en ohms
     * @return float Porcentaje de humedad relativa (50-100%)
     */
    float convertResistanceToHumidity(float resistance);
};

#endif