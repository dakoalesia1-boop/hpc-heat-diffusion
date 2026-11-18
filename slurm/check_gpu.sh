#!/bin/bash
# Checks that a GPU can be allocated and which GPU compilers exist (for the OpenACC/CUDA experiment).
# Submit from the repository root:  sbatch slurm/check_gpu.sh
#SBATCH --job-name=gpucheck
#SBATCH --output=results/logs/%x_%j.log
#SBATCH --partition=gpu
#SBATCH --gpus=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=2
#SBATCH --mem=4G
#SBATCH --time=00:05:00

cd "$SLURM_SUBMIT_DIR"
echo "=== job $SLURM_JOB_ID on $(hostname)"
nvidia-smi --query-gpu=name,memory.total --format=csv,noheader || echo "nvidia-smi not available"
echo "--- compilers on the node:"
for c in nvc nvcc pgcc; do command -v $c || echo "$c: not found"; done
echo "--- GPU-related modules:"
module avail 2>&1 | grep -iE "nvhpc|cuda|pgi|nvidia" || echo "no NVHPC/CUDA modules found"
echo "--- course CUDA container (used by the Practice 2 GPU job):"
ls -l /data/software/containers/ 2>&1 | head -20
if command -v apptainer >/dev/null && [ -r /data/software/containers/hpc-course-cuda.sif ]; then
  apptainer exec --nv /data/software/containers/hpc-course-cuda.sif bash -lc 'nvcc --version | tail -2; command -v nvc || echo "nvc (OpenACC) not in container"'
fi
