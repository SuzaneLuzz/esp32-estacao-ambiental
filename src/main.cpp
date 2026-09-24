/**
 * Estação de Monitoramento Ambiental com ESP32
 * Autora: Suzane Luz — github.com/SuzaneLuzz
 *
 * Fluxo (loop não bloqueante, baseado em millis()):
 *   sensores -> filtragem -> lógica de controle -> atuadores
 *                                  |-> display OLED / painel web / log serial
 */
#include <Arduino.h>
#include "config.h"
#include "sensors.h"
#include "control.h"
#include "display.h"
#include "web.h"

static SensorData    data;
static ActuatorState act;
static uint8_t       page = 0;

// Botão com debounce: clique curto troca a tela, clique longo silencia o alarme.
static void handleButton() {
  static bool lastRead = HIGH, stable = HIGH, longHandled = false;
  static uint32_t lastChange = 0, pressStart = 0;
  const uint32_t now = millis();
  const bool r = digitalRead(PIN_BUTTON);

  if (r != lastRead) { lastRead = r; lastChange = now; }

  if (now - lastChange >= BUTTON_DEBOUNCE_MS && r != stable) {
    stable = r;
    if (stable == LOW) {                 // pressionou
      pressStart = now;
      longHandled = false;
    } else if (!longHandled) {           // soltou (clique curto)
      page = (page + 1) % DISPLAY_PAGES;
    }
  }

  if (stable == LOW && !longHandled && now - pressStart >= BUTTON_LONG_MS) {
    controlToggleMute();
    longHandled = true;
    Serial.println("[BOTAO] alarme silenciado/reativado");
  }
}

static void logCsv() {
  Serial.printf("%lu,%.2f,%.2f,%.2f,%d,%d,%d,%d,%s\n",
                (unsigned long)millis(), data.temperature, data.humidity, data.heatIndex,
                data.lightPct, data.soilPct, act.fan, act.led, alarmToStr(act.alarm));
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n=== Estacao de Monitoramento Ambiental - ESP32 ===");

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  sensorsBegin();
  controlBegin();
  displayBegin();
  webBegin(&data, &act);

  Serial.println("ms,temp_c,umid_pct,sensacao_c,luz_pct,solo_pct,ventilador,led,alarme");
}

void loop() {
  static uint32_t lastDisplay = 0, lastLog = 0;
  const uint32_t now = millis();

  sensorsUpdate(data);
  controlUpdate(data, act);
  handleButton();
  webLoop();

  if (now - lastDisplay >= DISPLAY_INTERVAL_MS) {
    lastDisplay = now;
    displayUpdate(data, act, page, webIp());
  }
  if (now - lastLog >= LOG_INTERVAL_MS) {
    lastLog = now;
    logCsv();
  }
}
