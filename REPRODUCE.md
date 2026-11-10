# REPRODUCE.md

## Project
3D Heat Diffusion HPC Project

## Goal
This project implements a 3D heat diffusion simulation and measures the performance of serial, OpenMP, MPI, and accelerator-based versions on the SciTech HPC cluster.

## Repository Structure
- `src/` — source code
- `slurm/` — Slurm submission scripts
- `results/` — output logs, CSV files, and plots
- `docs/` — report, slides, and scale-up plan
- `data/` — small input data or data-generation scripts
- `env/` — environment and dependency notes

## Prerequisites
### Local development
- GCC or Clang
- C standard library
- Math library (`-lm`)

### On the cluster
- Slurm
- C compiler module
- OpenMP-enabled compiler for parallel versions
- MPI module for MPI version
- CUDA/OpenACC tools if used for accelerator experiments

## Build Instructions

### Serial version
From the repository root:

```bash
gcc -O3 src/heat_diffusion_serial.c -o heat_serial -lm