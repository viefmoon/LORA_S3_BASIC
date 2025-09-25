#ifndef MT05_SENSOR_H
#define MT05_SENSOR_H

#include <Arduino.h>
#include "sensors/ISensor.h"
#include "config.h"
#include "debug.h"

/**
 * @brief Driver para el sensor de suelo MT05S
 *
 * El MT05S es un sensor de suelo que mide:
 * - Temperatura del suelo (°C)
 * - Humedad del suelo (%)
 * - Conductividad eléctrica (µS/cm)
 *
 * Utiliza protocolo OneWire con comandos personalizados.
 * Requiere configuración mediante Write Scratchpad (0x4E) con
 * CONFIG0=0x14 para habilitar las tres mediciones.
 */
class MT05Sensor : public ISensor {
public:
    /**
     * @brief Constructor
     * @param id Identificador único del sensor
     */
    explicit MT05Sensor(const std::string& id);

    /**
     * @brief Inicializa el sensor
     * @return true si la inicialización fue exitosa
     */
    bool begin() override;

    /**
     * @brief Lee las mediciones del sensor
     * @return SensorReading con temperatura, humedad y conductividad
     */
    SensorReading read() override;

    /**
     * @brief Obtiene el ID del sensor
     */
    const std::string& getId() const override { return _id; }

    /**
     * @brief Obtiene el tipo de sensor
     */
    SensorType getType() const override { return _type; }

    /**
     * @brief Obtiene el protocolo de comunicación
     */
    CommunicationProtocol getProtocol() const override { return CommunicationProtocol::ONE_WIRE; }

    /**
     * @brief Obtiene los requerimientos de alimentación
     */
    PowerRequirement getPowerRequirement() const override { return PowerRequirement::POWER_3V3_SWITCHED; }

private:
};

#endif