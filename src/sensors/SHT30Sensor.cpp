#include "sensors/SHT30Sensor.h"

SHT30Sensor::SHT30Sensor(const std::string& id) {
    this->_id = id;
    this->_type = SHT30;
}
    bool SHT30Sensor::begin() {
    _initialized = sht30Sensor.begin();
    if (_initialized) {
        sht30Sensor.reset();
    }
    return _initialized;
}
    SensorReading SHT30Sensor::read() {
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
   
    for (int i = 0;
    i < 15; i++) {
    if (sht30Sensor.read()) {
    float temp = sht30Sensor.getTemperature();
    float hum = sht30Sensor.getHumidity();
    if (temp != 0.0f && hum != 0.0f && temp > -40.0f && temp < 125.0f && hum > 0.0f && hum <= 100.0f) {
                reading.value = temp;
            reading.subValues.push_back({temp});
                reading.subValues.push_back({hum});
                return reading;
            }
        }
        delay(1);    }    reading.value = NAN;
    reading.subValues.push_back({NAN});
    reading.subValues.push_back({NAN});
    return reading;
}
