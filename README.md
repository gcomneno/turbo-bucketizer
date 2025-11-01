<h1 align="center">🌀 Turbo-Bucketizer</h1>
<p align="center"><em>Distribuisci IPv4 in modo uniforme e deterministico ad alta entropia.</em></p>
<p align="center">
  <img alt="Build Status" src="https://github.com/gcomneno/turbo-bucketizer/actions/workflows/ci.yml/badge.svg?branch=main">
</p>
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
  <a href="https://github.com/gcomneno/turbo-bucketizer/actions/workflows/ci.yml">
    <img alt="Build Status" src="https://github.com/gcomneno/turbo-bucketizer/actions/workflows/ci.yml/badge.svg">
  </a>
  <a href="https://github.com/gcomneno/turbo-bucketizer/blob/main/LICENSE">
    <img alt="License: MIT" src="https://img.shields.io/badge/License-MIT-green?style=for-the-badge">
  </a>
</p>

---

> 💖 Questo progetto è sostenuto dagli sponsor del **Laboratorio Semi-Serio Giadaware** — [Diventa sponsor!](https://github.com/sponsors/gcomneno)

---

## 🧩 Descrizione
**Turbo-Bucketizer** è un esperimento tecnico-poetico:  
un partizionatore deterministico di indirizzi IPv4 basato su **affine permutation mod 2³²**,  
ottimizzato per una distribuzione uniforme anche su input non casuali (es. blocchi CIDR).

In parole povere:  
> prendi un IP, scegli un K, e ottieni un bucket quasi perfettamente equilibrato.  
> Tutto in un colpo solo, senza database, senza magia (solo matematica ma con ironia).

---

## ⚙️ Esempio d’uso

```bash
$ ./turbo-bucketizer --ip 192.168.0.1 --k 12
Bucket: 2781

$ ./turbo-bucketizer --map subnet 10.0.0.0/8 --k 16
Buckets: 65536  (uniformità ~99.998%)
````

Output deterministico, copertura totale, velocità indecente.
Demo benchmark: 350–500 M/s su laptop, 1.3 G/s su macchina ottimizzata.

### 📊 Results

| Test | Preset | CIDR | k | Samples | Uniformity | χ² | Max Dev |
|-----:|:------:|:----:|:-:|--------:|-----------:|---:|--------:|
| Self-test | default | 10.0.0.0/8 | 12 | 986,896 | **99.459%** | 42.691 | 1.64% |
| Bench | wang | — | 12 | 20,000,000 iters | **~247 Mops/s** | — | — |

> 💡 `default` = a=0x9E3779B1, b=0x85EBCA77 — `wang` = a=0x27D4EB2D, b=0x165667B1

---

## 📜 Filosofia

> “Se puoi bucketizzarlo, allora puoi comprenderlo.”
> — *La Scimmia Curiosa, 2025*

---

### ⚙️ How it works

**Turbo-Bucketizer** suddivide lo spazio IPv4 in \(2^k\) bucket deterministici  
usando una permutazione affine modulo \(2^{32}\):

y = (a*x + b) mod 2^32; bucket(x) = y >> (32 - k)

Nessun database, nessun lookup: ogni IP trova il suo bucket in **O(1)** costante  
(una moltiplicazione, un add e uno shift).  

📘 [Approfondisci → Theory & Design Notes](./THEORY.md)

Nato per gioco nel laboratorio **Giadaware**,
il Turbo-Bucketizer è un tributo al caos che si lascia domare (ma solo per finta!).

🛠️ [Usage guide](docs/USAGE.md)

---

## 🛠️ Roadmap

* [X] Versione CLI pubblica
* [X] Doc tecnica + whitepaper breve
* [ ] Versione “Turbo-Pro” con preset tables adattive
* [ ] Binary per macOS/Windows
* [ ] Benchmark suite riproducibile + multi-thread
* [ ] API REST (`/bucket?ip=…&k=…`)
* [X] Banane per tutti 🍌

---

## 🚀 Release & Download

> ⚠️ La versione pubblica è in arrivo: la scimmia sta ancora lucidando i bucket.  
> Nel frattempo puoi **seguire il progetto** o **compilare la versione sperimentale**.

### 🔹 Release ufficiali
Quando la build pubblica sarà pronta, troverai qui:
- Binari per **Linux**, **macOS** e **Windows**
- Script Python demo (`demo_bucket.py`, `map_ipv4.py`)
- File di preset (`preset_table.tsv`)

📦 **Distribuzione prevista:**
- [Gumroad — Turbo-Bucketizer](https://gumroad.com/) *(coming soon)*
- [Itch.io — Club dell’Assurdo Edition](https://itch.io/) *(coming soon)*

---

### 🔹 Compila da sorgente (per smanettoni)

```bash
git clone https://github.com/gcomneno/turbo-bucketizer.git
cd turbo-bucketizer
make all     # o python3 build.py
./turbo-bucketizer --selftest
````

> Se funziona al primo colpo, probabilmente hai sbagliato qualcosa.
> — *Giadaware internal proverb*

---

> ⭐ Ti piace il progetto? Metti una stella, apri una [Discussion](https://github.com/gcomneno/turbo-bucketizer/discussions)  o proponi una [Feature](https://github.com/gcomneno/turbo-bucketizer/issues/new?labels=enhancement&template=feature.md)!  
> Ogni idea alimenta il laboratorio semi-serio di **Giadaware** 🎉

---

## 💡 Credits

Sviluppato da **Giancarlo** ([@gcomneno](https://github.com/gcomneno))
con il supporto morale di **La Scimmia Curiosa** e del **Club dell’Assurdo**.

---

<p align="center">
  <a href="https://www.giadaware.it">🌐 Giadaware.it</a> •
  <a href="https://www.clubdellassurdo.it">🌀 Club dell’Assurdo</a>
</p>

<p align="center"><em>💖 Sponsor by <a href="https://github.com/sponsors/gcomneno">La Scimmia Curiosa</a> — perché anche i bit hanno bisogno di banane.</em></p>
