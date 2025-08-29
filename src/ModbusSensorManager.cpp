#include "ModbusSensorManager.h"
#include "config.h"  // Para todas las constantes de configuración

#include "ModbusMaster.h"
#include "debug.h"     // Para DEBUG_END
#include "sensor_types.h" // Para todos los tipos y constantes de sensores
#include "utilities.h"
#include <string.h>

// Crear una instancia global de ModbusMaster
ModbusMaster modbus;

// Definir Serial2 para la comunicación Modbus
HardwareSerial modbusSerial(2); // Usar Serial2 para comunicación Modbus

/**
 * @note
 *  - Se usa la biblioteca ModbusMaster para la comunicación Modbus
 *  - Se asume que el pin DE/RE del transceiver RS485 está atado de forma que cuando
 *    se escribe en Serial, se habilita la transmisión, y al terminar, pasa a recepción.
 */

void ModbusSensorManager::beginModbus() {
    // Configurar Serial2 usando los parámetros definidos en config.h
    modbusSerial.begin(System::MODBUS_BAUD_RATE, System::MODBUS_SERIAL_CONFIG, Pins::MODBUS_RX, Pins::MODBUS_TX);

    // Inicializar ModbusMaster con Serial2
    modbus.begin(0, modbusSerial); // El slave ID se configurará en cada petición
}

void ModbusSensorManager::endModbus() {
    // Finalizar la comunicación Serial2 de Modbus
    modbusSerial.end();
}

bool ModbusSensorManager::readHoldingRegisters(uint8_t address, uint16_t startReg, uint16_t numRegs, uint16_t* outData) {
    // Guardar los bytes recibidos para depuración posterior
    uint8_t result;

    // Establecer el slave ID
    modbus.begin(address, modbusSerial);

    // Implementar reintentos de lectura
    for (uint8_t retry = 0; retry < System::MODBUS_MAX_RETRY; retry++) {
        // Registrar el tiempo de inicio para implementar timeout manual
        uint32_t startTime = millis();

        // Realizar la petición Modbus para leer registros holding
        result = modbus.readHoldingRegisters(startReg, numRegs);        if (result == modbus.ku8MBSuccess) {
            // Extraer datos de los registros
            for (uint8_t i = 0; i < numRegs; i++) {
                outData[i] = modbus.getResponseBuffer(i);
            }

            return true;
        }        if ((millis() - startTime) >= System::MODBUS_RESPONSE_TIMEOUT) {
            DEBUG_PRINTLN("Timeout en comunicación Modbus");
            break; // Salir del bucle de reintentos si se agota el tiempo
        }

        // Si no es el último intento, continuar con el siguiente intento
        DEBUG_PRINTF("Intento %d fallido, código: %d\n", retry + 1, result);
    }    DEBUG_PRINTF("Error Modbus después de %d intentos\n", System::MODBUS_MAX_RETRY);
    return false;
}

