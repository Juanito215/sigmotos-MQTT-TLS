#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include "secrets.h"

WiFiClientSecure espClient;
PubSubClient client(espClient);

static String getMacAddress() {
    uint64_t mac = ESP.getEfuseMac();
    char buf[13];
    snprintf(buf, sizeof(buf), "%02X%02X%02X%02X%02X%02X",
             (uint8_t)(mac >> 40),
             (uint8_t)(mac >> 32),
             (uint8_t)(mac >> 24),
             (uint8_t)(mac >> 16),
             (uint8_t)(mac >> 8),
             (uint8_t)(mac));
    return String("ESP32-") + buf;
}

#include <ArduinoJson.h>
#include "libota.h"

// Función que se ejecuta cuando llega un mensaje de MQTT
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Mensaje recibido en topico: ");
    Serial.println(topic);

    // Convertimos el payload a String
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.println("Contenido: " + message);

    // Parseamos el JSON para buscar la URL del OTA
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
        Serial.print("Fallo al leer JSON: ");
        Serial.println(error.c_str());
        return;
    }

    // Extraemos la URL de descarga
    const char* url = doc["url"];
    if (url) {
        // Llamamos a la función de OTA
        performOTA(String(url));
    }
}

void reconnect() {
    client.setCallback(mqttCallback);
    while (!client.connected()) {

        // PASO 1: Configurar el certificado raíz
        // Esto le dice al ESP32 qué autoridad certificadora confiar
        espClient.setCACert(root_ca); // se necesita el certificado raíz del broker MQTT para validar su identidad

        // PASO 2: Intentar la conexión MQTT
        // El cliente_id identifica a este dispositivo en el broker
        String client_id = getMacAddress();  // "ESP32-A1B2C3D4E5F6"

        if (client.connect(client_id.c_str(), MQTT_USER_VALUE, MQTT_PASSWORD_VALUE)) {
            // PASO 3: Conexión exitosa — suscribirse al tópico de entrada
            client.subscribe(MQTT_TOPIC_SUB_VALUE, 1);  // QoS 1

            Serial.println("Conectado al broker MQTT con TLS ✓");
        } else {
            // PASO 4: Conexión fallida
            int state = client.state();

            if (state == -4) {
                Serial.println("Timeout — broker no responde");
            } else if (state == 5) {
                // Credenciales incorrectas — dormir 5 minutos
                // para no saturar el broker con intentos fallidos
                Serial.println("Credenciales inválidas — deep sleep");
                esp_deep_sleep(300 * 1000000ULL);  // 5 minutos en microsegundos
            }

            delay(5000);  // esperar 5 segundos antes de reintentar
        }
    }
}
