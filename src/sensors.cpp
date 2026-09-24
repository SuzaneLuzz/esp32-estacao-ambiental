#include "sensors.h"
#include <DHT.h>
#include <math.h>
#include "config.h"
#include "filters.h"

static DHT dht(PIN_DHT, DHT22);

static MovingAverage<5>  tempAvg, humAvg;     // DHT: 5 amostras (~10 s)
static MovingAverage<10> lightAvg, soilAvg;   // ADC: 10 amostras (~1 s)

static uint32_t lastDhtMs = 0;
static uint32_t lastAdcMs = 0;

// Converte leitura bruta em porcentagem a partir de dois pontos de calibração.
// Funciona com escala invertida (raw0 > raw100).
static int rawToPct(float raw, int raw0, int raw100) {
  float pct = (raw - raw0) * 100.0f / float(raw100 - raw0);
  return constrain((int)lroundf(pct), 0, 100);
}

static bool dhtReadingValid(float t, float h) {
  return !isnan(t) && !isnan(h) && t > -40.0f && t < 80.0f && h >= 0.0f && h <= 100.0f;
}

void sensorsBegin() {
  dht.begin();
  analogReadResolution(12);        // 0-4095
  analogSetAttenuation(ADC_11db);  // faixa ~0-3,3 V
}

void sensorsUpdate(SensorData& d) {
  const uint32_t now = millis();

  // ---- Sensores analógicos (rápidos) ----
  if (now - lastAdcMs >= ADC_INTERVAL_MS) {
    lastAdcMs = now;
    float light = lightAvg.update(analogRead(PIN_LDR));
    float soil  = soilAvg.update(analogRead(PIN_SOIL));
    d.lightRaw = (int)light;
    d.soilRaw  = (int)soil;
    d.lightPct = rawToPct(light, LDR_RAW_DARK, LDR_RAW_BRIGHT);
    d.soilPct  = rawToPct(soil, SOIL_RAW_DRY, SOIL_RAW_WET);
  }

  // ---- DHT22 (lento) ----
  if (now - lastDhtMs >= DHT_INTERVAL_MS) {
    lastDhtMs = now;
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (dhtReadingValid(t, h)) {
      d.temperature = tempAvg.update(t);
      d.humidity    = humAvg.update(h);
      d.heatIndex   = dht.computeHeatIndex(d.temperature, d.humidity, false);
      d.dhtOk = true;
      d.dhtFailStreak = 0;
    } else {
      // Mantém o último valor filtrado, mas sinaliza a falha
      d.dhtOk = false;
      d.dhtFailStreak++;
      d.dhtErrors++;
    }
  }
}
