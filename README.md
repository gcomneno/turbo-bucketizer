<h1 align="center">🌀 Turbo-Bucketizer</h1>
<p align="center"><em>Distribuisci IPv4 in modo uniforme e deterministico ad alta entropia.</em></p>

<p align="center">
  <a href="https://github.com/gcomneno/turbo-bucketizer/stargazers">
    <img alt="Stars" src="https://img.shields.io/github/stars/gcomneno/turbo-bucketizer?style=for-the-badge&label=Stars">
  </a>
  <a href="https://github.com/gcomneno/turbo-bucketizer">
    <img alt="Last Commit" src="https://img.shields.io/github/last-commit/gcomneno/turbo-bucketizer?style=for-the-badge&label=Last%20Commit">
  </a>
  <a href="#">
    <img alt="Export-ready" src="https://img.shields.io/badge/Export--ready-yes-brightgreen?style=for-the-badge">
  </a>
  <a href="https://github.com/sponsors/gcomneno">
    <img alt="Sponsor" src="https://img.shields.io/badge/Sponsor%20La%20Scimmia%20Curiosa-💖-ea4aaa?style=for-the-badge">
  </a>
  <a href="https://github.com/gcomneno/turbo-bucketizer/blob/main/LICENSE">
    <img alt="License: MIT" src="https://img.shields.io/badge/License-MIT-green?style=for-the-badge">
  </a>
</p>

---

> 💖 Progetto sostenuto dagli sponsor del **Laboratorio Semi-Serio Giadaware** — [Diventa sponsor](https://github.com/sponsors/gcomneno)

---

## 🧩 Cos'è
**Turbo-Bucketizer** è un partizionatore deterministico per IPv4 basato su **affine permutation mod 2³²**.  
Obiettivo: ottenere bucket quasi perfettamente equilibrati anche con input non-IID (es. blocchi CIDR corposi), in **O(1)** per IP.

In parole povere:
> prendi un IP, scegli \(k\), ottieni un bucket tra \(0..2^k-1\).  
> Niente DB, niente lookup: solo una moltiplicazione, una somma e uno shift.

---

## ⚡ Quickstart
```bash
git clone https://github.com/gcomneno/turbo-bucketizer.git
cd turbo-bucketizer
./turbo-bucketizer --selftest
oppure
make test
````

### Esempi CLI

```bash
# Bucket singolo
./turbo-bucketizer --ip 192.168.0.1 --k 12
# -> Bucket: 2781

# Mappatura di un blocco CIDR
./turbo-bucketizer --map subnet 10.0.0.0/8 --k 16
# -> Buckets: 65536 (uniformità ~99.998%)*

# Pipeline: lista IP -> TSV (ip\tbucket)
cat ips.txt | ./turbo-bucketizer --stdin --k 14 > map.tsv
```

* Uniformità indicativa: aggiorna con i tuoi numeri reali nella sezione **Benchmark**.

---

### 📤 Export della sequenza di bucket (v0.1.1+)
Per esportare la **sequenza completa dei bucket** devi usare l’opzione `--export`.
Senza `--export`, il binario produce l’output standard (TSV/umano) e **non** crea file di export.

# TXT su stdout → pipe diretta verso i tuoi analyzer
./turbo-bucketizer --selftest --cidr 10.0.0.0/16 --k 12 --sample 100000 --export - \
  | python3 tools/digit_probe.py --mode schur --radix 10

# CSV su file (senza header), primi 50 bucket
./turbo-bucketizer --cidr 10.0.0.0/24 --k 12 \
  --export buckets.csv --export-format csv --no-header --limit 50

# Log di conferma (va su stderr)
# [export] items=50 checksum=0xA1B2C3D4E5F60789 fmt=csv dst=buckets.csv

**Note**
- `--export -` scrive i bucket su **stdout** (perfetto per le pipe).
- `--export <path>` salva su file; l’estensione `.csv` fa inferire il formato CSV (override con `--export-format`).
- Opzioni utili: `--limit N`, `--no-header` (CSV).

Se per caso non vedi l’export:
- assicurati di usare un binario **v0.1.1** (o superiore),
- verifica che il `Makefile` includa `src/tb_export.c` nei sorgenti,
- lancia `./turbo-bucketizer --help` e controlla che compaiano le opzioni `--export`, `--export-format`, `--limit`, `--no-header`.

---

## 🔬 Benchmark (indicativi)
> Aggiorna con i risultati della tua macchina. Qui un seed di formato.

| Test        | Preset  | Input           | k  | Samples       | Throughput       | Uniformity  | χ²     | Max Dev |
| ----------- | ------- | --------------- | -- | ------------- | ---------------- | ----------- | ------ | ------- |
| Self-test   | default | 10.0.0.0/8      | 12 | 986,896       | —                | **99.459%** | 42.691 | 1.64%   |
| Micro-bench | wang    | random (32-bit) | 12 | 20,000,000 it | **~247 Mops/s**  | —           | —      | —       |
| Map demo    | default | 0.0.0.0/0       | 16 | 1,000,000     | ~350–500 M/s (*) | ~99.99%     | —      | <2%     |

> (*) Laptop x86_64; su macchina ottimizzata: **~1.3 G/s** per la mappa in C.

**Preset note:**

* `default`: `a=0x9E3779B1`, `b=0x85EBCA77`
* `wang`:    `a=0x27D4EB2D`, `b=0x165667B1`

---

## 🧠 How it works
La funzione è una **permutazione** dello spazio (2^{32}):

```text
y = (a * x + b) mod 2^32
bucket(x) = y >> (32 - k)
```

Dove:
- (x) = IPv4 come intero 32-bit,
- (a) è **invertibile** modulo (2^{32}) (odd), (b) è l’offset,
- lo shift prende i bit alti di (y) per il bucket ([0 .. 2^k-1]).

Caratteristiche:
- **Deterministico** (seed = preset ((a,b))),
- **Uniformità** robusta su input non-IID (CIDR compatti),
- **O(1)** per IP, zero stato, zero DB.

📘 Approfondimenti: [`THEORY.md`](./docs/THEORY.md) · Guida d’uso: [`USAGE.md`](./docs/USAGE.md) · Glossario: [`GLOSSARY.md`](./docs/GLOSSARY.md)

---

## 🛠️ Roadmap
- [x] CLI pubblica
- [x] Note tecniche (whitepaper breve)
- [ ] **Preset tables adattive** (ricerca automatica di ((a,b)) per (k)/dataset)
- [ ] Benchmark suite riproducibile + multi-thread
- [ ] Binary per macOS/Windows
- [ ] API REST (`/bucket?ip=…&k=…`)
- [x] Banane per tutti 🍌

---

## 🤝 Contribuire
* Metti ⭐, apri una [Discussion](https://github.com/gcomneno/turbo-bucketizer/discussions)
* Proponi una [Feature](https://github.com/gcomneno/turbo-bucketizer/issues/new?labels=enhancement&template=feature.md) o una PR
* Condividi benchmark riproducibili (spec macchina + comandi)

> Se funziona al primo colpo, probabilmente hai sbagliato qualcosa.
> — *Proverbio interno Giadaware*

---

## 📚 Documentazione
La documentazione completa si trova in [`/docs`](./docs):

- [USAGE.md](./docs/USAGE.md) — guida rapida e CLI
- [THEORY.md](./docs/THEORY.md) — note di design e teoria
- [GLOSSARY.md](./docs/GLOSSARY.md) — definizioni tecniche
- [CHANGELOG.md](./docs/CHANGELOG.md) — cronologia delle versioni

---

## 💡 Credits
Sviluppato da **Giancarlo** ([@gcomneno](https://github.com/gcomneno))
con il supporto morale della **La Scimmia Curiosa** nel suo cervello.

<p align="center">
  <a href="https://www.giadaware.it">🌐 Giadaware.it</a> •
  <a href="https://github.com/sponsors/gcomneno">💖 GitHub Sponsors</a>
</p>
