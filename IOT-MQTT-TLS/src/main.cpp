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
  Serial.println("-----------------------------------------");
  Serial.print("Conectando a WiFi: ");
  Serial.println(WIFI_SSID_VALUE);

  // Configuraciones de robustez para ESP32
  WiFi.disconnect(true); 
  delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false); // Desactivar ahorro de energía ayuda con la estabilidad en hotspots

  WiFi.begin(WIFI_SSID_VALUE, WIFI_PASSWORD_VALUE);

  int timeout_counter = 0;
  while (WiFi.status() != WL_CONNECTED && timeout_counter < 60) { // 30 segundos max
    delay(500);
    Serial.print(".");
    timeout_counter++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n[ERROR] No se pudo conectar. Posibles causas:");
    Serial.println("1. Hotspot en 5GHz (Cambia a 2.4GHz / 'Maximizar compatibilidad')");
    Serial.println("2. Contraseña incorrecta");
    Serial.println("3. Señal debil");
    Serial.println("Reiniciando ESP32...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("");
  Serial.println("WiFi conectado!");
  Serial.print("Direccion IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("-----------------------------------------");
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n\n--- INICIANDO ESP32 SIGMOTOS (VERSION OTA MAGICA) ---");

  // Inicializamos el bus I2C — SDA=GPIO 8, SCL=GPIO 9 (con pull-ups internos)
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  Wire.begin(8, 9);

  // === ESCANER I2C para detectar la dirección del sensor ===
  Serial.println("Escaneando bus I2C...");
  int deviceCount = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    byte error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("  Dispositivo encontrado en 0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
      deviceCount++;
    }
  }
  if (deviceCount == 0) {
    Serial.println("  [!] Ningun dispositivo I2C encontrado. Revisa el cableado.");
  }
  Serial.println("Escaneo finalizado.");
  // ==========================================================

  // Inicializamos el sensor SHT31 (dirección I2C 0x44)
  if (!sht.begin()) {
    Serial.println("[ERROR] Sensor SHT3X no encontrado. Verifica la conexion I2C.");
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