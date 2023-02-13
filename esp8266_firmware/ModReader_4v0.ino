#include "declarations.h"

void setup() {
  pinMode(LED_POWER_PIN, OUTPUT);
  pinMode(LED_WIFI_CONNECT_PIN, OUTPUT);
  pinMode(LED_TX_PIN, OUTPUT);
  pinMode(LED_RX_PIN, OUTPUT);
  pinMode(LED_PIN_5, OUTPUT);
  pinMode(RS485_PIN, OUTPUT);
  
  digitalWrite(RS485_PIN, LOW);
  digitalWrite(LED_POWER_PIN, HIGH);
  serverInit();
  Serial.begin(9600);
}

void loop() {
	if (WiFi.softAPgetStationNum() > 0) {
		digitalWrite(LED_WIFI_CONNECT_PIN, HIGH);
	} else {
		digitalWrite(LED_WIFI_CONNECT_PIN, LOW);
	}
	HTTP.handleClient();
}
