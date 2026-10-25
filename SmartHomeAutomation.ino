/*
  Smart Home Automation Using IoT — ESP32 starter firmware
  Demo outputs: GPIO 18 and GPIO 19 (use LEDs + resistors first)
  Optional PIR: GPIO 27
  Optional DHT22: GPIO 4 (requires Adafruit DHT sensor library)

  SAFETY: Do not connect mains appliances directly to ESP32 GPIO.
  Use only low-voltage demo loads unless properly rated isolated hardware
  is installed by a qualified person.
*/
#include <WiFi.h>
#include <WebServer.h>

#define USE_PIR 1
#define USE_DHT 0

#if USE_DHT
  #include <DHT.h>
#endif

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const int OUTPUT_1_PIN = 18;
const int OUTPUT_2_PIN = 19;
const int PIR_PIN = 27;
const int DHT_PIN = 4;

WebServer server(80);
bool output1State = false;
bool output2State = false;
unsigned long bootTimeMs = 0;

#if USE_DHT
DHT dht(DHT_PIN, DHT22);
#endif

const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Smart Home Automation</title>
  <style>
    :root{font-family:system-ui,Arial,sans-serif;color:#182230;background:#f3f6fb}
    body{margin:0;padding:24px;max-width:900px;margin-inline:auto}
    h1{margin:0 0 6px;color:#17365d} .muted{color:#5b6573}
    .grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(230px,1fr));gap:16px;margin-top:22px}
    .card{background:white;border:1px solid #d8e0ea;border-radius:16px;padding:18px;box-shadow:0 3px 14px #17365d0d}
    .state{font-size:1.5rem;font-weight:700;margin:10px 0}
    button{border:0;border-radius:10px;padding:11px 16px;margin-right:8px;cursor:pointer;font-weight:650}
    .on{background:#d9ead3;color:#274e13}.off{background:#f4cccc;color:#990000}
    .pill{display:inline-block;background:#e7eef8;border-radius:99px;padding:5px 10px;font-size:.85rem}
    footer{margin-top:24px;font-size:.85rem;color:#5b6573}
  </style>
</head>
<body>
  <h1>Smart Home Automation</h1>
  <div class="muted">ESP32 local dashboard · low-voltage demo</div>
  <div class="grid">
    <section class="card">
      <h2>Demo Output 1</h2><div id="out1" class="state">Loading…</div>
      <button class="on" onclick="setOutput(1,true)">Turn ON</button>
      <button class="off" onclick="setOutput(1,false)">Turn OFF</button>
    </section>
    <section class="card">
      <h2>Demo Output 2</h2><div id="out2" class="state">Loading…</div>
      <button class="on" onclick="setOutput(2,true)">Turn ON</button>
      <button class="off" onclick="setOutput(2,false)">Turn OFF</button>
    </section>
    <section class="card">
      <h2>Sensor Status</h2>
      <p>PIR motion: <span id="pir" class="pill">—</span></p>
      <p>Temperature: <span id="temp" class="pill">Not enabled</span></p>
      <p>Humidity: <span id="hum" class="pill">Not enabled</span></p>
      <p>Uptime: <span id="uptime" class="pill">—</span></p>
    </section>
  </div>
  <footer>Use only with trusted local Wi-Fi. This demo API has no authentication. Do not expose it to the public internet or connect mains loads without qualified, isolated hardware.</footer>
<script>
async function refreshStatus(){
  try{
    const r=await fetch('/api/status'); if(!r.ok) throw new Error('status unavailable');
    const d=await r.json();
    document.getElementById('out1').textContent=d.output1?'ON':'OFF';
    document.getElementById('out2').textContent=d.output2?'ON':'OFF';
    document.getElementById('pir').textContent=d.pirEnabled?(d.motion?'Motion detected':'No motion'):'Disabled';
    document.getElementById('temp').textContent=d.temperature===null?'Not enabled':d.temperature.toFixed(1)+' °C';
    document.getElementById('hum').textContent=d.humidity===null?'Not enabled':d.humidity.toFixed(1)+' %';
    document.getElementById('uptime').textContent=Math.floor(d.uptimeSeconds)+' s';
  }catch(e){
    document.getElementById('pir').textContent='Device unavailable';
  }
}
async function setOutput(id,state){
  try{
    const r=await fetch('/api/output',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({id,state})});
    if(!r.ok) throw new Error('command failed');
    await refreshStatus();
  }catch(e){alert('Could not send command. Check that the ESP32 is online on the same Wi-Fi network.');}
}
refreshStatus(); setInterval(refreshStatus,2000);
</script>
</body>
</html>
)HTML";

String jsonBool(bool value) { return value ? "true" : "false"; }

void handleRoot() {
  server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

void handleStatus() {
  String json = "{";
  json += "\"output1\":" + jsonBool(output1State) + ",";
  json += "\"output2\":" + jsonBool(output2State) + ",";
  json += "\"pirEnabled\":" + String(USE_PIR ? "true" : "false") + ",";
#if USE_PIR
  json += "\"motion\":" + String(digitalRead(PIR_PIN) == HIGH ? "true" : "false") + ",";
#else
  json += "\"motion\":false,";
#endif
#if USE_DHT
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  json += "\"temperature\":" + (isnan(t) ? String("null") : String(t, 1)) + ",";
  json += "\"humidity\":" + (isnan(h) ? String("null") : String(h, 1)) + ",";
#else
  json += "\"temperature\":null,\"humidity\":null,";
#endif
  json += "\"uptimeSeconds\":" + String((millis() - bootTimeMs) / 1000) + "}";
  server.send(200, "application/json", json);
}

void handleOutput() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"Missing JSON body\"}");
    return;
  }
  String body = server.arg("plain");
  int idPos = body.indexOf("\"id\"");
  int statePos = body.indexOf("\"state\"");
  if (idPos < 0 || statePos < 0) {
    server.send(400, "application/json", "{\"error\":\"Expected id and state\"}");
    return;
  }
  int colon = body.indexOf(':', idPos);
  int comma = body.indexOf(',', colon);
  if (colon < 0 || comma < 0) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON format\"}");
    return;
  }
  int id = body.substring(colon + 1, comma).toInt();
  int stateColon = body.indexOf(':', statePos);
  String stateText = body.substring(stateColon + 1);
  stateText.trim();
  bool state;
  if (stateText.startsWith("true")) state = true;
  else if (stateText.startsWith("false")) state = false;
  else {
    server.send(400, "application/json", "{\"error\":\"state must be boolean\"}");
    return;
  }
  if (id == 1) {
    output1State = state;
    digitalWrite(OUTPUT_1_PIN, state ? HIGH : LOW);
  } else if (id == 2) {
    output2State = state;
    digitalWrite(OUTPUT_2_PIN, state ? HIGH : LOW);
  } else {
    server.send(400, "application/json", "{\"error\":\"Unknown output id\"}");
    return;
  }
  server.send(200, "application/json", "{\"ok\":true}");
}

void setup() {
  Serial.begin(115200);
  bootTimeMs = millis();
  pinMode(OUTPUT_1_PIN, OUTPUT);
  pinMode(OUTPUT_2_PIN, OUTPUT);
  digitalWrite(OUTPUT_1_PIN, LOW);
  digitalWrite(OUTPUT_2_PIN, LOW);
#if USE_PIR
  pinMode(PIR_PIN, INPUT);
#endif
#if USE_DHT
  dht.begin();
#endif
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Dashboard URL: http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Wi-Fi connection failed. Check credentials and restart.");
  }
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/output", HTTP_POST, handleOutput);
  server.begin();
}
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    // Outputs are not automatically changed on Wi-Fi loss; define the desired safe state for your setup.
  }
  server.handleClient();
}
