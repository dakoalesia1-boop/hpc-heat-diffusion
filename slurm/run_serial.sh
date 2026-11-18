#!/bin/bash
# Baseline: one core, three problem sizes, three repetitions each.
# Submit from the repository root (after `make`):  sbatch slurm/run_serial.sh
#SBATCH --job-name=serial
#SBATCH --output=results/logs/%x_%j.log
#SBATCH --partition=cpu
#SBATCH --time=00:30:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=6G

cd "$SLURM_SUBMIT_DIR"
source slurm/env.sh

STEPS=${STEPS:-100}
REPS=${REPS:-3}
for N in ${SIZES:-128 256 512}; do
  for r in $(seq $REPS); do
    echo "--- serial N=$N steps=$STEPS rep=$r"
    ./bin/heat_serial $N $N $N $STEPS
  done
done
