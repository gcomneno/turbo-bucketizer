# --- build/Makefile ---
CC       ?= gcc
CFLAGS   ?= -O3 -march=native -mtune=native -Wall -Wextra -std=c11
LDFLAGS  ?=

SRC      := ../src/main.c ../src/turbok_core.c ../src/tb_export.c
INC      := -I../include
BIN      := ../turbo-bucketizer
TMP		 := ../.tmp_tests
DISTDIR  := ../dist
TAGDIR   := $(DISTDIR)/tag_$(VERSION)

VERSION  := v0.1

.PHONY: all clean distclean selftest bench export-demo test release valgrind

# -------------------------------
all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(INC) -o $@ $^ $(LDFLAGS)
	@echo "✅ Build completata: $@"

clean:
	rm -f $(BIN)
	rm -Rf $(TMP)
	@echo "🧹 Pulito!"

distclean: clean
	rm -rf $(DISTDIR)
	@echo "🧹 Pulizia profonda dist/"

# -------------------------------
selftest: $(BIN)
	$(BIN) --selftest --cidr 10.0.0.0/8 --k 12 --sample 1000000 --preset default

bench: $(BIN)
	$(BIN) --bench 20000000 --k 12 --preset wang

# Verifica export (TXT su stdout, limit 50)
exportdemo: $(BIN)
	$(BIN) --selftest --cidr 10.0.0.0/8 --k 12 --sample 100000 --preset default --export - --limit 50 1>/dev/null

# -------------------------------
# Test globale
test: $(BIN)
	@echo "🧪 Avvio test suite…"
	./run_tests.sh

valgrind: $(BIN)
	valgrind --quiet --leak-check=full --error-exitcode=1 \
	  $(BIN) --selftest --cidr 10.0.0.0/16 --k 12 --sample 4096 --export - >/dev/null
	@echo "✅ Valgrind OK"
