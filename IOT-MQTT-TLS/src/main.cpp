#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "secrets.h"

// Referenciamos las funciones y objetos de los otros archivos
extern void reconnect();
extern PubSubClient client;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a WiFi: ");
  Serial.println(WIFI_SSID_VALUE);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID_VALUE, WIFI_PASSWORD_VALUE);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado!");
  Serial.print("Direccion IP: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  // Inicializamos el monitor serial
  Serial.begin(115200);
  
  // Damos un pequeño tiempo para que el puerto serial se abra
  delay(2000);
  Serial.println("\n\n--- INICIANDO ESP32 SIGMOTOS (VERSION OTA MAGICA) ---");

  // Conectamos a WiFi
  setup_wifi();

  // Configuramos el servidor MQTT
  client.setServer(MQTT_SERVER_VALUE, MQTT_PORT_VALUE);
}

void loop() {
  // Si se desconecta de MQTT (o es la primera vez), intentamos conectar
  if (!client.connected()) {
    reconnect();
  }
  
  // Mantiene vivo el ciclo de MQTT
  client.loop();
}