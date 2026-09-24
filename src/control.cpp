#include "control.h"
#include "config.h"
#include "filters.h"

static Hysteresis fanCtl(TEMP_FAN_ON, TEMP_FAN_OFF, true);     // liga ao subir
static Hysteresis ledCtl(LIGHT_ON_PCT, LIGHT_OFF_PCT, false);  // liga ao descer

static bool muted = false;
static uint32_t lastBeepMs = 0;

static void writeRelay(bool on) {
  digitalWrite(PIN_RELAY, (on ^ RELAY_ACTIVE_LOW) ? HIGH : LOW);
}

void controlBegin() {
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  writeRelay(false);
  digitalWrite(PIN_LED, LOW);
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

  const uint32_t now = millis();
  if (s == AlarmState::CRITICAL && !muted) {
    if (now - lastBeepMs >= 1000) {         // bipe de 200 ms a cada 1 s
      lastBeepMs = now;
      tone(PIN_BUZZER, 2000, 200);
    }
  } else if (s == AlarmState::WARNING && !muted) {
    if (now - lastBeepMs >= 10000) {        // bipe curto a cada 10 s
      lastBeepMs = now;
      tone(PIN_BUZZER, 1000, 80);
    }
  } else {
    noTone(PIN_BUZZER);
  }
}
