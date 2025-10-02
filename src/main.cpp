#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Preferences.h>
#include <vector>
#include <ArduinoJson.h>
#include <cmath>
#include <map>
#include <string>

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

bool wokeFromConfigPin = false;

void preinit() {
  setCpuFrequencyMhz(System::CPU_FREQUENCY_MHZ);
}

ESP32Time rtc;
uint32_t timeToSleep;
unsigned long setupStartTime;

String deviceId;
String stationId;
bool systemInitialized;

Preferences preferences;
Preferences store;

extern const LoRaWANBand_t US915;
const LoRaWANBand_t Region = US915;
const uint8_t subBand = LoRa::SUBBAND;

SPIClass spiLora(FSPI);
SPISettings spiRadioSettings(LoRa::SPI_CLOCK, MSBFIRST, SPI_MODE0);
SX1262 radio = new Module(Pins::LoRaSPI::NSS, Pins::LoRaSPI::DIO1, Pins::LoRaSPI::RST, Pins::LoRaSPI::BUSY, spiLora, spiRadioSettings);
LoRaWANNode node(&radio, &Region, subBand);
RTC_DATA_ATTR uint8_t LWsession[RADIOLIB_LORAWAN_SESSION_BUF_SIZE];

// Cachear configuraci\u00f3n en RTC RAM para evitar lecturas de NVS cada wake
RTC_DATA_ATTR bool configCached = false;
RTC_DATA_ATTR uint32_t cachedTimeToSleep = 0;
RTC_DATA_ATTR char cachedDeviceId[32] = {0};
RTC_DATA_ATTR char cachedStationId[32] = {0};

// Track de inicializaci\u00f3n de hardware para optimizar wakeups
RTC_DATA_ATTR bool hardwareInitialized = false;
RTC_DATA_ATTR uint32_t wakeupCount = 0;

std::vector<SensorReading> normalReadings;

SensorManager sensorManager;


/**
 * @brief Inicializa el hardware y la configuración del sistema
 * @return true si la inicialización fue exitosa, false en caso de error
 */
bool initHardware() {
    SleepManager::releaseHeldPins();

    //DESCOMENTAR LA SIGUIENTE LÍNEA PARA BORRAR TODA LA MEMORIA FLASH Y REINICIALIZAR
    //ConfigManager::clearAllPreferences();

    // Usar configuración cacheada si está disponible
    if (configCached) {
        // Usar valores cacheados en RTC RAM (mucho más rápido)
        timeToSleep = cachedTimeToSleep;
        deviceId = String(cachedDeviceId);
        stationId = String(cachedStationId);
        systemInitialized = true;
        DEBUG_PRINTLN("Usando configuración cacheada de RTC RAM");
    } else {
        // Primera vez o después de power-on reset - leer de NVS
        if (!ConfigManager::checkInitialized()) {
            DEBUG_PRINTLN("Creando configuración por defecto...");
            ConfigManager::initializeDefaultConfig();
        }
        ConfigManager::getSystemConfig(systemInitialized, timeToSleep, deviceId, stationId);

        // Cachear en RTC RAM para próximos wakeups
        cachedTimeToSleep = timeToSleep;
        strncpy(cachedDeviceId, deviceId.c_str(), sizeof(cachedDeviceId) - 1);
        strncpy(cachedStationId, stationId.c_str(), sizeof(cachedStationId) - 1);
        configCached = true;
        DEBUG_PRINTLN("Configuración cacheada en RTC RAM");
    }

    DEBUG_PRINTF("Config: Device=%s, Station=%s, Sleep=%ds\n",
                 deviceId.c_str(), stationId.c_str(), timeToSleep);

    if (!HardwareManager::initHardware(spiLora)) {
        return false;
    }

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        rtc.setTime(0, 0, 0, 1, 1, 2023);
    }

    sensorManager.registerSensorsFromConfig();
    sensorManager.beginAll();

    return true;
}

/**
 * @brief Configura y activa la radio LoRa
 * @return true si la configuración fue exitosa, false en caso de error
 */
bool configureLoRa() {
    int16_t state = radio.begin();
    if (state != RADIOLIB_ERR_NONE) {
        return false;
    }

    state = LoRaManager::lwActivate(node);
    if (state != RADIOLIB_LORAWAN_NEW_SESSION &&
        state != RADIOLIB_LORAWAN_SESSION_RESTORED) {
        return false;
    }

    DEBUG_PRINTLN("Tiempo después de activar radio: " + String(millis() - setupStartTime));
    return true;
}

/**
 * @brief Lee los datos de todos los sensores habilitados
 */
void readSensors() {
    normalReadings.clear();

    normalReadings = sensorManager.readAll();
}

/**
 * @brief Envía los datos de los sensores a través de LoRa
 */
void sendData() {
    sensorManager.powerDown();

    LoRaManager::sendDelimitedPayload(normalReadings,
                                    node, deviceId, stationId, rtc);

    unsigned long elapsedTime = millis() - setupStartTime;
    DEBUG_PRINTF("Tiempo transcurrido antes de sleep: %lu ms\n", elapsedTime);
}

void setup() {
    setupStartTime = millis();
    DEBUG_BEGIN(System::SERIAL_BAUD_RATE);

    // Incrementar contador de wakeups
    wakeupCount++;
    DEBUG_PRINTF("Wakeup #%lu\n", wakeupCount);

    // Nota: El cristal externo de 32kHz está configurado mediante sdkconfig
    // Ver: boards/sdkconfig.esp32s3 y platformio.ini

    SleepManager::handleWakeupCause(wokeFromConfigPin);

    // Si se despert\u00f3 por CONFIG_PIN, resetear inicializaci\u00f3n
    if (wokeFromConfigPin) {
        hardwareInitialized = false;
        configCached = false;
    }

    if (!initHardware()) {
        SleepManager::goToDeepSleep(timeToSleep, &radio, node, LWsession, spiLora);
    }

    if (BLEHandler::checkConfigMode()) {
        return;
    }

    if (!configureLoRa()) {
        SleepManager::goToDeepSleep(timeToSleep, &radio, node, LWsession, spiLora);
    }
}

void loop() {
    if (BLEHandler::checkConfigMode()) {
        return;
    }

    readSensors();
    sendData();
    SleepManager::goToDeepSleep(timeToSleep, &radio, node, LWsession, spiLora);
}