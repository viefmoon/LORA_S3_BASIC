/*******************************************************************************************
 * Archivo: src/HardwareManager.cpp
 * Descripción: Implementación de la gestión de hardware del sistema.
 *******************************************************************************************/

#include "HardwareManager.h"
#include "debug.h"
#include <map>
#include <string>
#include "SensorManager.h"

// Eliminamos la declaración externa del mapa global

// time execution < 10 ms
bool HardwareManager::initHardware(SPIClass& spiLora, 
                                 const std::vector<SensorConfig>& enabledNormalSensors) {
    // Configurar GPIO one wire con pull-up
    pinMode(Pins::ONE_WIRE_BUS, INPUT_PULLUP);
    
    // Inicializar el pin de control de batería y desactivar la medición
    pinMode(Pins::BATTERY_CONTROL, OUTPUT);
    digitalWrite(Pins::BATTERY_CONTROL, HIGH);
    pinMode(Pins::CONFIG_LED, OUTPUT);
    digitalWrite(Pins::CONFIG_LED, LOW); // Apagar LED inicialmente
    
    // Verificar si hay algún sensor I2C habilitado
    bool someI2cSensorEnabled = false;

    // Primero, determinar si algún sensor I2C está habilitado y marcar todos como no inicializados
    for (const auto& sensor : enabledNormalSensors) {
        if (sensor.enable) {
            if (sensor.type == SHT30 || sensor.type == SHT40 || sensor.type == CO2 || 
                sensor.type == BME680 || sensor.type == BME280 || 
                sensor.type == VEML7700) {
                someI2cSensorEnabled = true;
                // Asumir fallo inicial para todos los sensores habilitados
                SensorManager::setSensorInitialized(sensor.sensorId, false);
            } else {
                // Asumir éxito para sensores no-I2C
                SensorManager::setSensorInitialized(sensor.sensorId, true);
            }
        }
    }

    // Inicializar I2C solo si es necesario
    if (someI2cSensorEnabled) {
        Wire.begin(Pins::I2C_SDA, Pins::I2C_SCL);
    }
    
    // Inicializar SPI para LORA con pines definidos
    spiLora.begin(Pins::LoRaSPI::SCK, Pins::LoRaSPI::MISO, Pins::LoRaSPI::MOSI);
    
    // Inicializar los pines de selección SPI (SS)
    initializeSPISSPins();
    
    //Inicializar PowerManager para control de energía
    PowerManager::begin();
    
    return true;
}

void HardwareManager::initializeSPISSPins() {
    // Inicializar SS del LORA conectado directamente
    pinMode(Pins::LoRaSPI::NSS, OUTPUT);
    digitalWrite(Pins::LoRaSPI::NSS, HIGH);

    // Inicializar SS de PT100 como pin nativo
    pinMode(Pins::RtdSPI::PT100_CS, OUTPUT);
    digitalWrite(Pins::RtdSPI::PT100_CS, HIGH);
}