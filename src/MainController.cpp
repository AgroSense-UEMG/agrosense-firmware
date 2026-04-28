#include "MainController.h"
#include <ArduinoJson.h> // Importa a biblioteca para manipular JSON
#include <WiFi.h>        // Necessário para capturar o MAC Address real
#include <HTTPClient.h>  // ADICIONADO NA SPRINT 2: Cliente HTTP para os POSTs

#define LED_PIN 2

// Constantes de Endpoints da API (Ajustadas para o padrão do projeto)
const char* API_URL_REGISTER = "http://agrosense.eco.br/api/devices/register";
const char* API_URL_DATA = "http://agrosense.eco.br/api/devices/data";

// Inicializamos o sensor e as variáveis de tempo
MainController::MainController() : environmentSensor(4, DHT11), interval(2000) {
    currentState = BOOT;
    previousMillis = 0;
    ledState = LOW;
}

void MainController::setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT); // Pino do LED_PIN definido pelo Paulo
    environmentSensor.begin(); // Inicializa o hardware do sensor ambiente
    soilSensor.begin();
    Serial.println("AgroSense Node-Six: Boot Inicializado!");
}

void MainController::run() {
    unsigned long currentMillis = millis(); // Pega o tempo atual em milissegundos

    switch (currentState) {
        case BOOT:
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
            Serial.println("Enviando POST /register (Handshake)...");

            if (WiFi.status() == WL_CONNECTED) {
                HTTPClient http;
                http.begin(API_URL_REGISTER);
                http.addHeader("Content-Type", "application/json");

                // Envia o payload gerado pela Maria Luisa
                int httpResponseCode = http.POST(manifest);

                if (httpResponseCode > 0) {
                    Serial.print("Handshake Aceito! Código HTTP: ");
                    Serial.println(httpResponseCode);
                    String response = http.getString();
                    Serial.println("Resposta do Servidor: " + response);
                    
                    // Só vai para o WORKING se a API aceitou o dispositivo
                    currentState = WORKING;
                } else {
                    Serial.print("Erro de Conexão no Handshake. Código HTTP: ");
                    Serial.println(httpResponseCode);
                    // Fica retido no HANDSHAKE para tentar novamente no próximo ciclo
                }
                http.end();
            }
            break;
        }

        case WORKING: {
            // Verifica se já passaram 2 segundos desde a última vez
            if (currentMillis - previousMillis >= interval) {
                previousMillis = currentMillis; 

                // Pisca o LED indicando coleta
                ledState = (ledState == LOW ? HIGH : LOW); 
                digitalWrite(LED_PIN, ledState);
                
                // Lê dados reais do sensor ambiente
                EnvironmentSensor::EnvData reading = environmentSensor.readData();
                
                if (reading.valid) {
                    Serial.print("Lido! Temp: ");
                    Serial.print(reading.temperature, 1);
                    Serial.print("C | Umid: ");
                    Serial.print(reading.humidity, 1);
                    Serial.println("%");

                    // INÍCIO DO ENVIO POST /data
                    if (WiFi.status() == WL_CONNECTED) {
                        JsonDocument dataDoc;
                        dataDoc["hardware_id"] = WiFi.macAddress();
                        
                        JsonArray readings = dataDoc["readings"].to<JsonArray>();
                        
                        // Objeto de Temperatura
                        JsonObject t = readings.add<JsonObject>();
                        t["id_componente"] = "dht_temp";
                        t["valor"] = reading.temperature;

                        // Objeto de Umidade
                        JsonObject h = readings.add<JsonObject>();
                        h["id_componente"] = "dht_hum";
                        h["valor"] = reading.humidity;

                        // (A leitura dos sensores de solo será acoplada aqui futuramente)

                        String dataPayload;
                        serializeJson(dataDoc, dataPayload);

                        HTTPClient http;
                        http.begin(API_URL_DATA);
                        http.addHeader("Content-Type", "application/json");

                        int httpResponseCode = http.POST(dataPayload);
                        if (httpResponseCode > 0) {
                            Serial.print("Dados sincronizados com a nuvem. HTTP: ");
                            Serial.println(httpResponseCode);
                        } else {
                            Serial.print("Falha no POST /data. HTTP: ");
                            Serial.println(httpResponseCode);
                        }
                        http.end();
                    }
                } else {
                    Serial.println("Erro: Leitura do DHT11 falhou. Pulando envio.");
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
    temp["id_componente"] = "dht_temp";
    temp["nome_exibicao"] = "Temperatura do Ar";
    temp["tipo"] = "sensor";
    temp["unidade_medida"] = "°C";

    // Sensor de Umidade (DHT)
    JsonObject hum = components.add<JsonObject>();
    hum["id_componente"] = "dht_hum";
    hum["nome_exibicao"] = "Umidade do Ar";
    hum["tipo"] = "sensor";
    hum["unidade_medida"] = "%";

    // Mapeamento dos 6 sensores de solo
    for (int i = 1; i <= 6; i++) {
        JsonObject soil = components.add<JsonObject>();
        // IDs e Nomes dinâmicos: soil_1, Umidade Solo 1...
        soil["id_componente"] = "soil_" + String(i);
        soil["nome_exibicao"] = "Umidade Solo " + String(i);
        soil["tipo"] = "sensor";
        soil["unidade_medida"] = "%";
    }

    // Transforma o objeto JSON em uma String para envio
    String output;
    serializeJson(doc, output);
    
    return output; // Retorna o JSON pronto para o HTTP POST
}