#include "display.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

static Adafruit_SSD1306 oled(128, 64, &Wire, -1);
static bool oledOk = false;

static void header(const char* title, uint8_t page) {
  oled.setCursor(0, 0);
  oled.print(title);
  oled.setCursor(104, 0);
  oled.printf("%u/%u", page + 1, DISPLAY_PAGES);
  oled.drawFastHLine(0, 10, 128, SSD1306_WHITE);
  oled.setCursor(0, 14);
}

static void printFloat(const char* label, float v, const char* unit, bool ok) {
  oled.print(label);
  if (ok && !isnan(v)) oled.printf("%.1f %s\n", v, unit);
  else oled.println("--");
}

bool displayBegin() {
  Wire.begin(PIN_SDA, PIN_SCL);
  oledOk = oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  if (!oledOk) {
    Serial.println("[OLED] display nao encontrado - seguindo sem tela");
    return false;
  }
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 20);
  oled.println("Estacao Ambiental");
  oled.println("ESP32 - iniciando...");
  oled.display();
  return true;
}

void displayUpdate(const SensorData& d, const ActuatorState& a, uint8_t page, const String& ip) {
  if (!oledOk) return;
  oled.clearDisplay();

  switch (page) {
    case 0:
      header("LEITURAS", page);
      printFloat("Temp:  ", d.temperature, "C", d.dhtOk);
      printFloat("Umid:  ", d.humidity, "%", d.dhtOk);
      printFloat("Sens.: ", d.heatIndex, "C", d.dhtOk);
      oled.printf("Luz:   %d %%\n", d.lightPct);
      oled.printf("Solo:  %d %%\n", d.soilPct);
      break;

    case 1:
      header("ATUADORES", page);
      oled.printf("Ventilacao: %s\n", a.fan ? "LIGADA" : "desl.");
      oled.printf("Iluminacao: %s\n", a.led ? "LIGADA" : "desl.");
      oled.printf("Alarme: %s%s\n", alarmToStr(a.alarm), a.muted ? " (mudo)" : "");
      oled.printf("Falhas DHT: %lu\n", (unsigned long)d.dhtErrors);
      break;

    default:
      header("SISTEMA", page);
      oled.printf("IP: %s\n", ip.c_str());
      oled.printf("Uptime: %lus\n", (unsigned long)(millis() / 1000));
      oled.printf("LDR raw:  %d\n", d.lightRaw);
      oled.printf("Solo raw: %d\n", d.soilRaw);
      break;
  }

  // Alerta piscando no rodapé
  if (a.alarm != AlarmState::OK && (millis() / 500) % 2) {
    oled.fillRect(0, 56, 128, 8, SSD1306_WHITE);
    oled.setTextColor(SSD1306_BLACK);
    oled.setCursor(2, 56);
    oled.printf("! ALARME %s", alarmToStr(a.alarm));
    oled.setTextColor(SSD1306_WHITE);
  }
  oled.display();
}
