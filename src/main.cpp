#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Preferences.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <vector>
#include <ArduinoJson.h>
#include <cmath>

#include "config.h"
#include "debug.h"
#include "PowerManager.h"
#include <RadioLib.h>
#include <ESP32Time.h>
#include "sensor_types.h"
#include "SensorManager.h"
#include "nvs_flash.h"
#include "esp_sleep.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEAdvertising.h>
#include "config_manager.h"
#include "utilities.h"
#include <SensirionI2cSht3x.h>
#include "LoRaManager.h"
#include "BLE.h"
#include "HardwareManager.h"
#include "SleepManager.h"
#include "SHT31.h"
#include <Adafruit_MAX31865.h>
//--------------------------------------------------------------------------------------------
// Variables globales
//--------------------------------------------------------------------------------------------
const LoRaWANBand_t Region = LORA_REGION;
const uint8_t subBand = LORA_SUBBAND;

Preferences preferences;
uint32_t timeToSleep;
String deviceId;
String stationId;
bool systemInitialized;
unsigned long setupStartTime; // Variable para almacenar el tiempo de inicio

// Configuraciones de sensores
std::vector<SensorConfig> enabledNormalSensors;
std::vector<ModbusSensorConfig> enabledModbusSensors;

ESP32Time rtc;
PowerManager powerManager;

SPIClass spiLora(FSPI);
SPISettings spiRadioSettings(SPI_LORA_CLOCK, MSBFIRST, SPI_MODE0);

Adafruit_MAX31865 rtdSensor = Adafruit_MAX31865(PT100_CS_PIN, SPI_MOSI_PIN, SPI_MISO_PIN, SPI_SCK_PIN);

SHT31 sht30Sensor(0x44, &Wire);

SX1262 radio = new Module(LORA_NSS_PIN, LORA_DIO1_PIN, LORA_RST_PIN, LORA_BUSY_PIN, spiLora, spiRadioSettings);
LoRaWANNode node(&radio, &Region, subBand);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature dallasTemp(&oneWire);

RTC_DATA_ATTR uint16_t bootCount = 0;
RTC_DATA_ATTR uint16_t bootCountSinceUnsuccessfulJoin = 0;
RTC_DATA_ATTR uint8_t LWsession[RADIOLIB_LORAWAN_SESSION_BUF_SIZE];
Preferences store;

//--------------------------------------------------------------------------------------------
// setup()
//--------------------------------------------------------------------------------------------
void setup() {
    
    // Inicializar contador de tiempo y log
    setupStartTime = millis();
    DEBUG_BEGIN(SERIAL_BAUD_RATE);
    
    // Incrementar contador de boot y mostrar estadísticas
    bootCount++;
    DEBUG_PRINTF("Boot count: %d - Boot count desde último join exitoso: %d\n", bootCount, bootCountSinceUnsuccessfulJoin);

    // Liberar pines que se mantuvieron en estado específico durante el deep sleep
    SleepManager::releaseHeldPins();

    // // Inicialización del NVS y de hardware I2C/IO
    // preferences.clear();
    // nvs_flash_erase();
    // nvs_flash_init();

    // Inicialización de configuración
    if (!ConfigManager::checkInitialized()) {
        ConfigManager::initializeDefaultConfig();
    }
    ConfigManager::getSystemConfig(systemInitialized, timeToSleep, deviceId, stationId);

    // Obtener configuraciones de sensores habilitados
    enabledNormalSensors = ConfigManager::getEnabledSensorConfigs();
    enabledModbusSensors = ConfigManager::getEnabledModbusSensorConfigs();

    // Inicialización de hardware
    if (!HardwareManager::initHardware(powerManager, sht30Sensor, spiLora, enabledNormalSensors)) {
        SleepManager::goToDeepSleep(timeToSleep, powerManager, &radio, node, LWsession, spiLora);
    }

    // Verificar y entrar en modo configuración BLE si es necesario
    if (BLEHandler::checkConfigMode()) {
        return;
    }

    // Comprobar si tenemos un timestamp válido
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        // Establecer una fecha/hora predeterminada si el RTC no está configurado
        rtc.setTime(0, 0, 0, 1, 1, 2023);  // 01/01/2023 00:00:00
    }

    // Inicializar sensores
    SensorManager::beginSensors(enabledNormalSensors);

    // TIEMPO TRASCURRIDO HASTA EL MOMENTO ≈ 98 ms
    // Inicializar radio LoRa
    int16_t state = radio.begin();
    if (state != RADIOLIB_ERR_NONE) {
        SleepManager::goToDeepSleep(timeToSleep, powerManager, &radio, node, LWsession, spiLora);
    }

    // Activar LoRaWAN
    state = LoRaManager::lwActivate(node);
    if (state != RADIOLIB_LORAWAN_NEW_SESSION && 
        state != RADIOLIB_LORAWAN_SESSION_RESTORED) {
        SleepManager::goToDeepSleep(timeToSleep, powerManager, &radio, node, LWsession, spiLora);
    }
}

//--------------------------------------------------------------------------------------------
// loop()
//--------------------------------------------------------------------------------------------
void loop() {

    // Verificar si se mantiene presionado para modo config
    if (BLEHandler::checkConfigMode()) {
        return;
    }

    // Obtener todas las lecturas de sensores (normales y Modbus)
    std::vector<SensorReading> normalReadings;
    std::vector<ModbusSensorReading> modbusReadings;
    SensorManager::getAllSensorReadings(normalReadings, modbusReadings, enabledNormalSensors, enabledModbusSensors);

    //Apgar las fuentes de alimentacion de sensores antes de enviar datos
    powerManager.allPowerOff();
    LoRaManager::sendDelimitedPayload(normalReadings, modbusReadings, node, deviceId, stationId, rtc);

    // Calcular y mostrar el tiempo transcurrido antes de dormir
    unsigned long elapsedTime = millis() - setupStartTime;
    DEBUG_PRINTF("Tiempo transcurrido antes de sleep: %lu ms\n", elapsedTime);
    delay(10);

    // Dormir
    SleepManager::goToDeepSleep(timeToSleep, powerManager, &radio, node, LWsession, spiLora);
}
