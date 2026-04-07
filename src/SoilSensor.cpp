#include "SoilSensor.h"

SoilSensor::SoilSensor() {
    // Construtor vazio, a configuração dos pinos já foi feita no header
}

void SoilSensor::begin() {
    // Configura todos os 6 pinos como entrada
    for (int i = 0; i < numSensors; i++) {
        pinMode(sensorPins[i], INPUT);
    }
    Serial.println("Driver SoilSensor (6 canais) Inicializado!");
}

void SoilSensor::readAllSensors(int* results) {
    // Passa por cada um dos 6 sensores
    for (int i = 0; i < numSensors; i++) {
        long sum = 0; // Usamos long para evitar overflow na soma
        
        // Faz 10 leituras do mesmo sensor muito rápido
        for (int j = 0; j < numReadings; j++) {
            sum += analogRead(sensorPins[i]);
            delay(2); // Pequeno atraso para o conversor do ESP32 respirar
        }
        
        // Tira a média e guarda no array de resultados
        results[i] = sum / numReadings;
    }
}