#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <vector>
#include "sensor_types.h"
#include <ESP32Time.h>
#include "PowerManager.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "SHT31.h"
#include "ModbusSensorManager.h"
#include "sensors/NtcManager.h"
#include <SPI.h>
#include <Adafruit_MAX31865.h>
#include <Adafruit_VEML7700.h>
#include <map>
#include <string>

// Variables y objetos globales declarados en main.cpp
extern ESP32Time rtc;
extern PowerManager powerManager;
extern SPIClass spiLora;
extern SPISettings spiAdcSettings;
extern OneWire oneWire;
extern DallasTemperature dallasTemp;
extern SHT31 sht30Sensor;
extern Adafruit_MAX31865 rtdSensor;
extern Adafruit_VEML7700 veml7700;
extern std::map<std::string, bool> sensorInitStatus; // Declaración externa del mapa de estado

/**
 * @brief Clase que maneja la inicialización y lecturas de todos los sensores
 *        incluyendo sensores normales y Modbus.
 */
class SensorManager {
  public:
    /**
     * @brief Inicializa todos los sensores habilitados en el sistema.
     * 
     * Este método se encarga de:
     * 1. Inicializar sensores I2C (SHT30, SHT4X, CO2, BME680, BME280, VEML7700)
     * 2. Inicializar sensores RTD si están habilitados
     * 3. Inicializar sensores DS18B20 si están habilitados
     * 4. Configurar pines analógicos para sensores específicos (NTC, pH, conductividad, etc.)
     * 5. Configurar el ADC interno
     * 
     * @param enabledNormalSensors Vector con las configuraciones de sensores habilitados
     */
    static void beginSensors(const std::vector<SensorConfig>& enabledNormalSensors);

    // Devuelve la lectura (o lecturas) de un sensor NO-Modbus según su configuración.
    static SensorReading getSensorReading(const SensorConfig& cfg);
    
    // Devuelve la lectura de un sensor Modbus según su configuración
    static ModbusSensorReading getModbusSensorReading(const ModbusSensorConfig& cfg);
    
    // Obtiene todas las lecturas de sensores (normales y Modbus) habilitados
    static void getAllSensorReadings(std::vector<SensorReading>& normalReadings,
                                    std::vector<ModbusSensorReading>& modbusReadings,
                                    const std::vector<SensorConfig>& enabledNormalSensors,
                                    const std::vector<ModbusSensorConfig>& enabledModbusSensors);

  private:
    // Métodos de lectura internos
    static float readSensorValue(const SensorConfig &cfg, SensorReading &reading);
};

#endif // SENSOR_MANAGER_H
