#include "secrets.h"
#include <cstdlib>

// Use distinct variable names to avoid macro/name collisions when
// values are provided via build-system macros (e.g. -D flags).
const char* WIFI_SSID_VALUE = WIFI_SSID_VALUE;
const char* WIFI_PASSWORD_VALUE = WIFI_PASSWORD_VALUE;

const char* MQTT_SERVER_VALUE = MQTT_SERVER_VALUE;
const char* MQTT_PORT = (MQTT_PORT);

const char* MQTT_USER_VALUE = MQTT_USER_VALUE;
const char* MQTT_PASSWORD_VALUE = MQTT_PASSWORD_VALUE;

#ifndef MQTT_TOPIC_SUB
#define MQTT_TOPIC_SUB "sigmotos/in"
#endif
const char* MQTT_TOPIC_SUB_VALUE = MQTT_TOPIC_SUB;
