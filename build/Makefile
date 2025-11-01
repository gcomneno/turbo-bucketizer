# --- build/Makefile ---
CC       ?= gcc
CFLAGS   ?= -O3 -march=native -mtune=native -Wall -Wextra -std=c11
LDFLAGS  ?=

SRC      := ../src/main.c ../src/turbok_core.c
INC      := -I../include
BIN      := ../turbo-bucketizer
DISTDIR  := ../dist
TAGDIR   := $(DISTDIR)/tag_$(VERSION)

VERSION  := v0.1

.PHONY: all clean selftest bench release

# -------------------------------
all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(INC) -o $@ $^ $(LDFLAGS)
	@echo "✅ Build completata: $@"

clean:
	rm -f $(BIN)
	@echo "🧹 Pulito!"

# -------------------------------
selftest: $(BIN)
	$(BIN) --selftest --cidr 10.0.0.0/8 --k 12 --sample 1000000 --preset default

bench: $(BIN)
	$(BIN) --bench 20000000 --k 12 --preset wang

# -------------------------------
release: all
	@echo "📦 Creo pacchetto release $(VERSION)..."
	mkdir -p $(TAGDIR)
	cp $(BIN) ../README.md ../LICENSE ../preset_table.tsv $(TAGDIR)/
	strip $(TAGDIR)/$(notdir $(BIN))
	cd $(TAGDIR) && zip -9 turbo-bucketizer-linux-x86_64-$(VERSION).zip * >/dev/null
	cd $(TAGDIR) && sha256sum turbo-bucketizer-linux-x86_64-$(VERSION).zip > turbo-bucketizer-linux-x86_64-$(VERSION).zip.sha256
	@echo "✅ Pacchetto pronto in $(TAGDIR)"
