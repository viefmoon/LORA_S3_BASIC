/*******************************************************************************************
 * Archivo: src/HardwareManager.cpp
 * Descripción: Implementación de la gestión de hardware del sistema.
 *******************************************************************************************/

#include "HardwareManager.h"
#include "debug.h"
#include "ModbusSensorManager.h"
#include "SensorManager.h"

// Inicialización de variables estáticas
bool HardwareManager::i2cInitialized = false;
bool HardwareManager::oneWireInitialized = false;
bool HardwareManager::modbusInitialized = false;
bool HardwareManager::analogInitialized = false;
bool HardwareManager::spiInitialized = false;

// Inicializa componentes básicos
void HardwareManager::initialize() {
    // Configurar GPIO one wire con pull-up
    pinMode(Pins::ONE_WIRE_BUS, INPUT_PULLUP);

    // Inicializar el pin de control de batería y desactivar la medición
    pinMode(Pins::BATTERY_CONTROL, OUTPUT);
    digitalWrite(Pins::BATTERY_CONTROL, HIGH);
    pinMode(Pins::CONFIG_LED, OUTPUT);
    digitalWrite(Pins::CONFIG_LED, LOW); // Apagar LED inicialmente

    // Inicializar PowerManager para control de energía
    PowerManager::begin();
}

// Inicializa un bus de comunicación específico si no lo ha hecho ya
void HardwareManager::initializeBus(CommunicationProtocol protocol) {
    switch (protocol) {
        case CommunicationProtocol::I2C:
            if (!i2cInitialized) {
                Wire.begin(Pins::I2C_SDA, Pins::I2C_SCL);
                i2cInitialized = true;
                DEBUG_PRINTLN("I2C bus initialized");
            }
            break;
        case CommunicationProtocol::ONE_WIRE:
            if (!oneWireInitialized) {
                // La inicialización de OneWire se maneja a través del objeto DallasTemperature
                // que ya es global. Solo marcarlo como inicializado.
                oneWireInitialized = true;
                DEBUG_PRINTLN("OneWire bus initialized");
            }
            break;
        case CommunicationProtocol::MODBUS:
            if (!modbusInitialized) {
                ModbusSensorManager::beginModbus();
                modbusInitialized = true;
                DEBUG_PRINTLN("Modbus initialized");
            }
            break;
        case CommunicationProtocol::ANALOG_ADC:
            if (!analogInitialized) {
                analogReadResolution(13);
                analogSetAttenuation(ADC_11db);
                analogInitialized = true;
                DEBUG_PRINTLN("Analog ADC initialized");
            }
            break;
        case CommunicationProtocol::SPI:
            if (!spiInitialized) {
                // SPI se inicializa en initHardware para LoRa
                spiInitialized = true;
                DEBUG_PRINTLN("SPI bus initialized");
            }
            break;
        default:
            break;
    }
}

// Método para inicializar hardware con SPI para LoRa
bool HardwareManager::initHardware(SPIClass& spiLora,
                                 const std::vector<SensorConfig>& enabledNormalSensors) {
    // Inicializar componentes básicos
    initialize();

    // Inicializar SPI para LORA con pines definidos
    spiLora.begin(Pins::LoRaSPI::SCK, Pins::LoRaSPI::MISO, Pins::LoRaSPI::MOSI);

    // Inicializar los pines de selección SPI (SS)
    initializeSPISSPins();

    spiInitialized = true;

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