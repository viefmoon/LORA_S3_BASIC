#include "sensors/SoilHumiditySensor.h"

SoilHumiditySensor::SoilHumiditySensor(const std::string& id) {
    this->_id = id;
    this->_type = SOILH;
}
    bool SoilHumiditySensor::begin() {
    // El sensor de humedad del suelo solo necesita configuración del ADC
    _initialized = true;
    return true;
}
    SensorReading SoilHumiditySensor::read() {
    SensorReading reading;
    strncpy(reading.sensorId, _id.c_str(), sizeof(reading.sensorId) - 1);
    reading.sensorId[sizeof(reading.sensorId) - 1] = '\0';
    reading.type = _type;
    if (!_initialized) {
        reading.value = NAN;
        return reading;
    }
    int adcValue = analogRead(Pins::SOILH_SENSOR);
    float voltage = adcValue * (3.3f / 4095.0f);
    if (voltage <= 0.0f || voltage >= 3.3f) {
        reading.value = NAN;
    } else {
        reading.value = (voltage / 3.3f) * 100.0f;
    }
    DEBUG_PRINTF("SOILH ADC: %d, voltaje: %.3f, valor: %.3f%%\n", adcValue, voltage, reading.value);
return reading;
}
