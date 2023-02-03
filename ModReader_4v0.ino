#include "declarations.h"

void setup() {
  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  pinMode(LED_PIN_3, OUTPUT);
  pinMode(LED_PIN_4, OUTPUT);
  pinMode(LED_PIN_5, OUTPUT);
  
  digitalWrite(LED_PIN_1, HIGH);
  serverInit();
  Serial.begin(9600);
}

void loop() {
  HTTP.handleClient();
}
