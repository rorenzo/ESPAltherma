# Deploy del firmware ESPAltherma (fork)

Questa guida spiega come compilare il firmware del fork e caricarlo sull'ESP32
collegato alla pompa di calore, da una workstation Fedora. Tutti i passaggi
passano da `deploy.sh`, nella radice del repository.

Il firmware si carica in due modi:

- **USB**: con il cavo collegato all'ESP32. Serve per il primo caricamento e per
  il recupero, quando l'ESP non si collega più alla rete.
- **OTA**: via Wi-Fi, senza smontare nulla. È il modo normale per gli
  aggiornamenti.

## Comandi

| Comando | Cosa fa |
|---|---|
| `./deploy.sh setup` | installa PlatformIO in `.venv-pio` e controlla i permessi USB |
| `./deploy.sh check` | controlla `src/secrets.h`, PlatformIO e lo stato Git |
| `./deploy.sh build` | compila il firmware (ambiente `esp32-ds18b20`) |
| `./deploy.sh test` | esegue i test unitari del progetto originale |
| `./deploy.sh usb [porta]` | compila e carica via cavo USB |
| `./deploy.sh ota [host o ip]` | compila e carica via rete (default `ESPAltherma.local`) |
| `./deploy.sh monitor [porta]` | mostra i messaggi seriali dell'ESP32 |

`./deploy.sh help` mostra lo stesso elenco.

## Preparazione (una volta sola)

### 1. Repository

```bash
git clone git@github.com:rorenzo/ESPAltherma.git
cd ESPAltherma
git remote add upstream https://github.com/raomin/ESPAltherma.git
git switch custom/hybrid-ds18b20
```

Il branch da compilare è sempre `custom/hybrid-ds18b20`. `main` è solo una copia
del progetto originale. Come tenerlo aggiornato è spiegato in
`doc/fork-sync-workflow.md`.

### 2. Credenziali: `src/secrets.h`

Il fork è pubblico, quindi le credenziali stanno in `src/secrets.h`, un file
escluso da Git. Si parte dal modello:

```bash
cp src/secrets.example.h src/secrets.h
chmod 600 src/secrets.h
```

e si compilano tutti i valori:

| Valore | Contenuto |
|---|---|
| `WIFI_SSID`, `WIFI_PWD` | rete Wi-Fi a cui si collega l'ESP32 |
| `MQTT_SERVER` | IP di Home Assistant (broker Mosquitto) |
| `MQTT_USERNAME`, `MQTT_PASSWORD` | login dedicato `espaltherma` in Mosquitto |
| `OTA_PASSWORD` | password chiesta dall'ESP32 per gli aggiornamenti via rete |

Se il file esiste già su un altro computer, copiarlo invece di ricrearlo.
`./deploy.sh check` segnala valori vuoti o segnaposto `CHANGE_ME` rimasti.
Senza il file la compilazione si ferma con un messaggio esplicito.

La password MQTT deve coincidere con quella del login in Mosquitto. In
`/config/secrets.yaml` di Home Assistant:

```yaml
mqtt_espaltherma_username: "espaltherma"
mqtt_espaltherma_password: "<stesso valore di MQTT_PASSWORD>"
```

e nella configurazione dell'app **Mosquitto broker**, tra i `logins`:

```yaml
  - username: '!secret mqtt_espaltherma_username'
    password: '!secret mqtt_espaltherma_password'
```

Dopo la modifica: `ha supervisor reload`, poi salvare e riavviare Mosquitto.

### 3. PlatformIO

```bash
./deploy.sh setup
```

Crea l'ambiente Python `.venv-pio` (escluso da Git) e ci installa PlatformIO.
La prima compilazione scarica la toolchain ESP32 e richiede alcuni minuti.

### 4. Permessi USB (solo per il caricamento via cavo)

Su Fedora la porta seriale (`/dev/ttyUSB0`) è accessibile al gruppo `dialout`:

```bash
sudo usermod -aG dialout $USER
```

poi uscire e rientrare dalla sessione. `./deploy.sh setup` segnala se manca.

## Aggiornamento via rete (OTA)

```bash
./deploy.sh ota
```

Lo script:

1. controlla `src/secrets.h` e avvisa se ci sono modifiche non committate (il
   firmware non corrisponderebbe a un commit preciso);
2. trova l'indirizzo di `ESPAltherma.local` e verifica che l'ESP32 risponda;
3. controlla che firewalld lasci entrare la connessione di ritorno dell'ESP32
   sulla porta **40000/tcp**: durante l'upload è l'ESP32 che si collega alla
   workstation. Nella zona standard di Fedora Workstation la porta è già aperta.
   Se è chiusa, lo script propone il comando per aprirla fino al prossimo riavvio;
4. compila e invia il firmware con la password OTA di `src/secrets.h`;
5. aspetta che l'ESP32 si riavvii e torni raggiungibile.

Se il nome `.local` non viene risolto, usare l'IP: `./deploy.sh ota 192.168.5.x`.

Lanciare il comando dalla workstation, non da un container o da una VM in NAT:
l'ESP32 deve riuscire a raggiungere il computer.

### Primo aggiornamento dal firmware precedente

Il firmware installato prima del riallineamento non ha password OTA: il primo
`./deploy.sh ota` funziona senza fare altro. Dopo, l'ESP32 accetta solo la
password di `src/secrets.h`.

Prima di questo aggiornamento il login `espaltherma` deve già esistere in
Mosquitto. Se manca, l'ESP32 si collega al Wi-Fi ma non a MQTT. L'OTA resta
comunque disponibile per correggere.

## Caricamento via USB

```bash
./deploy.sh usb            # porta rilevata automaticamente
./deploy.sh usb /dev/ttyUSB0
./deploy.sh monitor        # per vedere l'avvio
```

Serve quando l'ESP32 non è raggiungibile in rete, per esempio dopo un errore
in `WIFI_SSID`/`WIFI_PWD`, oppure su una scheda nuova.

## Verifica dei dati prima e dopo l'aggiornamento

Per essere sicuri di ricevere gli stessi valori di prima, si salva lo stato del
sensore `sensor.althermasensors` prima e dopo il caricamento e si confrontano.
Dall'app **Terminal & SSH** di Home Assistant:

```sh
# prima dell'aggiornamento
curl -s -H "Authorization: Bearer $SUPERVISOR_TOKEN" \
  http://supervisor/core/api/states/sensor.althermasensors > /config/espaltherma_before.json

# dopo l'aggiornamento, attesi almeno due cicli di lettura (circa un minuto)
curl -s -H "Authorization: Bearer $SUPERVISOR_TOKEN" \
  http://supervisor/core/api/states/sensor.althermasensors > /config/espaltherma_after.json

curl -sL -o /config/compare_attr_snapshots.py \
  https://raw.githubusercontent.com/rorenzo/ESPAltherma/custom/hybrid-ds18b20/contrib/compare_attr_snapshots.py
python3 /config/compare_attr_snapshots.py /config/espaltherma_before.json /config/espaltherma_after.json
```

Lo script elenca:

- `REMOVED`: valori che non arrivano più. Se sono attesi, si dichiarano con
  `--expect-removed 'nome'`. Ogni `REMOVED (unexpected)` va capito prima di
  proseguire;
- `ADDED`: valori nuovi;
- `TYPE CHANGED`: un valore che era un numero ed è diventato testo, o il
  contrario;
- i valori cambiati, affiancati, per individuare unità o scale sbagliate.

Per il primo aggiornamento dopo il riallineamento, le rimozioni attese sono i
valori del bollitore ACS:

```sh
python3 /config/compare_attr_snapshots.py /config/espaltherma_before.json /config/espaltherma_after.json \
  --expect-removed 'DHW tank temp. (R5T)' --expect-removed 'DHW setpoint' \
  --expect-removed '3way valve(On:DHW_Off:Space)'
```

## Dopo il primo avvio: sonda ACS

All'avvio l'ESP32 pubblica su `espaltherma/log` le sonde trovate:

```text
DS18B20 index 0 ROM 28FF64A2B31604B3
```

Copiare il codice in `include/custom_sensors_config.h`:

```cpp
#define CUSTOM_TEMP1_ROM "28FF64A2B31604B3"
```

committare e ricaricare con `./deploy.sh ota`. Così `ExtTemp1` resta legata a
quella sonda anche se in futuro se ne aggiunge un'altra. In Home Assistant il
valore è il sensore **Altherma ACS Uscita Caldaia**.

## Cambio delle password

| Password | Procedura |
|---|---|
| MQTT | cambiare `mqtt_espaltherma_password` in Home Assistant e `MQTT_PASSWORD` in `src/secrets.h`, riavviare Mosquitto, poi `./deploy.sh ota` |
| OTA | cambiare `OTA_PASSWORD`, poi caricare usando la vecchia: `ESPALTHERMA_OTA_PASSWORD_CURRENT='<vecchia>' ./deploy.sh ota` |
| Wi-Fi | cambiare `WIFI_PWD` e caricare con `./deploy.sh ota` **prima** di cambiare la password sul router. L'ESP32 resta scollegato finché il router non usa la nuova password, poi si ricollega da solo |

Un errore di battitura in `WIFI_SSID` o `WIFI_PWD` rende l'ESP32 irraggiungibile
via rete: si recupera solo con `./deploy.sh usb`.

## Problemi comuni

| Messaggio | Causa e soluzione |
|---|---|
| `impossibile risolvere ESPAltherma.local` | mDNS non disponibile: usare l'IP, oppure `sudo dnf install nss-mdns` |
| `non risponde al ping` | ESP32 spento, IP sbagliato o non collegato al Wi-Fi: controllare con `./deploy.sh usb` + `monitor` |
| `No response from the ESP` | l'ESP32 non risponde all'invito OTA sulla porta UDP 3232: firmware senza OTA o dispositivo in riavvio, riprovare dopo un minuto |
| `Authenticating...FAIL` | password OTA diversa da quella sul dispositivo: vedi cambio password OTA |
| `Waiting for device...` seguito da `No response from device` | password accettata, ma l'ESP32 non riesce a collegarsi alla workstation: firewalld blocca la porta 40000/tcp (aprirla come proposto dallo script) oppure il comando gira in un container/VM |
| `non è nel gruppo dialout` | vedi Permessi USB |
| `Missing src/secrets.h` | vedi Credenziali |
| `il branch atteso è custom/hybrid-ds18b20` | si sta compilando un altro branch: passare a quello giusto |
