#include "control.h"
#include "config.h"
#include "filters.h"

static Hysteresis fanCtl(TEMP_FAN_ON, TEMP_FAN_OFF, true);     // liga ao subir
static Hysteresis ledCtl(LIGHT_ON_PCT, LIGHT_OFF_PCT, false);  // liga ao descer

static bool muted = false;

// ---- Buzzer via LEDC (PWM do ESP32), inicializado uma única vez ----
constexpr uint8_t BUZZER_CHANNEL = 0;
static bool     buzzerActive = false;
static uint32_t buzzerOffAt  = 0;
static uint32_t lastBeepMs   = 0;

static void buzzerStart(uint32_t freq, uint32_t durationMs) {
  ledcWriteTone(BUZZER_CHANNEL, freq);
  buzzerOffAt = millis() + durationMs;
  buzzerActive = true;
}

static void buzzerStop() {
  if (!buzzerActive) return;          // só desliga se estiver tocando
  ledcWrite(BUZZER_CHANNEL, 0);
  buzzerActive = false;
}

static void writeRelay(bool on) {
  digitalWrite(PIN_RELAY, (on ^ RELAY_ACTIVE_LOW) ? HIGH : LOW);
}

void controlBegin() {
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  writeRelay(false);
  digitalWrite(PIN_LED, LOW);

  ledcSetup(BUZZER_CHANNEL, 2000, 8);       // canal 0, 2 kHz, 8 bits
  ledcAttachPin(PIN_BUZZER, BUZZER_CHANNEL);
  ledcWrite(BUZZER_CHANNEL, 0);             // começa em silêncio
}

void controlToggleMute() { muted = !muted; }

const char* alarmToStr(AlarmState s) {
  switch (s) {
    case AlarmState::WARNING:  return "ATENCAO";
    case AlarmState::CRITICAL: return "CRITICO";
    default:                   return "OK";
  }
}

void controlUpdate(const SensorData& d, ActuatorState& a) {
  // 1) Ventilação: só decide com leitura válida. Sem leitura -> desliga (falha segura).
  if (d.dhtOk) {
    a.fan = fanCtl.update(d.temperature);
  } else {
    fanCtl.reset();
    a.fan = false;
  }

  // 2) Iluminação automática
  a.led = ledCtl.update(d.lightPct);

  // 3) Máquina de estados do alarme (prioridade: crítico > atenção > ok)
  AlarmState s = AlarmState::OK;
  if (d.dhtFailStreak >= DHT_MAX_FAILS || (d.dhtOk && d.temperature >= TEMP_ALARM_C)) {
    s = AlarmState::CRITICAL;
  } else if (d.soilPct <= SOIL_DRY_PCT) {
    s = AlarmState::WARNING;
  }
  if (s == AlarmState::OK) muted = false;  // silêncio vale só para o evento atual
  a.alarm = s;
  a.muted = muted;

  // 4) Saídas
  writeRelay(a.fan);
  digitalWrite(PIN_LED, a.led ? HIGH : LOW);

  // 5) Buzzer
  const uint32_t now = millis();
  if (buzzerActive && (int32_t)(now - buzzerOffAt) >= 0) buzzerStop();  // fim do bipe

  if (s == AlarmState::CRITICAL && !muted) {
    if (now - lastBeepMs >= 1000) {          // bipe de 200 ms a cada 1 s
      lastBeepMs = now;
      buzzerStart(2000, 200);
    }
  } else if (s == AlarmState::WARNING && !muted) {
    if (now - lastBeepMs >= 10000) {         // bipe curto a cada 10 s
      lastBeepMs = now;
      buzzerStart(1000, 80);
    }
  } else {
    buzzerStop();                            // alarme OK ou silenciado
  }
}
