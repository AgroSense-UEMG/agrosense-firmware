#ifndef ENVIRONMENT_SENSOR_H
#define ENVIRONMENT_SENSOR_H

#include <Arduino.h>
#include <DHT.h>

class EnvironmentSensor {
public:
  // Estrutura de dados para retornar leitura de temperatura e umidade.
  struct EnvData {
    float temperature; // Temperatura em graus Celsius
    float humidity;    // Umidade relativa em porcentagem
    bool valid;        // Indica se a leitura foi válida
  };

  // Cria um sensor DHT11 no pino informado.
  EnvironmentSensor(uint8_t dhtPin = 4, uint8_t dhtType = DHT11);

  // Inicializa o sensor (deve ser chamado no setup).
  void begin();

  // Lê os dados do sensor e devolve EnvData contendo valid.
  EnvData readData();

private:
  DHT _dht;
  uint8_t _pin;
  uint8_t _type;
};

#endif // ENVIRONMENT_SENSOR_H
