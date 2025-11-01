CC ?= gcc
CFLAGS ?= -O3 -march=native -mtune=native -Wall -Wextra -std=c11
LDFLAGS ?=
SRC := ../src/main.c ../src/turbok_core.c
INC := -I../include
BIN := ../turbo-bucketizer

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(INC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(BIN)

selftest: $(BIN)
	../turbo-bucketizer --selftest --cidr 10.0.0.0/8 --k 12 --sample 1000000 --preset default

bench: $(BIN)
	../turbo-bucketizer --bench 20000000 --k 12 --preset wang

.PHONY: all clean selftest bench
