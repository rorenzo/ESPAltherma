# Custom DS18B20 temperature probes

The fork publishes up to two DS18B20 probes in the standard `espaltherma/ATTR`
payload. Probe 1 measures the DHW temperature at the gas boiler outlet.

## Added fields in MQTT JSON

- `ExtTempProbeCount`: number of DS18B20 probes found on the bus
- `ExtTemp1`: temperature in °C, or `"NA"` when the reading is not valid
- `ExtTemp2`: temperature in °C, or `"NA"` when the reading is not valid

A reading is not valid when the probe is disconnected (-127 °C), outside the
DS18B20 range, or exactly 85 °C (the power-on value returned when a conversion
did not complete). Consumers must treat `"NA"` as unavailable.

These fields are not part of the upstream Home Assistant device discovery:
Home Assistant reads them as attributes of `sensor.althermasensors`.

## Configuration

File `include/custom_sensors_config.h`:

- `CUSTOM_TEMP_SENSOR_BUS_PIN`, default `25`
- `CUSTOM_MQTT_*_KEY` to rename the JSON fields
- `CUSTOM_TEMP2_ENABLED`, set to `0` to omit the second probe from readings and MQTT
- `CUSTOM_TEMP1_INDEX` / `CUSTOM_TEMP2_INDEX` for index-based mapping
- `CUSTOM_TEMP1_ROM` / `CUSTOM_TEMP2_ROM` for a fixed mapping by ROM ID (recommended)

## Wiring (AZDelivery ESP32 DevKit)

- data line of every probe on `GPIO25` (shared 1-Wire bus)
- `4.7k` pull-up resistor between data (`GPIO25`) and `3.3V`
- probe power: `3.3V` and `GND`

Each DS18B20 has a unique ROM ID, so several probes on one pin are read separately.

## Stable mapping

By default probes are read by bus index (`0` and `1`), which can change when a
probe is added or replaced. On boot the ESP logs the detected ROM IDs on
`espaltherma/log`:

- `DS18B20 index 0 ROM ...`

Copy the ROM ID into the configuration:

```cpp
#define CUSTOM_TEMP1_ROM "28FF64A2B31604B3"
```

## Build environment

Use the PlatformIO environment `esp32-ds18b20`. It extends `esp32` and adds
`OneWire` and `DallasTemperature`. Without these libraries the firmware still
builds and publishes `"LIB_MISSING"` for both temperatures.
