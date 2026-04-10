
#include <Arduino.h>
#include "MainController.h"
#include "WiFiManager.h" 

// Instancia os dois "cérebros" separadamente
MainController controller;
WiFiManager wifiManager;

void setup() {
    Serial.begin(115200);
    
    // 1. Inicia o Wi-Fi e o Portal Cativo 
    wifiManager.begin(); 
    
    // 2. Inicia os sensores e a máquina de estados 
    controller.setup(); 
}

void loop() {
    // Roda as duas tarefas ao mesmo tempo sem bloquear o processador!
    wifiManager.loop(); // Mantém o Wi-Fi vivo
    controller.run();   // Lê os sensores e pisca o LED
}