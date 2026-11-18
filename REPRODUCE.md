# REPRODUCE.md

How to reproduce our experiments on the SciTech cluster. Run every command from the repository root.

## 1. Connect and get the code

```bash
# from outside campus
ssh -J <username>@ssh.iesci.tech <username>@10.205.20.10
# from the campus network
ssh <username>@10.205.20.10

git clone https://github.com/dakoalesia1-boop/hpc-heat-diffusion.git
cd hpc-heat-diffusion
```

## 2. Environment

All Slurm jobs load the same modules from `slurm/env.sh` (currently `foss/2023b`: GCC + OpenMPI)
and run on the `cpu` partition (GPU checks on `gpu`). Load the same modules before compiling:

```bash
source slurm/env.sh
```

## 3. Build

```bash
make            # builds bin/heat_serial, bin/heat_omp, bin/heat_mpi, bin/compare_fields
```

Compiler flags are in the `Makefile` and recorded in `env/compiler_flags.txt`.

## 4. Command line (same for every version)

```
./bin/heat_<version> nx ny nz steps [alpha] [dumpfile]
```

- `nx ny nz`: grid size including the fixed boundary layer (default 128 128 128)
- `steps`: number of time steps (default 100)
- `alpha`: diffusion number, must be <= 1/6 (default 0.1)
- `dumpfile`: optional; writes the final field as raw doubles plus `<dumpfile>.vtk` for ParaView (small grids only)

Every run prints one line that the scripts collect:

```
RESULT,version,nx,ny,nz,steps,nodes,ranks,threads,gpus,total_s,compute_s,comm_s,checksum,center,mlups
```

`total_s` times only the time-step loop. `mlups` = million cell updates per second.
There is no randomness; results are deterministic.

## 5. Experiments (all through Slurm)

| Step | Command | What it produces |
| --- | --- | --- |
| Hardware info | `sbatch slurm/hardware.sh` | `env/lscpu.txt`, `env/sinfo.txt`, `env/modules.txt`, ... |
| Correctness | `sbatch slurm/validate.sh` | Log says `ALL VERSIONS MATCH SERIAL` |
| Serial baseline | `sbatch slurm/run_serial.sh` | 128^3, 256^3, 512^3, 3 repetitions each |
| OpenMP scaling | `sbatch slurm/run_openmp.sh` | 1, 2, 4, ... threads up to all cores, 256^3 |
| MPI scaling (1 node) | `sbatch slurm/run_mpi.sh` | 1, 2, 4, 8, 16 ranks, 256^3 |
| Multi-node check | `sbatch slurm/check_multinode.sh` | Whether MPI can span 2 nodes on SciTech |
| GPU check | `sbatch slurm/check_gpu.sh` | `nvidia-smi` output and available GPU compilers |

MPI programs are started with `mpirun --mca plm ^slurm --bind-to none ...` (variable `MPIRUN` in
`slurm/env.sh`), the same launch settings as the course's SciTech jobs: the compute nodes do not
expose `srun` to MPI, and `--bind-to core` fails under the cluster's CPU limits.

More CPUs or ranks: `sbatch --cpus-per-task=32 slurm/run_openmp.sh`, `sbatch --ntasks=32 slurm/run_mpi.sh`
(check the CPUs per node with `sinfo -N -o "%N %P %c %m"` first).

Variables change a run without editing the script, for example
`N=512 BIND=spread sbatch slurm/run_openmp.sh` or `RANKS="1 4 16" sbatch slurm/run_mpi.sh`.

Logs go to `results/logs/<jobname>_<jobid>.log`. Each log starts with the job ID, node, git commit and modules.

## 6. Results and plots

```bash
python3 scripts/collect.py     # all RESULT lines -> results/csv/results.csv
python3 scripts/plot.py        # speedup/efficiency plots -> results/plots/, summary table on screen
```

`plot.py` needs matplotlib (install with `pip install --user matplotlib` if it is missing,
or copy `results/csv/results.csv` to a laptop and run it there).

## 7. Final results

The numbers in the report come from git tag `v1.0-final` (to be created at code freeze).
