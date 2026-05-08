#include <Arduino.h>
#include "app_controller.h"

void setup() {
  AppController::begin();
}

void loop() {
  AppController::loop();
}
