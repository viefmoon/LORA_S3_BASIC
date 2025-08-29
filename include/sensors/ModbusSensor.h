#ifndef MODBUS_SENSOR_H
#define MODBUS_SENSOR_H

#include <Arduino.h>
#include "sensors/ISensor.h"
#include "ModbusSensorManager.h"
#include "config.h"

/**
 * @brief Clase base para sensores Modbus
 */
class ModbusSensor : public ISensor {
public:
    ModbusSensor(const std::string& id, SensorType type, uint8_t slaveId, uint16_t startReg, uint8_t numRegs);

    bool begin() override;
    SensorReading read() override;
    const std::string& getId() const override { return _id; }
    SensorType getType() const override { return _type; }
    CommunicationProtocol getProtocol() const override { return CommunicationProtocol::MODBUS; }
    PowerRequirement getPowerRequirement() const override { return PowerRequirement::POWER_12V; }

protected:
    uint8_t _slaveId;
    uint16_t _startRegister;
    uint8_t _numRegisters;

    virtual SensorReading processModbusData(uint16_t* data, uint8_t numRegs) = 0;
};

/**
 * @brief Sensor Modbus ENV4 - Lee 4 valores: Humedad, Temperatura, Presión, Iluminación
 */
class ENV4ModbusSensor : public ModbusSensor {
public:
    ENV4ModbusSensor(const std::string& id, uint8_t slaveId);

protected:
    SensorReading processModbusData(uint16_t* data, uint8_t numRegs) override;
};

#endif