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
- [ ] Serial baseline implementation
- [ ] OpenMP parallel version
- [ ] MPI parallel version
- [ ] Accelerator experiments
- [ ] Performance experiments
- [ ] Profiling and optimization
- [ ] Final report and presentation

## How to Build and Run
Build and run instructions will be added here as the implementation develops.

## Notes
This project focuses on experimentation and performance analysis, not just final runtime.  
We will compare serial and parallel versions, test scalability, and explain bottlenecks and optimization results.