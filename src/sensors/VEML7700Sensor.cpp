#include "sensors/VEML7700Sensor.h"

static Adafruit_VEML7700 veml7700;

VEML7700Sensor::VEML7700Sensor(const std::string& id) {
    this->_id = id;
    this->_type = VEML7700;
}
    bool VEML7700Sensor::begin() {
    for (int i = 0;
    i < 3; i++) {
    _initialized = veml7700.begin();
    if (_initialized) break;
        delay(5);    }
    if (!_initialized) {
    return false;
    }    veml7700.setGain(VEML7700_GAIN_1_8);

    veml7700.setIntegrationTime(VEML7700_IT_25MS);
    return true;
}
    SensorReading VEML7700Sensor::read() {
    SensorReading reading;
    strncpy(reading.sensorId, _id.c_str(), sizeof(reading.sensorId) - 1);
    reading.sensorId[sizeof(reading.sensorId) - 1] = '\0';
    reading.type = _type;
    if (!_initialized) {
        reading.value = NAN;
        return reading;
    }

    float lux = veml7700.readLux();
    if (isnan(lux) || lux < 0.0f) {
        reading.value = NAN;
    } else {
        reading.value = lux;
    }
return reading;
}
