CC = gcc
MPICC = mpicc
CFLAGS = -std=c99 -pedantic-errors -O3 -Wall -Wextra
LIBS_OMP = -lm -fopenmp
LIBS_MPI = -lm

SRCDIR = ./src
BINDIR = ./bin

_SRC_OMP = main_omp.c hllpp_omp.c xxhash.c
SRC_OMP = $(patsubst %,$(SRCDIR)/%,$(_SRC_OMP))

_SRC_MPI = main_mpi.c hllpp_mpi.c xxhash.c
SRC_MPI = $(patsubst %,$(SRCDIR)/%,$(_SRC_MPI))

hllpp_omp: $(SRC_OMP)
	$(CC) -o $(BINDIR)/$@ $^ $(CFLAGS) $(LIBS_OMP)

hllpp_mpi: $(SRC_MPI)
	$(MPICC) -o $(BINDIR)/$@ $^ $(CFLAGS) $(LIBS_MPI)

$(shell mkdir -p $(BINDIR))