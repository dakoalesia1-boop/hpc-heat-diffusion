#!/bin/bash
# Checks whether MPI can run across TWO nodes on SciTech.
# The course slides list multi-node as "needs validation" and the course MPI jobs run on one node
# only, so a failure here is a real finding (write it down), not a bug in our code.
# Submit from the repository root (after `make`):  sbatch slurm/check_multinode.sh
#SBATCH --job-name=multinode
#SBATCH --output=results/logs/%x_%j.log
#SBATCH --partition=cpu
#SBATCH --time=00:10:00
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=2
#SBATCH --cpus-per-task=1
#SBATCH --mem=2G

cd "$SLURM_SUBMIT_DIR"
source slurm/env.sh

echo "--- allocated nodes: $SLURM_JOB_NODELIST"
echo "--- test 1: srun across both nodes"
srun hostname 2>&1 | sort | uniq -c || echo "srun failed"
echo "--- test 2: mpirun with the Slurm launcher (spans nodes if srun works)"
mpirun -np 4 --bind-to none --mca pml ob1 --mca btl self,vader,tcp ./bin/heat_mpi 128 128 128 100 2>&1 | tail -5
echo "--- test 3: mpirun with an explicit host list (uses ssh between nodes)"
scontrol show hostnames "$SLURM_JOB_NODELIST" | sed 's/$/ slots=2/' > results/logs/hosts_$SLURM_JOB_ID.txt
$MPIRUN -np 4 --hostfile results/logs/hosts_$SLURM_JOB_ID.txt ./bin/heat_mpi 128 128 128 100 2>&1 | tail -5
