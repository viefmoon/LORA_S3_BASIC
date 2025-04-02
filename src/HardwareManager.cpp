/*******************************************************************************************
 * Archivo: src/HardwareManager.cpp
 * Descripción: Implementación de la gestión de hardware del sistema.
 *******************************************************************************************/

#include "HardwareManager.h"
#include "debug.h"
#include "sensors/CO2Sensor.h"
#include "sensors/VEML7700Sensor.h"
#include "sensors/SHT4xSensor.h"
#include <Adafruit_BME680.h>
#include <Adafruit_BME280.h>
#include <Adafruit_VEML7700.h>
#include <map>
#include <string>

// Variables externas
extern Adafruit_BME680 bme680Sensor;
extern Adafruit_BME280 bme280Sensor;
extern Adafruit_VEML7700 veml7700;
extern SensirionI2cSht4x sht4xSensor;
extern std::map<std::string, bool> sensorInitStatus;

// time execution < 10 ms
bool HardwareManager::initHardware(PowerManager& powerManager, 
                                 SHT31& sht30Sensor,
                                 Adafruit_BME680& bme680Sensor,
                                 Adafruit_BME280& bme280Sensor,
                                 Adafruit_VEML7700& veml7700Sensor,
                                 SensirionI2cSht4x& sht4xSensor,
                                 SPIClass& spiLora, 
                                 const std::vector<SensorConfig>& enabledNormalSensors) {
    // Configurar GPIO one wire con pull-up
    pinMode(ONE_WIRE_BUS, INPUT_PULLUP);
    
    // Inicializar el pin de control de batería y desactivar la medición
    pinMode(BATTERY_CONTROL_PIN, OUTPUT);
    digitalWrite(BATTERY_CONTROL_PIN, HIGH);
    pinMode(CONFIG_LED_PIN, OUTPUT);
    digitalWrite(CONFIG_LED_PIN, LOW); // Apagar LED inicialmente
    
    // Verificar si hay algún sensor I2C habilitado
    bool i2cInitialized = false;
    bool someI2cSensorEnabled = false;

    // Primero, determinar si algún sensor I2C está habilitado y marcar todos como no inicializados
    for (const auto& sensor : enabledNormalSensors) {
        if (sensor.enable) {
            if (sensor.type == SHT30 || sensor.type == SHT4X || sensor.type == CO2 || 
                sensor.type == BME680 || sensor.type == BME280 || 
                sensor.type == VEML7700) {
                someI2cSensorEnabled = true;
                // Asumir fallo inicial para todos los sensores habilitados
                sensorInitStatus[sensor.sensorId] = false;
            } else {
                // Asumir éxito para sensores no-I2C
                sensorInitStatus[sensor.sensorId] = true;
            }
        }
    }

    // Inicializar I2C solo si es necesario
    if (someI2cSensorEnabled) {
        Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
        i2cInitialized = true;
    }

    // Ahora inicializar cada sensor I2C individualmente si I2C está inicializado
    if (i2cInitialized) {
        for (const auto& sensor : enabledNormalSensors) {
            if (!sensor.enable) continue; // Saltar sensores deshabilitados

            bool success = false;
            std::string currentSensorId = sensor.sensorId; // Para búsqueda en el mapa

            switch (sensor.type) {
                case SHT30:
                    success = sht30Sensor.begin();
                    if (success) sht30Sensor.reset();
                    break;
                case SHT4X:
                    success = SHT4xSensor::begin();
                    break;
                case CO2:
                    success = CO2Sensor::begin();
                    break;
                case BME680:
                    success = bme680Sensor.begin();
                    if (success) {
                        bme680Sensor.setTemperatureOversampling(BME680_OS_8X);
                        bme680Sensor.setHumidityOversampling(BME680_OS_2X);
                        bme680Sensor.setPressureOversampling(BME680_OS_4X);
                        bme680Sensor.setIIRFilterSize(BME680_FILTER_SIZE_3);
                        bme680Sensor.setGasHeater(320, 150); // 320°C durante 150 ms
                    }
                    break;
                case BME280:
                    success = bme280Sensor.begin(BME280_I2C_ADDR, &Wire);
                    if (success) {
                        bme280Sensor.setSampling(Adafruit_BME280::MODE_FORCED,
                                              Adafruit_BME280::SAMPLING_X1, // temperature
                                              Adafruit_BME280::SAMPLING_X1, // pressure
                                              Adafruit_BME280::SAMPLING_X1, // humidity
                                              Adafruit_BME280::FILTER_OFF);
                    }
                    break;
                case VEML7700:
                    success = VEML7700Sensor::begin();
                    break;
                default:
                    // Para sensores no-I2C, no necesitamos hacer nada aquí
                    continue;
            }
            
            // Actualizar el estado en el mapa global
            sensorInitStatus[currentSensorId] = success;
        }
    } else if (someI2cSensorEnabled) {
        // Si había sensores I2C habilitados pero el bus I2C no se inicializó
        DEBUG_PRINTLN("ERROR: Bus I2C no pudo inicializarse, sensores I2C marcados como no disponibles.");
    }
    
    // Inicializar SPI para LORA con pines definidos
    spiLora.begin(SPI_LORA_SCK_PIN, SPI_LORA_MISO_PIN, SPI_LORA_MOSI_PIN);
    
    // Inicializar los pines de selección SPI (SS)
    initializeSPISSPins();
    
    //Inicializar PowerManager para control de energía
    powerManager.begin();
    
    return true;
}

void HardwareManager::initializeSPISSPins() {
    // Inicializar SS del LORA conectado directamente
    pinMode(LORA_NSS_PIN, OUTPUT);
    digitalWrite(LORA_NSS_PIN, HIGH);

    // Inicializar SS de PT100 como pin nativo
    pinMode(PT100_CS_PIN, OUTPUT);
    digitalWrite(PT100_CS_PIN, HIGH);
}