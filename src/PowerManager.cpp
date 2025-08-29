#include "PowerManager.h"
#include "config.h" // Para todas las constantes de configuración

void PowerManager::begin() {
    // Configurar pines como salidas
    pinMode(Pins::POWER_3V3, OUTPUT);
    pinMode(Pins::POWER_12V, OUTPUT);

    // Asegurar que todas las fuentes están apagadas al inicio
    allPowerOff();
}

void PowerManager::power3V3On() {
    digitalWrite(Pins::POWER_3V3, LOW);
    delay(Sensors::POWER_STABILIZE_DELAY_MS);
}

void PowerManager::power3V3Off() {
    digitalWrite(Pins::POWER_3V3, HIGH);
}

void PowerManager::power12VOn() {
    digitalWrite(Pins::POWER_12V, HIGH);
    delay(Sensors::POWER_STABILIZE_DELAY_MS);
}

void PowerManager::power12VOff() {
    digitalWrite(Pins::POWER_12V, LOW);
}

void PowerManager::allPowerOff() {
    power3V3Off();
    power12VOff();
}