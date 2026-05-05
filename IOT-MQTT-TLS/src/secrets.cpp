#include "secrets.h"
#include <cstdlib>

// Use distinct variable names to avoid macro/name collisions when
// values are provided via build-system macros (e.g. -D flags).
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

const char* WIFI_SSID_VALUE = STR(WIFI_SSID);
const char* WIFI_PASSWORD_VALUE = STR(WIFI_PASSWORD);

const char* MQTT_SERVER_VALUE = STR(MQTT_SERVER);
const int MQTT_PORT_VALUE = MQTT_PORT;

const char* MQTT_USER_VALUE = STR(MQTT_USER);
const char* MQTT_PASSWORD_VALUE = STR(MQTT_PASSWORD);

#ifndef MQTT_TOPIC_SUB
#define MQTT_TOPIC_SUB "sigmotos/in"
#endif
const char* MQTT_TOPIC_SUB_VALUE = MQTT_TOPIC_SUB;
