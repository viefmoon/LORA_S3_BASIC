#ifndef NTC_SENSOR_H
#define NTC_SENSOR_H

#include <Arduino.h>
#include "sensors/ISensor.h"
#include "config.h"

class NtcSensor : public ISensor {
public:
    /**
     * @brief Constructor para sensor NTC
     * @param id Identificador del sensor
     * @param type Tipo de NTC (N100K o N10K)
     * @param configKey Clave de configuración (para N100K)
     */
    NtcSensor(const std::string& id, SensorType type, const char* configKey = nullptr);

    bool begin() override;
    SensorReading read() override;
    const std::string& getId() const override { return _id; }
    SensorType getType() const override { return _type; }
    CommunicationProtocol getProtocol() const override { return CommunicationProtocol::ANALOG_ADC; }
    PowerRequirement getPowerRequirement() const override { return PowerRequirement::POWER_3V3_SWITCHED; }

    static float readNtc10kTemperatureStatic();

private:
    const char* _configKey;

    float readNtc100kTemperature();
    float readNtc10kTemperature();
    
    /**
     * Calcula los coeficientes A, B, C para la ecuación de Steinhart-Hart
     * basado en tres pares de puntos temperatura-resistencia
     */
    static void calculateSteinhartHartCoeffs(double T1, double R1,
                                      double T2, double R2,
                                      double T3, double R3,
                                      double &A, double &B, double &C);
    
    /**
     * Calcula la temperatura usando la ecuación de Steinhart-Hart
     * @param resistance La resistencia del termistor en ohms
     * @param A,B,C Los coeficientes de Steinhart-Hart
     * @return Temperatura en grados Celsius
     */
    static double steinhartHartTemperature(double resistance, double A, double B, double C);
    
    /**
     * Calcula la resistencia del NTC basado en un divisor de voltaje
     * @param voltage Voltaje medido en el punto medio del divisor
     * @param vRef Voltaje de referencia
     * @param rFixed Resistencia fija del divisor
     * @param ntcTop true si el NTC está conectado a Vref, false si está conectado a GND
     * @return Resistencia del NTC en ohms, o -1 si hay error
     */
    static double computeNtcResistanceFromVoltageDivider(double voltage, double vRef, double rFixed, bool ntcTop);
};

#endif