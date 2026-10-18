#pragma once
// Copy this file to src/secrets.h (git-ignored) and replace the placeholders.

#define WIFI_SSID "SSID"
#define WIFI_PWD "password" // leave empty if open (bad!)

#define MQTT_SERVER "192.168.1.4" // IP address of your MQTT server
#define MQTT_USERNAME "espaltherma" // dedicated broker login, not a personal account
#define MQTT_PASSWORD "CHANGE_ME"

// Password required by ArduinoOTA for network uploads. Leave undefined to disable it (bad!).
#define OTA_PASSWORD "CHANGE_ME"
