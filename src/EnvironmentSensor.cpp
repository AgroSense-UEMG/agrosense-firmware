#include "EnvironmentSensor.h"

EnvironmentSensor::EnvironmentSensor(uint8_t dhtPin, uint8_t dhtType)
    : _dht(dhtPin, dhtType), _pin(dhtPin), _type(dhtType), _lastReadTime(0), _hasLastValid(false) {
  _lastData.temperature = NAN;
  _lastData.humidity = NAN;
  _lastData.valid = false;
}

void EnvironmentSensor::begin() {
  _dht.begin();
}

bool EnvironmentSensor::isSanityOk(float temperature, float humidity) const {
  if (!_hasLastValid) {
    return true;
  }

  // Regras conservadoras de salto impossível.
  const float maxTempDelta = 20.0; // variação máxima aceitável em 2s
  const float maxHumidityDelta = 30.0; // variação máxima aceitável em 2s

  if (fabs(temperature - _lastData.temperature) > maxTempDelta) {
    return false;
  }
  if (fabs(humidity - _lastData.humidity) > maxHumidityDelta) {
    return false;
  }
  return true;
}

EnvironmentSensor::EnvData EnvironmentSensor::readData() {
  unsigned long now = millis();

  // Requisito DHT11: não ler mais de 1x a cada 2 segundos.
  if (now - _lastReadTime < 2000 && _lastData.valid) {
    Serial.println("Aguardando intervalo do sensor DHT11 (2000ms)... Retornando última leitura válida.");
    return _lastData;
  }

  Serial.println("Lendo DHT11...");
  float temp = _dht.readTemperature();
  float hum = _dht.readHumidity();
  _lastReadTime = now;

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Erro de leitura: valor NaN detectado. Reinicializando sensor...");
    _dht.begin();
    _lastData.valid = false;
    _lastData.temperature = NAN;
    _lastData.humidity = NAN;
    return _lastData;
  }

  if (!isSanityOk(temp, hum)) {
    Serial.println("Erro de sanidade: salto de leitura muito grande. Descartando leitura.");
    // Mantém última leitura válida e retorna ela para estabilidade.
    return _lastData;
  }

  _lastData.temperature = temp;
  _lastData.humidity = hum;
  _lastData.valid = true;
  _hasLastValid = true;

  Serial.print("Sucesso: Temp=");
  Serial.print(temp, 1);
  Serial.print(" C, Umid=");
  Serial.print(hum, 1);
  Serial.println(" %");

  return _lastData;
}
