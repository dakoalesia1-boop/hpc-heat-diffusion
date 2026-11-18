#!/bin/bash
# MPI strong scaling on one node: 1, 2, 4, 8, ... ranks up to --ntasks.
# Submit from the repository root (after `make`):  sbatch slurm/run_mpi.sh
# More ranks:  sbatch --ntasks=32 slurm/run_mpi.sh   (check `sinfo -N -o "%N %P %c"` first)
#SBATCH --job-name=mpi
#SBATCH --output=results/logs/%x_%j.log
#SBATCH --partition=cpu
#SBATCH --time=00:30:00
#SBATCH --nodes=1
#SBATCH --ntasks=16
#SBATCH --cpus-per-task=1
#SBATCH --mem=8G

cd "$SLURM_SUBMIT_DIR"
source slurm/env.sh

N=${N:-256}
STEPS=${STEPS:-100}
REPS=${REPS:-3}
MAXP=${SLURM_NTASKS:-4}

RANKS=${RANKS:-}
if [ -z "$RANKS" ]; then
  p=1; while [ $p -lt $MAXP ]; do RANKS="$RANKS $p"; p=$((p * 2)); done
  RANKS="$RANKS $MAXP"
fi

for P in $RANKS; do
  for r in $(seq $REPS); do
    echo "--- mpi N=$N ranks=$P rep=$r"
    $MPIRUN -np $P ./bin/heat_mpi $N $N $N $STEPS
  done
done
