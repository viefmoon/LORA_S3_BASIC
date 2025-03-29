/*******************************************************************************************
 * Archivo: src/HardwareManager.cpp
 * Descripción: Implementación de la gestión de hardware del sistema.
 *******************************************************************************************/

#include "HardwareManager.h"
#include "debug.h"
// time execution < 10 ms
bool HardwareManager::initHardware(PowerManager& powerManager, SHT31& sht30Sensor, SPIClass& spiLora, const std::vector<SensorConfig>& enabledNormalSensors) {
    // Configurar GPIO one wire con pull-up
    pinMode(ONE_WIRE_BUS, INPUT_PULLUP);
    
    // Inicializar el pin de control de batería y desactivar la medición
    pinMode(BATTERY_CONTROL_PIN, OUTPUT);
    digitalWrite(BATTERY_CONTROL_PIN, HIGH);
    
    // Verificar si hay algún sensor SHT30
    bool sht30SensorEnabled = false;
    for (const auto& sensor : enabledNormalSensors) {
        if (sensor.type == SHT30 && sensor.enable) {
            sht30SensorEnabled = true;
            break;
        }
    }

    // Inicializar I2C con pines definidos solo si se encuentra un sensor SHT30 habilitado
    // IMPORTANTE: La inicialización del SHT30 debe realizarse primero ya que, si no se 
    // inicializa correctamente, puede interferir con otros dispositivos I2C en el bus
    if (sht30SensorEnabled) {
        Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
        
        //Inicializar SHT30 para reset y dummy lectura
        sht30Sensor.begin();
        sht30Sensor.reset();
    }
    
    // Inicializar SPI para LORA con pines definidos
    spiLora.begin(SPI_LORA_SCK_PIN, SPI_LORA_MISO_PIN, SPI_LORA_MOSI_PIN);
    
    // Inicializar los pines de selección SPI (SS)
    initializeSPISSPins();
    
    // Inicializar PowerManager para control de energía
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