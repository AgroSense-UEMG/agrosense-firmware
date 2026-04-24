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
            if (WiFi.status() == WL_CONNECTED){  
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
            // Verifica se já passaram os 2 segundos (ou o intervalo definido)
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
                        t["id_componente"] = "dht11_temp";
                        t["valor"] = reading.temperature;

                        // Objeto de Umidade
                        JsonObject h = readings.add<JsonObject>();
                        h["id_componente"] = "dht11_hum";
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
    JsonDocument doc;

    doc["hardware_id"] = WiFi.macAddress();
    doc["model"] = "Node-Six";

    JsonArray components = doc["components"].to<JsonArray>();

    // JÁ APLICANDO A CORREÇÃO DE CONTRATO (Especificação de Software pg 39/40)
    JsonObject temp = components.add<JsonObject>();
    temp["id_componente"] = "dht11_temp";
    temp["tipo"] = "sensor";
    temp["nome_exibicao"] = "Temperatura do Ar";
    temp["unidade_medida"] = "°C";

    JsonObject hum = components.add<JsonObject>();
    hum["id_componente"] = "dht11_hum";
    hum["tipo"] = "sensor";
    hum["nome_exibicao"] = "Umidade do Ar";
    hum["unidade_medida"] = "%";

    for (int i = 1; i <= 6; i++) {
        JsonObject soil = components.add<JsonObject>();
        soil["id_componente"] = "soil_" + String(i);
        soil["tipo"] = "sensor";
        soil["nome_exibicao"] = "Umidade Solo " + String(i);
        soil["unidade_medida"] = "%";
    }

    String output;
    serializeJson(doc, output);
    
    return output; 
}