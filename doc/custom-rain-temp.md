# Custom Rain + Temp Sensors

This project can publish external sensors together with standard `espaltherma/ATTR` payload.

## Added fields in MQTT JSON

- `RainDetected` (`0` or `1`)
- `RainState` (`"DRY"` or `"WET"`)
- `ExtTempProbeCount` (number of detected DS18B20 probes)
- `ExtTemp1` (temperature in `degC` or `"NA"`)
- `ExtTemp2` (temperature in `degC` or `"NA"`)

## Configuration

Use file `include/custom_sensors_config.h`:

- `CUSTOM_RAIN_SENSOR_PIN` default `27`
- `CUSTOM_RAIN_SENSOR_ACTIVE_STATE` default `LOW`
- `CUSTOM_RAIN_SENSOR_INPUT_MODE` default `INPUT_PULLUP`
- `CUSTOM_TEMP_SENSOR_BUS_PIN` default `25`
- `CUSTOM_MQTT_*_KEY` to rename JSON fields published to MQTT
- `CUSTOM_TEMP1_INDEX` / `CUSTOM_TEMP2_INDEX` for index-based mapping
- `CUSTOM_TEMP1_ROM` / `CUSTOM_TEMP2_ROM` for fixed DS18B20 mapping by ROM ID (recommended)

## Wiring (AZDelivery ESP32 DevKit)

- Rain on/off contact:
  - one wire to `GND`
  - one wire to `GPIO27`
  - keep `INPUT_PULLUP` in config
- DS18B20 probes:
  - both data lines on `GPIO25` (same 1-Wire bus)
  - add `4.7k` pull-up resistor between data (`GPIO25`) and `3.3V`
  - probe power: `3.3V` and `GND`
  - each DS18B20 has a unique ROM ID, so two probes on one pin are read separately

## Stable mapping for 2 temperatures

By default temperatures are read by bus index (`0` and `1`).
For stable mapping, set ROM IDs in config:

```cpp
#define CUSTOM_TEMP1_ROM "28FF64A2B31604B3"
#define CUSTOM_TEMP2_ROM "28FF7A5CB31603A1"
```

On boot, ESP logs detected ROM IDs on `espaltherma/log`:

- `DS18B20 index 0 ROM ...`
- `DS18B20 index 1 ROM ...`

## Build environment

Use dedicated PlatformIO environment to keep upstream defaults untouched:

- `esp32-rain-temp`

It adds:

- `OneWire`
- `DallasTemperature`
