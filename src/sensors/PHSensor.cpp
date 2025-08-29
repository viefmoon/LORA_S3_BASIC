#include "sensors/PHSensor.h"
#include <cmath>
#include "sensors/NtcSensor.h"
#include "config.h"
#include "config_manager.h"

PHSensor::PHSensor(const std::string& id) {
    this->_id = id;
    this->_type = PH;
}
    bool PHSensor::begin() {
    // El sensor de pH solo necesita configuración del ADC
    // que se hace en HardwareManager cuando se llama initializeBus(ANALOG)
    _initialized = true;
    return true;
}
    SensorReading PHSensor::read() {
    SensorReading reading;
    strncpy(reading.sensorId, _id.c_str(), sizeof(reading.sensorId) - 1);
    reading.sensorId[sizeof(reading.sensorId) - 1] = '\0';
    reading.type = _type;
    if (!_initialized) {
        reading.value = NAN;
        return reading;
    }
    int adcValue = analogRead(Pins::PH_SENSOR);
    float voltage = adcValue * (3.3f / 4095.0f);

    // Ajuste del offset: en el sistema anterior, un pH neutro daba un voltaje
    // cercano a 0V, pero ahora puede necesitar un offset diferente
    // dependiendo de cómo esté conectado el circuito
    voltage = voltage - 1.65f;
    if (isnan(voltage) || voltage < -2.5f || voltage > 2.5f) {
        reading.value = NAN;
        return reading;
    }
    float waterTemp = NtcSensor::readNtc10kTemperatureStatic();
    reading.value = convertVoltageToPH(voltage, waterTemp);
    return reading;
}

/**
 * @brief Convierte el voltaje medido a valor de pH
 *
 * @param voltage Voltaje medido del sensor de pH
 * @param tempC Temperatura del agua en grados Celsius para compensación
 * @return float Valor de pH (0-14)
 */
float PHSensor::convertVoltageToPH(float voltage, float tempC) {
    float V1, T1, V2, T2, V3, T3, TEMP_CAL;
    ConfigManager::getPHConfig(V1, T1, V2, T2, V3, T3, TEMP_CAL);

    // Si solutionTemp es NAN, usar la temperatura de calibración como valor por defecto
    if (isnan(tempC)) {
    tempC = TEMP_CAL;
    }
    const double pH_calib[] = {T1, T2, T3};
    const double V_calib[] = {V1, V2, V3};
    const int n = 3;
    double sum_pH = 0.0;
    double sum_V = 0.0;
    double sum_pHV = 0.0;
    double sum_pH2 = 0.0;
    for (int i = 0;
    i < n; i++) {
        sum_pH += pH_calib[i];
        sum_V += V_calib[i];
        sum_pHV += pH_calib[i] * V_calib[i];
        sum_pH2 += pH_calib[i] * pH_calib[i];
    }
    double S_CAL = ((n * sum_pHV) - (sum_pH * sum_V)) / ((n * sum_pH2) - (sum_pH * sum_pH));
    double E0 = ((sum_V) + (S_CAL * sum_pH)) / n;
    const double tempK = (tempC + 273.15);
    const double tempCalK = (TEMP_CAL + 273.15);
    const double S_T = S_CAL * (tempK / tempCalK);
    double pH = ((E0 + voltage) / S_T);
    pH = constrain(pH, 0.0, 14.0);
    return pH;
}

