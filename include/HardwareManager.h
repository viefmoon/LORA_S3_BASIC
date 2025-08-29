/*******************************************************************************************
 * Archivo: include/HardwareManager.h
 * Descripción: Gestión de inicialización y configuración del hardware del sistema.
 * Incluye funciones para inicialización de periféricos y control de energía.
 *******************************************************************************************/

#ifndef HARDWARE_MANAGER_H
#define HARDWARE_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "config.h"
#include "PowerManager.h"
#include "sensors/ISensor.h"
#include "sensor_types.h"
#include <vector>
#include <map>
#include <string>

class HardwareManager {
public:
    /**
     * @brief Inicializa el hardware básico del sistema (GPIO, LED, Power)
     */
    static void initialize();

    /**
     * @brief Inicializa un bus de comunicación específico si no lo ha hecho ya
     * @param protocol El protocolo de comunicación a inicializar
     */
    static void initializeBus(CommunicationProtocol protocol);

    /**
     * @brief Inicializa el hardware básico del sistema (GPIO, I2C, SPI, etc.) - Compatibilidad
     * @param spiLora Referencia al bus SPI para el módulo LoRa
     * @param enabledNormalSensors Vector de sensores habilitados en la configuración
     * @return true si la inicialización fue exitosa, false en caso contrario
     */
    static bool initHardware(SPIClass& spiLora,
                           const std::vector<SensorConfig>& enabledNormalSensors);

    /**
     * @brief Inicializa los pines CS/SS para los dispositivos SPI
     */
    static void initializeSPISSPins();

private:
    static bool i2cInitialized;
    static bool oneWireInitialized;
    static bool modbusInitialized;
    static bool analogInitialized;
    static bool spiInitialized;
};

#endif // HARDWARE_MANAGER_H
