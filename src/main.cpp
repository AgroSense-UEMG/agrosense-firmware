#include <Arduino.h>

#define LED_PIN 2

// Variáveis para controlar o tempo sem travar
unsigned long previousMillis = 0;  // Guarda o último momento que o LED piscou
const long interval = 1000;        // Intervalo desejado (1000ms = 1 segundo)

// Estado atual do LED
int ledState = LOW;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("AgroSense Node-Six: Boot Inicializado! 🚀");
}

void loop() {
  // 1. Pega o tempo atual da máquina
  unsigned long currentMillis = millis();

  // 2. Pergunta: "Já passou 1 segundo desde a última vez?"
  if (currentMillis - previousMillis >= interval) {
    // Sim, passou! Salva o tempo atual para a próxima contagem
    previousMillis = currentMillis;

    // Troca o estado do LED (Se tá ligado, desliga. Se tá desligado, liga)
    if (ledState == LOW) {
      ledState = HIGH;
      Serial.println("Status: Aguardando sensores... (LED ON)");
    } else {
      ledState = LOW;
      Serial.println("Status: Aguardando sensores... (LED OFF)");
    }

    // Aplica a mudança no pino físico
    digitalWrite(LED_PIN, ledState);
  }

  // --- ÁREA LIVRE ---
  // Aqui você pode colocar código para ler sensores ou verificar Wi-Fi.
  // O processador vai passar por aqui milhares de vezes por segundo
  // enquanto o "if" lá de cima não for verdadeiro.
}
