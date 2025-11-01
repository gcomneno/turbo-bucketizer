# 🧾 Changelog — Turbo-Bucketizer

Tutti i cambiamenti rilevanti del progetto **Turbo-Bucketizer** sono documentati qui.  
Formato basato su [Keep a Changelog](https://keepachangelog.com/it/1.1.0/).

---

## [v0.1] — 2025-11-01
### 🎉 Primo rilascio pubblico (MVP)
**Nome in codice:** *“La scimmia l’ha fatta grossa”* 🐒💥

#### Aggiunto
- CLI completa:
  - `--ip` → calcolo bucket deterministico per IPv4
  - `--k` → definizione di profondità (2^k bucket)
  - `--preset` (`default`, `wang`)
  - `--selftest` e `--cidr` → test di uniformità
  - `--sample` → campionamento su CIDR grandi
  - `--bench` → benchmark interno con checksum anti-DCE
- Preset solidi:
  - `default`: `a=0x9E3779B1`, `b=0x85EBCA77`
  - `wang`: `a=0x27D4EB2D`, `b=0x165667B1`
- Output statistici:
  - `chi²`, `MAD`, `max deviation`, `uniformity`
- Integrazione `Makefile` completo per build e test
- Target `release` (zip + sha256 automatici)
- Mini-paper tecnico (PDF, `docs/paper.tex`)
- Pagina sponsor GitHub attiva 💖

#### Performance
| Test | CIDR | k | Uniformity | Throughput |
|------|------|---|-------------|-------------|
| default | 10.0.0.0/8 | 12 | 99.459 % | — |
| wang | — | 12 | — | 247–281 Mops/s |

#### Filosofia
> “Se funziona ed è assurdo, allora è perfettamente logico.”

---

## [Unreleased]
### In sviluppo
- 🌐 **REST API / FastAPI** (`GET /bucket?ip=&k=&preset=`)
- ⚙️ Compilazioni per macOS e Windows
- 🧪 “Adaptive Presets” (autotuning via uniformity feedback)
- 🪄 Gumroad / SaaS gateway per uso commerciale
- 📚 Doc PDF “v1.0 Whitepaper”

---

## [v0.2] — (in preparazione)
**Nome in codice:** *“Il Bucket e la Banana”* 🍌🌀  
- Server API locale in FastAPI
- Config JSON (`config.json`) con preset personalizzati
- Nuovi benchmark multi-thread
- Target `api` nel Makefile
- Prima release binaria macOS (x86_64 + ARM64)

---

## 🧩 Credits
Progetto sviluppato da **Giancarlo (Giadaware)**  
→ [github.com/gcomneno](https://github.com/gcomneno)  
Sponsor ufficiale: [💖 La Scimmia Curiosa](https://github.com/sponsors/gcomneno)

