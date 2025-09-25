/*******************************************************************************************
 * Archivo: src/HardwareManager.cpp
 * Descripción: Implementación de la gestión de hardware del sistema.
 *******************************************************************************************/

#include "HardwareManager.h"
#include "debug.h"
#include "ModbusSensorManager.h"
#include "SensorManager.h"
#include "driver/gpio.h"

bool HardwareManager::i2cInitialized = false;
bool HardwareManager::oneWireInitialized = false;
bool HardwareManager::modbusInitialized = false;
bool HardwareManager::analogInitialized = false;
bool HardwareManager::spiInitialized = false;

void HardwareManager::initialize() {

    pinMode(Pins::BATTERY_CONTROL, OUTPUT);
    digitalWrite(Pins::BATTERY_CONTROL, HIGH);
    pinMode(Pins::CONFIG_LED, OUTPUT);
    digitalWrite(Pins::CONFIG_LED, LOW);

    PowerManager::begin();
}

void HardwareManager::initializeBus(CommunicationProtocol protocol) {
    switch (protocol) {
        case CommunicationProtocol::I2C:
            if (!i2cInitialized) {
                Wire.begin(Pins::I2C_SDA, Pins::I2C_SCL);
                i2cInitialized = true;
            }
            break;
        case CommunicationProtocol::ONE_WIRE:
            if (!oneWireInitialized) {
                oneWireInitialized = true;
            }
            break;
        case CommunicationProtocol::MODBUS:
            if (!modbusInitialized) {
                ModbusSensorManager::beginModbus();
                modbusInitialized = true;
            }
            break;
        case CommunicationProtocol::ANALOG_ADC:
            if (!analogInitialized) {
                analogReadResolution(13);
                analogSetAttenuation(ADC_11db);
                analogInitialized = true;
            }
            break;
        case CommunicationProtocol::SPI:
            if (!spiInitialized) {
                spiInitialized = true;
            }
            break;
        default:
            break;
    }
}

bool HardwareManager::initHardware(SPIClass& spiLora) {
    initialize();

    spiLora.begin(Pins::LoRaSPI::SCK, Pins::LoRaSPI::MISO, Pins::LoRaSPI::MOSI);

    initializeSPISSPins();

    spiInitialized = true;

    return true;
}

void HardwareManager::initializeSPISSPins() {
    pinMode(Pins::LoRaSPI::NSS, OUTPUT);
    digitalWrite(Pins::LoRaSPI::NSS, HIGH);

    pinMode(Pins::RtdSPI::PT100_CS, OUTPUT);
    digitalWrite(Pins::RtdSPI::PT100_CS, HIGH);
}