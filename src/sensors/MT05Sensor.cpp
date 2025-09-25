#include "sensors/MT05Sensor.h"
#include <OneWire.h>

MT05Sensor::MT05Sensor(const std::string& id) {
    this->_id = id;
    this->_type = MT05S;
}

bool MT05Sensor::begin() {
    _initialized = true;
    return true;
}

SensorReading MT05Sensor::read() {
    SensorReading reading;
    strncpy(reading.sensorId, _id.c_str(), sizeof(reading.sensorId) - 1);
    reading.sensorId[sizeof(reading.sensorId) - 1] = '\0';
    reading.type = _type;
    reading.subValues.clear();

    if (!_initialized) {
        SubValue temp, humidity, conductivity;
        temp.value = NAN;
        humidity.value = NAN;
        conductivity.value = NAN;
        reading.subValues.push_back(temp);
        reading.subValues.push_back(humidity);
        reading.subValues.push_back(conductivity);
        return reading;
    }

    // Crear instancia OneWire local para esta lectura
    OneWire ds(Pins::ONE_WIRE_BUS);

    // Configurar el sensor MT05S
    ds.reset();
    ds.skip();
    ds.write(0x4E);  // Write Scratchpad
    ds.write(0x14);  // CONFIG0: 0x14 = Habilitar temperatura, humedad y conductividad
    ds.write(0x00);  // CONFIG1: Por defecto
    ds.write(0x00);  // Dummy byte requerido por el protocolo
    delay(10);
    // Iniciar conversión de todas las mediciones
    ds.reset();
    ds.skip();
    ds.write(0x44);  // Convert T

    // Esperar conversión completa (100ms típico según datasheet)
    delay(110);

    // Leer los resultados
    ds.reset();
    ds.skip();
    ds.write(0xBE);  // Read Scratchpad

    uint8_t scratchpad[9];
    for (int i = 0; i < 9; i++) {
        scratchpad[i] = ds.read();
    }

    #ifdef DEBUG_MODE
    DEBUG_PRINT("MT05S Scratchpad: ");
    for (int i = 0; i < 9; i++) {
        DEBUG_PRINTF("%02X ", scratchpad[i]);
    }
    DEBUG_PRINTLN("");
    #endif

    // Verificar integridad de datos con CRC
    if (OneWire::crc8(scratchpad, 8) != scratchpad[8]) {
        DEBUG_PRINTLN("MT05S: Error CRC");
        SubValue temp, humidity, conductivity;
        temp.value = NAN;
        humidity.value = NAN;
        conductivity.value = NAN;
        reading.subValues.push_back(temp);
        reading.subValues.push_back(humidity);
        reading.subValues.push_back(conductivity);
        // Reset final para forzar standby inmediato
        ds.reset();
        return reading;
    }

    // Interpretar datos (formato little-endian, valores en centésimas)
    int16_t temperature = makeWord(scratchpad[0], scratchpad[1]);
    int16_t moisture = makeWord(scratchpad[2], scratchpad[3]);
    int16_t conductivity = makeWord(scratchpad[4], scratchpad[5]);

    // Convertir a valores reales
    SubValue temp, humidity, conductivityVal;
    temp.value = temperature / 100.0f;      // Temperatura en °C
    humidity.value = moisture / 100.0f;      // Humedad en %
    conductivityVal.value = conductivity;    // Conductividad en µS/cm

    reading.subValues.push_back(temp);
    reading.subValues.push_back(humidity);
    reading.subValues.push_back(conductivityVal);

    DEBUG_PRINTF("MT05S: T=%.2f°C, H=%.2f%%, C=%d µS/cm\n",
                 temp.value, humidity.value, (int)conductivityVal.value);

    // Reset final para forzar al sensor a standby inmediato
    // Esto evita el timeout de 500ms esperando el siguiente comando
    ds.reset();

    return reading;
}