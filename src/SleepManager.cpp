#include "SleepManager.h"
#include "debug.h"
#include "LoRaManager.h"
#include "esp_sleep.h"

void SleepManager::goToDeepSleep(uint32_t timeToSleep, 
                               PowerManager& powerManager,
                               SX1262* radio,
                               LoRaWANNode& node,
                               uint8_t* LWsession,
                               SPIClass& spiLora) {
    // Guardar sesión en RTC y otras rutinas de apagado
    uint8_t *persist = node.getBufferSession();
    memcpy(LWsession, persist, RADIOLIB_LORAWAN_SESSION_BUF_SIZE);
    
    // Apagar todos los reguladores
    powerManager.allPowerOff();
    
    // Flush Serial antes de dormir
    DEBUG_FLUSH();
    DEBUG_END();
    
    // Apagar módulos
    LoRaManager::prepareForSleep(radio);
    btStop();

    // Deshabilitar I2C y SPI
    Wire.end();
    spiLora.end();
    
    // Configurar el temporizador y GPIO para despertar
    esp_sleep_enable_timer_wakeup(timeToSleep * 1000000ULL);
    // gpio_wakeup_enable((gpio_num_t)CONFIG_PIN, GPIO_INTR_LOW_LEVEL);
    // esp_sleep_enable_gpio_wakeup();
    // esp_sleep_enable_ext0_wakeup((gpio_num_t)CONFIG_PIN, 0); // 0 para nivel bajo
    
    // // Configurar pines para deep sleep
    // configurePinsForDeepSleep();

    
    // Entrar en deep sleep
    esp_deep_sleep_start();
}

/**
 * @brief Configura los pines no utilizados en alta impedancia para reducir el consumo durante deep sleep.
 */
void SleepManager::configurePinsForDeepSleep() {
    // // Configurar pines específicos del módulo LoRa como ANALOG
    // pinMode(ONE_WIRE_BUS, ANALOG);

    // pinMode(LORA_RST_PIN, ANALOG);
    // pinMode(LORA_BUSY_PIN, ANALOG);
    // pinMode(LORA_DIO1_PIN, ANALOG);
    // pinMode(SPI_LORA_SCK_PIN, ANALOG);
    // pinMode(SPI_LORA_MISO_PIN, ANALOG);
    // pinMode(SPI_LORA_MOSI_PIN, ANALOG);

    // // Serial
    // pinMode(20, ANALOG); //Serial RX
    // pinMode(21, ANALOG); //Serial TX

    // // I2C
    // pinMode(I2C_SDA_PIN, ANALOG); //I2C SDA
    // pinMode(I2C_SCL_PIN, ANALOG); //I2C SCL

    // // Configurar explícitamente LORA_NSS_PIN como salida en alto para mantener el chip select del módulo LoRa desactivado
    // pinMode(LORA_NSS_PIN, OUTPUT);
    // digitalWrite(LORA_NSS_PIN, HIGH);
    // digitalWrite(CONFIG_PIN, HIGH);
    digitalWrite(POWER_3V3_PIN, LOW);
    // gpio_hold_en((gpio_num_t)LORA_NSS_PIN);
    // gpio_hold_en((gpio_num_t)CONFIG_PIN);
    gpio_hold_en((gpio_num_t)POWER_3V3_PIN);
}

/**
 * @brief Libera el estado de retención (hold) de los pines que fueron configurados para deep sleep.
 * Esto permite que los pines puedan ser reconfigurados adecuadamente tras salir del deep sleep.
 */
void SleepManager::releaseHeldPins() {
    // Liberar específicamente el pin NSS de LoRa
    // gpio_hold_dis((gpio_num_t)LORA_NSS_PIN);
    // gpio_hold_dis((gpio_num_t)CONFIG_PIN);
    gpio_hold_dis((gpio_num_t)POWER_3V3_PIN);
    // Liberar otros pines si se ha aplicado retención
}
