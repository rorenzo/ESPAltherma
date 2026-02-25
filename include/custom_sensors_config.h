#pragma once

// External custom sensors configuration.
// Keep this file separate from upstream setup files to simplify future pulls.

// Rain sensor (digital on/off)
// Typical dry-contact wiring: one side to GND, the other to GPIO below.
// With INPUT_PULLUP, GPIO reads LOW when contact closes.
#define CUSTOM_RAIN_SENSOR_PIN 27
#define CUSTOM_RAIN_SENSOR_ACTIVE_STATE LOW
#define CUSTOM_RAIN_SENSOR_INPUT_MODE INPUT_PULLUP

// 1-Wire temperature probes (DS18B20) on shared bus.
// Requires OneWire + DallasTemperature libraries.
#define CUSTOM_TEMP_SENSOR_BUS_PIN 25

// MQTT/JSON field names (customizable).
// Keep simple names (letters/numbers/underscore) for easier integrations.
#define CUSTOM_MQTT_RAIN_DETECTED_KEY "RainDetected"
#define CUSTOM_MQTT_RAIN_STATE_KEY "RainState"
#define CUSTOM_MQTT_TEMP_COUNT_KEY "ExtTempProbeCount"
#define CUSTOM_MQTT_TEMP1_KEY "ExtTemp1"
#define CUSTOM_MQTT_TEMP2_KEY "ExtTemp2"

// DS18B20 mapping mode:
// - Default uses index on the 1-Wire bus (0 and 1).
// - Optional: set ROM IDs for stable mapping (recommended in production).
#define CUSTOM_TEMP1_INDEX 0
#define CUSTOM_TEMP2_INDEX 1

// Optional ROM IDs as 16 hex chars (no spaces, no separators), example:
// #define CUSTOM_TEMP1_ROM "28FF64A2B31604B3"
// #define CUSTOM_TEMP2_ROM "28FF7A5CB31603A1"
#define CUSTOM_TEMP1_ROM ""
#define CUSTOM_TEMP2_ROM ""
