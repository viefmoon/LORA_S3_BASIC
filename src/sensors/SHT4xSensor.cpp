#include "sensors/SHT4xSensor.h"

/**
 * @brief Inicializa el sensor SHT4x
 * 
 * @return true si se inicializó correctamente, false en caso contrario
 */
bool SHT4xSensor::begin() {
    // Inicializar el objeto SHT4x con el bus I2C y la dirección
    sht4xSensor.begin(Wire, SHT40_I2C_ADDR_44);
    
    // Hacer un reset para asegurar que el sensor está en un estado limpio
    int16_t error = sht4xSensor.softReset();
    if (error != 0) {
        return false;
    }
    
    // Realizar una lectura de prueba
    float temp, hum;
    error = sht4xSensor.measureHighPrecision(temp, hum);
    if (error != 0) {
        return false;
    }
    
    return true;
}

/**
 * @brief Lee temperatura y humedad del sensor SHT4x
 * 
 * @param outTemp Variable donde se almacenará la temperatura en °C
 * @param outHum Variable donde se almacenará la humedad relativa en %
 * @return true si la lectura fue exitosa, false en caso contrario
 */
bool SHT4xSensor::read(float &outTemp, float &outHum) {
    // Intentar hasta 3 veces obtener una lectura válida
    for (int i = 0; i < 3; i++) {
        int16_t error = sht4xSensor.measureHighPrecision(outTemp, outHum);
        
        if (error == 0) {
            // Verificar que los valores sean válidos (no cero y dentro de rangos razonables)
            if (!isnan(outTemp) && !isnan(outHum) && 
                outTemp > -40.0f && outTemp < 125.0f && 
                outHum >= 0.0f && outHum <= 100.0f) {
                return true; // Retornar inmediatamente con la primera lectura válida
            }
        }
        
        delay(5); // Pequeña pausa entre intentos
    }
    
    // Si no se encontró ninguna lectura válida
    outTemp = NAN;
    outHum = NAN;
    return false;
} 