#pragma once
#include <Arduino.h>

struct SensorData {
  float temperature = NAN;   // °C (filtrada)
  float humidity    = NAN;   // % (filtrada)
  float heatIndex   = NAN;   // sensação térmica °C
  int   lightPct    = 0;     // 0-100 %
  int   soilPct     = 0;     // 0-100 %
  int   lightRaw    = 0;     // leitura bruta média (útil p/ calibrar)
  int   soilRaw     = 0;
  bool  dhtOk       = false; // última leitura do DHT foi válida
  uint32_t dhtFailStreak = 0; // falhas consecutivas
  uint32_t dhtErrors     = 0; // total de falhas desde o boot
};

void sensorsBegin();
// Não bloqueante: chame a cada loop; cada sensor respeita seu próprio intervalo.
void sensorsUpdate(SensorData& d);
