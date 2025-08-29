#include "sensors/ModbusSensor.h"
#include "debug.h"

ModbusSensor::ModbusSensor(const std::string& id, SensorType type, uint8_t slaveId, uint16_t startReg, uint8_t numRegs) {
    this->_id = id;
    this->_type = type;
    this->_slaveId = slaveId;
    this->_startRegister = startReg;
    this->_numRegisters = numRegs;
}
    bool ModbusSensor::begin() {
    _initialized = true;
    return true;
}
SensorReading ModbusSensor::read() {
    SensorReading reading;
    strncpy(reading.sensorId, _id.c_str(), sizeof(reading.sensorId) - 1);
    reading.sensorId[sizeof(reading.sensorId) - 1] = '\0';
    reading.type = _type;
    
    if (!_initialized) {
        reading.value = NAN;
        return reading;
    }
    
    uint16_t data[_numRegisters];
    bool success = ModbusSensorManager::readRegisters(_slaveId, _startRegister, _numRegisters, data);
    
    if (success) {
        reading = processModbusData(data, _numRegisters);
        strncpy(reading.sensorId, _id.c_str(), sizeof(reading.sensorId) - 1);
        reading.sensorId[sizeof(reading.sensorId) - 1] = '\0';
        reading.type = _type;
    } else {
        reading.value = NAN;
    }
    
    return reading;
}

ENV4ModbusSensor::ENV4ModbusSensor(const std::string& id, uint8_t slaveId)
    : ModbusSensor(id, ENV4, slaveId, 500, 8) {
}

SensorReading ENV4ModbusSensor::processModbusData(uint16_t* data, uint8_t numRegs) {
    SensorReading reading;
    reading.subValues.clear();
    
    if (numRegs < 8) {
        for (int i = 0; i < 4; i++) {
            SubValue sv;
            sv.value = NAN;
            reading.subValues.push_back(sv);
        }
        reading.value = NAN;
        return reading;
    }
    
    float values[4];
    for (int i = 0; i < 4; i++) {
        uint32_t combined = ((uint32_t)data[i*2] << 16) | data[i*2 + 1];
        values[i] = *((float*)&combined);
    }
    
    for (int i = 0; i < 4; i++) {
        SubValue sv;
        sv.value = values[i];
        reading.subValues.push_back(sv);
    }
    
    reading.value = values[0];
    
    return reading;
}
