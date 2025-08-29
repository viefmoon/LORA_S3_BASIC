#include "sensors/BME280Sensor.h"

BME280Sensor::BME280Sensor(const std::string& id) {
    this->_id = id;
    this->_type = BME280;
}
    bool BME280Sensor::begin() {
    _initialized = bme280Sensor.begin(Sensors::BME280_I2C_ADDR, &Wire);
    if (_initialized) {        bme280Sensor.setSampling(Adafruit_BME280::MODE_FORCED,
        Adafruit_BME280::SAMPLING_X1, // temperature
        Adafruit_BME280::SAMPLING_X1, // pressure
        Adafruit_BME280::SAMPLING_X1, // humidity
        Adafruit_BME280::FILTER_OFF);
    }
    return _initialized;
}
    SensorReading BME280Sensor::read() {
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
    }    bme280Sensor.takeForcedMeasurement();
    float temp = bme280Sensor.readTemperature();
    float hum = bme280Sensor.readHumidity();
    float pressure = bme280Sensor.readPressure() / 100.0f;
    if (isnan(temp) || isnan(hum) || isnan(pressure)) {
        reading.value = NAN;
            reading.subValues.push_back({NAN});
        reading.subValues.push_back({NAN});
        reading.subValues.push_back({NAN});
    } else {
        reading.value = temp;
    reading.subValues.push_back({temp});
    reading.subValues.push_back({hum});        reading.subValues.push_back({pressure});    }
return reading;
}
