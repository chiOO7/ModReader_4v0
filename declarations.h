#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h> 

const int RS485_PIN = 2;      //D4
const int LED_POWER_PIN = 15;     //D8
const int LED_WIFI_CONNECT_PIN = 13;     //D7
const int LED_TX_PIN = 12;     //D6
const int LED_RX_PIN = 14;     //D5
const int LED_PIN_5 = 16;     //D0

const int RESPONSE_DELAY = 200;
const int WAIT_SERIAL_DELAY = 10;
const int WAIT_COUNT_DELAY = 10;

const char ESPControllerWifiAP_SSID[40] = "Modreader 4v0";
const char ESPControllerWifiAP_password[40] = "12345678";
const String BAD_RX = "[\"ERROR!\", \"BAD_RX\"]";
const String SET_OK = "[\"SET\", \"OK\"]";

int id;
int command;
int reg;
int val;

String printToWeb = "";

byte rxBuffer[64];

IPAddress ESPControllerWifiAP_ip(192, 168, 0, 1);
IPAddress  ESPControllerWifiAP_dns (192, 168, 0, 1);
IPAddress  ESPControllerWifiAP_gateway (192, 168, 0, 1);
IPAddress ESPControllerWifiAP_subnet (255, 255, 255, 0);
IPAddress apIP(192, 168, 0, 1);

ESP8266WebServer HTTP(80); 
