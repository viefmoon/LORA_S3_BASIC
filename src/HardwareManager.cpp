/*******************************************************************************************
 * Archivo: src/HardwareManager.cpp
 * Descripción: Implementación de la gestión de hardware del sistema.
 *******************************************************************************************/

#include "HardwareManager.h"
#include "debug.h"
#include <map>
#include <string>

// Variables externas
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