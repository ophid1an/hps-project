CC = gcc
CFLAGS = -std=c99 -pedantic-errors -O3 -Wall -Wextra
LIBS_OMP = -lm -fopenmp

SRCDIR = ./src
BINDIR = ./bin

_SRC_OMP = main_omp.c sorting.c hllpp_omp.c xxhash.c
SRC_OMP = $(patsubst %,$(SRCDIR)/%,$(_SRC_OMP))

hllpp_omp: $(SRC_OMP)
	$(CC) -o $(BINDIR)/$@ $^ $(CFLAGS) $(LIBS_OMP)

$(shell mkdir -p $(BINDIR))