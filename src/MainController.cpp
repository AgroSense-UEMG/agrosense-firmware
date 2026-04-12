#include "MainController.h"
#include <ArduinoJson.h> // Importa a biblioteca para manipular JSON
#include <WiFi.h>        // Necessário para capturar o MAC Address real

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
            Serial.println("Transição: Mudando de BOOT para CONNECTING...");
            currentState = CONNECTING;
            break;
        
        case CONNECTING:
            if (WiFi.status() == WL_CONNECTED){  // O WiFi.status() é uma função interna do ESP32 que diz se a rede está OK
                Serial.println("Wi-Fi Conectado! Indo para Handshake...");
                currentState = HANDSHAKE;
            }
            break;

        case HANDSHAKE: {
            Serial.println("Gerando Manifesto JSON...");
            String manifest = getManifest();
            Serial.println("Manifesto Gerado:"); 
            Serial.println(manifest); // Aqui valida visualmente no Serial Monitor
            currentState = WORKING;
            break;
        }

        case WORKING: {
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
        }
        
        case SLEEP: 
            // Será implementado na Sprint 3 (Deep Sleep)
            break;

        default:
            break;
    }
}

String MainController::getManifest() {
    // Cria um documento JSON (estimamos 1024 bytes de memória)
    JsonDocument doc;

    // Captura o MAC Address real do ESP32 para o hardware_id
    doc["hardware_id"] = WiFi.macAddress();
    doc["model"] = "Node-Six";

    // Cria a lista de componentes (sensores)
    JsonArray components = doc["components"].to<JsonArray>();

    // Sensor de Temperatura (DHT)
    JsonObject temp = components.add<JsonObject>();
    temp["id"] = "dht_temp";
    temp["unit"] = "°C";

    // Sensor de Umidade (DHT)
    JsonObject hum = components.add<JsonObject>();
    hum["id"] = "dht_hum";
    hum["unit"] = "%";

    // Mapeamento dos 6 sensores de solo
    for (int i = 1; i <= 6; i++) {
        JsonObject soil = components.add<JsonObject>();
        // Cria IDs dinâmicos: soil_1, soil_2...
        soil["id"] = "soil_" + String(i);
        soil["unit"] = "%";
    }

    // Transforma o objeto JSON em uma String para envio
    String output;
    serializeJson(doc, output);
    
    return output; // Retorna o JSON pronto para o HTTP POST
}