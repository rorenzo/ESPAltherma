#include "custom_sensors.h"
#include "custom_sensors_config.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#if defined(CUSTOM_TEMP_SENSOR_BUS_PIN) && __has_include(<OneWire.h>) && __has_include(<DallasTemperature.h>)
#include <OneWire.h>
#include <DallasTemperature.h>
#define CUSTOM_TEMP_SENSORS_READY
#endif

#ifndef CUSTOM_MQTT_RAIN_DETECTED_KEY
#define CUSTOM_MQTT_RAIN_DETECTED_KEY "RainDetected"
#endif
#ifndef CUSTOM_MQTT_RAIN_STATE_KEY
#define CUSTOM_MQTT_RAIN_STATE_KEY "RainState"
#endif
#ifndef CUSTOM_MQTT_TEMP_COUNT_KEY
#define CUSTOM_MQTT_TEMP_COUNT_KEY "ExtTempProbeCount"
#endif
#ifndef CUSTOM_MQTT_TEMP1_KEY
#define CUSTOM_MQTT_TEMP1_KEY "ExtTemp1"
#endif
#ifndef CUSTOM_MQTT_TEMP2_KEY
#define CUSTOM_MQTT_TEMP2_KEY "ExtTemp2"
#endif
#ifndef CUSTOM_TEMP1_INDEX
#define CUSTOM_TEMP1_INDEX 0
#endif
#ifndef CUSTOM_TEMP2_INDEX
#define CUSTOM_TEMP2_INDEX 1
#endif
#ifndef CUSTOM_TEMP1_ROM
#define CUSTOM_TEMP1_ROM ""
#endif
#ifndef CUSTOM_TEMP2_ROM
#define CUSTOM_TEMP2_ROM ""
#endif

namespace
{
Stream *s_log = &Serial;

void logLine(const char *fmt, ...)
{
  if (s_log == nullptr)
  {
    return;
  }

  char line[128];
  va_list args;
  va_start(args, fmt);
  vsnprintf(line, sizeof(line), fmt, args);
  va_end(args);
  s_log->println(line);
}

void appendJson(char *jsonBuffer, size_t jsonBufferSize, const char *fmt, ...)
{
  if (jsonBuffer == nullptr || jsonBufferSize == 0)
  {
    return;
  }

  size_t len = strlen(jsonBuffer);
  if (len >= jsonBufferSize - 1)
  {
    return;
  }

  va_list args;
  va_start(args, fmt);
  vsnprintf(jsonBuffer + len, jsonBufferSize - len, fmt, args);
  va_end(args);
}

#ifdef CUSTOM_RAIN_SENSOR_PIN
int s_rainRaw = HIGH;
bool s_rainDetected = false;
#endif

#ifdef CUSTOM_TEMP_SENSORS_READY
OneWire s_oneWire(CUSTOM_TEMP_SENSOR_BUS_PIN);
DallasTemperature s_tempSensors(&s_oneWire);
float s_tempProbe1 = NAN;
float s_tempProbe2 = NAN;
uint8_t s_probeCount = 0;
DeviceAddress s_temp1Address = {0};
DeviceAddress s_temp2Address = {0};
bool s_temp1UseAddress = false;
bool s_temp2UseAddress = false;

bool isValidTemp(float tempC)
{
  return tempC != DEVICE_DISCONNECTED_C && tempC > -55.1f && tempC < 125.1f;
}

void addressToHex(const DeviceAddress address, char *out, size_t outSize)
{
  if (out == nullptr || outSize < 17)
  {
    return;
  }

  out[0] = '\0';
  for (uint8_t i = 0; i < 8; ++i)
  {
    snprintf(out + (i * 2), outSize - (i * 2), "%02X", address[i]);
  }
}

bool parseHexByte(char high, char low, uint8_t &out)
{
  if (!isxdigit(static_cast<unsigned char>(high)) || !isxdigit(static_cast<unsigned char>(low)))
  {
    return false;
  }

  char buff[3] = {static_cast<char>(toupper(static_cast<unsigned char>(high))), static_cast<char>(toupper(static_cast<unsigned char>(low))), '\0'};
  out = static_cast<uint8_t>(strtoul(buff, nullptr, 16));
  return true;
}

bool parseRomString(const char *romString, DeviceAddress outAddress)
{
  if (romString == nullptr || outAddress == nullptr)
  {
    return false;
  }

  if (strlen(romString) != 16)
  {
    return false;
  }

  for (uint8_t i = 0; i < 8; ++i)
  {
    uint8_t parsed = 0;
    if (!parseHexByte(romString[i * 2], romString[i * 2 + 1], parsed))
    {
      return false;
    }
    outAddress[i] = parsed;
  }

  return true;
}

bool hasRomString(const char *romString)
{
  return romString != nullptr && romString[0] != '\0';
}
#endif
} // namespace

void customSensorsSetup(Stream &logStream)
{
  s_log = &logStream;

#ifdef CUSTOM_RAIN_SENSOR_PIN
  pinMode(CUSTOM_RAIN_SENSOR_PIN, CUSTOM_RAIN_SENSOR_INPUT_MODE);
  s_rainRaw = digitalRead(CUSTOM_RAIN_SENSOR_PIN);
  s_rainDetected = (s_rainRaw == CUSTOM_RAIN_SENSOR_ACTIVE_STATE);
  logLine("Custom rain sensor enabled on GPIO %d", CUSTOM_RAIN_SENSOR_PIN);
#else
  logLine("Custom rain sensor disabled");
#endif

#ifdef CUSTOM_TEMP_SENSORS_READY
  s_tempSensors.begin();
  s_probeCount = s_tempSensors.getDeviceCount();
  logLine("Custom temp probes enabled on GPIO %d, found %u", CUSTOM_TEMP_SENSOR_BUS_PIN, static_cast<unsigned>(s_probeCount));

  for (uint8_t i = 0; i < s_probeCount; ++i)
  {
    DeviceAddress address = {0};
    if (s_tempSensors.getAddress(address, i))
    {
      char romHex[17] = {0};
      addressToHex(address, romHex, sizeof(romHex));
      logLine("DS18B20 index %u ROM %s", static_cast<unsigned>(i), romHex);
    }
  }

  if (hasRomString(CUSTOM_TEMP1_ROM))
  {
    s_temp1UseAddress = parseRomString(CUSTOM_TEMP1_ROM, s_temp1Address);
    if (!s_temp1UseAddress)
    {
      logLine("Invalid CUSTOM_TEMP1_ROM format, using index %d", CUSTOM_TEMP1_INDEX);
    }
    else if (!s_tempSensors.isConnected(s_temp1Address))
    {
      logLine("CUSTOM_TEMP1_ROM not found on bus");
    }
  }

  if (hasRomString(CUSTOM_TEMP2_ROM))
  {
    s_temp2UseAddress = parseRomString(CUSTOM_TEMP2_ROM, s_temp2Address);
    if (!s_temp2UseAddress)
    {
      logLine("Invalid CUSTOM_TEMP2_ROM format, using index %d", CUSTOM_TEMP2_INDEX);
    }
    else if (!s_tempSensors.isConnected(s_temp2Address))
    {
      logLine("CUSTOM_TEMP2_ROM not found on bus");
    }
  }
#elif defined(CUSTOM_TEMP_SENSOR_BUS_PIN)
  logLine("Custom temp probes requested but OneWire/DallasTemperature libs are missing");
#else
  logLine("Custom temp probes disabled");
#endif
}

void customSensorsRead()
{
#ifdef CUSTOM_RAIN_SENSOR_PIN
  s_rainRaw = digitalRead(CUSTOM_RAIN_SENSOR_PIN);
  s_rainDetected = (s_rainRaw == CUSTOM_RAIN_SENSOR_ACTIVE_STATE);
#endif

#ifdef CUSTOM_TEMP_SENSORS_READY
  s_tempSensors.requestTemperatures();
  s_probeCount = s_tempSensors.getDeviceCount();
  s_tempProbe1 = s_temp1UseAddress ? s_tempSensors.getTempC(s_temp1Address) : s_tempSensors.getTempCByIndex(CUSTOM_TEMP1_INDEX);
  s_tempProbe2 = s_temp2UseAddress ? s_tempSensors.getTempC(s_temp2Address) : s_tempSensors.getTempCByIndex(CUSTOM_TEMP2_INDEX);
#endif
}

void customSensorsAppendToJson(char *jsonBuffer, size_t jsonBufferSize)
{
#ifdef CUSTOM_RAIN_SENSOR_PIN
  appendJson(jsonBuffer, jsonBufferSize, "\"%s\":%d,", CUSTOM_MQTT_RAIN_DETECTED_KEY, s_rainDetected ? 1 : 0);
  appendJson(jsonBuffer, jsonBufferSize, "\"%s\":\"%s\",", CUSTOM_MQTT_RAIN_STATE_KEY, s_rainDetected ? "WET" : "DRY");
#endif

#ifdef CUSTOM_TEMP_SENSORS_READY
  appendJson(jsonBuffer, jsonBufferSize, "\"%s\":%u,", CUSTOM_MQTT_TEMP_COUNT_KEY, static_cast<unsigned>(s_probeCount));
  if (isValidTemp(s_tempProbe1))
  {
    appendJson(jsonBuffer, jsonBufferSize, "\"%s\":%.2f,", CUSTOM_MQTT_TEMP1_KEY, s_tempProbe1);
  }
  else
  {
    appendJson(jsonBuffer, jsonBufferSize, "\"%s\":\"NA\",", CUSTOM_MQTT_TEMP1_KEY);
  }

  if (isValidTemp(s_tempProbe2))
  {
    appendJson(jsonBuffer, jsonBufferSize, "\"%s\":%.2f,", CUSTOM_MQTT_TEMP2_KEY, s_tempProbe2);
  }
  else
  {
    appendJson(jsonBuffer, jsonBufferSize, "\"%s\":\"NA\",", CUSTOM_MQTT_TEMP2_KEY);
  }
#elif defined(CUSTOM_TEMP_SENSOR_BUS_PIN)
  appendJson(jsonBuffer, jsonBufferSize, "\"%s\":\"LIB_MISSING\",", CUSTOM_MQTT_TEMP1_KEY);
  appendJson(jsonBuffer, jsonBufferSize, "\"%s\":\"LIB_MISSING\",", CUSTOM_MQTT_TEMP2_KEY);
#endif
}
