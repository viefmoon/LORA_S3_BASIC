#include "sensors/CO2Sensor.h"

static SCD4x scd4x(SCD4x_SENSOR_SCD41);

CO2Sensor::CO2Sensor(const std::string& id) {
    this->_id = id;
    this->_type = CO2;
}
    bool CO2Sensor::begin() {
    _initialized = scd4x.begin(false, true, false);
    return _initialized;
}
    SensorReading CO2Sensor::read() {
    SensorReading reading;
    strncpy(reading.sensorId, _id.c_str(), sizeof(reading.sensorId) - 1);
    reading.sensorId[sizeof(reading.sensorId) - 1] = '\0';
    reading.type = _type;
    if (!_initialized) {
        reading.value = NAN;
            reading.subValues.push_back({NAN}); // CO2
        reading.subValues.push_back({NAN}); // Temperatura
        reading.subValues.push_back({NAN}); // Humedad
        return reading;
    }

    if (!scd4x.measureSingleShot()) {
        reading.value = NAN;
            reading.subValues.push_back({NAN});
        reading.subValues.push_back({NAN});
        reading.subValues.push_back({NAN});
        return reading;
    }
    uint32_t counter = 0;
    const uint32_t maxAttempts = 200;

    while (counter < maxAttempts) {
    delay(50);
    counter++;
    if (scd4x.readMeasurement()) {
            float co2 = (float)scd4x.getCO2();
    float temp = scd4x.getTemperature();
    float hum = scd4x.getHumidity();
    reading.value = co2;
            reading.subValues.push_back({co2});
                reading.subValues.push_back({temp});            reading.subValues.push_back({hum});
                return reading;
        }
    }

    reading.value = NAN;
    reading.subValues.push_back({NAN});
    reading.subValues.push_back({NAN});
    reading.subValues.push_back({NAN});
    return reading;
}
