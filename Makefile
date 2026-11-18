# Build every version of the 3D heat diffusion solver.
#   make            -> serial, openmp, mpi, compare tool
#   make serial     -> bin/heat_serial
#   make omp        -> bin/heat_omp
#   make mpi        -> bin/heat_mpi      (needs mpicc: module load OpenMPI/...)
#   make clean
#   make ARCH=-march=native   -> tune for the CPU that compiles (only if login and compute nodes have the same CPU)
# Record any flag change in env/compiler_flags.txt.

CC      = gcc
MPICC   = mpicc
ARCH    ?=
CFLAGS  ?= -O3 $(ARCH) -std=c11 -Wall -Wextra
OMPFLAG ?= -fopenmp
LDLIBS  = -lm

BIN  = bin
CORE = src/heat3d.c src/heat3d.h

.PHONY: all serial omp mpi tools clean

all: serial omp mpi tools

serial: $(BIN)/heat_serial
omp:    $(BIN)/heat_omp
mpi:    $(BIN)/heat_mpi
tools:  $(BIN)/compare_fields

$(BIN):
	mkdir -p $(BIN)

$(BIN)/heat_serial: src/heat_serial.c $(CORE) | $(BIN)
	$(CC) $(CFLAGS) -o $@ src/heat_serial.c src/heat3d.c $(LDLIBS)

$(BIN)/heat_omp: src/heat_omp.c $(CORE) | $(BIN)
	$(CC) $(CFLAGS) $(OMPFLAG) -o $@ src/heat_omp.c src/heat3d.c $(LDLIBS)

$(BIN)/heat_mpi: src/heat_mpi.c $(CORE) | $(BIN)
	$(MPICC) $(CFLAGS) -o $@ src/heat_mpi.c src/heat3d.c $(LDLIBS)

$(BIN)/compare_fields: tools/compare_fields.c | $(BIN)
	$(CC) $(CFLAGS) -o $@ tools/compare_fields.c $(LDLIBS)

clean:
	rm -rf $(BIN)
