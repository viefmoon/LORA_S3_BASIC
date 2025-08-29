#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <memory>
#include "sensors/ISensor.h"
#include "sensor_types.h"
#include "config_manager.h"
#include "sensors/ModbusSensor.h"
#include "ModbusSensorManager.h"
#include <ESP32Time.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "SHT31.h"
#include <SPI.h>
#include <Adafruit_MAX31865.h>
#include <Adafruit_VEML7700.h>
#include <map>
#include <string>
#include <Adafruit_BME680.h>
#include <Adafruit_BME280.h>

extern ESP32Time rtc;
extern SPIClass spiLora;
extern SPISettings spiAdcSettings;
extern OneWire oneWire;
extern DallasTemperature dallasTemp;
extern SHT31 sht30Sensor;

/**
 * @brief Clase que maneja la inicialización y lecturas de todos los sensores
 *        usando el patrón de interfaz ISensor para mayor escalabilidad.
 */
class SensorManager {
  public:
    /**
     * @brief Carga las configuraciones y crea las instancias de los sensores.
     */
    void registerSensorsFromConfig();

    /**
     * @brief Inicializa todos los sensores registrados.
     */
    void beginAll();

    /**
     * @brief Lee todos los sensores y devuelve las mediciones.
     * @return Vector con todas las lecturas de sensores
     */
    std::vector<SensorReading> readAll();


    /**
     * @brief Apaga las fuentes de alimentación controlables cuando ya no se necesitan
     *        Útil para ahorrar energía después de leer los sensores
     */
    void powerDown();

  private:
    /**
     * @brief "Fábrica" para crear el objeto sensor correcto según su tipo.
     * @param config Configuración del sensor
     * @return Puntero único al sensor creado
     */
    std::unique_ptr<ISensor> createSensor(const SensorConfig& config);

    std::vector<std::unique_ptr<ISensor>> _sensors;

};

#endif
