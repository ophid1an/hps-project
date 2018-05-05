CC = gcc
CFLAGS = -std=c99 -pedantic-errors -O3 -Wall -Wextra
LIBS = -lm

SRCDIR = ./src
BINDIR = ./bin

_SRC = main.c hashing.c xxhash.c hll.c hllpp.c sorting.c
SRC = $(patsubst %,$(SRCDIR)/%,$(_SRC))

hll: $(SRC)
	$(CC) -o $(BINDIR)/$@ $^ $(CFLAGS) $(LIBS)

$(shell mkdir -p $(BINDIR))