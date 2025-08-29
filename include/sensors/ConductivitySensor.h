#ifndef CONDUCTIVITY_SENSOR_H
#define CONDUCTIVITY_SENSOR_H

#include <Arduino.h>
#include "sensors/ISensor.h"
#include "config.h"
#include "debug.h"
#include "config_manager.h"

class ConductivitySensor : public ISensor {
public:
    explicit ConductivitySensor(const std::string& id);

    bool begin() override;
    SensorReading read() override;
    const std::string& getId() const override { return _id; }
    SensorType getType() const override { return _type; }
    CommunicationProtocol getProtocol() const override { return CommunicationProtocol::ANALOG_ADC; }
    PowerRequirement getPowerRequirement() const override { return PowerRequirement::POWER_3V3_SWITCHED; }

private:
    /**
     * @brief Convierte el voltaje medido a conductividad/TDS en ppm
     *
     * @param voltage Voltaje medido del sensor
     * @param tempC Temperatura actual del agua para compensación
     * @return float Valor de TDS en ppm (partes por millón)
     */
    float convertVoltageToConductivity(float voltage, float tempC);
};

#endif // CONDUCTIVITY_SENSOR_H