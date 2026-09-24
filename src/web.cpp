#include "web.h"
#include <WiFi.h>
#include <WebServer.h>
#include "config.h"

static WebServer server(80);
static const SensorData* sd = nullptr;
static const ActuatorState* as = nullptr;
static bool serverStarted = false;

static const char INDEX_HTML[] = R"rawliteral(<!DOCTYPE html>
<html lang="pt-BR"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Estacao Ambiental ESP32</title>
<style>
body{font-family:system-ui,sans-serif;background:#0f172a;color:#e2e8f0;margin:0;padding:24px}
h1{font-size:1.3rem;margin:0 0 16px}
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:12px}
.card{background:#1e293b;border-radius:12px;padding:16px}
.label{font-size:.8rem;color:#94a3b8}.value{font-size:1.6rem;font-weight:600;margin-top:4px}
.on{color:#4ade80}.off{color:#64748b}.OK{color:#4ade80}.ATENCAO{color:#facc15}.CRITICO{color:#f87171}
small{color:#64748b;display:block;margin-top:16px}
</style></head><body>
<h1>Estação de Monitoramento Ambiental</h1>
<div class="grid" id="g"></div>
<small id="u">carregando...</small>
<script>
const f=(v,u)=>v===null?'--':v.toFixed(1)+' '+u;
async function load(){
 try{
  const d=await (await fetch('/api/data')).json();
  const c=[['Temperatura',f(d.temperatura,'°C')],['Umidade',f(d.umidade,'%')],
   ['Sensação térmica',f(d.sensacao,'°C')],['Luminosidade',d.luz+' %'],['Umidade do solo',d.solo+' %'],
   ['Ventilação',d.ventilador?'<span class=on>LIGADA</span>':'<span class=off>desligada</span>'],
   ['Iluminação',d.led?'<span class=on>LIGADA</span>':'<span class=off>desligada</span>'],
   ['Alarme','<span class='+d.alarme+'>'+d.alarme+'</span>']];
  document.getElementById('g').innerHTML=c.map(x=>'<div class=card><div class=label>'+x[0]+'</div><div class=value>'+x[1]+'</div></div>').join('');
  document.getElementById('u').textContent='Uptime: '+d.uptime_s+' s - atualiza a cada 2 s';
 }catch(e){document.getElementById('u').textContent='sem conexão com o ESP32';}
}
load();setInterval(load,2000);
</script></body></html>)rawliteral";

static void fmtOrNull(char* out, size_t len, float v, bool ok) {
  if (!ok || isnan(v)) snprintf(out, len, "null");
  else snprintf(out, len, "%.2f", v);
}

static void handleRoot() { server.send(200, "text/html; charset=utf-8", INDEX_HTML); }

static void handleApi() {
  char t[12], h[12], hi[12], json[320];
  fmtOrNull(t, sizeof t, sd->temperature, sd->dhtOk);
  fmtOrNull(h, sizeof h, sd->humidity, sd->dhtOk);
  fmtOrNull(hi, sizeof hi, sd->heatIndex, sd->dhtOk);
  snprintf(json, sizeof json,
           "{\"temperatura\":%s,\"umidade\":%s,\"sensacao\":%s,\"luz\":%d,\"solo\":%d,"
           "\"ventilador\":%s,\"led\":%s,\"alarme\":\"%s\",\"mudo\":%s,"
           "\"falhas_dht\":%lu,\"uptime_s\":%lu}",
           t, h, hi, sd->lightPct, sd->soilPct,
           as->fan ? "true" : "false", as->led ? "true" : "false",
           alarmToStr(as->alarm), as->muted ? "true" : "false",
           (unsigned long)sd->dhtErrors, (unsigned long)(millis() / 1000));
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

void webBegin(const SensorData* data, const ActuatorState* act) {
  sd = data;
  as = act;
  if (!ENABLE_WIFI) return;
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);  // conexão segue em segundo plano
  Serial.printf("[WiFi] conectando a \"%s\"...\n", WIFI_SSID);
}

void webLoop() {
  if (!ENABLE_WIFI) return;
  if (!serverStarted && WiFi.status() == WL_CONNECTED) {
    server.on("/", handleRoot);
    server.on("/api/data", handleApi);
    server.onNotFound([] { server.send(404, "text/plain", "Nao encontrado"); });
    server.begin();
    serverStarted = true;
    Serial.printf("[WiFi] conectado! Painel em http://%s/\n", WiFi.localIP().toString().c_str());
  }
  if (serverStarted) server.handleClient();
}

String webIp() {
  if (!ENABLE_WIFI) return "WiFi desativado";
  return WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : String("conectando...");
}
