#ifndef PINS_CONFIG_H
#define PINS_CONFIG_H

// Pines one wire
#define ONE_WIRE_BUS        38

// Pines I2C
#define I2C_SDA_PIN         48
#define I2C_SCL_PIN         47

//Serial 1
#define SERIAL1_RX_PIN      43
#define SERIAL1_TX_PIN      44

// Pines para activar medición de batería
#define BATTERY_CONTROL_PIN 37

// Pines analógicos para sensores
#define NTC100K_0_PIN       3  // IO3
#define NTC100K_1_PIN       5  // IO5 
#define NTC10K_PIN          6  // IO6 
#define PH_SENSOR_PIN       17  // IO17 
#define COND_SENSOR_PIN     20  // IO20 
#define HDS10_SENSOR_PIN    7  // IO7 
#define BATTERY_SENSOR_PIN  1  // IO1 
#define SOILH_SENSOR_PIN    2  // IO2 // Pin para el sensor de humedad del suelo

// SPI PARA LORA
#define SPI_LORA_SCK_PIN        9
#define SPI_LORA_MISO_PIN       11
#define SPI_LORA_MOSI_PIN       10
#define LORA_NSS_PIN            8
#define LORA_BUSY_PIN           13
#define LORA_RST_PIN            12
#define LORA_DIO1_PIN           14

// SPI PARA RTD
#define SPI_SCK_PIN        39
#define SPI_MISO_PIN       40
#define SPI_MOSI_PIN       41

// PT100
#define PT100_CS_PIN        46

// Modo Config
#define CONFIG_PIN          2
#define CONFIG_LED_PIN      35

//Leds
#define LED1_PIN      33
#define LED2_PIN      34

// FlowSensor
#define FLOW_SENSOR_PIN         4

// Batería
#define POWER_3V3_PIN           36
#define POWER_12V_PIN           19

// Modbus
#define MODBUS_RX_PIN           21
#define MODBUS_TX_PIN           26

#endif // PINS_CONFIG_H 