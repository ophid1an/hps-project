CC = gcc
CFLAGS = -std=c99 -pedantic-errors -O3 -Wall -Wextra
LIBS = -lm -fopenmp

SRCDIR = ./src
BINDIR = ./bin

_SRC = main.c sorting.c hllpp_omp.c xxhash.c
SRC = $(patsubst %,$(SRCDIR)/%,$(_SRC))

hll: $(SRC)
	$(CC) -o $(BINDIR)/$@ $^ $(CFLAGS) $(LIBS)

$(shell mkdir -p $(BINDIR))