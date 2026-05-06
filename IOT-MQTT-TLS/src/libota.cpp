#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPUpdate.h>
#include "libota.h"

void performOTA(String url) {
    Serial.println("\n--- INICIANDO ACTUALIZACION OTA ---");
    Serial.println("Descargando firmware desde: " + url);

    // Creamos un cliente HTTPS que no verifica el certificado del servidor
    // (Porque AWS S3 usa un certificado de Amazon, no el de Let's Encrypt de EMQX)
    WiFiClientSecure otaClient;
    otaClient.setInsecure(); 

    // Iniciamos la actualización
    t_httpUpdate_return ret = httpUpdate.update(otaClient, url);

    // Revisamos el resultado
    switch (ret) {
        case HTTP_UPDATE_FAILED:
            Serial.printf("Error en OTA (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
            break;
        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("No hay actualizaciones disponibles.");
            break;
        case HTTP_UPDATE_OK:
            Serial.println("¡Actualizacion OTA exitosa!");
            Serial.println("Reiniciando placa...");
            ESP.restart();
            break;
    }
}
