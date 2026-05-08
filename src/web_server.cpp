#include "web_server.h"

#include <ArduinoJson.h>
#include <WebServer.h>

#include "app_controller.h"
#include "board_config.h"
#include "led_control.h"
#include "oled_control.h"
#include "wifi_ap.h"

namespace {

WebServer server(80);

const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="fr">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
  <title>Badge3000</title>
  <style>
    :root { color-scheme: dark; --bg:#05070d; --card:#101521; --line:#283247; --text:#f8fbff; --muted:#8d9ab0; --accent:#63e3ff; }
    * { box-sizing:border-box; -webkit-tap-highlight-color:transparent; }
    body { margin:0; min-height:100vh; background:linear-gradient(150deg,#111d34,#05070d 62%); color:var(--text); font-family:system-ui,-apple-system,Segoe UI,sans-serif; }
    main { width:min(500px,100%); margin:0 auto; padding:12px 12px calc(18px + env(safe-area-inset-bottom)); }
    .tabs { position:sticky; top:0; z-index:2; display:grid; grid-template-columns:1fr 1fr; gap:8px; padding:10px 0; background:linear-gradient(180deg,#07101f 70%,rgba(7,16,31,0)); }
    .tab { min-height:52px; border:1px solid var(--line); border-radius:18px; background:#151c2b; color:var(--text); font-weight:850; font-size:1.02rem; }
    .tab.active { background:var(--accent); color:#031019; border-color:var(--accent); }
    .panel { display:none; }
    .panel.active { display:block; }
    .card { background:rgba(16,21,33,.94); border:1px solid var(--line); border-radius:24px; padding:16px; margin:10px 0; box-shadow:0 18px 48px rgba(0,0,0,.25); }
    label { display:grid; gap:8px; margin:14px 0; color:#dce6f7; font-weight:750; }
    input, select, textarea, button { font:inherit; }
    input[type="range"] { width:100%; min-height:42px; accent-color:var(--accent); }
    input[type="color"] { width:100%; height:160px; padding:0; border:0; border-radius:24px; background:transparent; overflow:hidden; }
    select, textarea { width:100%; border:1px solid var(--line); border-radius:18px; background:#080d18; color:var(--text); padding:14px; }
    textarea { min-height:170px; resize:vertical; line-height:1.35; font-size:1.08rem; }
    .row { display:grid; grid-template-columns:1fr 1fr; gap:10px; }
    .grid3 { display:grid; grid-template-columns:repeat(3,1fr); gap:9px; }
    .actions { display:grid; grid-template-columns:1fr 1fr; gap:10px; margin-top:12px; }
    button { min-height:50px; border:0; border-radius:18px; padding:11px; background:var(--accent); color:#031019; font-weight:850; }
    button.secondary { background:#20293b; color:var(--text); border:1px solid var(--line); }
    .swatch { min-height:46px; border-radius:999px; border:1px solid rgba(255,255,255,.2); color:transparent; }
    .preset { background:#1d2637; color:var(--text); border:1px solid var(--line); }
    .toggle { display:flex; align-items:center; justify-content:space-between; gap:12px; min-height:52px; }
    .toggle input { width:26px; height:26px; }
    .status { min-height:1.4em; color:var(--muted); font-size:.9rem; margin:8px 4px 0; }
  </style>
</head>
<body>
  <main>
    <nav class="tabs">
      <button id="tabColor" class="tab active">Couleur</button>
      <button id="tabText" class="tab">Texte</button>
    </nav>

    <section id="panelColor" class="panel active">
      <div class="card">
        <label><span>Couleur</span><input id="color" type="color" value="#000000"></label>
        <label><span>Intensite</span><input id="brightness" type="range" min="1" max="255" value="96"></label>
        <div class="grid3">
          <button class="swatch" data-c="#ff0000" style="background:#ff0000">.</button>
          <button class="swatch" data-c="#00ff00" style="background:#00ff00">.</button>
          <button class="swatch" data-c="#0000ff" style="background:#0000ff">.</button>
          <button class="swatch" data-c="#ff00ff" style="background:#ff00ff">.</button>
          <button class="swatch" data-c="#00ffff" style="background:#00ffff">.</button>
          <button class="swatch" data-c="#ffffff" style="background:#ffffff">.</button>
        </div>
      </div>
      <div class="card">
        <label><span>Effet</span>
          <select id="effect">
            <option value="solid">Fixe</option>
            <option value="blink">Clignotement</option>
            <option value="pulse">Pulse</option>
            <option value="rainbow">Rainbow</option>
          </select>
        </label>
        <label><span>Vitesse</span><input id="effectSpeed" type="range" min="1" max="10" value="5"></label>
        <div class="grid3">
          <button class="preset" data-e="blink">Blink</button>
          <button class="preset" data-e="pulse">Pulse</button>
          <button class="preset" data-e="rainbow">RGB</button>
        </div>
        <div class="actions">
          <button id="ledOff" class="secondary">Off</button>
          <button id="ledTest" class="secondary">Test</button>
        </div>
      </div>
    </section>

    <section id="panelText" class="panel">
      <div class="card">
        <label><span>Texte</span><textarea id="text" maxlength="180" autocomplete="off" autocapitalize="sentences"></textarea></label>
        <div class="row">
          <label><span>Taille</span><input id="textSize" type="range" min="1" max="8" value="1"></label>
          <label><span>Hauteur</span><input id="textY" type="range" min="0" max="56" value="0"></label>
        </div>
        <label><span>Alignement</span>
          <select id="textAlign">
            <option value="0">Gauche</option>
            <option value="1">Centre</option>
            <option value="2">Droite</option>
          </select>
        </label>
        <label class="toggle"><span>Gras</span><input id="textBold" type="checkbox"></label>
        <label class="toggle"><span>Defilement</span><input id="scroll" type="checkbox"></label>
        <label><span>Vitesse defilement</span><input id="scrollSpeed" type="range" min="1" max="10" value="5"></label>
        <label class="toggle"><span>Inverser</span><input id="invert" type="checkbox"></label>
        <div class="actions">
          <button id="sendText">Afficher</button>
          <button id="clear" class="secondary">Effacer</button>
        </div>
      </div>
    </section>

    <p id="status" class="status"></p>
  </main>
  <script>
    const $ = (id) => document.getElementById(id);
    let ledTimer = 0;
    let textTimer = 0;
    let styleTimer = 0;

    async function api(path, options = {}) {
      const headers = options.body ? { 'Content-Type': 'application/json' } : {};
      const response = await fetch(path, { ...options, headers });
      const data = await response.json().catch(() => ({}));
      if (!response.ok) throw new Error(data.error || response.statusText);
      return data;
    }

    function status(message) { $('status').textContent = message; }
    function hexToRgb(hex) { return { r:parseInt(hex.slice(1,3),16), g:parseInt(hex.slice(3,5),16), b:parseInt(hex.slice(5,7),16) }; }
    function rgbToHex(r,g,b) { return '#' + [r,g,b].map(v => Number(v).toString(16).padStart(2,'0')).join(''); }
    function textOptions() { return { size:+$('textSize').value, align:+$('textAlign').value, y:+$('textY').value, bold:$('textBold').checked, scroll:$('scroll').checked, scrollSpeed:+$('scrollSpeed').value }; }

    function showTab(name) {
      $('tabColor').classList.toggle('active', name === 'color');
      $('tabText').classList.toggle('active', name === 'text');
      $('panelColor').classList.toggle('active', name === 'color');
      $('panelText').classList.toggle('active', name === 'text');
    }

    async function sendLed() {
      await api('/api/led', { method:'POST', body:JSON.stringify(hexToRgb($('color').value)) });
      status('OK');
    }

    async function sendEffect() {
      await api('/api/led/effect', { method:'POST', body:JSON.stringify({ effect:$('effect').value, speed:+$('effectSpeed').value }) });
      status('OK');
    }

    async function sendText() {
      await api('/api/oled/text', { method:'POST', body:JSON.stringify({ text:$('text').value, ...textOptions() }) });
      status('OK');
    }

    async function sendTextStyle() {
      await api('/api/oled/style', { method:'POST', body:JSON.stringify(textOptions()) });
      status('OK');
    }

    function ledDebounced() { clearTimeout(ledTimer); ledTimer = setTimeout(() => sendLed().catch(e => status(e.message)), 80); }
    function textDebounced() { clearTimeout(textTimer); textTimer = setTimeout(() => sendText().catch(e => status(e.message)), 420); }
    function styleDebounced() { clearTimeout(styleTimer); styleTimer = setTimeout(() => sendTextStyle().catch(e => status(e.message)), 140); }

    async function loadState() {
      const data = await api('/api/state');
      $('color').value = rgbToHex(data.led.r, data.led.g, data.led.b);
      $('brightness').value = data.led.config.brightness;
      $('effect').value = data.led.effect;
      $('effectSpeed').value = data.led.speed;
      $('text').value = data.oled.text || '';
      $('textSize').value = data.oled.size;
      $('textY').value = data.oled.y;
      $('textAlign').value = data.oled.align;
      $('textBold').checked = data.oled.bold;
      $('scroll').checked = data.oled.scroll;
      $('scrollSpeed').value = data.oled.scrollSpeed;
      $('invert').checked = data.oled.inverted;
    }

    $('tabColor').addEventListener('click', () => showTab('color'));
    $('tabText').addEventListener('click', () => showTab('text'));
    $('color').addEventListener('input', ledDebounced);
    $('brightness').addEventListener('input', async () => { try { await api('/api/led/config', { method:'POST', body:JSON.stringify({ brightness:+$('brightness').value }) }); await sendLed(); } catch(e) { status(e.message); } });
    $('effect').addEventListener('change', () => sendEffect().catch(e => status(e.message)));
    $('effectSpeed').addEventListener('input', () => sendEffect().catch(e => status(e.message)));
    document.querySelectorAll('.swatch').forEach(b => b.addEventListener('click', () => { $('color').value = b.dataset.c; ledDebounced(); }));
    document.querySelectorAll('.preset').forEach(b => b.addEventListener('click', () => { $('effect').value = b.dataset.e; sendEffect().catch(e => status(e.message)); }));
    $('ledOff').addEventListener('click', async () => { $('color').value = '#000000'; $('effect').value = 'solid'; await sendEffect(); await sendLed(); });
    $('ledTest').addEventListener('click', () => api('/api/led/test', { method:'POST' }).catch(e => status(e.message)));

    $('text').addEventListener('input', textDebounced);
    ['textSize','textY','textAlign','textBold','scroll','scrollSpeed'].forEach(id => $(id).addEventListener('input', styleDebounced));
    $('sendText').addEventListener('click', () => sendText().catch(e => status(e.message)));
    $('clear').addEventListener('click', async () => { await api('/api/oled/clear', { method:'POST' }); $('text').value = ''; });
    $('invert').addEventListener('change', () => api('/api/oled/invert', { method:'POST', body:JSON.stringify({ enabled:$('invert').checked }) }).catch(e => status(e.message)));

    loadState().catch(e => status(e.message));
  </script>
</body>
</html>
)HTML";

void sendJson(const JsonDocument& doc, int statusCode = 200) {
  String payload;
  serializeJson(doc, payload);
  server.send(statusCode, "application/json", payload);
}

void sendError(int statusCode, const char* message) {
  StaticJsonDocument<160> doc;
  doc["ok"] = false;
  doc["error"] = message;
  sendJson(doc, statusCode);
}

template <size_t Capacity>
bool parseJsonBody(StaticJsonDocument<Capacity>& doc) {
  if (!server.hasArg("plain")) {
    sendError(400, "missing body");
    return false;
  }
  const DeserializationError error = deserializeJson(doc, server.arg("plain"));
  if (error) {
    sendError(400, "invalid json");
    return false;
  }
  return true;
}

bool readByte(JsonVariant value, uint8_t& output, uint8_t minimum = 0, uint8_t maximum = 255) {
  if (!value.is<int>()) {
    return false;
  }
  const int raw = value.as<int>();
  if (raw < minimum || raw > maximum) {
    return false;
  }
  output = static_cast<uint8_t>(raw);
  return true;
}

uint8_t effectFromString(const String& effect) {
  if (effect == "blink") {
    return BoardConfig::LED_EFFECT_BLINK;
  }
  if (effect == "pulse") {
    return BoardConfig::LED_EFFECT_PULSE;
  }
  if (effect == "rainbow") {
    return BoardConfig::LED_EFFECT_RAINBOW;
  }
  return BoardConfig::LED_EFFECT_SOLID;
}

const char* effectName(uint8_t effect) {
  switch (effect) {
    case BoardConfig::LED_EFFECT_BLINK:
      return "blink";
    case BoardConfig::LED_EFFECT_PULSE:
      return "pulse";
    case BoardConfig::LED_EFFECT_RAINBOW:
      return "rainbow";
    default:
      return "solid";
  }
}

String sanitizeText(String text) {
  String output;
  output.reserve(min(text.length(), BoardConfig::API_MAX_TEXT_LENGTH));
  for (size_t index = 0; index < text.length() && output.length() < BoardConfig::API_MAX_TEXT_LENGTH; ++index) {
    const char value = text.charAt(index);
    if (value == '\r') {
      continue;
    }
    if (value == '\n' || value == '\t' || static_cast<unsigned char>(value) >= 32) {
      output += value;
    }
  }
  return output;
}

void handleIndex() {
  server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

void handleHealth() {
  StaticJsonDocument<192> doc;
  doc["ok"] = true;
  doc["uptime_ms"] = millis();
  doc["oled_ready"] = OledControl::isReady();
  sendJson(doc);
}

void handleState() {
  const DeviceState& state = AppController::state();
  const LedConfig ledConfig = LedControl::config();

  StaticJsonDocument<768> doc;
  doc["ok"] = true;
  JsonObject led = doc.createNestedObject("led");
  led["r"] = state.ledRed;
  led["g"] = state.ledGreen;
  led["b"] = state.ledBlue;
  led["effect"] = effectName(state.ledEffect);
  led["speed"] = state.ledEffectSpeed;
  JsonObject config = led.createNestedObject("config");
  config["mode"] = LedControl::modeName(ledConfig.mode);
  config["brightness"] = ledConfig.brightness;

  JsonObject oled = doc.createNestedObject("oled");
  oled["ready"] = OledControl::isReady();
  oled["text"] = state.oledText;
  oled["inverted"] = state.oledInverted;
  oled["size"] = state.oledTextSize;
  oled["align"] = state.oledTextAlign;
  oled["y"] = state.oledTextY;
  oled["bold"] = state.oledTextBold;
  oled["scroll"] = state.oledScroll;
  oled["scrollSpeed"] = state.oledScrollSpeed;

  JsonObject wifi = doc.createNestedObject("wifi");
  wifi["ssid"] = WifiAp::ssid();
  wifi["ip"] = WifiAp::ip().toString();
  wifi["clients"] = WifiAp::connectedClients();
  sendJson(doc);
}

void handleLed() {
  StaticJsonDocument<192> doc;
  if (!parseJsonBody(doc)) {
    return;
  }
  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;
  if (!readByte(doc["r"], red) || !readByte(doc["g"], green) || !readByte(doc["b"], blue)) {
    sendError(400, "r/g/b must be 0..255 integers");
    return;
  }
  AppController::setLed(red, green, blue);
  handleState();
}

void handleLedConfig() {
  StaticJsonDocument<192> doc;
  if (!parseJsonBody(doc)) {
    return;
  }
  LedConfig config = LedControl::config();
  config.mode = BoardConfig::LedHardware::BuiltinRgb;
  config.rgbPin = BoardConfig::PIN_LED_RGB;
  if (!doc["brightness"].isNull() && !readByte(doc["brightness"], config.brightness, 1, 255)) {
    sendError(400, "brightness must be 1..255");
    return;
  }
  AppController::setLedConfig(config);
  handleState();
}

void handleLedEffect() {
  StaticJsonDocument<192> doc;
  if (!parseJsonBody(doc)) {
    return;
  }
  uint8_t effect = AppController::state().ledEffect;
  uint8_t speed = AppController::state().ledEffectSpeed;
  if (!doc["effect"].isNull()) {
    effect = doc["effect"].is<const char*>() ? effectFromString(doc["effect"].as<String>()) : static_cast<uint8_t>(doc["effect"].as<int>());
  }
  if (!doc["speed"].isNull() && !readByte(doc["speed"], speed, 1, 10)) {
    sendError(400, "speed must be 1..10");
    return;
  }
  AppController::setLedEffect(effect, speed);
  handleState();
}

void handleLedTest() {
  LedControl::selfTest();
  handleState();
}

void applyOledOptions(StaticJsonDocument<448>& doc, bool persist) {
  const DeviceState& state = AppController::state();
  uint8_t size = state.oledTextSize;
  uint8_t align = state.oledTextAlign;
  uint8_t y = state.oledTextY;
  bool bold = state.oledTextBold;
  bool scroll = state.oledScroll;
  uint8_t scrollSpeed = state.oledScrollSpeed;

  if (!doc["size"].isNull()) {
    readByte(doc["size"], size, 1, BoardConfig::OLED_TEXT_SIZE_MAX);
  }
  if (!doc["align"].isNull()) {
    readByte(doc["align"], align, 0, 2);
  }
  if (!doc["y"].isNull()) {
    readByte(doc["y"], y, 0, BoardConfig::OLED_HEIGHT - 8);
  }
  if (!doc["bold"].isNull() && doc["bold"].is<bool>()) {
    bold = doc["bold"].as<bool>();
  }
  if (!doc["scroll"].isNull() && doc["scroll"].is<bool>()) {
    scroll = doc["scroll"].as<bool>();
  }
  if (!doc["scrollSpeed"].isNull()) {
    readByte(doc["scrollSpeed"], scrollSpeed, 1, 10);
  }

  AppController::setOledTextStyle(size, align, y, bold, false);
  AppController::setOledScroll(scroll, scrollSpeed, persist);
}

void handleOledText() {
  StaticJsonDocument<448> doc;
  if (!parseJsonBody(doc)) {
    return;
  }
  if (!doc["text"].is<const char*>()) {
    sendError(400, "text must be a string");
    return;
  }
  applyOledOptions(doc, false);
  AppController::setOledText(sanitizeText(doc["text"].as<String>()));
  handleState();
}

void handleOledStyle() {
  StaticJsonDocument<448> doc;
  if (!parseJsonBody(doc)) {
    return;
  }
  applyOledOptions(doc, true);
  handleState();
}

void handleOledClear() {
  AppController::clearOled();
  handleState();
}

void handleOledInvert() {
  StaticJsonDocument<192> doc;
  if (!parseJsonBody(doc)) {
    return;
  }
  if (!doc["enabled"].is<bool>()) {
    sendError(400, "enabled must be a boolean");
    return;
  }
  AppController::setOledInverted(doc["enabled"].as<bool>());
  handleState();
}

void handleNotFound() {
  sendError(404, "not found");
}

}  // namespace

namespace WebServerControl {

void begin() {
  server.on("/", HTTP_GET, handleIndex);
  server.on("/api/health", HTTP_GET, handleHealth);
  server.on("/api/state", HTTP_GET, handleState);
  server.on("/api/led", HTTP_POST, handleLed);
  server.on("/api/led/config", HTTP_POST, handleLedConfig);
  server.on("/api/led/effect", HTTP_POST, handleLedEffect);
  server.on("/api/led/test", HTTP_POST, handleLedTest);
  server.on("/api/oled/text", HTTP_POST, handleOledText);
  server.on("/api/oled/style", HTTP_POST, handleOledStyle);
  server.on("/api/oled/clear", HTTP_POST, handleOledClear);
  server.on("/api/oled/invert", HTTP_POST, handleOledInvert);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  server.handleClient();
}

}  // namespace WebServerControl
