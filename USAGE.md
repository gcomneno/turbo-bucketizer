# 📘 USAGE — Turbo-Bucketizer

Guida rapida per usare il binario, fare self-test, benchmark e integrare nei tuoi script.

---

## 1) Download & verifica

**Opzione A — Release pronta (consigliata)**
1. Scarica l’ultima release: [Releases](https://github.com/gcomneno/turbo-bucketizer/releases/latest)
2. Verifica lo **sha256**:
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

## 2) Uso base

Calcolo del bucket per un IPv4:

```bash
./turbo-bucketizer --ip 192.168.0.1 --k 12
# Output atteso:
# IP=192.168.0.1  k=12  bucket=3076
```

**Parametri (a,b)**

* Preset pronti: `default` (buona uniformità), `wang` (veloce)
* Override manuale:

  ```bash
  ./turbo-bucketizer --ip 10.0.0.1 --k 16 --a 9E3779B1 --b 85EBCA77
  ```

---

## 3) Self-test (uniformità)

Valuta la distribuzione su un blocco CIDR:

```bash
# Scan completo su /16 (4096 bucket, 65536 IP)
./turbo-bucketizer --selftest --cidr 10.0.0.0/16 --k 12 --preset default

# Campionamento su /8 (≈1M campioni stride)
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

## 4) Benchmark

Misura throughput (iterazioni su indirizzi sintetici):

```bash
./turbo-bucketizer --bench 20000000 --k 12 --preset wang
# Esempio:
# Benchmark Turbo-Bucketizer
# preset=wang  a=0x27D4EB2D  b=0x165667B1  k=12
# N=20000000  time=0.071s  rate=281.160 Mops/s  checksum=3255
```

💡 `checksum` evita che il compilatore elimini il loop.

---

## 5) Integrazione da shell (snippet utili)

**Bucketizza una lista di IP (bash):**

```bash
while read -r ip; do
  ./turbo-bucketizer --ip "$ip" --k 12 --preset default | awk '{print $1,$2,$3}'
done < ips.txt
```

**CIDR → bucket distribution quick-check:**

```bash
./turbo-bucketizer --selftest --cidr 192.168.0.0/16 --k 12 --preset default \
| grep -E 'chi2|max_deviation|uniformity'
```

---

## 6) FAQ / Troubleshooting

**“implicit declaration of `clock_gettime`” in build**

* Aggiungi la macro POSIX o link `-lrt` (vecchie distro):

  * In `src/main.c` la prima riga deve essere:

    ```c
    #define _POSIX_C_SOURCE 200809L
    ```
  * In `build/Makefile` se serve:

    ```makefile
    LDFLAGS ?= -lrt
    ```

**Eseguibile non parte (`Permission denied`)**

```bash
chmod +x ./turbo-bucketizer
./turbo-bucketizer --help
```

---

## 7) Riferimenti

* 📘 Teoria: [Theory & Design Notes](docs/THEORY.md)
* 📄 Mini-paper: `./README.md`
* 🧾 Changelog: [CHANGELOG.md](../CHANGELOG.md)
