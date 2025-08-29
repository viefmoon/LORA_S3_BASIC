#include "sensors/ConductivitySensor.h"
#include <cmath>
#include "sensors/NtcSensor.h"
#include "config.h"
#include "config_manager.h"

ConductivitySensor::ConductivitySensor(const std::string& id) {
    this->_id = id;
    this->_type = COND;
}
    bool ConductivitySensor::begin() {
    // El sensor de conductividad solo necesita configuración del ADC
    _initialized = true;
    return true;
}
    SensorReading ConductivitySensor::read() {
    SensorReading reading;
    strncpy(reading.sensorId, _id.c_str(), sizeof(reading.sensorId) - 1);
    reading.sensorId[sizeof(reading.sensorId) - 1] = '\0';
    reading.type = _type;
    if (!_initialized) {
        reading.value = NAN;
        return reading;
    }
    int adcValue = analogRead(Pins::COND_SENSOR);
    float voltage = adcValue * (3.3f / 4095.0f);
    if (isnan(voltage) || voltage < 0.0f || voltage > 3.3f) {
        reading.value = NAN;
        return reading;
    }
    float waterTemp = NtcSensor::readNtc10kTemperatureStatic();
    reading.value = convertVoltageToConductivity(voltage, waterTemp);
    return reading;
}

/**
 * @brief Convierte el voltaje medido a valor de conductividad/TDS en ppm
 *
 * @param voltage Voltaje medido del sensor
 * @param tempC Temperatura del agua en grados Celsius para compensación
 * @return float Valor de TDS en ppm (partes por millón)
 */
float ConductivitySensor::convertVoltageToConductivity(float voltage, float tempC) {
    float calTemp, coefComp, V1, T1, V2, T2, V3, T3;
    ConfigManager::getConductivityConfig(calTemp, coefComp, V1, T1, V2, T2, V3, T3);

    // Si tempC es NAN, usar la temperatura de calibración como valor por defecto
    if (isnan(tempC)) {
    tempC = calTemp;
    }

    // Matriz para resolver el sistema de ecuaciones
    // Basado en 3 puntos de calibración
    const double det = V1*V1*(V2 - V3) - V1*(V2*V2 - V3*V3) + (V2*V2*V3 - V2*V3*V3);
    if(fabs(det) > 1e-6) {
    const double a = (T1*(V2 - V3) - T2*(V1 - V3) + T3*(V1 - V2)) / det;
    const double b = (T1*(V3*V3 - V2*V2) + T2*(V1*V1 - V3*V3) + T3*(V2*V2 - V1*V1)) / det;
    const double c = (T1*(V2*V2*V3 - V2*V3*V3) - T2*(V1*V1*V3 - V1*V3*V3) + T3*(V1*V1*V2 - V1*V2*V2)) / det;

        // Aplicar compensación de temperatura
        const double compensation = 1.0 + coefComp * (tempC - calTemp);
    double compensatedVoltage = voltage / compensation;
    double conductivity = a * (compensatedVoltage * compensatedVoltage)
        + b * compensatedVoltage
        + c;
        return fmax(conductivity, 0.0);
    }
    else {
    return NAN;
    }
}
