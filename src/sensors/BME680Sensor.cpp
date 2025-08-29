#include "sensors/BME680Sensor.h"

BME680Sensor::BME680Sensor(const std::string& id) {
    this->_id = id;
    this->_type = BME680;
}
    bool BME680Sensor::begin() {
    _initialized = bme680Sensor.begin();
    if (_initialized) {        bme680Sensor.setTemperatureOversampling(BME680_OS_8X);
        bme680Sensor.setHumidityOversampling(BME680_OS_2X);
        bme680Sensor.setPressureOversampling(BME680_OS_4X);
        bme680Sensor.setIIRFilterSize(BME680_FILTER_SIZE_3);
        bme680Sensor.setGasHeater(320, 150); // 320°C durante 150 ms
    }
    return _initialized;
}
    SensorReading BME680Sensor::read() {
    SensorReading reading;
    strncpy(reading.sensorId, _id.c_str(), sizeof(reading.sensorId) - 1);
    reading.sensorId[sizeof(reading.sensorId) - 1] = '\0';
    reading.type = _type;
    if (!_initialized) {
        reading.value = NAN;
            reading.subValues.push_back({NAN}); // Temperatura
        reading.subValues.push_back({NAN}); // Humedad
        reading.subValues.push_back({NAN}); // Presión
        return reading;
    }

    // Realizar lectura
    if (!bme680Sensor.performReading()) {
        reading.value = NAN;
            reading.subValues.push_back({NAN});
        reading.subValues.push_back({NAN});
        reading.subValues.push_back({NAN});
        return reading;
    }
    float temp = bme680Sensor.temperature;
    float hum = bme680Sensor.humidity;
    float pressure = bme680Sensor.pressure / 100.0f;
    reading.value = temp;
    reading.subValues.push_back({temp});
    reading.subValues.push_back({hum});    reading.subValues.push_back({pressure});
    return reading;
}
