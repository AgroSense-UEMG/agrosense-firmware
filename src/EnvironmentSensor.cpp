#include "EnvironmentSensor.h"

EnvironmentSensor::EnvironmentSensor(uint8_t dhtPin, uint8_t dhtType)
    : _dht(dhtPin, dhtType), _pin(dhtPin), _type(dhtType) {
}

void EnvironmentSensor::begin() {
  _dht.begin();
}

EnvironmentSensor::EnvData EnvironmentSensor::readData() {
  EnvironmentSensor::EnvData data;

  // Leitura de temperatura e umidade do DHT11
  data.temperature = _dht.readTemperature();
  data.humidity = _dht.readHumidity();
  data.valid = true;

  // Tratamento de erros com isnan()
  if (isnan(data.temperature) || isnan(data.humidity)) {
    data.valid = false;
    Serial.println("ERRO: Falha na leitura do DHT11. Verifique conexão e alimentação do sensor.");
    data.temperature = NAN;
    data.humidity = NAN;
  }

  return data;
}
