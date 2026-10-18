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
| `deploy.sh` | build, USB flash, OTA update |

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
./deploy.sh build
git push origin custom/hybrid-ds18b20
```

## Build and upload (Fedora workstation)

`deploy.sh` wraps build, USB flash and OTA update. Full guide (Italian): `deploy.md`.

```bash
./deploy.sh setup     # once: PlatformIO in .venv-pio, checks the dialout group
./deploy.sh check     # src/secrets.h complete, tools, Git state
./deploy.sh build     # compile env esp32-ds18b20
./deploy.sh test      # upstream unit tests (env native)
./deploy.sh usb       # first flash or recovery, USB cable
./deploy.sh ota       # network update to ESPAltherma.local (or: ./deploy.sh ota <ip>)
./deploy.sh monitor   # serial monitor
```

`ota` reads `OTA_PASSWORD` from `src/secrets.h`, checks that the device answers,
and checks that firewalld lets the ESP connect back on TCP 40000 (the Fedora
Workstation zone already allows ports 1025-65535). Run it from the workstation,
not from a container: the ESP must reach the host during the upload.

When changing the OTA password, upload the new firmware with the old one:
`ESPALTHERMA_OTA_PASSWORD_CURRENT='<old>' ./deploy.sh ota`.

Before flashing a new version, take a snapshot of `espaltherma/ATTR`, and
take another one afterwards: `contrib/compare_attr_snapshots.py` lists the
values added, removed, or changed.
