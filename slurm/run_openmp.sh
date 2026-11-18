#!/bin/bash
# OpenMP strong scaling on one node: 1, 2, 4, 8, ... threads up to --cpus-per-task.
# Submit from the repository root (after `make`):  sbatch slurm/run_openmp.sh
# More CPUs:   sbatch --cpus-per-task=32 slurm/run_openmp.sh   (check `sinfo -N -o "%N %P %c"` first)
# Other size:  N=512 sbatch slurm/run_openmp.sh       Other binding:  BIND=spread sbatch slurm/run_openmp.sh
#SBATCH --job-name=openmp
#SBATCH --output=results/logs/%x_%j.log
#SBATCH --partition=cpu
#SBATCH --time=00:30:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=4G

cd "$SLURM_SUBMIT_DIR"
source slurm/env.sh

N=${N:-256}
STEPS=${STEPS:-100}
REPS=${REPS:-3}
MAXT=${SLURM_CPUS_PER_TASK:-4}
export OMP_PLACES=${PLACES:-cores}
export OMP_PROC_BIND=${BIND:-close}
echo "=== max threads: $MAXT  OMP_PLACES=$OMP_PLACES  OMP_PROC_BIND=$OMP_PROC_BIND"

THREADS=""; t=1
while [ $t -lt $MAXT ]; do THREADS="$THREADS $t"; t=$((t * 2)); done
THREADS="$THREADS $MAXT"

for T in $THREADS; do
  for r in $(seq $REPS); do
    echo "--- openmp N=$N threads=$T rep=$r"
    OMP_NUM_THREADS=$T ./bin/heat_omp $N $N $N $STEPS
  done
done
