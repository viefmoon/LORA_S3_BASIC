#include "sensors/SHT40Sensor.h"

SHT40Sensor::SHT40Sensor(const std::string& id) {
    this->_id = id;
    this->_type = SHT40;
}
    bool SHT40Sensor::begin() {
    // Inicializar el objeto SHT40 con el bus I2C y la dirección
    sht40Sensor.begin(Wire, SHT40_I2C_ADDR_44);

    // Hacer un reset para asegurar que el sensor está en un estado limpio
    int16_t error = sht40Sensor.softReset();
    if (error != 0) {
    _initialized = false;
        return false;
    }

    // Realizar una lectura de prueba
    float temp, hum;
    error = sht40Sensor.measureHighPrecision(temp, hum);
    if (error != 0) {
    _initialized = false;
        return false;
    }
    _initialized = true;
    return true;
}
    SensorReading SHT40Sensor::read() {
    SensorReading reading;
    strncpy(reading.sensorId, _id.c_str(), sizeof(reading.sensorId) - 1);
    reading.sensorId[sizeof(reading.sensorId) - 1] = '\0';
    reading.type = _type;
    if (!_initialized) {
        reading.value = NAN;
            reading.subValues.push_back({NAN}); // Temperatura
        reading.subValues.push_back({NAN}); // Humedad
        return reading;
    }
    float temp, hum;
    for (int i = 0;
    i < 3; i++) {
    int16_t error = sht40Sensor.measureHighPrecision(temp, hum);
    if (error == 0) {
    if (!isnan(temp) && !isnan(hum) &&
                temp > -40.0f && temp < 125.0f &&
                hum >= 0.0f && hum <= 100.0f) {
                reading.value = temp;
            reading.subValues.push_back({temp});
                reading.subValues.push_back({hum});
                return reading;
            }
        }
        delay(5);    }    reading.value = NAN;
    reading.subValues.push_back({NAN});
    reading.subValues.push_back({NAN});
    return reading;
}
