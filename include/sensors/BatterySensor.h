#ifndef BATTERY_SENSOR_H
#define BATTERY_SENSOR_H

#include <Arduino.h>
#include "sensors/ISensor.h"
#include "config.h"
#include "debug.h"

class BatterySensor : public ISensor {
public:    explicit BatterySensor(const std::string& id);

    bool begin() override;
    SensorReading read() override;
    const std::string& getId() const override { return _id; }
    SensorType getType() const override { return _type; }
    CommunicationProtocol getProtocol() const override { return CommunicationProtocol::ANALOG_ADC; }
    PowerRequirement getPowerRequirement() const override { return PowerRequirement::POWER_NONE; }

private:
    /**
     * @brief Calcula el voltaje real de la batería a partir de la lectura del ADC
     *
     * El circuito es un divisor de voltaje:
     *
     * Batería (+) ---- R2 (390k) ---- | ---- R1 (100k) ---- GND
     *                                 |
     *                                 +--- ADC Pin
     *
     * La fórmula es:
     * VBAT = VADC_IN1 / (R1 / (R1 + R2))
     *
     * @param adcVoltage Voltaje medido por el ADC
     * @return float Voltaje real de la batería
     */
    float calculateBatteryVoltage(float adcVoltage);
};

#endif // BATTERY_SENSOR_H