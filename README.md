# 3D Heat Diffusion HPC Project

## Project Overview
This repository contains our group project for the High-Level Computing course.  
We are developing a 3D heat diffusion application that simulates temperature spread over a volume using finite difference stencil updates.

The main goal is to study how different HPC techniques affect performance, scalability, and resource usage on the SciTech HPC cluster.

## Group Members
- Alesia Dako
- Andrés Befeler Mizrahi
- Kohana Rakipi
- Jose Miguel Reyes Alegria
- Viktoriia Stepanenko
- Alejandro Zapata Pinto

## Objectives
- Build a correct serial baseline
- Parallelize the application using OpenMP and MPI
- Experiment with an additional accelerator technology such as CUDA or OpenACC
- Measure runtime, speedup, efficiency, and scalability
- Profile the application and identify bottlenecks
- Implement at least one meaningful optimization
- Prepare a report and EuroHPC scale-up plan

## HPC Technologies
Planned technologies for this project:
- C
- OpenMP
- MPI
- CUDA or OpenACC
- Slurm for job submission

## Repository Structure
- `src/` — source code
- `slurm/` — Slurm batch scripts
- `results/` — logs, CSV files, and plots
- `docs/` — report, slides, and scale-up plan
- `data/` — input data or data-generation scripts
- `env/` — environment, module, and dependency notes

## Reproducibility
The repository includes the code, build instructions, Slurm scripts, and experiment notes needed to reproduce the main results.

See `REPRODUCE.md` for the exact commands and environment setup.

## Current Status
- [x] Repository created
- [x] Folder structure set up
- [x] Shared core (`src/heat3d.h/.c`) + Makefile
- [x] Serial baseline implementation (`src/heat_serial.c`)
- [x] OpenMP version (`src/heat_omp.c`) - first version, validated locally
- [x] MPI version (`src/heat_mpi.c`) - slab decomposition + halo exchange, validated locally
- [x] Slurm scripts: hardware, validate, serial, OpenMP, MPI, multi-node and GPU checks
- [x] Result collection and plotting scripts (`scripts/`)
- [ ] Runs on SciTech (validation, baseline, scaling)
- [ ] Hybrid MPI+OpenMP version
- [ ] Accelerator experiments (OpenACC / CUDA)
- [ ] Profiling and optimization
- [ ] Final report, EuroHPC plan and video

## Quick start (on SciTech, from the repository root)
```bash
source slurm/env.sh          # load compiler + MPI modules
make                         # build everything into bin/
sbatch slurm/validate.sh     # correctness: all versions must match serial
sbatch slurm/run_serial.sh   # baseline
sbatch slurm/run_openmp.sh   # OpenMP thread scaling
sbatch slurm/run_mpi.sh      # MPI scaling on one node
python3 scripts/collect.py && python3 scripts/plot.py
```
Full details: `REPRODUCE.md`.

## Code layout
| File | What it is |
| --- | --- |
| `src/heat3d.h/.c` | Shared core: grid layout, initial condition, stencil, timers, RESULT line |
| `src/heat_serial.c` | Reference version (1 core) |
| `src/heat_omp.c` | OpenMP version (threads on one node) |
| `src/heat_mpi.c` | MPI version (z-slabs + halo exchange, one or more nodes) |
| `src/heat_diffusion_serial.c` | First prototype, kept for history (not used by the Makefile) |
| `tools/compare_fields.c` | Checks two dumped fields are identical |
| `slurm/` | One script per experiment; `env.sh` holds the modules |
| `scripts/` | `collect.py` (logs -> CSV), `plot.py` (CSV -> plots) |

## Notes
This project focuses on experimentation and performance analysis, not just final runtime.  
We will compare serial and parallel versions, test scalability, and explain bottlenecks and optimization results.