#include "declarations.h"

void setup() {
  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  pinMode(LED_PIN_3, OUTPUT);
  pinMode(LED_PIN_4, OUTPUT);
  pinMode(LED_PIN_5, OUTPUT);
  pinMode(RS485_PIN, OUTPUT);
  
  digitalWrite(RS485_PIN, LOW);
  digitalWrite(LED_PIN_1, HIGH);
  serverInit();
  Serial.begin(9600);
}

void loop() {
	if (WiFi.softAPgetStationNum() > 0) {
		digitalWrite(LED_PIN_2, HIGH);
	} else {
		digitalWrite(LED_PIN_2, LOW);
	}
	HTTP.handleClient();
}
