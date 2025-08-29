#include "sensors/HDS10Sensor.h"
#include <cmath>
#include "config.h"

HDS10Sensor::HDS10Sensor(const std::string& id) {
    this->_id = id;
    this->_type = HDS10;
}
    bool HDS10Sensor::begin() {
    // El sensor HDS10 solo necesita configuración del ADC
    _initialized = true;
    return true;
}
    SensorReading HDS10Sensor::read() {
    SensorReading reading;
    strncpy(reading.sensorId, _id.c_str(), sizeof(reading.sensorId) - 1);
    reading.sensorId[sizeof(reading.sensorId) - 1] = '\0';
    reading.type = _type;
    if (!_initialized) {
        reading.value = NAN;
        return reading;
    }
    int adcValue = analogRead(Pins::HDS10_SENSOR);
    float voltage = adcValue * (3.3f / 4095.0f);
    const float R_ref = 10000.0f; // Resistencia de referencia 10kΩ
    float resistance = R_ref * ((3.3f / voltage) - 1.0f);
    reading.value = convertResistanceToHumidity(resistance);
    return reading;
}

/**
 * @brief Convierte la resistencia del sensor HDS10 a porcentaje de humedad usando interpolación logarítmica
 * @param sensorR Resistencia del sensor en ohms
 * @return Porcentaje de humedad relativa (50-100%)
 */
float HDS10Sensor::convertResistanceToHumidity(float sensorR) {
    // Tabla de valores aproximados de la curva "Average" del sensor
    // Valores en kΩ vs. %HR
    static const float Rvals[] = { 1.0f,   2.0f,   5.0f,   10.0f,  50.0f,  100.0f, 200.0f };
    static const float Hvals[] = { 50.0f, 60.0f, 70.0f,  80.0f, 90.0f, 95.0f, 100.0f };
    static const int   NPOINTS = sizeof(Rvals)/sizeof(Rvals[0]);

    // Pasar ohms a kΩ
    float Rk = sensorR * 1e-3f;
    if (Rk <= Rvals[0]) {
    return Hvals[0];
    }
    if (Rk >= Rvals[NPOINTS-1]) {
    return Hvals[NPOINTS-1];
    }
   
    for (int i=0;
    i < NPOINTS-1; i++) {
    float R1 = Rvals[i];
    float R2 = Rvals[i+1];
    if (Rk >= R1 && Rk <= R2) {
    float logR   = log10(Rk);
    float logR1  = log10(R1);
    float logR2  = log10(R2);
    float HR1 = Hvals[i];
    float HR2 = Hvals[i+1];
    float humidity = HR1 + (HR2 - HR1) * ((logR - logR1) / (logR2 - logR1));
                return humidity;
        }
    }
    return Hvals[NPOINTS-1];
}

