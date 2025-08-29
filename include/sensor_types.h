#ifndef SENSOR_TYPES_H
#define SENSOR_TYPES_H

#include <stdint.h>
#include <vector>
#include <map>

#include "config.h"

/************************************************************************
 * TIEMPOS DE ESTABILIZACIÓN PARA SENSORES MODBUS (en ms)
 ************************************************************************/

#define MODBUS_ENV4_STABILIZATION_TIME 5000

/**
 * @brief Estructura para variables múltiples en un solo sensor.
 *        Por ejemplo, un sensor SHT30 que da Temperature y Humidity.
 */
struct SubValue {
    float value;
};

/************************************************************************
 * SECCIÓN PARA SENSORES ESTÁNDAR (NO MODBUS)
 ************************************************************************/

/**
 * @brief Enumeración de tipos de sensores disponibles.
 */
enum SensorType {
    N100K,    // NTC 100K
    N10K,     // NTC 10K
    HDS10,    // Condensation Humidity
    RTD,      // RTD
    DS18B20,  // DS18B20
    PH,       // pH
    COND,     // Conductividad
    SOILH,    // Soil Humidity
    VEML7700, // Sensor de luz VEML7700 (Lux)
    BATTERY,  // Battery voltage sensor

    SHT30 = 100,  // Sensor SHT30: [0]=Temperatura(°C), [1]=Humedad(%)
    BME680 = 101, // Sensor BME680: [0]=Temp(°C), [1]=Hum(%), [2]=Press(hPa), [3]=Gas(KOhms)
    CO2 = 102,    // Sensor CO2 SCD4x: [0]=CO2(ppm), [1]=Temp(°C), [2]=Hum(%)
    BME280 = 103, // Sensor BME280: [0]=Temp(°C), [1]=Hum(%), [2]=Press(hPa)
    SHT40 = 104,  // Sensor SHT40: [0]=Temperatura(°C), [1]=Humedad(%)

    ENV4 = 110,   // Sensor ambiental 4 en 1: [0]=Humedad(%), [1]=Temperatura(°C), [2]=Presión(kPa), [3]=Iluminación(lux)
};

/**
 * @brief Estructura básica para almacenar una lectura de sensor.
 *        Para sensores con múltiples variables, se utilizará 'subValues'.
 */
struct SensorReading {
    char sensorId[20];         // Identificador del sensor (ej. "SHT30_1")
    SensorType type;           // Tipo de sensor
    float value;               // Valor único (si aplica)
    std::vector<SubValue> subValues;};

/**
 * @brief Estructura de configuración para sensores "normales" (no Modbus).
 */
struct SensorConfig {
    char configKey[20];
    char sensorId[20];
    SensorType type;
    bool enable;
};

/************************************************************************
 * SECCIÓN PARA SENSORES MODBUS
 ************************************************************************/

/**
 * @brief Estructura de configuración para sensores Modbus.
 */
struct ModbusSensorConfig {
    char sensorId[20];         // Identificador del sensor
    SensorType type;           // Tipo de sensor Modbus
    uint8_t address;           // Dirección Modbus del dispositivo
    bool enable;               // Si está habilitado o no
};

/**
 * @brief Estructura para almacenar la lectura completa de un sensor Modbus.
 */
struct ModbusSensorReading {
    char sensorId[20];         // Identificador del sensor
    SensorType type;           // Tipo de sensor Modbus
    std::vector<SubValue> subValues;};

#endif
