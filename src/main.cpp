#include <Arduino.h>
#include "EnvironmentSensor.h"

#define LED_PIN 2

// Variáveis para controlar o tempo sem travar
unsigned long previousMillis = 0;  // Guarda o último momento que o LED piscou
const long interval = 2000;        // Intervalo desejado (2000ms = 2 segundos)

// Estado atual do LED
int ledState = LOW;

// Sensor DHT11 no GPIO4
EnvironmentSensor environmentSensor(4, DHT11);

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  environmentSensor.begin();
  Serial.println("AgroSense Node-Six: Boot Inicializado! 🚀");
}

void loop() {
  unsigned long currentMillis = millis();

  // Pisca LED sem bloquear
  if (currentMillis - previousMillis >= interval) {
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
