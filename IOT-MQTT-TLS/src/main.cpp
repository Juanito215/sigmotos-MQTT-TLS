#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <SHT31.h>
#include <ArduinoJson.h>
#include "secrets.h"

// --- Referenciamos funciones y objetos externos ---
extern void reconnect();
extern PubSubClient client;

// --- Configuración del sensor SHT3X ---
// SDA = GPIO 8, SCL = GPIO 9
SHT31 sht;

// --- Tópico de publicación (formato requerido por el Receiver) ---
// Formato: pais/provincia/ciudad/id-dispositivo/usuario/out
const char* MQTT_TOPIC_PUB = "colombia/valle/cali/esp32-s3/sigmotos/out";

// --- Control de tiempo para publicar cada 10 segundos ---
unsigned long lastPublish = 0;
const unsigned long PUBLISH_INTERVAL = 10000; // 10 segundos

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
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n\n--- INICIANDO ESP32 SIGMOTOS (VERSION OTA MAGICA) ---");

  // Inicializamos el bus I2C — SDA=GPIO8, SCL=GPIO9
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  Wire.begin(8, 9);

  // === ESCANER I2C REPETITIVO para detectar el sensor en vivo ===
  Serial.println("Escaneando bus I2C (se repetira hasta encontrar algo)...");
  bool found = false;
  while (!found) {
    int deviceCount = 0;
    for (byte addr = 1; addr < 127; addr++) {
      Wire.beginTransmission(addr);
      if (Wire.endTransmission() == 0) {
        Serial.print("  [!] DISPOSITIVO ENCONTRADO en 0x");
        Serial.println(addr, HEX);
        deviceCount++;
        found = true;
      }
    }
    if (!found) {
      Serial.println("  . (No hay nada... revisa los cables)");
      delay(2000);
    }
  }
  Serial.println("Escaneo finalizado con exito.");
  // ==========================================================

  // Inicializamos el sensor SHT31
  if (!sht.begin()) {
    Serial.println("[ERROR] Sensor SHT3X no encontrado al iniciar.");
  } else {
    Serial.println("Sensor SHT3X inicializado OK");
  }

  // Conectamos a WiFi
  setup_wifi();

  // Configuramos el servidor MQTT
  client.setServer(MQTT_SERVER_VALUE, MQTT_PORT_VALUE);
}

void loop() {
  // Si se desconecta de MQTT, intentamos reconectar
  if (!client.connected()) {
    reconnect();
  }

  // Mantiene vivo el ciclo de MQTT
  client.loop();

  // Publicamos datos del sensor cada PUBLISH_INTERVAL milisegundos
  unsigned long now = millis();
  if (now - lastPublish >= PUBLISH_INTERVAL) {
    lastPublish = now;

    // Leemos el sensor SHT3X
    sht.read();
    float temperatura = sht.getTemperature();
    float humedad     = sht.getHumidity();

    // Validamos que la lectura sea coherente
    if (isnan(temperatura) || isnan(humedad)) {
      Serial.println("[ERROR] No se pudo leer el sensor SHT3X. Verifica la conexion I2C.");
      return;
    }

    // Construimos el JSON: {"temperatura": 25.3, "humedad": 60.1}
    JsonDocument doc;
    doc["temperatura"] = round(temperatura * 10.0) / 10.0;
    doc["humedad"]     = round(humedad * 10.0) / 10.0;

    char payload[128];
    serializeJson(doc, payload);

    // Publicamos al broker MQTT
    if (client.publish(MQTT_TOPIC_PUB, payload)) {
      Serial.print("[MQTT] Publicado en '");
      Serial.print(MQTT_TOPIC_PUB);
      Serial.print("': ");
      Serial.println(payload);
    } else {
      Serial.println("[ERROR] Fallo al publicar en MQTT.");
    }
  }
}