#include "sensors/BatterySensor.h"
#include "config.h" // Para las constantes de configuración

BatterySensor::BatterySensor(const std::string& id) {
    this->_id = id;
    this->_type = BATTERY;
}
    bool BatterySensor::begin() {
    pinMode(Pins::BATTERY_CONTROL, OUTPUT);
    digitalWrite(Pins::BATTERY_CONTROL, HIGH);
    _initialized = true;
    return true;
}
    SensorReading BatterySensor::read() {
    SensorReading reading;
    strncpy(reading.sensorId, _id.c_str(), sizeof(reading.sensorId) - 1);
    reading.sensorId[sizeof(reading.sensorId) - 1] = '\0';
    reading.type = _type;
    if (!_initialized) {
        reading.value = NAN;
        return reading;
    }
    digitalWrite(Pins::BATTERY_CONTROL, LOW);
    delay(10);
    int milliVolts = analogReadMilliVolts(Pins::BATTERY_SENSOR);
    digitalWrite(Pins::BATTERY_CONTROL, HIGH);
    float voltage = milliVolts / 1000.0f;
    if (isnan(voltage) || voltage <= 0.0f || voltage >= 3.3f) {
        reading.value = NAN;
        return reading;
    }    reading.value = calculateBatteryVoltage(voltage);
    return reading;
}

/**
 * @brief Calcula el voltaje real de la batería a partir de la lectura del ADC
 *
 * El circuito es un divisor de voltaje con:
 * R1 = 100k (a GND)
 * R2 = 390k (a batería)
 * VBAT = VADC_IN1 * (R1 + R2) / R1 = VADC_IN1 * (100k + 390k) / 100k
 *
 * @param adcVoltage Voltaje medido por el ADC
 * @return float Voltaje real de la batería
 */
float BatterySensor::calculateBatteryVoltage(float adcVoltage) {
    // VBAT = 100k / (100k+390k) * VADC_IN1 corregido a:
    // VBAT = VADC_IN1 / (100k / (100k+390k))
    return adcVoltage / (Calibration::BATTERY_R1 / (Calibration::BATTERY_R1 + Calibration::BATTERY_R2));
}

