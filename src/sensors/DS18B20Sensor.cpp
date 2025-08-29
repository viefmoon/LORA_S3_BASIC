#include "sensors/DS18B20Sensor.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// Objetos locales para el sensor DS18B20
static OneWire oneWire(Pins::ONE_WIRE_BUS);
static DallasTemperature dallasTemp(&oneWire);

DS18B20Sensor::DS18B20Sensor(const std::string& id) {
    this->_id = id;
    this->_type = DS18B20;
}
    bool DS18B20Sensor::begin() {
    // El bus OneWire ya debe estar inicializado en main
    // Solo necesitamos comenzar la comunicación con el sensor
    dallasTemp.begin();
    if (dallasTemp.getDeviceCount() == 0) {
    _initialized = false;
        return false;
    }

    // Hacer una lectura de prueba
    dallasTemp.requestTemperatures();
    float temp = dallasTemp.getTempCByIndex(0);
    _initialized = (temp != DEVICE_DISCONNECTED_C);
    return _initialized;
}
    SensorReading DS18B20Sensor::read() {
    SensorReading reading;
    strncpy(reading.sensorId, _id.c_str(), sizeof(reading.sensorId) - 1);
    reading.sensorId[sizeof(reading.sensorId) - 1] = '\0';
    reading.type = _type;
    if (!_initialized) {
        reading.value = NAN;
        return reading;
    }

    dallasTemp.requestTemperatures();
    float temp = dallasTemp.getTempCByIndex(0);
    if (temp == DEVICE_DISCONNECTED_C) {
        reading.value = NAN;
    } else {
        reading.value = temp;
    }
return reading;
}
