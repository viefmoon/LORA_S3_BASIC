#include "SensorManager.h"
#include "HardwareManager.h"
#include "PowerManager.h"
#include <Wire.h>
#include <SPI.h>
#include <cmath>
#include <DallasTemperature.h>
#include "sensor_types.h"
#include "config.h"
#include <Preferences.h>
#include "config_manager.h"
#include "debug.h"
#include "utilities.h"
#include <map>
#include <string>

// Inclusión de todos los sensores concretos
#include "sensors/SHT30Sensor.h"
#include "sensors/DS18B20Sensor.h"
#include "sensors/CO2Sensor.h"
#include "sensors/VEML7700Sensor.h"
#include "sensors/SHT40Sensor.h"
#include "sensors/BatterySensor.h"
#include "sensors/BME680Sensor.h"
#include "sensors/BME280Sensor.h"
#include "sensors/NtcSensor.h"
#include "sensors/PHSensor.h"
#include "sensors/ConductivitySensor.h"
#include "sensors/HDS10Sensor.h"
#include "sensors/SoilHumiditySensor.h"
#include "sensors/RTDSensor.h"
#include "sensors/ModbusSensor.h"
#include <Adafruit_BME680.h>
#include <Adafruit_BME280.h>
#include <Adafruit_VEML7700.h>

// Variables externas
extern Adafruit_BME680 bme680Sensor;
extern Adafruit_BME280 bme280Sensor;
extern Adafruit_VEML7700 veml7700;

void SensorManager::registerSensorsFromConfig() {
    _sensors.clear();

    // Registrar siempre el sensor de batería como sensor especial del sistema
    auto batterySensor = std::make_unique<BatterySensor>("BATT");
    _sensors.push_back(std::move(batterySensor));
    DEBUG_PRINTLN("Sensor de batería registrado: BATT");

    auto normalConfigs = ConfigManager::getEnabledSensorConfigs();
    for (const auto& config : normalConfigs) {
        if (config.enable) {
            auto sensor = createSensor(config);
            if (sensor) {
                _sensors.push_back(std::move(sensor));
                DEBUG_PRINTF("Sensor registrado: %s\n", config.sensorId);
            }
        }
    }

    auto modbusConfigs = ConfigManager::getEnabledModbusSensorConfigs();
    for (const auto& config : modbusConfigs) {
        if (config.enable) {
            if (config.type == ENV4) {
                auto sensor = std::make_unique<ENV4ModbusSensor>(config.sensorId, config.address);
                if (sensor) {
                    _sensors.push_back(std::move(sensor));
                    DEBUG_PRINTF("Sensor Modbus registrado: %s\n", config.sensorId);
                }
            }
        }
    }

    auto adcConfigs = ConfigManager::getEnabledAdcSensorConfigs();
    for (const auto& config : adcConfigs) {
        if (config.enable) {
            auto sensor = createSensor(config);
            if (sensor) {
                _sensors.push_back(std::move(sensor));
                DEBUG_PRINTF("Sensor ADC registrado: %s\n", config.sensorId);
            }
        }
    }
}

std::unique_ptr<ISensor> SensorManager::createSensor(const SensorConfig& config) {
    switch (config.type) {
        case SHT40:
            return std::make_unique<SHT40Sensor>(config.sensorId);
        case SHT30:
            return std::make_unique<SHT30Sensor>(config.sensorId);
        case DS18B20:
            return std::make_unique<DS18B20Sensor>(config.sensorId);
        case CO2:
            return std::make_unique<CO2Sensor>(config.sensorId);
        case BATTERY:
            return std::make_unique<BatterySensor>(config.sensorId);
        case BME680:
            return std::make_unique<BME680Sensor>(config.sensorId);
        case BME280:
            return std::make_unique<BME280Sensor>(config.sensorId);
        case VEML7700:
            return std::make_unique<VEML7700Sensor>(config.sensorId);
        case N100K:
            return std::make_unique<NtcSensor>(config.sensorId, N100K, config.configKey);
        case N10K:
            return std::make_unique<NtcSensor>(config.sensorId, N10K);
        case HDS10:
            return std::make_unique<HDS10Sensor>(config.sensorId);
        case PH:
            return std::make_unique<PHSensor>(config.sensorId);
        case COND:
            return std::make_unique<ConductivitySensor>(config.sensorId);
        case SOILH:
            return std::make_unique<SoilHumiditySensor>(config.sensorId);
        case RTD:
            return std::make_unique<RTDSensor>(config.sensorId);
        default:
            DEBUG_PRINTF("Tipo de sensor no reconocido: %d\n", config.type);
            return nullptr;
    }
}

void SensorManager::beginAll() {
    bool needs3V3Switched = false;
    bool needs12V = false;

    for (const auto& sensor : _sensors) {
        PowerRequirement powerReq = sensor->getPowerRequirement();

        switch (powerReq) {
            case PowerRequirement::POWER_3V3_SWITCHED:
                needs3V3Switched = true;
                break;
            case PowerRequirement::POWER_12V:
                needs12V = true;
                break;
            case PowerRequirement::POWER_3V3_MAIN:
            case PowerRequirement::POWER_NONE:
                break;
        }
    }

    if (needs3V3Switched) {
        PowerManager::power3V3On(); // Encender alimentación adicional de 3.3V
        DEBUG_PRINTLN("Alimentación 3.3V adicional activada");
        delay(100);
    }

    if (needs12V) {
        PowerManager::power12VOn(); // Encender alimentación de 12V para Modbus
        DEBUG_PRINTLN("Alimentación 12V activada");
        delay(500);
        ModbusSensorManager::beginModbus(); // Inicializar comunicación Modbus
        DEBUG_PRINTLN("Comunicación Modbus iniciada");
    }

    for (const auto& sensor : _sensors) {
        HardwareManager::initializeBus(sensor->getProtocol());

        bool success = sensor->begin();
        DEBUG_PRINTF("Sensor %s inicializado: %s (Power: %d)\n",
                    sensor->getId().c_str(),
                    success ? "OK" : "FALLO",
                    static_cast<int>(sensor->getPowerRequirement()));
    }
}

std::vector<SensorReading> SensorManager::readAll() {
    std::vector<SensorReading> readings;

    for (const auto& sensor : _sensors) {
        if (sensor->isInitialized()) {
            readings.push_back(sensor->read());
        } else {
            SensorReading errorReading;
            strncpy(errorReading.sensorId, sensor->getId().c_str(), sizeof(errorReading.sensorId) - 1);
            errorReading.sensorId[sizeof(errorReading.sensorId) - 1] = '\0';
            errorReading.type = sensor->getType();
            errorReading.value = NAN;
            readings.push_back(errorReading);
        }
    }

    return readings;
}


void SensorManager::powerDown() {
    bool has3V3Switched = false;
    bool has12V = false;

    for (const auto& sensor : _sensors) {
        PowerRequirement powerReq = sensor->getPowerRequirement();

        switch (powerReq) {
            case PowerRequirement::POWER_3V3_SWITCHED:
                has3V3Switched = true;
                break;
            case PowerRequirement::POWER_12V:
                has12V = true;
                break;
            default:
                break;
        }
    }

    if (has3V3Switched) {
        PowerManager::power3V3Off();
        DEBUG_PRINTLN("Alimentación 3.3V adicional apagada");
    }

    if (has12V) {
        ModbusSensorManager::endModbus(); // Terminar comunicación Modbus
        PowerManager::power12VOff();
        DEBUG_PRINTLN("Alimentación 12V apagada");
    }
}