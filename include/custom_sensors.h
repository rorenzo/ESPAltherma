#pragma once

#include <Arduino.h>
#include <stddef.h>

void customSensorsSetup(Stream &logStream);
void customSensorsRead();
void customSensorsAppendToJson(char *jsonBuffer, size_t jsonBufferSize);

