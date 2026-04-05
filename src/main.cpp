#include <Arduino.h>
#include "MainController.h"

MainController controller; // Cria o objeto da classe

void setup() {
    controller.setup(); // Chama a configuração uma única vez
}

void loop() {
    controller.run(); // Fica rodando a máquina de estados repetidamente
}