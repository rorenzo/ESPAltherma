# Workflow Fork: aggiornare dal repo originale senza perdere custom

Questa guida descrive come mantenere il tuo fork allineato al progetto originale (`upstream`) preservando le personalizzazioni locali (sensori custom, profilo Hybrid, ecc.).

## Obiettivo

- tenere `main` del fork uguale a `upstream/main`
- tenere le custom in un branch dedicato (es. `custom/hybrid-rain-ds18b20`)
- integrare periodicamente le novita upstream nel branch custom

## 1) Setup remoti (una sola volta)

Verifica remoti:

```bash
git remote -v
```

Configurazione consigliata:

- `origin` = tuo fork GitHub
- `upstream` = repo originale

Se serve:

```bash
git remote rename origin upstream
git remote add origin git@github.com:<tuo-user>/ESPAltherma.git
git fetch --all --prune
```

## 2) Branch model consigliato

- `main`: specchio di `upstream/main` (evita custom dirette qui)
- `custom/hybrid-rain-ds18b20`: tutte le tue modifiche personalizzate

## 3) Primo commit custom (se non gia fatto)

```bash
git switch custom/hybrid-rain-ds18b20
git add include/custom_sensors_config.h include/custom_sensors.h src/custom_sensors.cpp src/main.cpp platformio.ini "include/def/ALTHERMA(HYBRID).h" doc/custom-rain-temp.md
git commit -m "Add DS18B20 + rain sensor and hybrid profile tuning"
git push -u origin custom/hybrid-rain-ds18b20
```

Nota: il path con parentesi va tra virgolette.

## 4) Ciclo periodico di aggiornamento da upstream

### 4.1 Allinea `main` del fork al repo originale

```bash
git fetch upstream --prune
git switch main
git merge --ff-only upstream/main
git push origin main
```

### 4.2 Porta gli update nel branch custom

```bash
git switch custom/hybrid-rain-ds18b20
git merge main
git push origin custom/hybrid-rain-ds18b20
```

Se ci sono conflitti, risolvi e poi:

```bash
git add <file-risolti>
git commit
git push
```

## 5) File dove e piu probabile avere conflitti

- `src/main.cpp`
- `platformio.ini`
- `include/def/ALTHERMA(HYBRID).h`

Per ridurre conflitti:

- mantieni logica custom in file separati (`src/custom_sensors.cpp`, `include/custom_*`)
- lascia in `main.cpp` solo hook minimi
- evita modifiche non necessarie a file upstream

## 6) Regole pratiche per non perdere custom

- prima di un merge/sync: working tree pulito

```bash
git status
```

- committa in modo atomico (feature piccole)
- non lavorare direttamente su `main`
- fai push frequenti del branch custom
- usa PR dal branch custom verso `main` del tuo fork per avere storico chiaro

## 7) Verifiche rapide dopo ogni sync

```bash
git log --oneline --decorate --graph -20
git diff --name-only upstream/main..custom/hybrid-rain-ds18b20
```

La seconda lista deve mostrare solo i file custom attesi.

## 8) Opzione alternativa: rebase invece di merge

Se preferisci storia lineare:

```bash
git switch custom/hybrid-rain-ds18b20
git rebase main
git push --force-with-lease
```

Usa `rebase` solo se il branch e usato da poche persone (o solo da te).

