#!/bin/bash
# Records what a compute node looks like (cores, sockets, caches, memory) into env/.
# Submit from the repository root:  sbatch slurm/hardware.sh
#SBATCH --job-name=hardware
#SBATCH --output=results/logs/%x_%j.log
#SBATCH --partition=cpu
#SBATCH --time=00:05:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=1G

cd "$SLURM_SUBMIT_DIR"
source slurm/env.sh

lscpu                                  > env/lscpu.txt
free -g                                > env/memory.txt
nproc --all                            > env/nproc.txt
sinfo -N -o "%N %P %c %m %G %T"        > env/sinfo.txt
module list 2>&1                       > env/modules.txt
gcc --version | head -1                > env/compiler_version.txt
mpicc --version 2>&1 | head -1        >> env/compiler_version.txt
mpirun --version 2>&1 | head -1       >> env/compiler_version.txt
echo "Saved env/lscpu.txt, env/memory.txt, env/nproc.txt, env/sinfo.txt, env/modules.txt, env/compiler_version.txt"
grep -E "^(Model name|Socket|Core|Thread|CPU\(s\)|NUMA node\(s\)|L1d|L2|L3)" env/lscpu.txt
cat env/sinfo.txt
