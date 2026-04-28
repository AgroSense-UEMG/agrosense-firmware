#include "WiFiManager.h"

WiFiManager* WiFiManager::_instance = nullptr;

WiFiManager::WiFiManager(uint8_t maxReconnectAttempts,
                         unsigned long reconnectBackoffMs,
                         unsigned long reconnectResetMs)
    : _server(80),
      _connected(false),
      _portalStarted(false),
      _reconnectPending(false),
      _reconnectAttempts(0),
      _lastActionMillis(0),
      _lastStatusMillis(0),
      _maxReconnect(maxReconnectAttempts),
      _reconnectBackoffMs(reconnectBackoffMs),
      _reconnectResetMs(reconnectResetMs) {
  _instance = this;
}

void WiFiManager::begin() {
  loadConfig();
  WiFi.onEvent(onWiFiEventStatic);

  _server.on("/", HTTP_GET, [this]() { handleRoot(); });
  _server.on("/save", HTTP_POST, [this]() { handleSave(); });
  _server.onNotFound([this]() { handleNotFound(); });

  if (_config.isValid()) {
    startStation();
  } else {
    Serial.println("[WiFiManager] Nenhuma configuração de Wi-Fi encontrada. Iniciando portal cautivo.");
    startCaptivePortal();
  }
}

void WiFiManager::loop() {
  if (_portalStarted) {
    _dnsServer.processNextRequest();
    _server.handleClient();
    return;
  }

  unsigned long now = millis();
  if (_reconnectPending) {
    attemptReconnect();
  }

  if (now - _lastStatusMillis >= 5000UL) {
    _lastStatusMillis = now;
    wl_status_t status = WiFi.status();
    Serial.printf("[WiFiManager] Wi-Fi status: %d (%s)\n", status,
                  status == WL_CONNECTED ? "CONNECTED" : "DISCONNECTED");
    if (status == WL_CONNECTED) {
      Serial.printf("[WiFiManager] IP: %s\n", WiFi.localIP().toString().c_str());
      Serial.printf("[WiFiManager] SSID: %s\n", WiFi.SSID().c_str());
      Serial.printf("[WiFiManager] RSSI: %d dBm\n", WiFi.RSSI());
    }
  }
}

bool WiFiManager::isConnected() const {
  return _connected && WiFi.status() == WL_CONNECTED;
}

bool WiFiManager::portalActive() const {
  return _portalStarted;
}

const WiFiManager::Config& WiFiManager::getConfig() const {
  return _config;
}

void WiFiManager::onWiFiEventStatic(WiFiEvent_t event) {
  if (_instance) {
    _instance->onWiFiEvent(event);
  }
}

void WiFiManager::onWiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_STA_START:
      Serial.println("[WiFiManager] STA iniciado.");
      break;
    case SYSTEM_EVENT_STA_CONNECTED:
      Serial.println("[WiFiManager] Conectado ao AP.");
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      _connected = true;
      _reconnectAttempts = 0;
      _reconnectPending = false;
      Serial.printf("[WiFiManager] IP obtido: %s\n", WiFi.localIP().toString().c_str());
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      _connected = false;
      _reconnectPending = true;
      Serial.printf("[WiFiManager] Desconectado do AP. Tentativas: %u/%u\n",
                    _reconnectAttempts, _maxReconnect);
      break;
    default:
      Serial.printf("[WiFiManager] Evento Wi-Fi: %u\n", event);
      break;
  }
}

void WiFiManager::loadConfig() {
  _preferences.begin("agrosense", true);
  _config.ssid = _preferences.getString("ssid", "");
  _config.password = _preferences.getString("password", "");
  _config.userToken = _preferences.getString("token", "");
  _preferences.end();

  if (_config.isValid()) {
    Serial.println("[WiFiManager] Configuração de Wi-Fi carregada.");
    Serial.printf("[WiFiManager] SSID salvo: %s\n", _config.ssid.c_str());
    if (_config.userToken.length() > 0) {
      Serial.printf("[WiFiManager] Token de usuário carregado: %s\n", _config.userToken.c_str());
    }
  }
}

void WiFiManager::saveConfig(const String& ssid,
                             const String& password,
                             const String& userToken) {
  _preferences.begin("agrosense", false);
  _preferences.putString("ssid", ssid);
  _preferences.putString("password", password);
  _preferences.putString("token", userToken);
  _preferences.end();

  _config.ssid = ssid;
  _config.password = password;
  _config.userToken = userToken;

  Serial.println("[WiFiManager] Configuração de Wi-Fi salva no NVS.");
}

void WiFiManager::startStation() {
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("AgroSenseNode");
  Serial.println("[WiFiManager] Iniciando modo Station.");
  WiFi.begin(_config.ssid.c_str(), _config.password.c_str());
  _lastActionMillis = millis();
  _reconnectPending = true;
}

void WiFiManager::attemptReconnect() {
  if (_connected || _portalStarted) {
    return;
  }

  unsigned long now = millis();
  if (_reconnectAttempts >= _maxReconnect) {
    if (now - _lastActionMillis >= _reconnectResetMs) {
      Serial.println("[WiFiManager] Reiniciando contador de tentativas.");
      _reconnectAttempts = 0;
      _lastActionMillis = now;
    } else if (!_portalStarted) {
      Serial.println("[WiFiManager] Número máximo de tentativas alcançado. Abrindo portal cautivo.");
      startCaptivePortal();
    }
    return;
  }

  if (now - _lastActionMillis < _reconnectBackoffMs) {
    return;
  }

  _reconnectAttempts++;
  _lastActionMillis = now;
  Serial.printf("[WiFiManager] Tentativa de reconexão %u/%u...\n", _reconnectAttempts, _maxReconnect);
  WiFi.disconnect(true, true);
  WiFi.begin(_config.ssid.c_str(), _config.password.c_str());
}

void WiFiManager::startCaptivePortal() {
  if (_portalStarted) {
    return;
  }

  Serial.println("[WiFiManager] Iniciando captive portal...");

  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_AP_STA);
  IPAddress apIP(192, 168, 4, 1);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("AgroSense-Setup");
  delay(100);

  _dnsServer.start(53, "*", apIP);
  _server.begin();
  _portalStarted = true;
  _reconnectPending = false;
  Serial.println("[WiFiManager] Portal cautivo ativo em: AgroSense-Setup");
  Serial.println("[WiFiManager] Acesse qualquer URL para ver o portal de configuração.");
}

void WiFiManager::stopCaptivePortal() {
  if (!_portalStarted) {
    return;
  }

  Serial.println("[WiFiManager] Parando captive portal.");
  _server.stop();
  _dnsServer.stop();
  WiFi.softAPdisconnect(true);
  _portalStarted = false;
}

String WiFiManager::captivePortalPage(const String& message) const {
  String html = "<!DOCTYPE html><html lang=\"pt-BR\">";
  html += "<head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  html += "<title>Portal AgroSense</title>";
  html += "<style>body{font-family:Arial,sans-serif;background:#f4f7f9;color:#222;margin:0;padding:0;}";
  html += " .page{max-width:480px;margin:48px auto;padding:24px;background:#fff;border-radius:12px;box-shadow:0 10px 30px rgba(0,0,0,.08);} ";
  html += " h1{margin-top:0;font-size:24px;} label{display:block;margin:12px 0 6px;font-weight:600;}";
  html += " input[type=text], input[type=password]{width:100%;padding:10px;border:1px solid #ccd0d5;border-radius:6px;}";
  html += " button{margin-top:18px;padding:12px 18px;background:#0078d4;color:#fff;border:none;border-radius:8px;font-size:16px;cursor:pointer;}";
  html += " .hint{font-size:14px;color:#555;margin-top:12px;}";
  html += " .message{background:#e8f4ff;color:#003a6d;padding:12px;border-radius:8px;margin-bottom:16px;}";
  html += "</style></head><body><div class=\"page\">";
  html += "<h1>Portal AgroSense</h1>";
  html += "<p>Conecte o dispositivo à sua rede Wi-Fi e informe o token do usuário.</p>";
  if (message.length() > 0) {
    html += "<div class=\"message\">" + message + "</div>";
  }
  html += "<form method=\"POST\" action=\"/save\">";
  html += "<label for=\"ssid\">SSID</label>";
  html += "<input type=\"text\" id=\"ssid\" name=\"ssid\" value=\"" + _config.ssid + "\" required>";
  html += "<label for=\"password\">Senha Wi-Fi</label>";
  html += "<input type=\"password\" id=\"password\" name=\"password\" value=\"" + _config.password + "\" required>";
  html += "<label for=\"token\">Token do Usuário (opcional)</label>";
  html += "<input type=\"text\" id=\"token\" name=\"token\" value=\"" + _config.userToken + "\">";
  html += "<button type=\"submit\">Salvar e Conectar</button>";
  html += "</form>";
  html += "<p class=\"hint\">Após salvar, o dispositivo irá reiniciar e tentar conectar à rede configurada.</p>";
  html += "</div></body></html>";
  return html;
}

void WiFiManager::handleRoot() {
  _server.send(200, "text/html", captivePortalPage());
}

void WiFiManager::handleSave() {
  String ssid = _server.arg("ssid");
  String password = _server.arg("password");
  String token = _server.arg("token");

  if (ssid.length() == 0 || password.length() == 0) {
    _server.send(400, "text/html", captivePortalPage("SSID e senha são obrigatórios."));
    return;
  }

  saveConfig(ssid, password, token);
  _server.send(200, "text/html", captivePortalPage("Configurações salvas com sucesso. O dispositivo reiniciará em breve."));
  delay(2000);
  ESP.restart();
}

void WiFiManager::handleNotFound() {
  _server.sendHeader("Location", "/", true);
  _server.send(302, "text/plain", "");
}
