#!/bin/bash
# Correctness check: serial, OpenMP and MPI must produce the SAME temperature field.
# Submit from the repository root (after `make`):  sbatch slurm/validate.sh
#SBATCH --job-name=validate
#SBATCH --output=results/logs/%x_%j.log
#SBATCH --partition=cpu
#SBATCH --time=00:10:00
#SBATCH --nodes=1
#SBATCH --ntasks=4
#SBATCH --cpus-per-task=1
#SBATCH --mem=2G

cd "$SLURM_SUBMIT_DIR"
source slurm/env.sh

N=64; STEPS=200; ALPHA=0.1
OUT=results/validation; mkdir -p $OUT

./bin/heat_serial $N $N $N $STEPS $ALPHA $OUT/serial.raw
OMP_NUM_THREADS=4 ./bin/heat_omp $N $N $N $STEPS $ALPHA $OUT/omp.raw
$MPIRUN -np 4 ./bin/heat_mpi $N $N $N $STEPS $ALPHA $OUT/mpi.raw

status=0
./bin/compare_fields $OUT/serial.raw $OUT/omp.raw || status=1
./bin/compare_fields $OUT/serial.raw $OUT/mpi.raw || status=1
[ $status -eq 0 ] && echo "ALL VERSIONS MATCH SERIAL" || echo "MISMATCH - do not use these versions for measurements"
exit $status
