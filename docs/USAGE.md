# 📘 USAGE — Turbo-Bucketizer
Guida rapida per usare il binario, eseguire self-test, benchmark e integrare nei tuoi script o pipeline.

---

## 1️⃣ Download & verifica

**Opzione A — Release pronta (consigliata)**

1. Scarica l’ultima release: [Releases](https://github.com/gcomneno/turbo-bucketizer/releases/latest)
2. Verifica l’integrità (`sha256sum`):
```bash
  sha256sum -c turbo-bucketizer-*-v*.zip.sha256
````

3. Estrai e rendi eseguibile:

   ```bash
   unzip turbo-bucketizer-*-v*.zip
   chmod +x turbo-bucketizer
   ./turbo-bucketizer --help
   ```

**Opzione B — Compila da sorgente**

```bash
git clone https://github.com/gcomneno/turbo-bucketizer.git
cd turbo-bucketizer/build
make
../turbo-bucketizer --help
```

---

## 2️⃣ Uso base

Calcolo del bucket per un IPv4:

```bash
./turbo-bucketizer --ip 192.168.0.1 --k 12
# Output:
# IP=192.168.0.1  k=12  bucket=3076
```

**Parametri (a,b)**

* Preset pronti: `default` (ottima uniformità), `wang` (più rapido)
* Override manuale:

  ```bash
  ./turbo-bucketizer --ip 10.0.0.1 --k 16 --a 9E3779B1 --b 85EBCA77
  ```

---

## 3️⃣ Self-test (uniformità)

Valuta la distribuzione su un blocco CIDR:

```bash
# Scan completo su /16 (4096 bucket, 65536 IP)
./turbo-bucketizer --selftest --cidr 10.0.0.0/16 --k 12 --preset default

# Campionamento su /8 (~1M campioni con stride)
./turbo-bucketizer --selftest --cidr 10.0.0.0/8 --k 12 --sample 1000000 --preset default
```

Esempio di output:

```
Self-test Turbo-Bucketizer
preset=default  a=0x9E3779B1  b=0x85EBCA77
CIDR=10.0.0.0/8  k=12  buckets=4096  total=16777216  samples=986896  step=17
expected_per_bucket=240.94
chi2=42.691  max_deviation=1.64%  MAD=0.54%  uniformity≈99.459%
```

---

## 4️⃣ Benchmark

Misura throughput su indirizzi sintetici:

```bash
./turbo-bucketizer --bench 20000000 --k 12 --preset wang
# Output:
# Benchmark Turbo-Bucketizer
# preset=wang  a=0x27D4EB2D  b=0x165667B1  k=12
# N=20000000  time=0.071s  rate=281.160 Mops/s  checksum=3255
```

💡 `checksum` serve ad evitare che il compilatore ottimizzi via il loop.

---

## 5️⃣ Export della sequenza di bucket (v0.1.1+)

Puoi esportare la **sequenza completa dei bucket** in TXT o CSV tramite `--export`.

Senza questa opzione, il programma produce l’output standard (TSV/umano); con `--export` genera file o flusso compatibili con analizzatori esterni come `digit_probe.py`.

```bash
# TXT su stdout → pipe diretta verso un analyzer
./turbo-bucketizer --selftest --cidr 10.0.0.0/16 --k 12 --sample 100000 --export - \
  | python3 tools/digit_probe.py --mode schur --radix 10

# CSV su file, primi 50 bucket (senza header)
./turbo-bucketizer --cidr 10.0.0.0/24 --k 12 \
  --export buckets.csv --export-format csv --no-header --limit 50

# Log di conferma (stderr)
# [export] items=50 checksum=0xA1B2C3D4E5F60789 fmt=csv dst=buckets.csv
```

**Opzioni principali:**

| Opzione              | Descrizione                        |                                         |
| -------------------- | ---------------------------------- | --------------------------------------- |
| `--export <path      | ->`                                | Abilita export TXT o CSV (`-` = stdout) |
| `--export-format txt | csv`                               | Forza il formato (override estensione)  |
| `--no-header`        | CSV senza riga “index,bucket”      |                                         |
| `--limit N`          | Esporta solo i primi N bucket      |                                         |
| `--sample N`         | Campionamento interno per selftest |                                         |

---

## 6️⃣ Edge cases

| Caso                        | Comportamento                                              |
| --------------------------- | ---------------------------------------------------------- |
| `k=0`                       | Tutti i bucket = 0                                         |
| `k=32`                      | Nessuno shift (bucket = `y` pieno 32 bit)                  |
| CIDR VLSM                   | Gestiti in modo uniforme, indipendentemente dalla maschera |
| IPv4 multipli via `--stdin` | Output TSV: `<ip>\t<bucket>`                               |
| Export + stdin              | Compatibile (`--export file.csv --no-header`)              |

---

## 7️⃣ Integrazione da shell

**Bucketizza una lista di IP (bash):**

```bash
while read -r ip; do
  ./turbo-bucketizer --ip "$ip" --k 12 --preset default
done < ips.txt
```

**CIDR → quick-check distribuzione:**

```bash
./turbo-bucketizer --selftest --cidr 192.168.0.0/16 --k 12 --preset default \
| grep -E 'chi2|max_deviation|uniformity'
```

**Pipeline di analisi con `digit_probe.py`:**

```bash
./turbo-bucketizer --selftest --cidr 10.0.0.0/16 --k 12 --sample 100000 --export - \
  | python3 tools/digit_probe.py --mode schur --radix 10
```

---

## 8️⃣ FAQ / Troubleshooting

**Errore “implicit declaration of clock_gettime” durante la build**

* Aggiungi la macro POSIX o linka `-lrt` (solo vecchie distro):

  ```c
  // In src/main.c
  #define _POSIX_C_SOURCE 200809L
  ```

  ```makefile
  # In build/Makefile
  LDFLAGS ?= -lrt
  ```

**`Permission denied` all’avvio**

```bash
chmod +x ./turbo-bucketizer
./turbo-bucketizer --help
```

---

## 9️⃣ Riferimenti

* 📘 Teoria: [Theory & Design Notes](./docs/THEORY.md)
* 📄 Documentazione: [README](./README.md)
* 🧾 Changelog: [CHANGELOG.md](./docs/CHANGELOG.md)
