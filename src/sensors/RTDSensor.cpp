#include "sensors/RTDSensor.h"

RTDSensor::RTDSensor(const std::string& id) {
    this->_id = id;
    this->_type = RTD;
}
    bool RTDSensor::begin() {
    rtdSensor.begin(MAX31865_3WIRE);
    uint8_t fault = rtdSensor.readFault();
    if (fault) {
        rtdSensor.clearFault();
    _initialized = false;
        return false;
    }
    _initialized = true;
    return true;
}
    SensorReading RTDSensor::read() {
    SensorReading reading;
    strncpy(reading.sensorId, _id.c_str(), sizeof(reading.sensorId) - 1);
    reading.sensorId[sizeof(reading.sensorId) - 1] = '\0';
    reading.type = _type;
    if (!_initialized) {
        reading.value = NAN;
        return reading;
    }
    float temp = rtdSensor.temperature(RNOMINAL, RREF);
    uint8_t fault = rtdSensor.readFault();
    if (fault) {
        rtdSensor.clearFault();
        reading.value = NAN;
    }
    else if (isnan(temp) || temp < -200.0f || temp > 850.0f) {        reading.value = NAN;
    } else {
        reading.value = temp;
    }
return reading;
}
