#include "sensors/NtcSensor.h"
#include <cmath>  // Para fabs() y otras funciones matemáticas
#include "config_manager.h"
#include "debug.h"

NtcSensor::NtcSensor(const std::string& id, SensorType type, const char* configKey) {
    this->_id = id;
    this->_type = type;
    this->_configKey = configKey;
}
    bool NtcSensor::begin() {
    // Los sensores NTC solo necesitan configuración del ADC
    // que se hace en HardwareManager cuando se llama initializeBus(ANALOG)
    _initialized = true;
    return true;
}
    SensorReading NtcSensor::read() {
    SensorReading reading;
    strncpy(reading.sensorId, _id.c_str(), sizeof(reading.sensorId) - 1);
    reading.sensorId[sizeof(reading.sensorId) - 1] = '\0';
    reading.type = _type;
    if (!_initialized) {
        reading.value = NAN;
        return reading;
    }
    if (_type == N100K) {
        reading.value = readNtc100kTemperature();
    }
    else if (_type == N10K) {
        reading.value = readNtc10kTemperature();
    } else {
        reading.value = NAN;
    }
return reading;
}
float NtcSensor::readNtc100kTemperature() {
    if (_configKey == nullptr) {
        return NAN;
    }
    
    double t1=25.0, r1=100000.0, t2=35.0, r2=64770.0, t3=45.0, r3=42530.0;
    ConfigManager::getNTC100KConfig(t1, r1, t2, r2, t3, r3);
    double T1K = t1 + 273.15;
    double T2K = t2 + 273.15;
    double T3K = t3 + 273.15;
    double A=0, B=0, C=0;
    calculateSteinhartHartCoeffs(T1K, r1, T2K, r2, T3K, r3, A, B, C);
    
    int ntcPin = -1;
    if (strcmp(_configKey, "0") == 0 || strcmp(_configKey, "1") == 0) {
        ntcPin = Pins::NTC100K;
    } else {
        return NAN;
    }
    
    int adcValue = analogReadMilliVolts(ntcPin);
    float voltage = adcValue / 1000.0f;
    if (isnan(voltage) || voltage <= 0.0f || voltage >= 3.0f) {
        return NAN;
    }
    
    double vRef = 3.0;
    double rFixed = 100000.0;
    bool ntcTop = true;
    double Rntc = computeNtcResistanceFromVoltageDivider(voltage, vRef, rFixed, ntcTop);
    if (Rntc <= 0.0) {
        return NAN;
    }
    
    double tempC = steinhartHartTemperature(Rntc, A, B, C);
    if (isnan(tempC) || tempC < Sensors::NTC_TEMP_MIN || tempC > Sensors::NTC_TEMP_MAX) {
        return NAN;
    }
    return tempC;
}

float NtcSensor::readNtc10kTemperature() {
    // Delegar al método estático
    return readNtc10kTemperatureStatic();
}

// Método estático para uso externo
float NtcSensor::readNtc10kTemperatureStatic() {
    double t1=25.0, r1=10000.0, t2=50.0, r2=3893.0, t3=85.0, r3=1218.0;
    ConfigManager::getNTC10KConfig(t1, r1, t2, r2, t3, r3);
    double T1K = t1 + 273.15;
    double T2K = t2 + 273.15;
    double T3K = t3 + 273.15;
    double A=0, B=0, C=0;
    calculateSteinhartHartCoeffs(T1K, r1, T2K, r2, T3K, r3, A, B, C);
    
    int adcValue = analogReadMilliVolts(Pins::NTC10K);
    float voltage = adcValue / 1000.0f;
    if (isnan(voltage) || voltage <= 0.0f || voltage >= 3.0f) {
        return NAN;
    }
    
    double vRef = 3.0;
    double rFixed = 10000.0;
    bool ntcTop = true;
    double Rntc = computeNtcResistanceFromVoltageDivider(voltage, vRef, rFixed, ntcTop);
    if (Rntc <= 0.0) {
        return NAN;
    }
    
    double tempC = steinhartHartTemperature(Rntc, A, B, C);
    if (isnan(tempC) || tempC < Sensors::NTC_TEMP_MIN || tempC > Sensors::NTC_TEMP_MAX) {
        return NAN;
    }
    return tempC;
}

void NtcSensor::calculateSteinhartHartCoeffs(double T1, double R1,
                                             double T2, double R2,
                                             double T3, double R3,
                                             double &A, double &B, double &C) {
    // 1/T = A + B*ln(R) + C*(ln(R))^3
    double L1 = log(R1);
    double L2 = log(R2);
    double L3 = log(R3);
    double Y1 = 1.0 / T1;
    double Y2 = 1.0 / T2;
    double Y3 = 1.0 / T3;
    double L1_3 = L1 * L1 * L1;
    double L2_3 = L2 * L2 * L2;
    double L3_3 = L3 * L3 * L3;
    double denominator = (L2 - L1) * (L3 - L1) * (L3 - L2);
    
    if (fabs(denominator) < 1e-10) {
        A = NAN;
        B = NAN;
        C = NAN;
        return;
    }
    
    C = ((Y2 - Y1) * (L3 - L1) - (Y3 - Y1) * (L2 - L1)) /
        ((L2_3 - L1_3) * (L3 - L1) - (L3_3 - L1_3) * (L2 - L1));
    B = ((Y2 - Y1) - C * (L2_3 - L1_3)) / (L2 - L1);
    A = Y1 - B * L1 - C * L1_3;
}

double NtcSensor::steinhartHartTemperature(double resistance, double A, double B, double C) {
    if (resistance <= 0.0) {
        return NAN;
    }
    double lnR = log(resistance);
    double invT = A + B * lnR + C * lnR*lnR*lnR;  // 1/T en Kelvin^-1
    double tempK = 1.0 / invT;                     // Kelvin
    double tempC = tempK - 273.15;                 // °C
    return tempC;
}

double NtcSensor::computeNtcResistanceFromVoltageDivider(double voltage, double vRef, double rFixed, bool ntcTop) {
    // Validación de rangos
    if (voltage <= 0.0 || voltage >= vRef) {
        return -1.0;  // Indica valor inválido
    }
    
    double Rntc;
    if (ntcTop) {
        // NTC conectado a Vref (arriba) y resistencia fija a GND (abajo)
        // Fórmula: Rntc = rFixed * (vRef - voltage) / voltage
        Rntc = rFixed * ((vRef - voltage) / voltage);
    } else {
        // NTC conectado a GND (abajo) y resistencia fija a Vref (arriba)
        // Fórmula: Rntc = rFixed * voltage / (vRef - voltage)
        Rntc = rFixed * (voltage / (vRef - voltage));
    }
    return Rntc;
}
