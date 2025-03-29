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

Adafruit_MAX31865 thermo = Adafruit_MAX31865(PT100_CS_PIN, SPI_MOSI_PIN, SPI_MISO_PIN, SPI_SCK_PIN);

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
    // setupStartTime = millis(); // Inicia el contador de tiempo
    DEBUG_BEGIN(SERIAL_BAUD_RATE);

    thermo.begin(MAX31865_4WIRE);

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
        DEBUG_PRINTLN("Error en la inicialización del hardware");
        SleepManager::goToDeepSleep(timeToSleep, powerManager, &radio, node, LWsession, spiLora);
    }

    // Configuración de pines de modo config
    pinMode(CONFIG_PIN, INPUT);
    pinMode(CONFIG_LED_PIN, OUTPUT);

    // Modo configuración BLE
    if (BLEHandler::checkConfigMode()) {
        return;
    }

    // Ya no es necesario inicializar el RTC externo, el interno ya está disponible
    // Comprobar si tenemos un timestamp válido
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        DEBUG_PRINTLN("RTC no está configurado. Estableciendo una hora predeterminada.");
        // Establecer una fecha/hora predeterminada si el RTC no está configurado
        rtc.setTime(0, 0, 0, 1, 1, 2023);  // 01/01/2023 00:00:00
    }

    // Inicializar sensores
    SensorManager::beginSensors(enabledNormalSensors);

    //TIEMPO TRASCURRIDO HASTA EL MOMENTO ≈ 98 ms
    // Inicializar radio LoRa
    int16_t state = radio.begin();
    if (state != RADIOLIB_ERR_NONE) {
        DEBUG_PRINTF("Error iniciando radio: %d\n", state);
        SleepManager::goToDeepSleep(timeToSleep, powerManager, &radio, node, LWsession, spiLora);
    }

    // Activar LoRaWAN
    state = LoRaManager::lwActivate(node);
    if (state != RADIOLIB_LORAWAN_NEW_SESSION && 
        state != RADIOLIB_LORAWAN_SESSION_RESTORED) {
        DEBUG_PRINTF("Error activando LoRaWAN o sincronizando RTC: %d\n", state);
        SleepManager::goToDeepSleep(timeToSleep, powerManager, &radio, node, LWsession, spiLora);
    }
}

//--------------------------------------------------------------------------------------------
// loop()
//--------------------------------------------------------------------------------------------
void loop() {

    uint16_t rtd = thermo.readRTD();

  Serial.print("RTD value: "); Serial.println(rtd);
  float ratio = rtd;
  ratio /= 32768;
  Serial.print("Ratio = "); Serial.println(ratio,8);
  Serial.print("Resistance = "); Serial.println(RREF*ratio,8);
  Serial.print("Temperature = "); Serial.println(thermo.temperature(RNOMINAL, RREF));

  // Check and print any faults
  uint8_t fault = thermo.readFault();
  if (fault) {
    Serial.print("Fault 0x"); Serial.println(fault, HEX);
    if (fault & MAX31865_FAULT_HIGHTHRESH) {
      Serial.println("RTD High Threshold"); 
    }
    if (fault & MAX31865_FAULT_LOWTHRESH) {
      Serial.println("RTD Low Threshold"); 
    }
    if (fault & MAX31865_FAULT_REFINLOW) {
      Serial.println("REFIN- > 0.85 x Bias"); 
    }
    if (fault & MAX31865_FAULT_REFINHIGH) {
      Serial.println("REFIN- < 0.85 x Bias - FORCE- open"); 
    }
    if (fault & MAX31865_FAULT_RTDINLOW) {
      Serial.println("RTDIN- < 0.85 x Bias - FORCE- open"); 
    }
    if (fault & MAX31865_FAULT_OVUV) {
      Serial.println("Under/Over voltage"); 
    }
    thermo.clearFault();
  }
  Serial.println();
  delay(1000);

    // Verificar si se mantiene presionado para modo config
    if (BLEHandler::checkConfigMode()) {
        return;
    }

    // Obtener todas las lecturas de sensores (normales y Modbus)
    std::vector<SensorReading> normalReadings;
    std::vector<ModbusSensorReading> modbusReadings;
    SensorManager::getAllSensorReadings(normalReadings, modbusReadings, enabledNormalSensors, enabledModbusSensors);

    // Usar el nuevo formato delimitado en lugar de JSON
    LoRaManager::sendDelimitedPayload(normalReadings, modbusReadings, node, deviceId, stationId, rtc);

    // Calcular y mostrar el tiempo transcurrido antes de dormir
    unsigned long elapsedTime = millis() - setupStartTime;
    DEBUG_PRINTF("Tiempo transcurrido antes de sleep: %lu ms\n", elapsedTime);
    delay(10);

    // Dormir
    SleepManager::goToDeepSleep(timeToSleep, powerManager, &radio, node, LWsession, spiLora);
}
