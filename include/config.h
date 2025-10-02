#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include "sensor_types.h"

// =========================================================================
// 1. CONFIGURACIÓN DEL SISTEMA Y APLICACIÓN
// =========================================================================
namespace System {
    constexpr uint8_t CPU_FREQUENCY_MHZ = 40;
    constexpr uint32_t SERIAL_BAUD_RATE = 115200;
    constexpr uint32_t DEFAULT_TIME_TO_SLEEP = 30;
    constexpr const char* DEFAULT_DEVICE_ID = "DEV02";
    constexpr const char* DEFAULT_STATION_ID = "ST001";

    constexpr uint16_t JSON_DOC_SIZE_SMALL = 300;
    constexpr uint16_t JSON_DOC_SIZE_MEDIUM = 1024;
    constexpr uint16_t JSON_DOC_SIZE_LARGE = 2048;

    constexpr uint32_t MODBUS_BAUD_RATE = 9600;
    constexpr uint32_t MODBUS_SERIAL_CONFIG = SERIAL_8N1;
    constexpr uint16_t MODBUS_RESPONSE_TIMEOUT = 300;
    constexpr uint8_t MODBUS_MAX_RETRY = 3;
}

// =========================================================================
// 2. CONFIGURACIÓN DE PINES
// =========================================================================
namespace Pins {
    // Buses
    constexpr uint8_t ONE_WIRE_BUS = 33;
    constexpr uint8_t I2C_SDA = 48;
    constexpr uint8_t I2C_SCL = 47;

    // Serial
    constexpr uint8_t SERIAL1_RX = 43;
    constexpr uint8_t SERIAL1_TX = 44;

    // Control de batería
    constexpr uint8_t BATTERY_CONTROL = 37;

    // Pines analógicos para sensores
    constexpr uint8_t NTC100K = 5;
    constexpr uint8_t NTC10K = 6;
    constexpr uint8_t PH_SENSOR = 17;
    constexpr uint8_t COND_SENSOR = 20;
    constexpr uint8_t HDS10_SENSOR = 7;
    constexpr uint8_t BATTERY_SENSOR = 1;
    constexpr uint8_t SOILH_SENSOR = 2;

    // SPI para LoRa
    namespace LoRaSPI {
        constexpr uint8_t SCK = 9;
        constexpr uint8_t MISO = 11;
        constexpr uint8_t MOSI = 10;
        constexpr uint8_t NSS = 8;
        constexpr uint8_t BUSY = 13;
        constexpr uint8_t RST = 12;
        constexpr uint8_t DIO1 = 14;
    }

    // SPI para RTD
    namespace RtdSPI {
        constexpr uint8_t SCK = 39;
        constexpr uint8_t MISO = 40;
        constexpr uint8_t MOSI = 41;
        constexpr uint8_t PT100_CS = 46;
    }

    // Configuración y LEDs
    constexpr uint8_t CONFIG_PIN = 3;
    constexpr uint8_t CONFIG_LED = 35;
    constexpr uint8_t LED1 = 21;
    constexpr uint8_t LED2 = 19;

    // Flow sensor
    constexpr uint8_t FLOW_SENSOR = 4;

    // Power
    constexpr uint8_t POWER_3V3 = 36;
    constexpr uint8_t POWER_12V = 38;

    // Modbus
    constexpr uint8_t MODBUS_RX = 21;
    constexpr uint8_t MODBUS_TX = 26;
}

// =========================================================================
// 3. CONFIGURACIÓN DE LORA
// =========================================================================
namespace LoRa {
    constexpr uint32_t SPI_CLOCK = 1000000;
    constexpr uint16_t MAX_PAYLOAD = 200;

    // Usar pines definidos en ::Pins::LoRaSPI
    constexpr uint8_t NSS_PIN = ::Pins::LoRaSPI::NSS;
    constexpr uint8_t BUSY_PIN = ::Pins::LoRaSPI::BUSY;
    constexpr uint8_t RST_PIN = ::Pins::LoRaSPI::RST;
    constexpr uint8_t DIO1_PIN = ::Pins::LoRaSPI::DIO1;

    constexpr const char* DEFAULT_JOIN_EUI = "00,00,00,00,00,00,00,00";
    constexpr const char* DEFAULT_DEV_EUI = "09,15,41,f8,97,29,05,aa";
    constexpr const char* DEFAULT_APP_KEY = "81,c5,7b,02,11,26,b0,ef,94,52,71,be,78,d5,4d,60";
    constexpr const char* DEFAULT_NWK_KEY = "cd,9f,84,3d,bc,de,e3,e5,a3,ba,ea,9b,ad,2a,f8,fc";

    constexpr uint8_t SUBBAND = 2;

    /*
     * Lista de Data Rates (DR) para LoRaWAN US915
     *
     * Cada DR define un conjunto específico de parámetros:
     *   - SF (Spreading Factor): Cuanto mayor es, más alcance tiene pero menor velocidad de transmisión.
     *   - BW (Bandwidth): Mayor ancho de banda significa mayor velocidad pero menor alcance.
     *   - CR (Coding Rate): Relación de corrección de errores.
     *   - Payload Máximo: Tamaño máximo de datos en bytes que se pueden enviar en una sola transmisión.
     *
     * NOTA: DR5, DR6 y DR7 no están definidos en US915.
     *
     * Índice | SF   | BW     | CR  | Payload Máximo | Comentario
     * -------|------|--------|-----|----------------|------------------------------------
     * DR0    | SF10 | 125kHz | 4/5 | 19 bytes       | Mayor alcance, menor velocidad.
     * DR1    | SF9  | 125kHz | 4/5 | 61 bytes       | Menor alcance, más velocidad que DR0.
     * DR2    | SF8  | 125kHz | 4/5 | 133 bytes      | Velocidad media, menor alcance que DR1.
     * DR3    | SF7  | 125kHz | 4/5 | 250 bytes      | Mayor velocidad en 125kHz, menor alcance.
     * DR4    | SF8  | 500kHz | 4/5 | 250 bytes      | Mayor BW para mejorar la velocidad.
     * DR5    | N/A  | N/A    | N/A | No definido    | No definido en US915.
     * DR6    | N/A  | N/A    | N/A | No definido    | No definido en US915.
     * DR7    | N/A  | N/A    | N/A | No definido    | No definido en US915.
     * DR8    | SF12 | 500kHz | 4/5 | 41 bytes       | Mayor alcance en 500kHz, menor velocidad.
     * DR9    | SF11 | 500kHz | 4/5 | 117 bytes      | Más velocidad que DR8, menos alcance.
     * DR10   | SF10 | 500kHz | 4/5 | 230 bytes      | Más velocidad que DR9.
     * DR11   | SF9  | 500kHz | 4/5 | 230 bytes      | Velocidad media en 500kHz.
     * DR12   | SF8  | 500kHz | 4/5 | 230 bytes      | Más velocidad que DR11.
     * DR13   | SF7  | 500kHz | 4/5 | 230 bytes      | Mayor velocidad en 500kHz, menor alcance.
     * DR14   | N/A  | N/A    | N/A | No definido    | No definido en US915.
     *
     * - DR0 a DR3 se usan para uplink en los 64 canales de 125kHz.
     * - DR4 se usa para uplink en los 8 canales de 500kHz.
     * - DR8 a DR13 se usan para downlink en los 8 canales de 500kHz.
     * - El payload máximo puede verse afectado por la opción FOpt en el MAC layer.
     *
     * Uso recomendado:
     *   LoRaManager::setDatarate(node, 3);  // DR3 = SF7BW125, buen balance velocidad/alcance
     */

    // Data Rate por defecto (DR3 = SF7BW125)
    constexpr uint8_t DEFAULT_DATARATE = 3;
}

// =========================================================================
// 4. CONFIGURACIÓN DE BLE
// =========================================================================
namespace BLE {
    constexpr const char* SERVICE_UUID = "180A";
    constexpr const char* CHAR_SYSTEM_UUID = "2A37";
    constexpr const char* CHAR_SENSORS_UUID = "2A40";
    constexpr const char* CHAR_LORA_CONFIG_UUID = "2A41";
    constexpr const char* CHAR_NTC100K_UUID = "2A38";
    constexpr const char* CHAR_NTC10K_UUID = "2A39";
    constexpr const char* CHAR_CONDUCTIVITY_UUID = "2A3C";
    constexpr const char* CHAR_PH_UUID = "2A3B";
    constexpr const char* DEVICE_PREFIX = "AGRICOS-";

    constexpr uint32_t CONFIG_TRIGGER_TIME_MS = 5000;
    constexpr uint32_t CONFIG_TIMEOUT_MS = 30000;
    constexpr uint32_t CONFIG_WAIT_TIMEOUT_MS = 60000;
    constexpr uint32_t CONFIG_MAX_CONN_TIME_MS = 300000;
}

// =========================================================================
// 5. CLAVES JSON
// =========================================================================
namespace JsonKeys {
    // Namespaces
    constexpr const char* NS_SYSTEM = "system";
    constexpr const char* NS_SENSORS = "sensors";
    constexpr const char* NS_LORAWAN = "lorawan";
    constexpr const char* NS_LORA_SESSION = "lorasession";
    constexpr const char* NS_SENSORS_MODBUS = "sensors_modbus";
    constexpr const char* NS_SENSORS_ADC = "sensors_adc";
    constexpr const char* NS_NTC100K = "ntc_100k";
    constexpr const char* NS_NTC10K = "ntc_10k";
    constexpr const char* NS_COND = "cond";
    constexpr const char* NS_PH = "ph";

    // Claves generales
    constexpr const char* KEY_INITIALIZED = "initialized";
    constexpr const char* KEY_SLEEP_TIME = "sleep_time";
    constexpr const char* KEY_STATION_ID = "stationId";
    constexpr const char* KEY_DEVICE_ID = "deviceId";
    constexpr const char* KEY_VOLT = "volt";
    constexpr const char* KEY_SENSOR = "k";
    constexpr const char* KEY_SENSOR_ID = "id";
    constexpr const char* KEY_SENSOR_ID_TEMPERATURE = "ts";
    constexpr const char* KEY_SENSOR_TYPE = "t";
    constexpr const char* KEY_SENSOR_ENABLE = "e";

    // Claves LoRa
    constexpr const char* KEY_LORA_JOIN_EUI = "joinEUI";
    constexpr const char* KEY_LORA_DEV_EUI = "devEUI";
    constexpr const char* KEY_LORA_NWK_KEY = "nwkKey";
    constexpr const char* KEY_LORA_APP_KEY = "appKey";
    constexpr const char* KEY_LORAWAN_SESSION = "lorawan_session";

    // Claves Modbus
    constexpr const char* KEY_MODBUS_SENSOR_ID = "id";
    constexpr const char* KEY_MODBUS_SENSOR_TYPE = "t";
    constexpr const char* KEY_MODBUS_SENSOR_ADDR = "a";
    constexpr const char* KEY_MODBUS_SENSOR_ENABLE = "e";

    // Claves ADC
    constexpr const char* KEY_ADC_SENSOR = "k";
    constexpr const char* KEY_ADC_SENSOR_ID = "id";
    constexpr const char* KEY_ADC_SENSOR_TYPE = "t";
    constexpr const char* KEY_ADC_SENSOR_ENABLE = "e";

    // Claves NTC100K
    constexpr const char* KEY_NTC100K_T1 = "n100k_t1";
    constexpr const char* KEY_NTC100K_R1 = "n100k_r1";
    constexpr const char* KEY_NTC100K_T2 = "n100k_t2";
    constexpr const char* KEY_NTC100K_R2 = "n100k_r2";
    constexpr const char* KEY_NTC100K_T3 = "n100k_t3";
    constexpr const char* KEY_NTC100K_R3 = "n100k_r3";

    // Claves NTC10K
    constexpr const char* KEY_NTC10K_T1 = "n10k_t1";
    constexpr const char* KEY_NTC10K_R1 = "n10k_r1";
    constexpr const char* KEY_NTC10K_T2 = "n10k_t2";
    constexpr const char* KEY_NTC10K_R2 = "n10k_r2";
    constexpr const char* KEY_NTC10K_T3 = "n10k_t3";
    constexpr const char* KEY_NTC10K_R3 = "n10k_r3";

    // Claves Conductividad
    constexpr const char* KEY_CONDUCT_CT = "c_ct";
    constexpr const char* KEY_CONDUCT_CC = "c_cc";
    constexpr const char* KEY_CONDUCT_V1 = "c_v1";
    constexpr const char* KEY_CONDUCT_T1 = "c_t1";
    constexpr const char* KEY_CONDUCT_V2 = "c_v2";
    constexpr const char* KEY_CONDUCT_T2 = "c_t2";
    constexpr const char* KEY_CONDUCT_V3 = "c_v3";
    constexpr const char* KEY_CONDUCT_T3 = "c_t3";

    // Claves pH
    constexpr const char* KEY_PH_V1 = "ph_v1";
    constexpr const char* KEY_PH_T1 = "ph_t1";
    constexpr const char* KEY_PH_V2 = "ph_v2";
    constexpr const char* KEY_PH_T2 = "ph_t2";
    constexpr const char* KEY_PH_V3 = "ph_v3";
    constexpr const char* KEY_PH_T3 = "ph_t3";
    constexpr const char* KEY_PH_CT = "ph_ct";
}

// =========================================================================
// 6. CONFIGURACIÓN DE SENSORES
// =========================================================================
namespace Sensors {
    // Direcciones I2C
    constexpr uint8_t BME280_I2C_ADDR = 0x76;
    constexpr uint8_t SHT31_I2C_ADDR = 0x44;

    // Configuración RTD (PT100/PT1000)
    constexpr float RREF = 430.0f;
    constexpr float RNOMINAL = 100.0f;
    constexpr uint32_t SPI_RTD_CLOCK = 1000000;

    // Límites NTC
    constexpr float NTC_TEMP_MIN = -20.0f;
    constexpr float NTC_TEMP_MAX = 100.0f;

    // Tiempos
    constexpr uint16_t POWER_STABILIZE_DELAY_MS = 20; 
}

// =========================================================================
// 7. CALIBRACIÓN DE SENSORES
// =========================================================================
namespace Calibration {
    // Batería
    constexpr float BATTERY_R1 = 100000.0f;
    constexpr float BATTERY_R2 = 390000.0f;
    constexpr float BATTERY_CONVERSION_FACTOR = 1.0f / (BATTERY_R1 / (BATTERY_R1 + BATTERY_R2));

    // NTC 100K
    namespace NTC100K {
        constexpr float DEFAULT_T1 = 25.0f;
        constexpr float DEFAULT_R1 = 100000.0f;
        constexpr float DEFAULT_T2 = 35.0f;
        constexpr float DEFAULT_R2 = 64770.0f;
        constexpr float DEFAULT_T3 = 45.0f;
        constexpr float DEFAULT_R3 = 42530.0f;
    }

    // NTC 10K
    namespace NTC10K {
        constexpr float DEFAULT_T1 = 25.0f;
        constexpr float DEFAULT_R1 = 10000.0f;
        constexpr float DEFAULT_T2 = 35.0f;
        constexpr float DEFAULT_R2 = 6477.0f;
        constexpr float DEFAULT_T3 = 45.0f;
        constexpr float DEFAULT_R3 = 4253.0f;
    }

    // Conductividad
    namespace Conductivity {
        constexpr float DEFAULT_V1 = 0.010314f;
        constexpr float DEFAULT_T1 = 84.0f;
        constexpr float DEFAULT_V2 = 0.175384f;
        constexpr float DEFAULT_T2 = 1413.0f;
        constexpr float DEFAULT_V3 = 1.407745f;
        constexpr float DEFAULT_T3 = 12880.0f;
        constexpr float TEMP_COEF_COMPENSATION = 0.02f;
        constexpr float DEFAULT_TEMP = 24.22f;
    }

    // pH
    namespace PH {
        constexpr float DEFAULT_V1 = 0.4425f;
        constexpr float DEFAULT_T1 = 4.01f;
        constexpr float DEFAULT_V2 = 0.001f;
        constexpr float DEFAULT_T2 = 6.86f;
        constexpr float DEFAULT_V3 = -0.32155f;
        constexpr float DEFAULT_T3 = 9.18f;
        constexpr float DEFAULT_TEMP = 25.0f;
    }
}

// =========================================================================
// 8. CONFIGURACIONES POR DEFECTO DE SENSORES
// =========================================================================
namespace DEFAULT_CONFIGS {
    #define DEFAULT_SENSOR_CONFIGS { \
        {"B6", "BM6_1", BME680, false}, \
        {"C", "CO2_1", CO2, false}, \
        {"B2", "BM2_1", BME280, false}, \
        {"L", "LUX1", VEML7700, false}, \
        {"SH4", "SH4_1", SHT40, false}, \
        {"R", "RTD_1",  RTD, false}, \
        {"D", "DS_1",   DS18B20, false}, \
        {"SH3", "SH3_1", SHT30, false}, \
        {"MT05", "MT05_2", MT05S, true} \
    }

    #define DEFAULT_MODBUS_SENSOR_CONFIGS { \
        {"ModbusEnv1", ENV4, 1, false} \
    }

    #define DEFAULT_ADC_SENSOR_CONFIGS { \
        {"0", "NTC1",  N100K, false}, \
        {"2", "NTC3",  N10K, false}, \
        {"3", "HDS10", HDS10, false}, \
        {"4", "COND",  COND, false}, \
        {"5", "SM1",   SOILH, false}, \
        {"8", "PH",    PH, false} \
    }
}

// =========================================================================
// 9. OPCIONES DE DEPURACIÓN
// =========================================================================
#define DEBUG_ENABLED  // Comentar esta línea para desactivar los mensajes de depuración

#endif