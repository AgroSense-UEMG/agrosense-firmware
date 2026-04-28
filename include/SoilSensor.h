#ifndef SOILSENSOR_H
#define SOILSENSOR_H

#include <Arduino.h>

class SoilSensor {
private:
    // Pinos seguros do ADC1 para usar com Wi-Fi
    const int numSensors = 6;
    int sensorPins[6] = {32, 33, 34, 35, 36, 39}; 
    const int numReadings = 10; // Para a média móvel

public:
    SoilSensor();
    void begin();
    
    // Retorna um array com as 6 leituras já com a média calculada
    void readAllSensors(int* results); 
};

#endif