# AgroSense - Firmware (Node-Six) 📡

Bem-vindo ao repositório oficial do **Squad Embarcado** do projeto AgroSense.
Aqui fica o código C++ para os dispositivos ESP32.

## ⚠️ Regra de Ouro (LEIA ANTES DE CODAR)
A branch `main` está **bloqueada** para commits diretos.

**Como trabalhar:**
1. Sempre crie uma nova branch para sua tarefa: `git checkout -b feature/sensor-solo`
2. Faça seus commits.
3. Abra um **Pull Request (PR)** para a `main` quando terminar.

---
**Stack:** C++, PlatformIO, ESP32.

## 📘 Exemplo de uso: EnvironmentSensor (DHT11)

### 1) Conectar DHT11
- DHT11 Data -> GPIO4
- VCC -> 3.3V
- GND -> GND

### 2) Exemplo de código
```cpp
#include <Arduino.h>
#include "EnvironmentSensor.h"

EnvironmentSensor envSensor(4, DHT11);

void setup() {
  Serial.begin(115200);
  envSensor.begin();
}

void loop() {
  EnvironmentSensor::EnvData data = envSensor.readData();
  if (data.valid) {
    Serial.print("Temperatura: ");
    Serial.print(data.temperature, 1);
    Serial.print(" C, Umidade: ");
    Serial.print(data.humidity, 1);
    Serial.println(" %");
  } else {
    Serial.println("Leitura inválida do DHT11.");
  }
  delay(2000); // Em produção, prefira millis() para código não bloqueante.
}
```

### 3) O que a classe faz
- `EnvironmentSensor::EnvData` tem `temperature`, `humidity` e `valid`.
- `readData()` garante leitura DHT11 no mínimo a cada 2000ms (não bloqueante).
- Se houver `NaN`, o sensor é reinicializado suavemente e `valid` é `false`.
- Implementa verificação de sanidade para saltos impossíveis de temperatura/umidade.
- Logs de depuração são emitidos no Serial (`Lendo DHT11...`, `Aguardando intervalo...`, `Erro de sanidade...`).

### 4) Build / dependências
No `platformio.ini`:
```ini
lib_deps =
  bblanchon/ArduinoJson @ ^7.0.0
  adafruit/DHT sensor library @ ^1.4.6
  adafruit/Adafruit Unified Sensor @ ^1.1.14
```

### 5) Wi-Fi implementado
O firmware agora inclui um `WiFiManager` com captive portal para ESP32.
O dispositivo tenta conectar automaticamente com as credenciais armazenadas e, se falhar ou se não houver configuração, inicia um ponto de acesso chamado `AgroSense-Setup`.

Funcionalidades:
- conexão em modo Station quando credenciais estão salvas
- reconexão automática com limite de tentativas
- backoff entre tentativas para não travar o loop
- portal cautivo com DNS redirecionando para a página de configuração
- formulário para informar SSID, senha e token do usuário
- persistência via NVS/Preferences para salvar credenciais e token

Para testar:
1. Compile e faça upload para o ESP32.
2. Abra o Serial Monitor em `115200`.
3. Se o dispositivo não tiver credenciais válidas ou não conseguir se conectar, o AP `AgroSense-Setup` será iniciado.
4. Conecte um celular ou notebook em `AgroSense-Setup`.
5. Abra qualquer endereço no navegador; você será redirecionado para o portal.
6. Preencha o SSID, a senha da sua rede e, opcionalmente, o token do usuário.
7. Salve; o dispositivo reiniciará e tentará se conectar à rede configurada.

Para validar o funcionamento do captive portal, desconecte o roteador ou configure uma senha errada e observe o ESP32 abrir novamente o AP de configuração.

