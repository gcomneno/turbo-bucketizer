<h1 align="center">🌀 Turbo-Bucketizer</h1>
<p align="center"><em>Distribuisci IPv4 in modo uniforme e deterministico ad alta entropia.</em></p>

<p align="center">
  <a href="https://github.com/gcomneno/turbo-bucketizer/stargazers">
    <img alt="Stars" src="https://img.shields.io/github/stars/gcomneno/turbo-bucketizer?style=for-the-badge&label=Stars">
  </a>
  <a href="https://github.com/gcomneno/turbo-bucketizer">
    <img alt="Last Commit" src="https://img.shields.io/github/last-commit/gcomneno/turbo-bucketizer?style=for-the-badge&label=Last%20Commit">
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
make all         # oppure: python3 build.py
./turbo-bucketizer --selftest
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

* (x) = IPv4 come intero 32-bit,
* (a) è **invertibile** modulo (2^{32}) (odd), (b) è l’offset,
* lo shift prende i bit alti di (y) per il bucket ([0 .. 2^k-1]).

Caratteristiche:

* **Deterministico** (seed = preset ((a,b))),
* **Uniformità** robusta su input non-IID (CIDR compatti),
* **O(1)** per IP, zero stato, zero DB.

📘 Approfondimenti: [`THEORY.md`](./THEORY.md) · Guida d’uso: [`USAGE.md`](./USAGE.md) · Glossario: [`GLOSSARY.md`](./GLOSSARY.md)

---

## 🛠️ Roadmap

* [x] CLI pubblica
* [x] Note tecniche (whitepaper breve)
* [ ] **Preset tables adattive** (ricerca automatica di ((a,b)) per (k)/dataset)
* [ ] Benchmark suite riproducibile + multi-thread
* [ ] Binary per macOS/Windows
* [ ] API REST (`/bucket?ip=…&k=…`)
* [x] Banane per tutti 🍌

---

## 🚀 Release & Download

> Le **release binarie** sono in preparazione. Nel frattempo: compila da sorgente.

Quando saranno pronte troverai:

* Binari per **Linux**, **macOS**, **Windows**
* Script demo Python (`demo_bucket.py`, `map_ipv4.py`)
* `preset_table.tsv` (fallback + preset consigliati)

Possibili canali:

* Gumroad — Turbo-Bucketizer *(coming soon)*
* Itch.io — Club dell’Assurdo Edition *(coming soon)*

---

## 🤝 Contribuire

* Metti ⭐, apri una [Discussion](https://github.com/gcomneno/turbo-bucketizer/discussions)
* Proponi una [Feature](https://github.com/gcomneno/turbo-bucketizer/issues/new?labels=enhancement&template=feature.md) o una PR
* Condividi benchmark riproducibili (spec macchina + comandi)

> Se funziona al primo colpo, probabilmente hai sbagliato qualcosa.
> — *Proverbio interno Giadaware*

---

## 💡 Credits

Sviluppato da **Giancarlo** ([@gcomneno](https://github.com/gcomneno))
con il supporto morale di **La Scimmia Curiosa** e del **Club dell’Assurdo**.

<p align="center">
  <a href="https://www.giadaware.it">🌐 Giadaware.it</a> •
  <a href="https://www.clubdellassurdo.it">🌀 Club dell’Assurdo</a> •
  <a href="https://github.com/sponsors/gcomneno">💖 GitHub Sponsors</a>
</p>
