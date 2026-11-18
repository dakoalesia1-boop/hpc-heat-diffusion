#!/bin/bash
#SBATCH --job-name=profile
#SBATCH --partition=cpu
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=2
#SBATCH --time=00:30:00
#SBATCH --output=results/logs/profile_%j.log
# Profiling of the serial solver: gprof (where the time goes) and Valgrind cachegrind (cache behaviour).
# Hardware counters (perf, PAPI) are blocked on SciTech: perf_event_paranoid = 4.
source slurm/env.sh
echo "=== perf_event_paranoid: $(cat /proc/sys/kernel/perf_event_paranoid)"
OUT=results/profiling
mkdir -p $OUT

echo "--- gprof: 256^3, 100 steps"
gcc -O3 -fno-inline -pg -std=c11 -o bin/heat_serial_pg src/heat_serial.c src/heat3d.c -lm
./bin/heat_serial_pg 256 256 256 100 0.1
gprof -b -p bin/heat_serial_pg gmon.out > $OUT/gprof_serial_256.txt
head -15 $OUT/gprof_serial_256.txt
rm -f gmon.out

echo "--- cachegrind: 256^3, 5 steps, last-level cache 32 MB"
module load Valgrind/3.23.0-gompi-2023b
valgrind --tool=cachegrind --cache-sim=yes --LL=33554432,16,64 \
  --cachegrind-out-file=$OUT/cachegrind_serial_256.out \
  ./bin/heat_serial 256 256 256 5 0.1 2>&1 | grep -E "RESULT|refs|misses|miss rate"
