#include <Arduino.h>
#include <WiFi.h>
#include "EnvironmentSensor.h"

#define LED_PIN 2

// Configurações de Wi-Fi
const char* WIFI_SSID = "SEU_SSID";
const char* WIFI_PASSWORD = "SUA_SENHA";
const uint8_t WIFI_MAX_RECONNECT_ATTEMPTS = 5;
const unsigned long WIFI_RECONNECT_BACKOFF_MS = 5000UL; // 5 segundos
const unsigned long WIFI_STATUS_CHECK_MS = 5000UL;     // Checa status a cada 5 segundos
const unsigned long WIFI_RECONNECT_RESET_MS = 60000UL;  // Reinicia tentativas após 60 segundos

// Variáveis de controle de tempo
unsigned long previousMillis = 0;
unsigned long wifiLastActionMillis = 0;
unsigned long wifiLastStatusMillis = 0;

// Estado de conexão Wi-Fi
bool wifiConnected = false;
uint8_t wifiReconnectAttempts = 0;
bool wifiReconnectPending = false;

// Estado atual do LED
int ledState = LOW;

// Sensor DHT11 no GPIO4
EnvironmentSensor environmentSensor(4, DHT11);

void logWiFiEvent(const char* message) {
  Serial.printf("[WiFi] %s\n", message);
}

void printWiFiStatus() {
  wl_status_t status = WiFi.status();
  Serial.printf("[WiFi] status atual: %d (%s)\n", status,
                status == WL_CONNECTED ? "CONNECTED" : "DISCONNECTED");

  if (status == WL_CONNECTED) {
    Serial.printf("[WiFi] IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("[WiFi] SSID: %s\n", WiFi.SSID().c_str());
    Serial.printf("[WiFi] RSSI: %d dBm\n", WiFi.RSSI());
  }
}

void onWiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_STA_START:
      logWiFiEvent("Station iniciado.");
      break;
    case SYSTEM_EVENT_STA_CONNECTED:
      logWiFiEvent("Conectado ao AP.");
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      wifiConnected = true;
      wifiReconnectAttempts = 0;
      wifiReconnectPending = false;
      Serial.printf("[WiFi] IP obtido: %s\n", WiFi.localIP().toString().c_str());
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      wifiConnected = false;
      wifiReconnectPending = true;
      Serial.printf("[WiFi] Desconectado do AP. Tentativas: %u/%u\n",
                    wifiReconnectAttempts, WIFI_MAX_RECONNECT_ATTEMPTS);
      break;
    default:
      Serial.printf("[WiFi] Evento: %u\n", event);
      break;
  }
}

void startWiFiStation() {
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("AgroSenseNode");
  WiFi.onEvent(onWiFiEvent);
  logWiFiEvent("Iniciando modo Station.");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  wifiLastActionMillis = millis();
  wifiReconnectPending = true;
}

void attemptWiFiReconnect() {
  if (wifiConnected) {
    return;
  }

  unsigned long now = millis();

  if (wifiReconnectAttempts >= WIFI_MAX_RECONNECT_ATTEMPTS) {
    if (now - wifiLastActionMillis >= WIFI_RECONNECT_RESET_MS) {
      Serial.println("[WiFi] Reiniciando contagem de tentativas de reconexão.");
      wifiReconnectAttempts = 0;
      wifiLastActionMillis = now;
    } else {
      return;
    }
  }

  if (now - wifiLastActionMillis < WIFI_RECONNECT_BACKOFF_MS) {
    return;
  }

  wifiReconnectAttempts++;
  wifiLastActionMillis = now;
  Serial.printf("[WiFi] Tentativa de reconexão %u/%u...\n",
                wifiReconnectAttempts, WIFI_MAX_RECONNECT_ATTEMPTS);
  WiFi.disconnect(true, true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void monitorWiFi() {
  unsigned long now = millis();

  if (wifiReconnectPending) {
    attemptWiFiReconnect();
  }

  if (now - wifiLastStatusMillis >= WIFI_STATUS_CHECK_MS) {
    wifiLastStatusMillis = now;
    printWiFiStatus();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  environmentSensor.begin();
  Serial.println("AgroSense Node-Six: Boot Inicializado! 🚀");
  startWiFiStation();
}

void loop() {
  unsigned long currentMillis = millis();

  monitorWiFi();

  // Pisca LED sem bloquear
  if (currentMillis - previousMillis >= 2000UL) {
    previousMillis = currentMillis;
    ledState = (ledState == LOW ? HIGH : LOW);
    digitalWrite(LED_PIN, ledState);
    Serial.print("LED ");
    Serial.println(ledState == HIGH ? "ON" : "OFF");

    // Lê dados do sensor ambiente
    EnvironmentSensor::EnvData reading = environmentSensor.readData();
    if (reading.valid) {
      Serial.print("Temperatura: ");
      Serial.print(reading.temperature, 1);
      Serial.print(" C, Umidade: ");
      Serial.print(reading.humidity, 1);
      Serial.println(" %");
    } else {
      Serial.println("Leitura ambiente inválida.");
    }
  }
}
