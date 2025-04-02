#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Preferences.h>
#include <OneWire.h>
#include <DallasTemperature.h>
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
#include "SparkFun_SCD4x_Arduino_Library.h"
#include "LoRaManager.h"
#include "BLE.h"
#include "HardwareManager.h"
#include "SleepManager.h"
#include "SHT31.h"
#include <Adafruit_MAX31865.h>
#include <Adafruit_BME680.h>
#include <Adafruit_BME280.h>
#include <Adafruit_VEML7700.h>
#include <SensirionI2cSht4x.h>

// --- INICIO: Añadir bandera global ---
bool wokeFromConfigPin = false;
// --- FIN: Añadir bandera global ---

//--------------------------------------------------------------------------------------------
// Variables globales
//--------------------------------------------------------------------------------------------
const LoRaWANBand_t Region = LORA_REGION;
const uint8_t subBand = LORA_SUBBAND;

// Mapa para rastrear el estado de inicialización de los sensores
std::map<std::string, bool> sensorInitStatus;

Preferences preferences;
uint32_t timeToSleep;
String deviceId;
String stationId;
bool systemInitialized;
unsigned long setupStartTime; // Variable para almacenar el tiempo de inicio

// Configuraciones de sensores
std::vector<SensorConfig> enabledNormalSensors;
std::vector<ModbusSensorConfig> enabledModbusSensors;
std::vector<SensorConfig> enabledAdcSensors;

ESP32Time rtc;
PowerManager powerManager;

SPIClass spiLora(FSPI);
SPISettings spiRadioSettings(SPI_LORA_CLOCK, MSBFIRST, SPI_MODE0);

Adafruit_MAX31865 rtdSensor = Adafruit_MAX31865(PT100_CS_PIN, SPI_MOSI_PIN, SPI_MISO_PIN, SPI_SCK_PIN);

SHT31 sht30Sensor(0x44, &Wire);
SCD4x scd4x(SCD4x_SENSOR_SCD41);
Adafruit_BME680 bme680Sensor(&Wire);
Adafruit_BME280 bme280Sensor;
Adafruit_VEML7700 veml7700;
SensirionI2cSht4x sht40Sensor;

SX1262 radio = new Module(LORA_NSS_PIN, LORA_DIO1_PIN, LORA_RST_PIN, LORA_BUSY_PIN, spiLora, spiRadioSettings);
LoRaWANNode node(&radio, &Region, subBand);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature dallasTemp(&oneWire);

RTC_DATA_ATTR uint8_t LWsession[RADIOLIB_LORAWAN_SESSION_BUF_SIZE];
Preferences store;

//--------------------------------------------------------------------------------------------
// setup()
//--------------------------------------------------------------------------------------------
void setup() {
    setCpuFrequencyMhz(40); // 40MHz O 80MHz FUNCIONAN BIEN EN BAJO CONSUMO
    // Inicializar contador de tiempo y log
    setupStartTime = millis();
    DEBUG_BEGIN(SERIAL_BAUD_RATE);
    // --- INICIO: Comprobar causa del despertar ---
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
        DEBUG_PRINTLN("INFO: Despertado por EXT0 (CONFIG_PIN)");
        wokeFromConfigPin = true;
        // Opcional: Pequeño delay para estabilizar/debounce inicial
        delay(50);
    } else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
        DEBUG_PRINTLN("INFO: Despertado por Timer");
        wokeFromConfigPin = false;
    } else {
        DEBUG_PRINTF("INFO: Despertado por otra razón: %d\n", wakeup_reason);
        wokeFromConfigPin = false;
    }
    // --- FIN: Comprobar causa del despertar ---

    // Liberar pines que se mantuvieron en estado específico durante el deep sleep
    SleepManager::releaseHeldPins();

    // Inicialización del NVS y de hardware I2C/IO
    preferences.clear();       // Comentado para evitar borrar los nonces guardados
    nvs_flash_erase();         // Comentado para evitar borrar los nonces guardados
    nvs_flash_init();          // Comentado para preservar datos NVS entre boots

    // Inicialización de configuración
    if (!ConfigManager::checkInitialized()) {
        ConfigManager::initializeDefaultConfig();
    }
    ConfigManager::getSystemConfig(systemInitialized, timeToSleep, deviceId, stationId);

    // Obtener configuraciones de sensores habilitados
    enabledNormalSensors = ConfigManager::getEnabledSensorConfigs();
    enabledModbusSensors = ConfigManager::getEnabledModbusSensorConfigs();
    enabledAdcSensors = ConfigManager::getEnabledAdcSensorConfigs();

    // Inicialización de hardware básico (GPIO, I2C, SPI, etc.)
    if (!HardwareManager::initHardware(powerManager, 
                                     sht30Sensor, 
                                     bme680Sensor, 
                                     bme280Sensor, 
                                     veml7700,
                                     sht40Sensor,
                                     spiLora, 
                                     enabledNormalSensors)) {
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

    // Tiempo hasta que se inicializan los sensores = 4ms
    // Inicializar sensores
    SensorManager::beginSensors(enabledNormalSensors);

    DEBUG_PRINTLN("Tiempo después de iniciar sensores: " + String(millis() - setupStartTime));

    // Inicializar y activar radio LoRa = 61ms
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
    ///

    DEBUG_PRINTLN("Tiempo después de activar radio: " + String(millis() - setupStartTime));
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
    std::vector<SensorReading> adcReadings;

    SensorManager::getAllSensorReadings(normalReadings, modbusReadings, adcReadings, 
                                      enabledNormalSensors, enabledModbusSensors, enabledAdcSensors);

    //Apgar las fuentes de alimentacion de sensores antes de enviar datos
    powerManager.allPowerOff();
    LoRaManager::sendDelimitedPayload(normalReadings, modbusReadings, adcReadings, node, deviceId, stationId, rtc);

    // Calcular y mostrar el tiempo transcurrido antes de dormir
    unsigned long elapsedTime = millis() - setupStartTime;
    DEBUG_PRINTF("Tiempo transcurrido antes de sleep: %lu ms\n", elapsedTime);
    delay(10);

    // Dormir
    SleepManager::goToDeepSleep(timeToSleep, powerManager, &radio, node, LWsession, spiLora);
}
