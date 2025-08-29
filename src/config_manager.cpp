#include "config_manager.h"
#include <ArduinoJson.h>
#include <vector>
#include "sensor_types.h"
#include <Preferences.h>
#include <Arduino.h> // Incluido para usar Serial
#include "config.h" // Incluido para acceder a las constantes de configuración

/* =========================================================================
   FUNCIONES AUXILIARES
   ========================================================================= */
// Funciones auxiliares para leer y escribir el JSON completo en cada namespace.
static void writeNamespace(const char* ns, const StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM>& doc) {
    Preferences prefs;
    prefs.begin(ns, false);
    String jsonString;
    serializeJson(doc, jsonString);
    // Se usa el mismo nombre del namespace como clave interna
    prefs.putString(ns, jsonString.c_str());
    prefs.end();
}

static void readNamespace(const char* ns, StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM>& doc) {
    Preferences prefs;
    prefs.begin(ns, true);
    String jsonString = prefs.getString(ns, "{}");
    prefs.end();
    deserializeJson(doc, jsonString);
}

// Configuración por defecto de sensores NO-modbus
const SensorConfig ConfigManager::defaultConfigs[] = DEFAULT_SENSOR_CONFIGS;

// Configuración por defecto de sensores Modbus
const ModbusSensorConfig ConfigManager::defaultModbusSensors[] = DEFAULT_MODBUS_SENSOR_CONFIGS;

// Configuración por defecto de sensores ADC
const SensorConfig ConfigManager::defaultAdcSensors[] = DEFAULT_ADC_SENSOR_CONFIGS;

/* =========================================================================
   INICIALIZACIÓN Y CONFIGURACIÓN DEL SISTEMA
   ========================================================================= */
bool ConfigManager::checkInitialized() {
    StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
    readNamespace(JsonKeys::NS_SYSTEM, doc);
    return doc[JsonKeys::KEY_INITIALIZED] | false;
}

void ConfigManager::initializeDefaultConfig() {
    /* -------------------------------------------------------------------------
       1. INICIALIZACIÓN DE CONFIGURACIÓN DEL SISTEMA
       ------------------------------------------------------------------------- */
    // Sistema unificado: JsonKeys::NS_SYSTEM (incluye system, sleep y device)
    // Común para todos los tipos de dispositivo
    {
        StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
        doc[JsonKeys::KEY_STATION_ID] = System::DEFAULT_STATION_ID;
        doc[JsonKeys::KEY_INITIALIZED] = true;
        doc[JsonKeys::KEY_SLEEP_TIME] = System::DEFAULT_TIME_TO_SLEEP;
        doc[JsonKeys::KEY_DEVICE_ID] = System::DEFAULT_DEVICE_ID;
        writeNamespace(JsonKeys::NS_SYSTEM, doc);
    }
    
    /* -------------------------------------------------------------------------
       2. INICIALIZACIÓN DE SENSORES ANALÓGICOS
       ------------------------------------------------------------------------- */
    // NTC 100K: JsonKeys::NS_NTC100K
    {
        StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
        doc[JsonKeys::KEY_NTC100K_T1] = Calibration::NTC100K::DEFAULT_T1;
        doc[JsonKeys::KEY_NTC100K_R1] = Calibration::NTC100K::DEFAULT_R1;
        doc[JsonKeys::KEY_NTC100K_T2] = Calibration::NTC100K::DEFAULT_T2;
        doc[JsonKeys::KEY_NTC100K_R2] = Calibration::NTC100K::DEFAULT_R2;
        doc[JsonKeys::KEY_NTC100K_T3] = Calibration::NTC100K::DEFAULT_T3;
        doc[JsonKeys::KEY_NTC100K_R3] = Calibration::NTC100K::DEFAULT_R3;
        writeNamespace(JsonKeys::NS_NTC100K, doc);
    }
    
    // NTC 10K: JsonKeys::NS_NTC10K
    {
        StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
        doc[JsonKeys::KEY_NTC10K_T1] = Calibration::NTC10K::DEFAULT_T1;
        doc[JsonKeys::KEY_NTC10K_R1] = Calibration::NTC10K::DEFAULT_R1;
        doc[JsonKeys::KEY_NTC10K_T2] = Calibration::NTC10K::DEFAULT_T2;
        doc[JsonKeys::KEY_NTC10K_R2] = Calibration::NTC10K::DEFAULT_R2;
        doc[JsonKeys::KEY_NTC10K_T3] = Calibration::NTC10K::DEFAULT_T3;
        doc[JsonKeys::KEY_NTC10K_R3] = Calibration::NTC10K::DEFAULT_R3;
        writeNamespace(JsonKeys::NS_NTC10K, doc);
    }
    
    // Conductividad: JsonKeys::NS_COND
    {
        StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
        doc[JsonKeys::KEY_CONDUCT_CT] = Calibration::Conductivity::DEFAULT_TEMP;
        doc[JsonKeys::KEY_CONDUCT_CC] = Calibration::Conductivity::TEMP_COEF_COMPENSATION;
        doc[JsonKeys::KEY_CONDUCT_V1] = Calibration::Conductivity::DEFAULT_V1;
        doc[JsonKeys::KEY_CONDUCT_T1] = Calibration::Conductivity::DEFAULT_T1;
        doc[JsonKeys::KEY_CONDUCT_V2] = Calibration::Conductivity::DEFAULT_V2;
        doc[JsonKeys::KEY_CONDUCT_T2] = Calibration::Conductivity::DEFAULT_T2;
        doc[JsonKeys::KEY_CONDUCT_V3] = Calibration::Conductivity::DEFAULT_V3;
        doc[JsonKeys::KEY_CONDUCT_T3] = Calibration::Conductivity::DEFAULT_T3;
        writeNamespace(JsonKeys::NS_COND, doc);
    }
    
    // pH: JsonKeys::NS_PH
    {
        StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
        doc[JsonKeys::KEY_PH_V1] = Calibration::PH::DEFAULT_V1;
        doc[JsonKeys::KEY_PH_T1] = Calibration::PH::DEFAULT_T1;
        doc[JsonKeys::KEY_PH_V2] = Calibration::PH::DEFAULT_V2;
        doc[JsonKeys::KEY_PH_T2] = Calibration::PH::DEFAULT_T2;
        doc[JsonKeys::KEY_PH_V3] = Calibration::PH::DEFAULT_V3;
        doc[JsonKeys::KEY_PH_T3] = Calibration::PH::DEFAULT_T3;
        doc[JsonKeys::KEY_PH_CT] = Calibration::PH::DEFAULT_TEMP;
        writeNamespace(JsonKeys::NS_PH, doc);
    }
    
    /* -------------------------------------------------------------------------
       3. INICIALIZACIÓN DE SENSORES NO-MODBUS
       ------------------------------------------------------------------------- */
    {
        Preferences prefs;
        prefs.begin(JsonKeys::NS_SENSORS, false);
        StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
        JsonArray sensorArray = doc.to<JsonArray>(); // Array raíz

        for (const auto& config : ConfigManager::defaultConfigs) {
            JsonObject sensorObj = sensorArray.createNestedObject();
            sensorObj[JsonKeys::KEY_SENSOR] = config.configKey;
            sensorObj[JsonKeys::KEY_SENSOR_ID] = config.sensorId;
            sensorObj[JsonKeys::KEY_SENSOR_TYPE] = static_cast<int>(config.type);
            sensorObj[JsonKeys::KEY_SENSOR_ENABLE] = config.enable;
        }
        
        String jsonString;
        serializeJson(doc, jsonString);
        prefs.putString(JsonKeys::NS_SENSORS, jsonString.c_str());
        prefs.end();
    }
    
    /* -------------------------------------------------------------------------
       4. INICIALIZACIÓN DE CONFIGURACIÓN DE LORA
       ------------------------------------------------------------------------- */
    {
        StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
        doc[JsonKeys::KEY_LORA_JOIN_EUI]      = LoRa::DEFAULT_JOIN_EUI;
        doc[JsonKeys::KEY_LORA_DEV_EUI]       = LoRa::DEFAULT_DEV_EUI;
        doc[JsonKeys::KEY_LORA_NWK_KEY]       = LoRa::DEFAULT_NWK_KEY;
        doc[JsonKeys::KEY_LORA_APP_KEY]       = LoRa::DEFAULT_APP_KEY;
        writeNamespace(JsonKeys::NS_LORAWAN, doc);
    }

    /* -------------------------------------------------------------------------
       5. INICIALIZACIÓN DE SENSORES MODBUS
       ------------------------------------------------------------------------- */
    {
        Preferences prefs;
        prefs.begin(JsonKeys::NS_SENSORS_MODBUS, false);
        StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
        JsonArray sensorArray = doc.to<JsonArray>(); 

        // Cargamos un default (definido en config.h)
        size_t count = sizeof(defaultModbusSensors)/sizeof(defaultModbusSensors[0]);

        for (size_t i = 0; i < count; i++) {
            JsonObject sensorObj = sensorArray.createNestedObject();
            sensorObj[JsonKeys::KEY_MODBUS_SENSOR_ID]    = defaultModbusSensors[i].sensorId;
            sensorObj[JsonKeys::KEY_MODBUS_SENSOR_TYPE]  = (int)defaultModbusSensors[i].type;
            sensorObj[JsonKeys::KEY_MODBUS_SENSOR_ADDR] = defaultModbusSensors[i].address;
            sensorObj[JsonKeys::KEY_MODBUS_SENSOR_ENABLE]   = defaultModbusSensors[i].enable;
        }
        
        String jsonString;
        serializeJson(doc, jsonString);
        prefs.putString(JsonKeys::NS_SENSORS_MODBUS, jsonString.c_str());
        prefs.end();
    }

    /* -------------------------------------------------------------------------
       6. INICIALIZACIÓN DE SENSORES ADC
       ------------------------------------------------------------------------- */
    {
        Preferences prefs;
        prefs.begin(JsonKeys::NS_SENSORS_ADC, false);
        StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
        JsonArray sensorArray = doc.to<JsonArray>(); 

        // Cargamos sensores ADC por defecto
        size_t count = sizeof(defaultAdcSensors)/sizeof(defaultAdcSensors[0]);

        for (size_t i = 0; i < count; i++) {
            JsonObject sensorObj = sensorArray.createNestedObject();
            sensorObj[JsonKeys::KEY_ADC_SENSOR]      = defaultAdcSensors[i].configKey;
            sensorObj[JsonKeys::KEY_ADC_SENSOR_ID]   = defaultAdcSensors[i].sensorId;
            sensorObj[JsonKeys::KEY_ADC_SENSOR_TYPE] = (int)defaultAdcSensors[i].type;
            sensorObj[JsonKeys::KEY_ADC_SENSOR_ENABLE] = defaultAdcSensors[i].enable;
        }
        
        String jsonString;
        serializeJson(doc, jsonString);
        prefs.putString(JsonKeys::NS_SENSORS_ADC, jsonString.c_str());
        prefs.end();
    }
}

void ConfigManager::getSystemConfig(bool &initialized, uint32_t &sleepTime, String &deviceId, String &stationId) {
    StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
    readNamespace(JsonKeys::NS_SYSTEM, doc);
    initialized = doc[JsonKeys::KEY_INITIALIZED] | false;
    sleepTime = doc[JsonKeys::KEY_SLEEP_TIME] | System::DEFAULT_TIME_TO_SLEEP;
    deviceId = String(doc[JsonKeys::KEY_DEVICE_ID] | System::DEFAULT_DEVICE_ID);
    stationId = String(doc[JsonKeys::KEY_STATION_ID] | System::DEFAULT_STATION_ID);
}

void ConfigManager::setSystemConfig(bool initialized, uint32_t sleepTime, const String &deviceId, const String &stationId) {
    StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
    readNamespace(JsonKeys::NS_SYSTEM, doc);
    doc[JsonKeys::KEY_INITIALIZED] = initialized;
    doc[JsonKeys::KEY_SLEEP_TIME] = sleepTime;
    doc[JsonKeys::KEY_DEVICE_ID] = deviceId;
    doc[JsonKeys::KEY_STATION_ID] = stationId;
    writeNamespace(JsonKeys::NS_SYSTEM, doc);
}

/* =========================================================================
   CONFIGURACIÓN DE SENSORES NO-MODBUS
   ========================================================================= */
std::vector<SensorConfig> ConfigManager::getAllSensorConfigs() {
    std::vector<SensorConfig> configs;
    StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
    readNamespace(JsonKeys::NS_SENSORS, doc);
    
    if (!doc.is<JsonArray>()) {
        // Si no es un arreglo, no hay nada que leer
        return configs;
    }
    
    JsonArray sensorArray = doc.as<JsonArray>();
    for (JsonObject sensorObj : sensorArray) {
        SensorConfig config;
        const char* cKey = sensorObj[JsonKeys::KEY_SENSOR] | "";
        strncpy(config.configKey, cKey, sizeof(config.configKey));
        const char* sensorId = sensorObj[JsonKeys::KEY_SENSOR_ID] | "";
        strncpy(config.sensorId, sensorId, sizeof(config.sensorId));
        config.type = static_cast<SensorType>(sensorObj[JsonKeys::KEY_SENSOR_TYPE] | 0);
        config.enable = sensorObj[JsonKeys::KEY_SENSOR_ENABLE] | false;
        
        configs.push_back(config);
    }
    
    return configs;
}

std::vector<SensorConfig> ConfigManager::getEnabledSensorConfigs() {
    std::vector<SensorConfig> allSensors = getAllSensorConfigs();
    
    std::vector<SensorConfig> enabledSensors;
    for (const auto& sensor : allSensors) {
        if (sensor.enable && strlen(sensor.sensorId) > 0) {
            enabledSensors.push_back(sensor);
        }
    }
    
    return enabledSensors;
}

void ConfigManager::setSensorsConfigs(const std::vector<SensorConfig>& configs) {
    Preferences prefs;
    prefs.begin(JsonKeys::NS_SENSORS, false);
    StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
    JsonArray sensorArray = doc.to<JsonArray>();
    
    for (const auto& sensor : configs) {
        JsonObject sensorObj = sensorArray.createNestedObject();
        sensorObj[JsonKeys::KEY_SENSOR] = sensor.configKey;
        sensorObj[JsonKeys::KEY_SENSOR_ID] = sensor.sensorId;
        sensorObj[JsonKeys::KEY_SENSOR_TYPE] = static_cast<int>(sensor.type);
        sensorObj[JsonKeys::KEY_SENSOR_ENABLE] = sensor.enable;
    }
    
    String jsonString;
    serializeJson(doc, jsonString);
    prefs.putString(JsonKeys::NS_SENSORS, jsonString.c_str());
    prefs.end();
}

/* =========================================================================
   CONFIGURACIÓN DE LORA
   ========================================================================= */
LoRaConfig ConfigManager::getLoRaConfig() {
    StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
    readNamespace(JsonKeys::NS_LORAWAN, doc);
    
    LoRaConfig config;
    config.joinEUI  = doc[JsonKeys::KEY_LORA_JOIN_EUI] | LoRa::DEFAULT_JOIN_EUI;
    config.devEUI   = doc[JsonKeys::KEY_LORA_DEV_EUI]  | LoRa::DEFAULT_DEV_EUI;
    config.nwkKey   = doc[JsonKeys::KEY_LORA_NWK_KEY]  | LoRa::DEFAULT_NWK_KEY;
    config.appKey   = doc[JsonKeys::KEY_LORA_APP_KEY]  | LoRa::DEFAULT_APP_KEY;
    
    return config;
}

void ConfigManager::setLoRaConfig(
    const String &joinEUI, 
    const String &devEUI, 
    const String &nwkKey, 
    const String &appKey) {
    StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
    readNamespace(JsonKeys::NS_LORAWAN, doc);
    
    doc[JsonKeys::KEY_LORA_JOIN_EUI] = joinEUI;
    doc[JsonKeys::KEY_LORA_DEV_EUI]  = devEUI;
    doc[JsonKeys::KEY_LORA_NWK_KEY]  = nwkKey;
    doc[JsonKeys::KEY_LORA_APP_KEY]  = appKey;
    
    writeNamespace(JsonKeys::NS_LORAWAN, doc);
}

/* =========================================================================
   CONFIGURACIÓN DE SENSORES MODBUS
   ========================================================================= */
void ConfigManager::setModbusSensorsConfigs(const std::vector<ModbusSensorConfig>& configs) {
    Preferences prefs;
    prefs.begin(JsonKeys::NS_SENSORS_MODBUS, false);
    StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
    JsonArray sensorArray = doc.to<JsonArray>();
    
    for (const auto& sensor : configs) {
        JsonObject sensorObj = sensorArray.createNestedObject();
        sensorObj[JsonKeys::KEY_MODBUS_SENSOR_ID] = sensor.sensorId;
        sensorObj[JsonKeys::KEY_MODBUS_SENSOR_TYPE] = static_cast<int>(sensor.type);
        sensorObj[JsonKeys::KEY_MODBUS_SENSOR_ADDR] = sensor.address;
        sensorObj[JsonKeys::KEY_MODBUS_SENSOR_ENABLE] = sensor.enable;
    }
    
    String jsonString;
    serializeJson(doc, jsonString);
    prefs.putString(JsonKeys::NS_SENSORS_MODBUS, jsonString.c_str());
    prefs.end();
}

std::vector<ModbusSensorConfig> ConfigManager::getAllModbusSensorConfigs() {
    StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
    readNamespace(JsonKeys::NS_SENSORS_MODBUS, doc);
    
    std::vector<ModbusSensorConfig> configs;
    
    if (doc.is<JsonArray>()) {
        JsonArray array = doc.as<JsonArray>();
        
        for (JsonObject sensorObj : array) {
            ModbusSensorConfig config;
            strlcpy(config.sensorId, sensorObj[JsonKeys::KEY_MODBUS_SENSOR_ID] | "", sizeof(config.sensorId));
            config.type = static_cast<SensorType>(sensorObj[JsonKeys::KEY_MODBUS_SENSOR_TYPE] | 0);
            config.address = sensorObj[JsonKeys::KEY_MODBUS_SENSOR_ADDR] | 1;
            config.enable = sensorObj[JsonKeys::KEY_MODBUS_SENSOR_ENABLE] | false;
            
            configs.push_back(config);
        }
    }
    
    return configs;
}

std::vector<ModbusSensorConfig> ConfigManager::getEnabledModbusSensorConfigs() {
    std::vector<ModbusSensorConfig> all = getAllModbusSensorConfigs();
    std::vector<ModbusSensorConfig> enabled;
    for (auto &m : all) {
        if (m.enable) {
            enabled.push_back(m);
        }
    }
    return enabled;
}

/* =========================================================================
   CONFIGURACIÓN DE SENSORES ANALÓGICOS
   ========================================================================= */
void ConfigManager::getNTC100KConfig(double& t1, double& r1, double& t2, double& r2, double& t3, double& r3) {
    StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
    readNamespace(JsonKeys::NS_NTC100K, doc);
    t1 = doc[JsonKeys::KEY_NTC100K_T1] | Calibration::NTC100K::DEFAULT_T1;
    r1 = doc[JsonKeys::KEY_NTC100K_R1] | Calibration::NTC100K::DEFAULT_R1;
    t2 = doc[JsonKeys::KEY_NTC100K_T2] | Calibration::NTC100K::DEFAULT_T2;
    r2 = doc[JsonKeys::KEY_NTC100K_R2] | Calibration::NTC100K::DEFAULT_R2;
    t3 = doc[JsonKeys::KEY_NTC100K_T3] | Calibration::NTC100K::DEFAULT_T3;
    r3 = doc[JsonKeys::KEY_NTC100K_R3] | Calibration::NTC100K::DEFAULT_R3;
}

void ConfigManager::setNTC100KConfig(double t1, double r1, double t2, double r2, double t3, double r3) {
    StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
    readNamespace(JsonKeys::NS_NTC100K, doc);
    doc[JsonKeys::KEY_NTC100K_T1] = t1;
    doc[JsonKeys::KEY_NTC100K_R1] = r1;
    doc[JsonKeys::KEY_NTC100K_T2] = t2;
    doc[JsonKeys::KEY_NTC100K_R2] = r2;
    doc[JsonKeys::KEY_NTC100K_T3] = t3;
    doc[JsonKeys::KEY_NTC100K_R3] = r3;
    writeNamespace(JsonKeys::NS_NTC100K, doc);
}

void ConfigManager::getNTC10KConfig(double& t1, double& r1, double& t2, double& r2, double& t3, double& r3) {
    StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
    readNamespace(JsonKeys::NS_NTC10K, doc);
    t1 = doc[JsonKeys::KEY_NTC10K_T1] | Calibration::NTC10K::DEFAULT_T1;
    r1 = doc[JsonKeys::KEY_NTC10K_R1] | Calibration::NTC10K::DEFAULT_R1;
    t2 = doc[JsonKeys::KEY_NTC10K_T2] | Calibration::NTC10K::DEFAULT_T2;
    r2 = doc[JsonKeys::KEY_NTC10K_R2] | Calibration::NTC10K::DEFAULT_R2;
    t3 = doc[JsonKeys::KEY_NTC10K_T3] | Calibration::NTC10K::DEFAULT_T3;
    r3 = doc[JsonKeys::KEY_NTC10K_R3] | Calibration::NTC10K::DEFAULT_R3;
}

void ConfigManager::setNTC10KConfig(double t1, double r1, double t2, double r2, double t3, double r3) {
    StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
    readNamespace(JsonKeys::NS_NTC10K, doc);
    doc[JsonKeys::KEY_NTC10K_T1] = t1;
    doc[JsonKeys::KEY_NTC10K_R1] = r1;
    doc[JsonKeys::KEY_NTC10K_T2] = t2;
    doc[JsonKeys::KEY_NTC10K_R2] = r2;
    doc[JsonKeys::KEY_NTC10K_T3] = t3;
    doc[JsonKeys::KEY_NTC10K_R3] = r3;
    writeNamespace(JsonKeys::NS_NTC10K, doc);
}

void ConfigManager::getConductivityConfig(float& calTemp, float& coefComp, 
                                           float& v1, float& t1, float& v2, float& t2, float& v3, float& t3) {
    StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
    readNamespace(JsonKeys::NS_COND, doc);
    calTemp = doc[JsonKeys::KEY_CONDUCT_CT] | Calibration::Conductivity::DEFAULT_TEMP;
    coefComp = doc[JsonKeys::KEY_CONDUCT_CC] | Calibration::Conductivity::TEMP_COEF_COMPENSATION;
    v1 = doc[JsonKeys::KEY_CONDUCT_V1] | Calibration::Conductivity::DEFAULT_V1;
    t1 = doc[JsonKeys::KEY_CONDUCT_T1] | Calibration::Conductivity::DEFAULT_T1;
    v2 = doc[JsonKeys::KEY_CONDUCT_V2] | Calibration::Conductivity::DEFAULT_V2;
    t2 = doc[JsonKeys::KEY_CONDUCT_T2] | Calibration::Conductivity::DEFAULT_T2;
    v3 = doc[JsonKeys::KEY_CONDUCT_V3] | Calibration::Conductivity::DEFAULT_V3;
    t3 = doc[JsonKeys::KEY_CONDUCT_T3] | Calibration::Conductivity::DEFAULT_T3;
}

void ConfigManager::setConductivityConfig(float calTemp, float coefComp,
                                           float v1, float t1, float v2, float t2, float v3, float t3) {
    StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
    readNamespace(JsonKeys::NS_COND, doc);
    doc[JsonKeys::KEY_CONDUCT_CT] = calTemp;
    doc[JsonKeys::KEY_CONDUCT_CC] = coefComp;
    doc[JsonKeys::KEY_CONDUCT_V1] = v1;
    doc[JsonKeys::KEY_CONDUCT_T1] = t1;
    doc[JsonKeys::KEY_CONDUCT_V2] = v2;
    doc[JsonKeys::KEY_CONDUCT_T2] = t2;
    doc[JsonKeys::KEY_CONDUCT_V3] = v3;
    doc[JsonKeys::KEY_CONDUCT_T3] = t3;
    writeNamespace(JsonKeys::NS_COND, doc);
}

void ConfigManager::getPHConfig(float& v1, float& t1, float& v2, float& t2, float& v3, float& t3, float& defaultTemp) {
    StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
    readNamespace(JsonKeys::NS_PH, doc);
    v1 = doc[JsonKeys::KEY_PH_V1] | Calibration::PH::DEFAULT_V1;
    t1 = doc[JsonKeys::KEY_PH_T1] | Calibration::PH::DEFAULT_T1;
    v2 = doc[JsonKeys::KEY_PH_V2] | Calibration::PH::DEFAULT_V2;
    t2 = doc[JsonKeys::KEY_PH_T2] | Calibration::PH::DEFAULT_T2;
    v3 = doc[JsonKeys::KEY_PH_V3] | Calibration::PH::DEFAULT_V3;
    t3 = doc[JsonKeys::KEY_PH_T3] | Calibration::PH::DEFAULT_T3;
    defaultTemp = doc[JsonKeys::KEY_PH_CT] | Calibration::PH::DEFAULT_TEMP;
}

void ConfigManager::setPHConfig(float v1, float t1, float v2, float t2, float v3, float t3, float defaultTemp) {
    StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
    readNamespace(JsonKeys::NS_PH, doc);
    doc[JsonKeys::KEY_PH_V1] = v1;
    doc[JsonKeys::KEY_PH_T1] = t1;
    doc[JsonKeys::KEY_PH_V2] = v2;
    doc[JsonKeys::KEY_PH_T2] = t2;
    doc[JsonKeys::KEY_PH_V3] = v3;
    doc[JsonKeys::KEY_PH_T3] = t3;
    doc[JsonKeys::KEY_PH_CT] = defaultTemp;
    writeNamespace(JsonKeys::NS_PH, doc);
}

/* =========================================================================
   CONFIGURACIÓN DE SENSORES ADC
   ========================================================================= */
void ConfigManager::setAdcSensorsConfigs(const std::vector<SensorConfig>& configs) {
    Preferences prefs;
    prefs.begin(JsonKeys::NS_SENSORS_ADC, false);
    StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
    JsonArray sensorArray = doc.to<JsonArray>();
    
    for (const auto& sensor : configs) {
        JsonObject sensorObj = sensorArray.createNestedObject();
        sensorObj[JsonKeys::KEY_ADC_SENSOR] = sensor.configKey;
        sensorObj[JsonKeys::KEY_ADC_SENSOR_ID] = sensor.sensorId;
        sensorObj[JsonKeys::KEY_ADC_SENSOR_TYPE] = static_cast<int>(sensor.type);
        sensorObj[JsonKeys::KEY_ADC_SENSOR_ENABLE] = sensor.enable;
    }
    
    String jsonString;
    serializeJson(doc, jsonString);
    prefs.putString(JsonKeys::NS_SENSORS_ADC, jsonString.c_str());
    prefs.end();
}

std::vector<SensorConfig> ConfigManager::getAllAdcSensorConfigs() {
    StaticJsonDocument<System::JSON_DOC_SIZE_MEDIUM> doc;
    readNamespace(JsonKeys::NS_SENSORS_ADC, doc);
    
    std::vector<SensorConfig> configs;
    
    if (doc.is<JsonArray>()) {
        JsonArray array = doc.as<JsonArray>();
        
        for (JsonObject sensorObj : array) {
            SensorConfig config;
            const char* cKey = sensorObj[JsonKeys::KEY_ADC_SENSOR] | "";
            strncpy(config.configKey, cKey, sizeof(config.configKey));
            const char* sensorId = sensorObj[JsonKeys::KEY_ADC_SENSOR_ID] | "";
            strncpy(config.sensorId, sensorId, sizeof(config.sensorId));
            config.type = static_cast<SensorType>(sensorObj[JsonKeys::KEY_ADC_SENSOR_TYPE] | 0);
            config.enable = sensorObj[JsonKeys::KEY_ADC_SENSOR_ENABLE] | false;
            
            configs.push_back(config);
        }
    }
    
    return configs;
}

std::vector<SensorConfig> ConfigManager::getEnabledAdcSensorConfigs() {
    std::vector<SensorConfig> all = getAllAdcSensorConfigs();
    std::vector<SensorConfig> enabled;
    for (auto &s : all) {
        if (s.enable) {
            enabled.push_back(s);
        }
    }
    return enabled;
}
