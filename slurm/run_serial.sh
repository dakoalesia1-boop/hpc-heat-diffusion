#!/bin/bash
#SBATCH --job-name=heat-serial
#SBATCH --output=../results/serial_%j.out
#SBATCH --error=../results/serial_%j.err
#SBATCH --time=00:05:00
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=1G

cd "$SLURM_SUBMIT_DIR"

./heat_serial 64 64 64 50