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
#include "config/system_config.h"
#include "PowerManager.h"
#include "SHT31.h"
#include "sensor_types.h"
#include <vector>
#include <Adafruit_BME680.h>
#include <Adafruit_BME280.h>
#include <Adafruit_VEML7700.h>
#include <SensirionI2cSht4x.h>
#include <map>
#include <string>

extern SHT31 sht30Sensor;
extern Adafruit_BME680 bme680Sensor;
extern Adafruit_BME280 bme280Sensor;
extern Adafruit_VEML7700 veml7700Sensor;
extern SensirionI2cSht4x sht40Sensor;
extern std::map<std::string, bool> sensorInitStatus;

class HardwareManager {
public:
    /**
     * @brief Inicializa el hardware básico del sistema: pines GPIO, buses I2C/SPI y Power Manager.
     * 
     * Este método configura el hardware de bajo nivel pero ya no inicializa sensores específicos,
     * esa responsabilidad se ha trasladado a SensorManager.
     * 
     * @param powerManager Referencia al gestor de energía
     * @param sht30Sensor Referencia al sensor SHT30 (no se inicializa aquí)
     * @param bme680Sensor Referencia al sensor BME680 (no se inicializa aquí)
     * @param bme280Sensor Referencia al sensor BME280 (no se inicializa aquí)
     * @param veml7700Sensor Referencia al sensor VEML7700 (no se inicializa aquí)
     * @param sht40Sensor Referencia al sensor SHT40 (no se inicializa aquí)
     * @param spiLora Referencia a la interfaz SPI para LoRa
     * @param enabledNormalSensors Vector con las configuraciones de sensores habilitados
     * @return true si la inicialización fue exitosa, false en caso contrario
     */
    static bool initHardware(PowerManager& powerManager, 
                           SHT31& sht30Sensor, 
                           Adafruit_BME680& bme680Sensor,
                           Adafruit_BME280& bme280Sensor,
                           Adafruit_VEML7700& veml7700Sensor,
                           SensirionI2cSht4x& sht40Sensor,
                           SPIClass& spiLora,
                           const std::vector<SensorConfig>& enabledNormalSensors);

    /**
     * @brief Inicializa los pines de selección SPI (SS)
     */
    static void initializeSPISSPins();

};

#endif // HARDWARE_MANAGER_H
