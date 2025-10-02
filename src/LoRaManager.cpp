/*******************************************************************************************
 * Archivo: src/LoRaManager.cpp
 * Descripción: Implementación de la gestión de comunicaciones LoRa y LoRaWAN.
 *              Se ha modificado la forma de serializar los valores float a 3 decimales
 *              usando snprintf("%.3f", ...) antes de asignarlos al JSON.
 *******************************************************************************************/

#include "LoRaManager.h"
#include <Preferences.h>
#include "debug.h"
#include <RadioLib.h>
#include <ESP32Time.h>
#include "utilities.h"  // Incluido para acceder a formatFloatTo3Decimals
#include "config.h"  // Incluido para acceder a todas las constantes de configuración
#include "sensor_types.h"  // Incluido para acceder a ModbusSensorReading
#include "config_manager.h"

LoRaWANNode* LoRaManager::node = nullptr;
SX1262* LoRaManager::radioModule = nullptr;

extern RTC_DATA_ATTR uint8_t LWsession[RADIOLIB_LORAWAN_SESSION_BUF_SIZE];
extern ESP32Time rtc;

int16_t LoRaManager::begin(SX1262* radio, const LoRaWANBand_t* region, uint8_t subBand) {
    radioModule = radio;
    int16_t state = radioModule->begin();
    if (state != RADIOLIB_ERR_NONE) {
        return state;
    }

    node = new LoRaWANNode(radio, region, subBand);
    return RADIOLIB_ERR_NONE;
}

int16_t LoRaManager::lwActivate(LoRaWANNode& node) {
    int16_t state = RADIOLIB_ERR_UNKNOWN;
    Preferences store;

    LoRaConfig loraConfig = ConfigManager::getLoRaConfig();

    uint64_t joinEUI = 0, devEUI = 0;
    if (!parseEUIString(loraConfig.joinEUI.c_str(), &joinEUI) ||
        !parseEUIString(loraConfig.devEUI.c_str(), &devEUI)) {
        return state;
    }

    uint8_t nwkKey[16], appKey[16];
    parseKeyString(loraConfig.nwkKey, nwkKey, 16);
    parseKeyString(loraConfig.appKey, appKey, 16);

    node.beginOTAA(joinEUI, devEUI, nwkKey, appKey);

    store.begin("radiolib");

    if (store.isKey("nonces")) {
        uint8_t buffer[RADIOLIB_LORAWAN_NONCES_BUF_SIZE];
        store.getBytes("nonces", buffer, RADIOLIB_LORAWAN_NONCES_BUF_SIZE);
        state = node.setBufferNonces(buffer);

        if (state == RADIOLIB_ERR_NONE) {
            state = node.setBufferSession(LWsession);

            if (state == RADIOLIB_ERR_NONE) {
                state = node.activateOTAA();

                if (state == RADIOLIB_LORAWAN_SESSION_RESTORED) {
                    store.end();
                    DEBUG_PRINTLN("Sesión LoRaWAN restaurada");

                    // IMPORTANTE: Deshabilitar ADR para evitar que el servidor cambie el DR
                    node.setADR(false);
                    DEBUG_PRINTLN("ADR deshabilitado - manteniendo DR fijo");

                    // Forzar DR3 después de restaurar sesión
                    node.setDatarate(LoRa::DEFAULT_DATARATE);
                    DEBUG_PRINTF("Data Rate forzado a DR%d (SF7BW125)\n", LoRa::DEFAULT_DATARATE);

                    // Configurar dwell time para US915 (400ms límite)
                    node.setDwellTime(true, 400);

                    return state;
                }
            }
        }
    } else {
        DEBUG_PRINTLN("No hay nonces guardados - iniciando nuevo join");
    }

    state = RADIOLIB_ERR_NETWORK_NOT_JOINED;
    while (state != RADIOLIB_LORAWAN_NEW_SESSION) {
        state = node.activateOTAA();

        if (state == RADIOLIB_LORAWAN_NEW_SESSION) {
            uint8_t buffer[RADIOLIB_LORAWAN_NONCES_BUF_SIZE];
            uint8_t *persist = node.getBufferNonces();
            memcpy(buffer, persist, RADIOLIB_LORAWAN_NONCES_BUF_SIZE);
            store.putBytes("nonces", buffer, RADIOLIB_LORAWAN_NONCES_BUF_SIZE);

            // Solicitar DeviceTime después de un join exitoso
            delay(1000); // Pausa para estabilización
            node.setADR(false);
            node.setDatarate(LoRa::DEFAULT_DATARATE);

            int rtcAttempts = 0;
            bool rtcUpdated = false;
            const int maxAttempts = 3; // Máximo número de intentos para actualizar RTC
            
            while (!rtcUpdated && rtcAttempts < maxAttempts) {
                rtcAttempts++;

                bool macCommandSuccess = node.sendMacCommandReq(RADIOLIB_LORAWAN_MAC_DEVICE_TIME);
                if (macCommandSuccess) {
                    uint8_t fPort = 1;
                    uint8_t downlinkPayload[255];
                    size_t downlinkSize = 0;

                    int16_t rxState = node.sendReceive(nullptr, 0, fPort, downlinkPayload, &downlinkSize, true);
                    if (rxState == RADIOLIB_ERR_NONE) {
                        uint32_t unixEpoch;
                        uint8_t fraction;
                        int16_t dtState = node.getMacDeviceTimeAns(&unixEpoch, &fraction, true);
                        if (dtState == RADIOLIB_ERR_NONE) {
                            DEBUG_PRINTF("DeviceTime recibido: epoch = %lu s, fraction = %u\n", unixEpoch, fraction);

                            rtc.setTime(unixEpoch);
                            
                            if (abs((int32_t)rtc.getEpoch() - (int32_t)unixEpoch) < 10) {
                                DEBUG_PRINTLN("RTC actualizado exitosamente con tiempo del servidor");
                                rtcUpdated = true; // Marca como actualizado para salir del bucle
                            } else {
                                DEBUG_PRINTLN("Error al actualizar RTC con tiempo del servidor");
                                // Si estamos en el último intento, continuamos de todas formas
                                if (rtcAttempts >= maxAttempts) {
                                    DEBUG_PRINTLN("Agotados los intentos de actualización de RTC");
                                } else {
                                    delay(1000); // Esperar un segundo antes del siguiente intento
                                }
                            }
                        } else {
                            DEBUG_PRINTF("Error al obtener DeviceTime: %d\n", dtState);
                            // Si estamos en el último intento, continuamos de todas formas
                            if (rtcAttempts >= maxAttempts) {
                                DEBUG_PRINTLN("Agotados los intentos de actualización de RTC");
                            } else {
                                delay(1000); // Esperar un segundo antes del siguiente intento
                            }
                        }
                    } else {
                        DEBUG_PRINTF("Error al recibir respuesta DeviceTime: %d\n", rxState);
                        // Si estamos en el último intento, continuamos de todas formas
                        if (rtcAttempts >= maxAttempts) {
                            DEBUG_PRINTLN("Agotados los intentos de actualización de RTC");
                        } else {
                            delay(1000); // Esperar un segundo antes del siguiente intento
                        }
                    }
                } else {
                    DEBUG_PRINTLN("Error al solicitar DeviceTime: comando no pudo ser encolado");
                    // Si estamos en el último intento, continuamos de todas formas
                    if (rtcAttempts >= maxAttempts) {
                        DEBUG_PRINTLN("Agotados los intentos de actualización de RTC");
                    } else {
                        delay(1000); // Esperar un segundo antes del siguiente intento
                    }
                }
            }

            // Si no se pudo actualizar el RTC después de los intentos máximos, retornar error
            if (!rtcUpdated) {
                DEBUG_PRINTLN("No se pudo actualizar el RTC después de los intentos máximos, entrando en deep sleep");
                store.end();
                return RADIOLIB_ERR_RTC_SYNC_FAILED; // Error personalizado para indicar fallo en sincronización RTC
            }

            store.end();
            return RADIOLIB_LORAWAN_NEW_SESSION;
        } else {
            DEBUG_PRINTF("Join falló: %d\n", state);
            store.end();
            return state;
        }
    }
    
    store.end();
    return state;
}

/**
 * @brief Crea un payload optimizado con formato delimitado por | y , en lugar de JSON.
 * @param readings Vector con lecturas de sensores.
 * @param deviceId ID del dispositivo.
 * @param stationId ID de la estación.
 * @param battery Valor de la batería.
 * @param timestamp Timestamp del sistema.
 * @param buffer Buffer donde se almacenará el payload.
 * @param bufferSize Tamaño del buffer.
 * @return Tamaño del payload generado.
 */
size_t LoRaManager::createDelimitedPayload(
    const std::vector<SensorReading>& readings,
    const String& deviceId,
    const String& stationId,
    float battery,
    uint32_t timestamp,
    char* buffer,
    size_t bufferSize
) {
    buffer[0] = '\0';
    size_t offset = 0;

    char batteryStr[16];
    formatFloatTo3Decimals(battery, batteryStr, sizeof(batteryStr));


    offset += snprintf(buffer + offset, bufferSize - offset,
                      "%s|%s|%s|%lu",
                      stationId.c_str(),
                      deviceId.c_str(),
                      batteryStr,
                      timestamp);

    for (const auto& reading : readings) {
        if (offset >= bufferSize - 1) break; // Evitar desbordamiento

        buffer[offset++] = '|';
        buffer[offset] = '\0';

        offset += snprintf(buffer + offset, bufferSize - offset,
                          "%s,%d",
                          reading.sensorId,
                          reading.type);

        if (reading.subValues.empty()) {
            char valStr[16];
            formatFloatTo3Decimals(reading.value, valStr, sizeof(valStr));
            offset += snprintf(buffer + offset, bufferSize - offset, ",%s", valStr);
        } else {
            for (const auto& sv : reading.subValues) {
                char valStr[16];
                formatFloatTo3Decimals(sv.value, valStr, sizeof(valStr));
                offset += snprintf(buffer + offset, bufferSize - offset, ",%s", valStr);
            }
        }
    }

    return offset;
}


/**
 * @brief Envía el payload de sensores estándar usando formato delimitado.
 * @param readings Vector con todas las lecturas de sensores.
 * @param node Referencia al nodo LoRaWAN
 * @param deviceId ID del dispositivo
 * @param stationId ID de la estación
 * @param rtc Referencia al RTC para obtener timestamp
 */
void LoRaManager::sendDelimitedPayload(
    const std::vector<SensorReading>& readings,
    LoRaWANNode& node,
    const String& deviceId,
    const String& stationId,
    ESP32Time& rtc)
{
    char payloadBuffer[LoRa::MAX_PAYLOAD + 1];

    float battery = NAN;
    for (const auto& reading : readings) {
        if (reading.type == BATTERY) {
            battery = reading.value;
            break;
        }
    }

    uint32_t timestamp = rtc.getEpoch();

    size_t payloadSize = createDelimitedPayload(
        readings,
        deviceId,
        stationId,
        battery,
        timestamp,
        payloadBuffer,
        sizeof(payloadBuffer)
    );

    DEBUG_PRINTF("Enviando payload delimitado con tamaño %d bytes\n", payloadSize);
    DEBUG_PRINTLN(payloadBuffer);

    uint8_t fPort = 1;
    uint8_t downlinkPayload[255];
    size_t downlinkSize = 0;

    // Log del tiempo transcurrido antes del envío LoRa
    extern unsigned long setupStartTime;
    unsigned long elapsedTime = millis() - setupStartTime;
    DEBUG_PRINTF("Tiempo transcurrido antes del envío LoRa: %lu ms\n", elapsedTime);

    // VERIFICACIÓN CRÍTICA: Asegurar DR3 antes de cada transmisión
    // Esto previene el error -1114 si el servidor intentó cambiar el DR
    node.setDatarate(LoRa::DEFAULT_DATARATE);

    // Usar uplink() en lugar de sendReceive() para NO esperar ventanas RX
    // Esto reduce significativamente el tiempo de transmisión
    int16_t state = node.uplink(
        (uint8_t*)payloadBuffer,
        payloadSize,
        fPort,
        false  // unconfirmed message
    );

    if (state == RADIOLIB_ERR_NONE) {
        DEBUG_PRINTLN("Transmisión exitosa (sin esperar downlink)");
    } else {
        DEBUG_PRINTF("Error en transmisión: %d", state);

        // Agregar descripción del error para mejor debugging
        switch(state) {
            case RADIOLIB_ERR_DWELL_TIME_EXCEEDED:
                DEBUG_PRINTLN(" - Dwell time excedido (payload muy grande para el DR actual)");
                break;
            case RADIOLIB_ERR_INVALID_FREQUENCY:
                DEBUG_PRINTLN(" - Frecuencia inválida");
                break;
            case RADIOLIB_ERR_TX_TIMEOUT:
                DEBUG_PRINTLN(" - Timeout en transmisión");
                break;
            default:
                DEBUG_PRINTLN("");
                break;
        }

        // Si es error de frecuencia, intentar reinicializar el radio
        if (state == RADIOLIB_ERR_INVALID_FREQUENCY) {
            DEBUG_PRINTLN("Error de frecuencia detectado - reinicialización requerida");
        }
    }
}


void LoRaManager::prepareForSleep(SX1262* radio) {
    if (radio) {
        radio->sleep(true);
    }
}

void LoRaManager::setDatarate(LoRaWANNode& node, uint8_t datarate) {
    node.setDatarate(datarate);
}
