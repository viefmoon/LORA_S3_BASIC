#ifndef SHT4X_SENSOR_H
#define SHT4X_SENSOR_H

#include <Arduino.h>
#include "config.h"
#include "debug.h"
#include <SensirionI2cSht4x.h>

// Variable externa del sensor SHT4x que será declarada en otro lugar
extern SensirionI2cSht4x sht4xSensor;

/**
 * @brief Clase para manejar el sensor de temperatura y humedad SHT4x
 */
class SHT4xSensor {
public:
    /**
     * @brief Inicializa el sensor SHT4x
     * 
     * @return true si se inicializó correctamente, false en caso contrario
     */
    static bool begin();

    /**
     * @brief Lee temperatura y humedad del sensor SHT4x
     * 
     * @param outTemp Variable donde se almacenará la temperatura en °C
     * @param outHum Variable donde se almacenará la humedad relativa en %
     * @return true si la lectura fue exitosa, false en caso contrario
     */
    static bool read(float &outTemp, float &outHum);
};

#endif // SHT4X_SENSOR_H 