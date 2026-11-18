#!/bin/bash
# Memory bandwidth ceiling (STREAM) with the same CPUs as the OpenMP runs.
#SBATCH --job-name=stream
#SBATCH --output=results/logs/%x_%j.log
#SBATCH --partition=cpu
#SBATCH --nodelist=julia
#SBATCH --time=00:20:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=64
#SBATCH --mem=8G

cd "$SLURM_SUBMIT_DIR"
source slurm/env.sh
export OMP_PLACES=cores
export OMP_PROC_BIND=${BIND:-spread}

for T in 1 2 4 8 16 32 64; do
  echo "--- stream threads=$T"
  OMP_NUM_THREADS=$T ./bin/stream | grep -E "Copy|Triad"
done
