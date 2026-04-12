#ifndef MAIN_CONTROLLER_H
#define MAIN_CONTROLLER_H

#include <Arduino.h>
#include "EnvironmentSensor.h"

// Estados definidos no AgroSense
enum State {
    BOOT,
    CONNECTING,
    HANDSHAKE,
    WORKING,
    SLEEP
};

class MainController {
private:
    State currentState;           // Guarda em que fase está
    unsigned long previousMillis; // Para controlar o tempo sem travar o código (substitui delay)
    const long interval;          // Tempo entre as leituras (2 segundos)
    int ledState;                 // Estado atual do LED de status
    
    // Instancia o sensor do Paulo no pino D4 (GPIO 4) conforme o Pinout
    EnvironmentSensor environmentSensor;

    // SailSensor sailSensor; (aguardando)

public:
    MainController();           // Construtor: Inicializa as variáveis básicas
    void setup();               // Setup: Configura os pinos e inicia os sensores
    void run();                 // Run: A função que roda infinitamente no loop principal
    String getManifest();       // Função para gerar a identidade digital
};

#endif