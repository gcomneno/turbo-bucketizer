# 📘 GLOSSARY — Turbo-Bucketizer
Definizioni e concetti chiave del progetto.

---

### 🌀 Bucketizer
Algoritmo che partiziona lo spazio IPv4 in \(2^k\) bucket deterministici.

Formula di base:
```
y = (a*x + b) mod 2^32
bucket(x) = y >> (32 - k)
```

---

### 🌍 IPv4
Sistema di indirizzamento Internet a **32 bit**.
Ogni IP è un numero intero compreso tra 0 e 4 294 967 295,  
solitamente espresso in forma **puntata** (es. `192.168.0.1`).

La rappresentazione numerica è:
192256^3 + 168256^2 + 0*256 + 1 = 3 232 235 777

Turbo-Bucketizer lavora direttamente su questa rappresentazione intera,  
trattando ogni IP come un punto su una linea di 2³² posizioni.

---

### 🧮 CIDR (Classless Inter-Domain Routing)
Notazione per indicare un intervallo di indirizzi IP.
Esempio: `10.0.0.0/16` → 65 536 indirizzi da 10.0.0.0 a 10.0.255.255.

---

### 🧩 k (log2(buckets))
Parametro che definisce il numero di bucket:
\( \text{buckets} = 2^k \).

- `k=12` → 4096 bucket  
- `k=16` → 65 536 bucket

---

### ⚙️ Permutazione affine modulo \(2^{32}\)
Funzione matematica reversibile:
\( y = (a \cdot x + b) \bmod 2^{32} \)
che garantisce distribuzione uniforme su tutto l’intervallo IPv4.

---

### 🧠 Uniformity
Misura di quanto i bucket ricevono la stessa quantità di IP.
Valori tipici:
- `>99%` → ottima distribuzione
- `≈96%` → ancora buona
- `<90%` → non omogenea

---

### 🧮 Chi-square (χ²)
Test statistico usato per stimare la *uniformità*.
Valore basso = distribuzione più omogenea.

---

### ⚡ O(1)
Tempo di calcolo costante: il bucket viene calcolato con **una sola moltiplicazione, addizione e shift**, indipendentemente dal numero di indirizzi.

---

### 🧰 Preset
Coppia `(a,b)` predefinita che controlla la permutazione affine.
- `default` → distribuzione bilanciata
- `wang` → più veloce, ma meno uniforme
- `murmur` → per test di robustezza

---

### 🧾 Bench / Self-test
Comandi di validazione e misurazione delle prestazioni:
- `--selftest` → valuta uniformità
- `--bench` → misura Mops/s

---

### 🧾 🔢 Entropia
In questo contesto: grado di imprevedibilità nella distribuzione dei bucket.
Un buon preset deve massimizzare l’entropia, cioè far sì che IP simili vadano in bucket molto diversi.
Il coefficiente a e il mixing moltiplicativo sono ciò che assicura un’elevata entropia “pratica”.

Turbo-Bucketizer non genera numeri casuali — ma produce una mappatura pseudo-caotica deterministica,
ripetibile e senza bisogno di seme o stato interno.

---

### 🧾 ⚙️ Performance

Velocità di esecuzione (misurata in Mops/s).

Dipende da:
- CPU e ottimizzazioni di compilazione (-O3 -march=native)
- Preset scelto (wang è leggermente più veloce)
- Assenza di operazioni condizionali o memoria dinamica

Prestazioni tipiche:
per N=20,000,000  time=0.071s  rate=281 Mops/s  checksum=3255

> **Mops/s = Million operations per second** cioè **milioni di operazioni al secondo** ✅
```
rate = 281.160 Mops/s
```
significa che il Turbo-Bucketizer sta eseguendo **281 milioni di calcoli di bucket al secondo**!

### 🔢 Piccola scala di riferimento

| Sigla      | Significato           |        Valore |
| :--------- | :-------------------- | ------------: |
| **ops/s**  | operazioni al secondo |             1 |
| **Kops/s** | kilo-ops → migliaia   |         1 000 |
| **Mops/s** | mega-ops → milioni    |     1 000 000 |
| **Gops/s** | giga-ops → miliardi   | 1 000 000 000 |

---

💡 Nel tuo caso, con `rate = 281 Mops/s`,
il Turbo-Bucketizer processa circa **0.281 miliardi di IP/s** —
una **bestia deterministica** che spacca IPv4 come noccioline. 🦍💥


Ogni test viene verificato con checksum per garantire coerenza del loop.

---

### 🧑‍💻 Giadaware
Laboratorio semi-serio dove logica e follia convivono.
Casa madre di Turbo-Bucketizer!
