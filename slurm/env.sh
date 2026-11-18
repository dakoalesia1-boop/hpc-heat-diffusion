# Sourced by every Slurm script. Change modules HERE only, so all jobs use the same environment.
# SciTech CPU stack used by the course jobs (gcc + OpenMPI in one coherent toolchain).
module purge >/dev/null 2>&1
module load foss/2023b

# How MPI programs are started on SciTech. The compute nodes do not expose srun to MPI, so
# OpenMPI's Slurm launcher is switched off and ranks are started inside the allocated node
# (same settings as the course's Practice 2 jobs). --bind-to none avoids the core-binding
# error seen after the cluster's CPU-limit change.
MPIRUN="mpirun --mca plm ^slurm --bind-to none --mca pml ob1 --mca btl self,vader,tcp"

# Header written at the top of every job log (needed for reproducibility).
echo "=== job ${SLURM_JOB_ID:-local} (${SLURM_JOB_NAME:-}) on $(hostname) at $(date -Iseconds)"
echo "=== git commit: $(git rev-parse --short HEAD 2>/dev/null || echo unknown)"
echo "=== partition: ${SLURM_JOB_PARTITION:-n/a}  nodes: ${SLURM_JOB_NODELIST:-n/a}  ntasks: ${SLURM_NTASKS:-n/a}  cpus_per_task: ${SLURM_CPUS_PER_TASK:-n/a}"
echo "=== allowed cpus: $(taskset -cp $$ 2>/dev/null | sed 's/.*: //')"
echo "=== gcc: $(gcc --version | head -1)"
module list 2>&1 | sed 's/^/=== /'
