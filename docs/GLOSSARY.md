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
192×256³ + 168×256² + 0×256 + 1 = 3 232 235 777

Turbo-Bucketizer lavora direttamente su questa rappresentazione intera,  
trattando ogni IP come un punto su una linea di 2³² posizioni.

---

### 🧮 CIDR (Classless Inter-Domain Routing)
Notazione per indicare un intervallo di indirizzi IP.  
Esempio: `10.0.0.0/16` → 65 536 indirizzi da 10.0.0.0 a 10.0.255.255.

---

### 🧩 k (log₂(buckets))
Parametro che definisce il numero di bucket:
\( \text{buckets} = 2^k \).

- `k=12` → 4096 bucket  
- `k=16` → 65 536 bucket  
- `k=0`  → tutti i bucket = 0 (singolo bucket)  
- `k=32` → nessuno shift, bucket = `y` completo (32 bit)

---

### ⚙️ Permutazione affine modulo \(2^{32}\)
Funzione matematica reversibile:
\( y = (a \cdot x + b) \bmod 2^{32} \)
che garantisce distribuzione uniforme su tutto l’intervallo IPv4  
se `a` è **dispari** (condizione di invertibilità).

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
Tempo di calcolo costante: il bucket viene calcolato con **una sola moltiplicazione, una somma e uno shift**, indipendentemente dal numero di indirizzi.

---

### 🧰 Preset
Coppia `(a,b)` predefinita che controlla la permutazione affine.

- `default` → distribuzione bilanciata  
- `wang` → più veloce, leggermente meno uniforme  
- `murmur` → (sperimentale) per test di robustezza

---

### 🧾 Bench / Self-test
Comandi di validazione e misurazione delle prestazioni:
- `--selftest` → valuta uniformità (chi², MAD, max deviation, uniformity)  
- `--bench` → misura Mops/s e checksum di controllo  
- `--sample N` → esegue self-test su campione ridotto (stride)

---

### 📤 Export (v0.1.1+)
Funzionalità che consente di **salvare o esportare la sequenza completa dei bucket** generati.

| Opzione | Descrizione |
|----------|-------------|
| `--export <path|->` | Esporta i bucket (TXT o CSV). `-` → stdout |
| `--export-format txt|csv` | Forza il formato (override dell’estensione) |
| `--no-header` | CSV senza riga “index,bucket” |
| `--limit N` | Esporta solo i primi N bucket |

Esempio:
```

./turbo-bucketizer --selftest --cidr 10.0.0.0/16 --k 12 --sample 100000 --export - 
| python3 tools/digit_probe.py --mode schur --radix 10

```

Log di conferma su `stderr`:
```

[export] items=50000 checksum=0xA1B2C3D4E5F60789 fmt=csv dst=buckets.csv

```

💡 Il checksum usa **FNV-1a 64 bit** per verificare integrità del flusso esportato.

---

### 🧾 🔢 Entropia
Grado di imprevedibilità nella distribuzione dei bucket.  
Un buon preset massimizza l’entropia, cioè fa sì che IP simili vadano in bucket molto diversi.

Turbo-Bucketizer non genera numeri casuali,  
ma produce una mappatura **pseudo-caotica e deterministica**,  
ripetibile e senza seme o stato interno.

---

### 🧾 ⚙️ Performance
Velocità di esecuzione (misurata in Mops/s).

Dipende da:
- CPU e ottimizzazioni di compilazione (`-O3 -march=native`)
- Preset scelto (`wang` è leggermente più veloce)
- Assenza di operazioni condizionali o memoria dinamica

Prestazioni tipiche:
```

N=20,000,000  time=0.071s  rate=281.160 Mops/s  checksum=3255

```

> **Mops/s = Million operations per second** → **milioni di calcoli di bucket al secondo** ✅  
> 281 Mops/s = circa **0.281 miliardi di IP/s** → una **bestia deterministica** che spacca IPv4 come noccioline. 🦍💥

Ogni test include un checksum per garantire coerenza del loop e assenza di ottimizzazioni spurie del compilatore.

---

### 🧩 VLSM (Variable Length Subnet Mask)
Supporto a subnet di lunghezza variabile (es. `/24`, `/28`, `/30`).  
Il bucketizer tratta gli indirizzi come valori interi indipendenti:  
la distribuzione resta uniforme **a prescindere dalla maschera**.

---

### 🧑‍💻 Giadaware
Laboratorio semi-serio dove logica e follia convivono.  
Casa madre di Turbo-Bucketizer, e custode della scimmia deterministica. 🐒⚙️
