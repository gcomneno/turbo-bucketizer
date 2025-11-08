#!/usr/bin/env bash
# Global test-suite for Turbo-Bucketizer
# Robust to locales, piping, and stderr buffering.
set -euo pipefail
export LC_ALL=C

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$ROOT/turbo-bucketizer"
TMP="$ROOT/.tmp_tests"
DIGIT_PROBE="${DIGIT_PROBE:-}"   # optional hook to digit_probe.py

mkdir -p "$TMP"

say() { printf "▶ %s\n" "$*" >&2; }
ok()  { printf "✅ %s\n" "$*" >&2; }
ko()  { printf "❌ %s\n" "$*" >&2; exit 1; }

need() { command -v "$1" >/dev/null 2>&1 || ko "Manca comando richiesto: $1"; }

# ---------- prerequisites ----------
need awk
need wc
need tr
need diff
need printf
need head
need tail
need sed

# ---------- tiny helpers ----------
# count lines in file
lines() { wc -l < "$1"; }
# count whitespace-separated tokens in a single-line TXT
tokens() { tr -s ' ' '\n' < "$1" | wc -l; }

# robust check for [export] log: items=N + checksum hex64 present
assert_export_log_items() {
  local logfile="$1" want_items="$2"
  awk -v want="$want_items" '
    BEGIN { ok=0 }
    /^\[export\] / {
      # sample formats:
      # [export] items=1 checksum=0x3D54A66D05721B0D fmt=txt dst=-
      # items=<num>, checksum=0x<16 hex> (64-bit), fmt=txt|csv
      if ($0 ~ ("items=" want) &&
          $0 ~ /checksum=0x[0-9A-Fa-f]{16}/ &&
          $0 ~ /fmt=(txt|csv)/) ok=1
    }
    END { exit ok?0:1 }
  ' "$logfile" || { echo "---- LOG DUMP ----" >&2; cat "$logfile" >&2; ko "Log [export] mancante o malformato (atteso items=$want_items)"; }
}

# CSV row validator: "index,bucket" or "N,N"
is_csv_row() {
  local line="$1"
  echo "$line" | awk '
    BEGIN{ ok=0 }
    /^[0-9]+,[0-9]+$/ { ok=1 }
    END{ exit ok?0:1 }'
}

# header validator
assert_csv_header() {
  local file="$1"
  local h; h="$(head -n1 "$file")"
  [ "$h" = "index,bucket" ] || ko "CSV: header mancante o errato (got: '$h')"
}

# ---------- 0) Build ----------
say "Build binario…"
make -C "$ROOT/build" all >/dev/null
[ -x "$BIN" ] || ko "Eseguibile non trovato: $BIN"
ok "Build ok"

# ---------- 1) Export TXT robust (via selftest 1-sample) ----------
say "Export TXT (stdout) con --limit… (via selftest 1-sample)"
OUT_TXT="$TMP/out.txt"
LOG1="$TMP/log1.txt"
"$BIN" --selftest --cidr 10.0.0.0/24 --k 12 --sample 1 --export - --limit 1 1>"$OUT_TXT" 2>"$LOG1"

# Deve essere 1 riga con un solo token intero
[ "$(lines "$OUT_TXT")" -eq 1 ] || ko "TXT: attesa singola riga"
[ "$(tokens "$OUT_TXT")" -eq 1 ] || ko "TXT: atteso 1 token"

# Log [export] solido
assert_export_log_items "$LOG1" 1
ok "Export TXT ok"

# ---------- 2) Export CSV (file) con header ----------
say "Export CSV su file con header…"
OUT_CSV="$TMP/out.csv"
LOG2="$TMP/log2.txt"
"$BIN" --ip 10.0.0.1 --k 16 --export "$OUT_CSV" --export-format csv 1>/dev/null 2>"$LOG2"

# header + 1 riga
[ "$(lines "$OUT_CSV")" -eq 2 ] || ko "CSV: numero righe inatteso"
assert_csv_header "$OUT_CSV"
is_csv_row "$(tail -n1 "$OUT_CSV")" || ko "CSV: riga dati malformata"
assert_export_log_items "$LOG2" 1
ok "Export CSV ok"

# ---------- 3) Export CSV no-header + limit ----------
say "Export CSV senza header e --limit 5…"
OUT_CSV2="$TMP/out2.csv"
LOG3="$TMP/log3.txt"
"$BIN" --cidr 10.0.0.0/24 --k 12 --export "$OUT_CSV2" --export-format csv --no-header --limit 5 1>/dev/null 2>"$LOG3"
[ "$(lines "$OUT_CSV2")" -eq 5 ] || ko "CSV --limit: attese 5 righe"
# indicizzazione da 0
head -n1 "$OUT_CSV2" | awk '/^0,[0-9]+$/ { ok=1 } END{ exit ok?0:1 }' || ko "CSV --no-header: indice iniziale 0 mancante"
assert_export_log_items "$LOG3" 5
ok "CSV --no-header + --limit ok"

# ---------- 4) Determinismo vs preset diversity ----------
say "Determinismo (stesso comando → stesso export)…"
A="$TMP/det_a.txt"; B="$TMP/det_b.txt"
"$BIN" --cidr 192.168.1.0/28 --k 12 --preset default --export "$A" --export-format txt 1>/dev/null 2>/dev/null
"$BIN" --cidr 192.168.1.0/28 --k 12 --preset default --export "$B" --export-format txt 1>/dev/null 2>/dev/null
diff -q "$A" "$B" >/dev/null || ko "Non deterministico per stesso comando"
ok "Determinismo ok"

say "Preset diversi ⇒ sequenze diverse (statisticamente)…"
C="$TMP/p1.txt"; D="$TMP/p2.txt"
"$BIN" --cidr 10.0.0.0/20 --k 12 --preset default --export "$C" --export-format txt 1>/dev/null 2>/dev/null
"$BIN" --cidr 10.0.0.0/20 --k 12 --preset wang    --export "$D" --export-format txt 1>/dev/null 2>/dev/null
# se identici bit-a-bit è sospetto
if diff -q "$C" "$D" >/dev/null; then
  ko "I due preset hanno prodotto sequenze identiche (inaspettato)"
fi
ok "Preset differiscono"

# ---------- 5) Edge cases k ----------
say "k=0 → tutti bucket 0"
OUT0="$TMP/k0.txt"
"$BIN" --cidr 10.0.0.0/24 --k 0 --export "$OUT0" --export-format txt 1>/dev/null 2>/dev/null
# tutti token devono essere "0"
if tr -s ' ' '\n' < "$OUT0" | awk '/^0$/ {ok++} /^[^0]$/ {bad++} END{ exit (bad==0 && ok>0)?0:1 }'; then
  :
else
  ko "k=0: trovati bucket diversi da 0"
fi
ok "k=0 ok"

say "k=32 → shift nullo (CSV no-header) …"
OUT32="$TMP/k32.csv"
"$BIN" --cidr 10.0.0.0/30 --k 32 --export "$OUT32" --export-format csv --no-header 1>/dev/null 2>/dev/null
[ "$(lines "$OUT32")" -eq 4 ] || ko "k=32: attese 4 righe CSV"
is_csv_row "$(tail -n1 "$OUT32")" || ko "k=32: riga CSV malformata"
ok "k=32 ok"

# ---------- 6) Modalità stdin ----------
say "stdin: 3 IP → TSV (senza export)"
TSV="$TMP/stdin.tsv"
printf "1.1.1.1\n8.8.8.8\n10.0.0.1\n" | "$BIN" --stdin --k 12 > "$TSV"
[ "$(lines "$TSV")" -eq 3 ] || ko "stdin: attese 3 righe"
head -n1 "$TSV" | awk '/^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\t[0-9]+$/ {ok=1} END{exit ok?0:1}' \
  || ko "stdin: formato TSV inatteso"
ok "stdin (TSV) ok"

say "stdin + export CSV limit 2"
TSV2="$TMP/stdin2.csv"
printf "1.1.1.1\n8.8.8.8\n10.0.0.1\n" | "$BIN" --stdin --k 12 --export "$TSV2" --export-format csv --no-header --limit 2 1>/dev/null
[ "$(lines "$TSV2")" -eq 2 ] || ko "stdin+export: attese 2 righe CSV"
ok "stdin + export ok"

# ---------- 7) Selftest & Bench con export ----------
say "selftest con export (campione ridotto)…"
S1="$TMP/self.csv"
LOGS="$TMP/self.log"
"$BIN" --selftest --cidr 10.0.0.0/16 --k 10 --sample 4096 --export "$S1" --export-format csv 1>/dev/null 2>"$LOGS"
# header + 4096 righe
[ "$(lines "$S1")" -eq $((4096 + 1)) ] || ko "selftest export: attese 4096+header righe"
assert_csv_header "$S1"
assert_export_log_items "$LOGS" 4096
ok "selftest+export ok"

say "bench con export (N ridotto)…"
B1="$TMP/bench.txt"
LOGB="$TMP/bench.log"
"$BIN" --bench 1000000 --k 12 --preset wang --export "$B1" --export-format txt 1>/dev/null 2>"$LOGB"
awk '/^Benchmark Turbo-Bucketizer/ {ok=1} END{exit ok?0:1}' "$LOGB" || ko "bench: log mancante"
# deve contenere esattamente 1M token in TXT (una riga)
[ "$(tokens "$B1")" -eq 1000000 ] || ko "bench export: attesi 1,000,000 token"
assert_export_log_items "$LOGB" 1000000
ok "bench+export ok"

# ---------- 8) VLSM smoke-tests ----------
say "VLSM smoke-test (subnet diverse)…"
"$BIN" --cidr 10.0.0.0/24 --k 12 --export "$TMP/v1.txt" --export-format txt 1>/dev/null 2>/dev/null
"$BIN" --cidr 10.0.1.0/28 --k 12 --export "$TMP/v2.txt" --export-format txt 1>/dev/null 2>/dev/null
"$BIN" --cidr 10.0.2.0/30 --k 12 --export "$TMP/v3.txt" --export-format txt 1>/dev/null 2>/dev/null
[ -s "$TMP/v1.txt" ] && [ -s "$TMP/v2.txt" ] && [ -s "$TMP/v3.txt" ] || ko "VLSM: files export vuoti"
ok "VLSM ok"

# ---------- 9) Optional: digit_probe hook ----------
if [ -n "${DIGIT_PROBE:-}" ] && command -v python3 >/dev/null 2>&1; then
  say "digit_probe.py: analisi rapida (opzionale)…"
  "$BIN" --selftest --cidr 10.0.0.0/16 --k 12 --sample 10000 --export - \
    | python3 "$DIGIT_PROBE" --mode schur --radix 10 >/dev/null \
    || ko "digit_probe: esito non OK"
  ok "digit_probe hook ok"
else
  say "digit_probe.py non configurato (salto test opzionale)."
fi

ok "TUTTI I TEST SONO PASSATI 🎉"
