#!/usr/bin/env bash
# Build, USB flash and OTA update of the ESPAltherma fork from a Fedora workstation.
# Usage: ./deploy.sh help
set -euo pipefail

cd "$(dirname "$(readlink -f "$0")")"

ENV_USB=esp32-ds18b20
ENV_OTA=esp32-ds18b20-ota
BRANCH=custom/hybrid-ds18b20
SECRETS=src/secrets.h
VENV=.venv-pio
OTA_HOST=${ESPALTHERMA_HOST:-ESPAltherma.local}
OTA_HOST_PORT=40000 # must match --host_port in platformio.ini [env:esp32-ds18b20-ota]
MONITOR_SPEED=115200

die() { echo "ERRORE: $*" >&2; exit 1; }
info() { echo "==> $*"; }
warn() { echo "ATTENZIONE: $*" >&2; }

usage() {
  cat <<EOF
Uso: ./deploy.sh <comando> [argomento]

  setup            installa PlatformIO in $VENV e verifica i permessi USB
  check            controlla $SECRETS, strumenti e stato Git
  build            compila il firmware ($ENV_USB)
  test             esegue i test unitari (env native)
  usb [porta]      compila e carica via cavo USB (primo caricamento o recupero)
  ota [host|ip]    compila e aggiorna via rete (default: $OTA_HOST)
  monitor [porta]  apre il monitor seriale a $MONITOR_SPEED baud

Variabili d'ambiente:
  ESPALTHERMA_HOST                 host o IP per l'OTA (alternativa all'argomento)
  ESPALTHERMA_OTA_PASSWORD_CURRENT password OTA attualmente sul dispositivo, se
                                   diversa da OTA_PASSWORD in $SECRETS (cambio password)

Il primo aggiornamento OTA dal firmware precedente (senza password OTA) funziona
comunque: il dispositivo non chiede l'autenticazione.
Cambio password Wi-Fi: prima caricare via OTA il firmware con la nuova password,
poi cambiarla sul router. Un errore in WIFI_SSID/WIFI_PWD richiede il cavo USB.
EOF
}

pio_bin() {
  if [[ -x $VENV/bin/pio ]]; then
    echo "$VENV/bin/pio"
  elif command -v pio >/dev/null 2>&1; then
    command -v pio
  else
    die "PlatformIO non trovato: eseguire ./deploy.sh setup"
  fi
}

secret() {
  sed -n "s/^[[:space:]]*#define[[:space:]]\{1,\}$1[[:space:]]\{1,\}\"\([^\"]*\)\".*/\1/p" "$SECRETS" | head -n 1
}

check_secrets() {
  [[ -f $SECRETS ]] || die "$SECRETS mancante: copiare src/secrets.example.h in $SECRETS e compilarlo"
  local name value
  for name in WIFI_SSID WIFI_PWD MQTT_SERVER MQTT_USERNAME MQTT_PASSWORD OTA_PASSWORD; do
    value=$(secret "$name")
    [[ -n $value ]] || die "$name vuoto o mancante in $SECRETS"
    [[ $value != CHANGE_ME ]] || die "$name in $SECRETS contiene ancora il segnaposto CHANGE_ME"
  done
  git check-ignore -q "$SECRETS" || die "$SECRETS non è escluso da Git: controllare .gitignore"
}

show_revision() {
  local branch revision
  branch=$(git rev-parse --abbrev-ref HEAD)
  revision=$(git describe --always --dirty)
  info "Sorgente: $branch @ $revision"
  [[ $branch == "$BRANCH" ]] || warn "il branch atteso è $BRANCH"
  [[ $revision != *-dirty ]] || warn "ci sono modifiche non committate: il firmware non corrisponde a un commit"
}

check_dialout() {
  id -nG | tr ' ' '\n' | grep -qx dialout && return 0
  die "l'utente $USER non è nel gruppo dialout e non può usare la porta USB.
Eseguire: sudo usermod -aG dialout $USER  e poi uscire e rientrare dalla sessione."
}

resolve_host() {
  local host=$1 address
  if [[ $host =~ ^[0-9]+(\.[0-9]+){3}$ ]]; then
    echo "$host"
    return
  fi
  address=$(getent ahostsv4 "$host" | awk 'NR == 1 { print $1 }')
  [[ -n $address ]] || die "impossibile risolvere $host. Indicare l'IP: ./deploy.sh ota <ip>
(per .local servono avahi e nss-mdns: sudo dnf install nss-mdns)"
  echo "$address"
}

port_allowed() {
  # $1 = port, $2 = list like "1025-65535/tcp 8080/tcp"
  local entry range low high
  for entry in $2; do
    [[ $entry == */tcp ]] || continue
    range=${entry%/tcp}
    low=${range%-*}
    high=${range#*-}
    (( $1 >= low && $1 <= high )) && return 0
  done
  return 1
}

check_firewall() {
  # espota opens a local TCP server the ESP connects back to during the upload.
  local address=$1 iface zone ports
  command -v firewall-cmd >/dev/null 2>&1 || return 0
  systemctl is-active --quiet firewalld || return 0
  iface=$(ip -o route get "$address" 2>/dev/null | sed -n 's/.* dev \([^ ]*\).*/\1/p')
  zone=$(firewall-cmd --get-zone-of-interface="$iface" 2>/dev/null || firewall-cmd --get-default-zone 2>/dev/null) || return 0
  ports=$(firewall-cmd --zone="$zone" --list-ports 2>/dev/null) || {
    warn "impossibile leggere le regole di firewalld (zona $zone): se l'OTA resta ferma dopo l'invito, aprire $OTA_HOST_PORT/tcp"
    return 0
  }
  port_allowed "$OTA_HOST_PORT" "$ports" && return 0
  warn "firewalld (zona $zone) blocca la porta $OTA_HOST_PORT/tcp necessaria all'OTA."
  echo "Comando per aprirla fino al prossimo riavvio: sudo firewall-cmd --zone=$zone --add-port=$OTA_HOST_PORT/tcp"
  if [[ -t 0 ]]; then
    read -r -p "Eseguirlo ora? [s/N] " answer
    [[ $answer == [sS] ]] && sudo firewall-cmd --zone="$zone" --add-port="$OTA_HOST_PORT/tcp" && return 0
  fi
  die "porta $OTA_HOST_PORT/tcp chiusa: OTA non avviata"
}

wait_online() {
  local address=$1 _
  info "Attesa del riavvio del dispositivo..."
  sleep 5
  for _ in $(seq 1 60); do
    if ping -c 1 -W 1 "$address" >/dev/null 2>&1; then
      info "Dispositivo di nuovo raggiungibile su $address."
      return 0
    fi
    sleep 1
  done
  warn "il dispositivo non risponde da 60 secondi: controllare Wi-Fi e alimentazione"
  return 1
}

cmd_setup() {
  command -v python3 >/dev/null 2>&1 || die "python3 mancante: sudo dnf install python3"
  if [[ ! -x $VENV/bin/pio ]]; then
    info "Installazione di PlatformIO in $VENV"
    python3 -m venv "$VENV"
    "$VENV/bin/pip" install --quiet --upgrade pip platformio
  fi
  "$VENV/bin/pio" --version
  [[ -f $SECRETS ]] || warn "$SECRETS mancante: copiare src/secrets.example.h in $SECRETS e compilarlo"
  if ! id -nG | tr ' ' '\n' | grep -qx dialout; then
    warn "per il caricamento USB: sudo usermod -aG dialout $USER  e poi uscire e rientrare dalla sessione"
  fi
}

cmd_check() {
  check_secrets
  info "$SECRETS completo"
  info "PlatformIO: $("$(pio_bin)" --version)"
  show_revision
}

cmd_build() {
  check_secrets
  show_revision
  "$(pio_bin)" run -e "$ENV_USB"
}

cmd_test() {
  "$(pio_bin)" test -e native
}

cmd_usb() {
  local args=()
  check_secrets
  check_dialout
  show_revision
  [[ -z ${1:-} ]] || args=(--upload-port "$1")
  "$(pio_bin)" run -e "$ENV_USB" -t upload "${args[@]}"
  info "Caricamento USB completato. Log: ./deploy.sh monitor ${1:-}"
}

cmd_ota() {
  local host=${1:-$OTA_HOST} address password
  check_secrets
  show_revision
  address=$(resolve_host "$host")
  info "Dispositivo: $host ($address)"
  ping -c 1 -W 2 "$address" >/dev/null 2>&1 || die "$address non risponde al ping: dispositivo spento o non in rete"
  check_firewall "$address"
  password=${ESPALTHERMA_OTA_PASSWORD_CURRENT:-$(secret OTA_PASSWORD)}
  info "Compilazione e invio OTA (porta locale $OTA_HOST_PORT/tcp)"
  ESPALTHERMA_OTA_PASSWORD=$password "$(pio_bin)" run -e "$ENV_OTA" -t upload --upload-port "$address"
  wait_online "$address"
  info "Verificare in Home Assistant: espaltherma/LWT = Online e i log su espaltherma/log"
}

cmd_monitor() {
  local args=(-b "$MONITOR_SPEED")
  check_dialout
  [[ -z ${1:-} ]] || args+=(-p "$1")
  "$(pio_bin)" device monitor "${args[@]}"
}

case ${1:-help} in
  setup) cmd_setup ;;
  check) cmd_check ;;
  build) cmd_build ;;
  test) cmd_test ;;
  usb) cmd_usb "${2:-}" ;;
  ota) cmd_ota "${2:-}" ;;
  monitor) cmd_monitor "${2:-}" ;;
  help | -h | --help) usage ;;
  *) usage >&2; exit 2 ;;
esac
