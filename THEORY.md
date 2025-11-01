# 🌀 Turbo-Bucketizer — Theory & Design Notes
*Laboratorio Giadaware – “Se funziona ed è assurdo, allora è perfettamente logico!”*

---

## 1️⃣ Concept

**Turbo-Bucketizer** partiziona l’intero spazio IPv4 in \(2^k\) bucket deterministici,  
usando una *permutazione affine modulo \(2^{32}\)*:

\[
y = (a \cdot x + b) \bmod 2^{32}
\]
\[
\text{bucket}(x) = y \gg (32 - k)
\]

Dove:
- \(x\) è l’indirizzo IPv4 in formato numerico (32 bit)
- \(a\) e \(b\) sono costanti intere (parametri della permutazione)
- \(k\) è la profondità dei bucket (es. `k=12 → 4096 bucket`)

Niente database, niente hash table, niente stato:  
ogni bucket viene calcolato **direttamente**, in tempo **O(1)** costante.

---

## 2️⃣ Perché è diverso da un hash classico

| Aspetto | Hash classico | Turbo-Bucketizer |
|----------|----------------|------------------|
| Struttura dati | Hash table o lookup | Nessuna |
| Complessità | \(O(1)\) media (con overhead) | \(O(1)\) pura |
| Collisions | Possibili, da gestire | Nessuna (permutazione) |
| Determinismo | Parziale, seed-dipendente | Totale |
| Invertibilità | No | Sì, se \(a\) è dispari |

💡 **Idea chiave:** invece di “mappare” valori, li **permutiamo** nell’intero dominio \(2^{32}\).  
Ogni indirizzo IP trova il suo posto con una moltiplicazione, un somma e uno shift. Fine.

---

## 3️⃣ Implicazioni pratiche

- ⚡ **Zero stato**
  → nessuna tabella, nessuna cache: ogni nodo può ricomputare lo stesso bucket in locale.

- 🧩 **Uniformità quasi perfetta**
  → la permutazione affine distribuisce i 32 bit in modo regolare anche su intervalli contigui (es. subnet /8).

- 🧠 **Determinismo riproducibile**
  → stesso IP + stessi parametri → stesso bucket, sempre.

- 🧱 **Scalabilità lineare**
  → perfetto per load-balancing, sharding, proxy distribuiti, simulazioni o metriche su spazi IPv4.

- 🏎️ **Prestazioni**
  → 247–281 Mops/s su CPU x86_64 a 3 GHz (`-O3 -march=native`)  
    = miliardi di bucketizzazioni al minuto, in single thread.

---

## 4️⃣ La geometria dell’indirizzo

Pensa a ogni IP come a un punto su un cerchio di 4 miliardi di tacche.  
Moltiplicandolo per un numero dispari \(a\) e aggiungendo \(b\),  
lo ruotiamo e lo trasliamo sul cerchio in modo *pseudo-casuale ma deterministico*.

Estrarre i bit più alti (MSB) equivale a guardare in quale “spicchio” finisce il punto.  
Ogni spicchio è un bucket, e l’intero cerchio è una partizione perfettamente bilanciata.

---

## 5️⃣ Confronto mentale

> Se un hash classico è un **mescolatore caotico**,  
> Turbo-Bucketizer è un **rotatore geometrico**.  
>  
> Non genera rumore, genera simmetria.  
> Non cerca equilibrio, lo costruisce con aritmetica modulare.

---

## 6️⃣ Esempi d’uso reali

| Scenario | Beneficio |
|----------|------------|
| Load-balancing IPv4 | Bucket costante → routing deterministico |
| Sharding dataset IP | Divisione equa senza lookup |
| Proxy/firewall distribuiti | Regole di bucket uguali su nodi indipendenti |
| Analisi statistica subnet | Campionamento uniforme |
| DNS simulati o reti virtuali | Mapping rapido, ripetibile |

---

## 7️⃣ Complessità e invertibilità

- **Tempo:** O(1) esatto  
  \> una sola moltiplicazione, un add, un modulo implicito (overflow 32 bit), uno shift.  
- **Spazio:** O(1)  
  \> non memorizza nulla, opera in-place.  
- **Invertibilità:** se \(a\) è dispari → \(\exists a^{-1}\) mod \(2^{32}\)  
  → puoi risalire all’IP originale (funzione affine invertibile).

---

## 8️⃣ Filosofia

> Il caso è solo una forma di ordine non ancora compresa.  
>  
> Turbo-Bucketizer dimostra che la casualità può essere calcolata,  
> e che perfino un indirizzo IP ha la sua eleganza geometrica.

---

📘 **Riferimenti**
- D. E. Knuth – *The Art of Computer Programming*, Vol. 2 (mixing by multiplication)  
- T. Wang – *Integer Hash Functions* (2007)  
- Appleby – *MurmurHash design notes* (affine mixers)
