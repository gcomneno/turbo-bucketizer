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

---

## 📜 Filosofia

> “Se puoi bucketizzarlo, allora puoi comprenderlo.”
> — *La Scimmia Curiosa, 2025*

Nato per gioco nel laboratorio **Giadaware**,
il Turbo-Bucketizer è un tributo al caos che si lascia domare (ma solo per finta!).

---

## 🛠️ Roadmap

* [ ] Versione CLI pubblica
* [ ] API REST (`/bucket?ip=…&k=…`)
* [ ] Doc tecnica + whitepaper breve
* [ ] Versione “Turbo-Pro” con preset tables adattive
* [ ] Banane per tutti 🍌

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
