#pragma once
/**
 * config.h
 * Configurações centrais do projeto: pinos, tempos, limiares de controle
 * e calibração dos sensores analógicos. Ajuste aqui sem mexer na lógica.
 */
#include <Arduino.h>

// ============================ PINOS ============================
constexpr uint8_t PIN_DHT    = 15;  // DHT22 (dados)
constexpr uint8_t PIN_LDR    = 34;  // Módulo LDR - saída analógica (ADC1)
constexpr uint8_t PIN_SOIL   = 35;  // Sensor de umidade do solo (ADC1)
constexpr uint8_t PIN_RELAY  = 26;  // Módulo relé (ventilação)
constexpr uint8_t PIN_LED    = 25;  // LED de iluminação automática
constexpr uint8_t PIN_BUZZER = 27;  // Buzzer de alarme
constexpr uint8_t PIN_BUTTON = 4;   // Botão (INPUT_PULLUP)
constexpr uint8_t PIN_SDA    = 21;  // OLED I2C
constexpr uint8_t PIN_SCL    = 22;  // OLED I2C

// Muitos módulos relé reais acionam com nível BAIXO. Mude para true se for o seu caso.
constexpr bool RELAY_ACTIVE_LOW = false;

// ========================= TEMPOS (ms) =========================
constexpr uint32_t DHT_INTERVAL_MS     = 2000;  // DHT22 aceita no máx. 1 leitura a cada 2 s
constexpr uint32_t ADC_INTERVAL_MS     = 100;   // amostragem dos sensores analógicos
constexpr uint32_t DISPLAY_INTERVAL_MS = 500;
constexpr uint32_t LOG_INTERVAL_MS     = 5000;  // log CSV na serial
constexpr uint32_t BUTTON_DEBOUNCE_MS  = 30;
constexpr uint32_t BUTTON_LONG_MS      = 1000;  // pressão longa = silenciar alarme

// ====================== LÓGICA DE CONTROLE ======================
// Ventilação com histerese: liga em 30 °C, só desliga abaixo de 28 °C
constexpr float TEMP_FAN_ON  = 30.0f;
constexpr float TEMP_FAN_OFF = 28.0f;

// Iluminação com histerese: liga abaixo de 30 % de luz, desliga acima de 40 %
constexpr float LIGHT_ON_PCT  = 30.0f;
constexpr float LIGHT_OFF_PCT = 40.0f;

constexpr float    TEMP_ALARM_C   = 38.0f;  // temperatura crítica
constexpr int      SOIL_DRY_PCT   = 25;     // solo seco -> alerta
constexpr uint32_t DHT_MAX_FAILS  = 3;      // falhas seguidas -> alarme crítico

// ===================== CALIBRAÇÃO DO ADC =====================
// Valores brutos (0-4095) nos extremos. Meça com o seu sensor e ajuste.
// LDR: no escuro a tensão sobe; com luz, cai.
constexpr int LDR_RAW_DARK   = 4095;
constexpr int LDR_RAW_BRIGHT = 0;
// Solo (sensor capacitivo): seco = valor alto, molhado = valor baixo.
// Sensor real típico: seco ~3000, na água ~1200.
constexpr int SOIL_RAW_DRY = 4095;
constexpr int SOIL_RAW_WET = 0;

// ============================ REDE ============================
constexpr bool ENABLE_WIFI = true;
constexpr const char* WIFI_SSID = "Wokwi-GUEST";  // troque pela sua rede
constexpr const char* WIFI_PASS = "";
