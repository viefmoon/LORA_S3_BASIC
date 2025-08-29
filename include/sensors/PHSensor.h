#ifndef PH_SENSOR_H
#define PH_SENSOR_H

#include <Arduino.h>
#include "sensors/ISensor.h"
#include "config.h"
#include "debug.h"
#include "config_manager.h"

class PHSensor : public ISensor {
public:
    explicit PHSensor(const std::string& id);

    bool begin() override;
    SensorReading read() override;
    const std::string& getId() const override { return _id; }
    SensorType getType() const override { return _type; }
    CommunicationProtocol getProtocol() const override { return CommunicationProtocol::ANALOG_ADC; }
    PowerRequirement getPowerRequirement() const override { return PowerRequirement::POWER_3V3_SWITCHED; }

private:
    /**
     * @brief Convierte el voltaje medido a valor de pH
     *
     * @param voltage Voltaje medido del sensor de pH
     * @param tempC Temperatura del agua en grados Celsius para compensación
     * @return float Valor de pH (0-14)
     */
    float convertVoltageToPH(float voltage, float tempC);
};

#endif // PH_SENSOR_H