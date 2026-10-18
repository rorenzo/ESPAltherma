# Fork workflow: stay aligned with upstream without losing the customizations

## Branch model

| Branch | Content | Rule |
|---|---|---|
| `main` | exact mirror of `upstream/main` | only fast-forward from upstream, no commits of ours |
| `custom/hybrid-ds18b20` | upstream + fork customizations | the branch that is built and flashed |

Remotes:

- `origin`: the fork (`rorenzo/ESPAltherma`)
- `upstream`: the original project (`raomin/ESPAltherma`)

GitHub forks of a public repository are public: never commit credentials.

## Fork customizations

Keep them in separate files, with minimal hooks in upstream files:

| File | Purpose |
|---|---|
| `src/secrets.h` (git-ignored) | WiFi, MQTT, OTA credentials; template in `src/secrets.example.h` |
| `include/def/Fork_Hybrid_gas_boiler.h` | Hybrid profile without DHW (entries copied from `ALTHERMA(HYBRID).h`) |
| `include/custom_sensors*.h`, `src/custom_sensors.cpp` | DS18B20 probes, see `doc/custom-ds18b20.md` |
| `contrib/compare_attr_snapshots.py` | compares two `espaltherma/ATTR` snapshots |

Hooks in upstream files:

- `src/setup.h`: includes `secrets.h` and the fork profile
- `src/main.cpp`: three `customSensors*` calls and `ArduinoOTA.setPassword`
- `platformio.ini`: environments `esp32-ds18b20` and `esp32-ds18b20-ota`
- `.gitignore`: `src/secrets.h`

These are the only places where an upstream merge can conflict.

## First setup on a new clone

```bash
git remote add upstream https://github.com/raomin/ESPAltherma.git
git fetch --all --prune
git switch custom/hybrid-ds18b20
cp src/secrets.example.h src/secrets.h   # then fill in the values
git config core.autocrlf input           # Linux/macOS: keep LF line endings
```

## Periodic update from upstream

```bash
git fetch upstream --prune

# 1. main follows upstream
git switch main
git merge --ff-only upstream/main
git push origin main

# 2. bring the updates into the custom branch
git switch custom/hybrid-ds18b20
git merge main
```

If there are conflicts, they can only be in the hook files listed above.
Resolve them keeping both upstream changes and our hooks, then:

```bash
git add <resolved files>
git commit
```

Before pushing, check what differs from upstream, build, and flash:

```bash
git diff --stat main..custom/hybrid-ds18b20   # only fork files and hooks
pio run -e esp32-ds18b20
git push origin custom/hybrid-ds18b20
```

## Upload

- `esp32-ds18b20`: USB serial upload.
- `esp32-ds18b20-ota`: network upload to `ESPAltherma.local`. The OTA password
  defined in `src/secrets.h` must also be exported before the upload:

```bash
export ESPALTHERMA_OTA_PASSWORD='...'
pio run -e esp32-ds18b20-ota -t upload
```

Before flashing a new version, take a snapshot of `espaltherma/ATTR`, and
take another one afterwards: `contrib/compare_attr_snapshots.py` lists the
values added, removed, or changed.
