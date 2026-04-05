#include "MainController.h"

#define LED_PIN 2

// Inicializamos o sensor e as variáveis de tempo
MainController::MainController() : environmentSensor(4, DHT11), interval(2000) {
    currentState = BOOT;
    previousMillis = 0;
    ledState = LOW;
}

void MainController::setup() {
    Serial.begin(115200);
    pinMode(2, OUTPUT); // Pino do LED_PIN definido pelo Paulo
    environmentSensor.begin(); // Inicializa o hardware do sensor ambiente
    Serial.println("AgroSense Node-Six: Boot Inicializado!");
}

void MainController::run() {
    unsigned long currentMillis = millis(); // Pega o tempo atual em milissegundos

    switch (currentState) {
        case BOOT:
            // O estado BOOT apenas confirma que tudo ligou e passa para o próximo
            Serial.println("Estado: BOOT - Inicializando Drivers...");
            Serial.println("Transição: Mudando de BOOT para WORKING...");
            currentState = WORKING; // Por enquanto, pulamos direto para o trabalho
            break;
        
        case CONNECTING:
            // Será implementado na Sprint 2 (Wi-Fi)
            break;

        case HANDSHAKE:
            // Será implementado na Sprint 2 (Manifesto JSON)
            break;

        case WORKING:
            // Verifica se já passaram 2 segundos desde a última vez
            if (currentMillis - previousMillis >= interval) {
                previousMillis = currentMillis; // Atualiza o tempo do último ciclo

                // Lógica de piscar o LED (Status do dispositivo)
                ledState = (ledState == LOW ? HIGH : LOW); // inverte o estado
                digitalWrite(LED_PIN, ledState);

                Serial.print("LED ");
                Serial.println(ledState == HIGH ? "ON" : "OFF");
                
                // Chama a função que o Paulo criou para ler dados do sensor ambiente
                EnvironmentSensor::EnvData reading = environmentSensor.readData();
                
                if (reading.valid) {
                    Serial.print("Sucesso! Temp: ");
                    Serial.print(reading.temperature, 1);
                    Serial.print("C | Umid: ");
                    Serial.print(reading.humidity, 1);
                    Serial.println("%");
                } else {
                    Serial.println("Erro: Leitura do DHT11 falhou.");
                }
            }
            break;
        
        case SLEEP:
            // Será implementado na Sprint 3 (Deep Sleep)
            break;

        default:
            break;
    }
}