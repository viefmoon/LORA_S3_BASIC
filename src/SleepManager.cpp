#include "SleepManager.h"
#include "config.h"
#include "debug.h"
#include "LoRaManager.h"
#include "esp_sleep.h"
#include "driver/rtc_io.h"

void SleepManager::goToDeepSleep(uint32_t timeToSleep,
                               SX1262* radio,
                               LoRaWANNode& node,
                               uint8_t* LWsession,
                               SPIClass& spiLora) {
    // Guardar sesión en RTC y otras rutinas de apagado
    uint8_t *persist = node.getBufferSession();
    memcpy(LWsession, persist, RADIOLIB_LORAWAN_SESSION_BUF_SIZE);

    // Flush Serial antes de dormir
    DEBUG_FLUSH();
    DEBUG_END();

    // Apagar módulos
    LoRaManager::prepareForSleep(radio);
    btStop();

    // Deshabilitar I2C y SPI
    Wire.end();
    spiLora.end();

    // Configurar pines para deep sleep (excepto CONFIG_PIN)
    configurePinsForDeepSleep();

    // --- INICIO: Configuración específica del pin de despertar ---
    gpio_num_t wakePin = (gpio_num_t)Pins::CONFIG_PIN;

    // 1. Desinicializar por si acaso (buena práctica)
    rtc_gpio_deinit(wakePin);

    // 2. Inicializar como RTC GPIO
    rtc_gpio_init(wakePin);

    // 3. Establecer dirección como ENTRADA
    rtc_gpio_set_direction(wakePin, RTC_GPIO_MODE_INPUT_ONLY);

    // 4. Habilitar PULL-UP (Incluso con externa, no hace daño y es robusto)
    rtc_gpio_pullup_en(wakePin);

    // 5. Deshabilitar PULL-DOWN explícitamente
    rtc_gpio_pulldown_dis(wakePin);

    // 6. Asegurarse de que el "hold" esté deshabilitado para este pin
    rtc_gpio_hold_dis(wakePin);
    // --- FIN: Configuración específica del pin de despertar ---

    // Configurar el temporizador y GPIO para despertar
    esp_sleep_enable_timer_wakeup(timeToSleep * 1000000ULL);
    esp_sleep_enable_ext0_wakeup(wakePin, 0); // 0 para nivel bajo

    esp_deep_sleep_start();
}

/**
 * @brief Configura los pines no utilizados en alta impedancia para reducir el consumo durante deep sleep.
 */
void SleepManager::configurePinsForDeepSleep() {
    // Configurar pines específicos del módulo LoRa como ANALOG
    pinMode(Pins::ONE_WIRE_BUS, ANALOG); //alta impedancia

    // Configurar pines I2C como ANALOG
    pinMode(Pins::I2C_SCL, ANALOG); //alta impedancia
    pinMode(Pins::I2C_SDA, ANALOG); //alta impedancia

    // Serial 1
    pinMode(Pins::SERIAL1_RX, ANALOG); //alta impedancia
    pinMode(Pins::SERIAL1_TX, ANALOG); //alta impedancia

    // Configurar pin de control de batería como salida
    pinMode(Pins::BATTERY_CONTROL, ANALOG); //alta impedancia

    // Configurar pines del módulo LoRa como ANALOG
    pinMode(LoRa::RST_PIN, ANALOG); //alta impedancia
    pinMode(LoRa::BUSY_PIN, ANALOG); //alta impedancia
    pinMode(LoRa::DIO1_PIN, ANALOG); //alta impedancia
    pinMode(Pins::LoRaSPI::SCK, ANALOG); //alta impedancia
    pinMode(Pins::LoRaSPI::MISO, ANALOG); //alta impedancia
    pinMode(Pins::LoRaSPI::MOSI, ANALOG); //alta impedancia

    // SPI PARA RTD
    pinMode(Pins::RtdSPI::SCK, ANALOG); //alta impedancia
    pinMode(Pins::RtdSPI::MISO, ANALOG); //alta impedancia
    pinMode(Pins::RtdSPI::MOSI, ANALOG); //alta impedancia

    // Configurar pines de sensores analógicos como ANALOG
    pinMode(Pins::NTC100K_0, ANALOG); //alta impedancia
    pinMode(Pins::NTC100K_1, ANALOG); //alta impedancia
    pinMode(Pins::NTC10K, ANALOG); //alta impedancia
    pinMode(Pins::PH_SENSOR, ANALOG); //alta impedancia
    pinMode(Pins::COND_SENSOR, ANALOG); //alta impedancia
    pinMode(Pins::HDS10_SENSOR, ANALOG); //alta impedancia
    pinMode(Pins::BATTERY_SENSOR, ANALOG); //alta impedancia
    pinMode(Pins::SOILH_SENSOR, ANALOG); //alta impedancia

    // Configurar pin de LED de configuración como alta impedancia
    pinMode(Pins::CONFIG_LED, ANALOG); //alta impedancia
    pinMode(Pins::LED1, ANALOG); //alta impedancia
    pinMode(Pins::LED2, ANALOG); //alta impedancia

    //Modbus
    pinMode(Pins::MODBUS_RX, ANALOG); //alta impedancia
    pinMode(Pins::MODBUS_TX, ANALOG); //alta impedancia

    // Alimentacion
    pinMode(Pins::POWER_3V3, ANALOG); //alta impedancia
    pinMode(Pins::POWER_12V, ANALOG); //alta impedancia

    // FlowSensor
    pinMode(Pins::FLOW_SENSOR, ANALOG); //alta impedancia

    digitalWrite(Pins::LoRaSPI::NSS, HIGH);
    gpio_hold_en((gpio_num_t)Pins::LoRaSPI::NSS);
    digitalWrite(Pins::RtdSPI::PT100_CS, HIGH);
    gpio_hold_en((gpio_num_t)Pins::RtdSPI::PT100_CS);
}

/**
 * @brief Libera el estado de retención (hold) de los pines que fueron configurados para deep sleep.
 * Esto permite que los pines puedan ser reconfigurados adecuadamente tras salir del deep sleep.
 */
void SleepManager::releaseHeldPins() {
    // Liberar otros pines si se ha aplicado retención
    gpio_hold_dis((gpio_num_t)Pins::LoRaSPI::NSS);
    gpio_hold_dis((gpio_num_t)Pins::RtdSPI::PT100_CS);
}

/**
 * @brief Maneja y determina la causa del despertar del dispositivo.
 * @param wokeFromConfigPin Referencia a la bandera que indica si el dispositivo despertó por el pin de configuración.
 */
void SleepManager::handleWakeupCause(bool& wokeFromConfigPin) {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
        DEBUG_PRINTLN("INFO: Despertado por EXT0 (CONFIG_PIN)");
        wokeFromConfigPin = true;
        // Pequeño delay para estabilizar/debounce inicial
        delay(50);
    } else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
        DEBUG_PRINTLN("INFO: Despertado por Timer");
        wokeFromConfigPin = false;
    } else {
        DEBUG_PRINTF("INFO: Despertado por otra razón: %d\n", wakeup_reason);
        wokeFromConfigPin = false;
    }
}
