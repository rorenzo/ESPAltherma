#pragma once

// External custom sensors configuration.
// Keep this file separate from upstream setup files to simplify future merges.

// 1-Wire temperature probes (DS18B20) on a shared bus.
// Requires OneWire + DallasTemperature (env:esp32-ds18b20 in platformio.ini).
// Probe 1: DHW temperature at the gas boiler outlet.
#define CUSTOM_TEMP_SENSOR_BUS_PIN 19

// MQTT/JSON field names. Keep letters/numbers/underscore for easier integrations.
#define CUSTOM_MQTT_TEMP_COUNT_KEY "ExtTempProbeCount"
#define CUSTOM_MQTT_TEMP1_KEY "ExtTemp1"
#define CUSTOM_MQTT_TEMP2_KEY "ExtTemp2"

// Enable the optional second probe only when it is actually installed.
#define CUSTOM_TEMP2_ENABLED 0

// DS18B20 mapping mode:
// - Default uses the index on the 1-Wire bus (0 and 1).
// - Set ROM IDs for a stable mapping (recommended): the boot log on
//   espaltherma/log prints "DS18B20 index N ROM ..." for every probe found.
#define CUSTOM_TEMP1_INDEX 0
#define CUSTOM_TEMP2_INDEX 1

// Optional ROM IDs as 16 hex chars (no spaces, no separators), example:
// #define CUSTOM_TEMP1_ROM "28FF64A2B31604B3"
#define CUSTOM_TEMP1_ROM "288CA3190000005F"
#define CUSTOM_TEMP2_ROM ""
