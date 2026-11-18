#!/usr/bin/env python3
"""Turn results/csv/results.csv into the tables and plots for the slides and report.

Usage (from the repository root, needs matplotlib):
    python3 scripts/plot.py            # one size per version is picked automatically
    python3 scripts/plot.py 512        # use grid size 512^3

Writes results/plots/*.png and prints a summary table (median of repetitions).
Speedup = serial time / parallel time at the same grid size. Efficiency = speedup / workers.
"""
import csv
import os
import statistics
import sys
from collections import defaultdict

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402

CSV = "results/csv/results.csv"
OUT = "results/plots"
STREAM_GBS = 89.5       # best STREAM Triad bandwidth on julia (GB/s); update from the newest stream log
BYTES_PER_UPDATE = 16   # one 8-byte read + one 8-byte write per cell update (ideal caching)


def load():
    with open(CSV) as f:
        return list(csv.DictReader(f))


def median_times(rows, version, n, key, bind=None):
    """Median total time per worker count (key = 'threads' or 'ranks') for one version and grid size."""
    groups = defaultdict(list)
    for r in rows:
        if r["version"] == version and int(r["nx"]) == n and (bind is None or r.get("bind", "") == bind):
            groups[int(r[key])].append(float(r["total_s"]))
    return {k: statistics.median(v) for k, v in sorted(groups.items())}


def comm_share(rows, n):
    groups = defaultdict(list)
    for r in rows:
        if r["version"] == "mpi" and int(r["nx"]) == n and float(r["total_s"]) > 0:
            groups[int(r["ranks"])].append(float(r["comm_s"]) / float(r["total_s"]))
    return {k: statistics.median(v) for k, v in sorted(groups.items())}


def scaling_plot(name, label, times, t_ref):
    workers = list(times)
    speedup = [t_ref / times[p] for p in workers]
    eff = [s / p for s, p in zip(speedup, workers)]
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(10, 4))
    ax1.plot(workers, speedup, "o-", label="measured")
    ax1.plot(workers, workers, "--", color="gray", label="ideal")
    ax1.set_xscale("log", base=2); ax1.set_yscale("log", base=2)
    ax1.set_xlabel(label); ax1.set_ylabel("speedup vs serial"); ax1.legend(); ax1.grid(alpha=0.3)
    ax2.plot(workers, [100 * e for e in eff], "o-")
    ax2.set_xscale("log", base=2); ax2.set_ylim(0, 110)
    ax2.set_xlabel(label); ax2.set_ylabel("parallel efficiency (%)"); ax2.grid(alpha=0.3)
    for ax in (ax1, ax2):
        ax.set_xticks(workers)
        ax.set_xticklabels([str(p) for p in workers])
    ax1.set_yticks(workers)
    ax1.set_yticklabels([str(p) for p in workers])
    fig.suptitle(name)
    fig.tight_layout()
    return fig, speedup, eff


def main():
    rows = load()
    os.makedirs(OUT, exist_ok=True)
    sizes = sorted({int(r["nx"]) for r in rows})
    want = int(sys.argv[1]) if len(sys.argv) > 1 else None

    serial = {n: statistics.median([float(r["total_s"]) for r in rows
                                    if r["version"] == "serial" and int(r["nx"]) == n])
              for n in sizes if any(r["version"] == "serial" and int(r["nx"]) == n for r in rows)}
    print("\nSerial baseline (median of repetitions)")
    for n, t in serial.items():
        print(f"  {n:4d}^3  {t:9.3f} s")

    series = {}
    for version, key, label, bind in [("openmp", "threads", "OpenMP threads", "close"),
                                      ("openmp", "threads", "OpenMP threads", "spread"),
                                      ("mpi", "ranks", "MPI ranks", None)]:
        tag = f"{version}_{bind}" if bind else version
        cand = [n for n in sizes if median_times(rows, version, n, key, bind)]
        if not cand:
            continue
        n = want if want in cand else max(cand)
        times = median_times(rows, version, n, key, bind)
        t_ref = serial.get(n, times.get(1))
        if t_ref is None:
            print(f"no serial or 1-worker run for {version} at {n}^3; skipping")
            continue
        fig, sp, ef = scaling_plot(f"{tag} strong scaling, {n}^3 grid", label, times, t_ref)
        path = f"{OUT}/{tag}_scaling_{n}.png"
        fig.savefig(path, dpi=150)
        plt.close(fig)
        series[tag] = (n, times, t_ref)
        print(f"\n{tag} at {n}^3 (reference {t_ref:.3f} s) -> {path}")
        print(f"  {label:>15}  time_s   speedup  efficiency")
        for (p, t), s, e in zip(times.items(), sp, ef):
            print(f"  {p:15d}  {t:7.3f}  {s:7.2f}  {100 * e:6.1f}%")
        if version == "mpi":
            for p, c in comm_share(rows, n).items():
                print(f"  ranks={p:3d}: communication = {100 * c:5.1f}% of runtime")
    if series:
        n0 = next(iter(series.values()))[0]
        ser = [float(r["mlups"]) for r in rows if r["version"] == "serial" and int(r["nx"]) == n0]
        fig, ax = plt.subplots(figsize=(7, 4.5))
        ticks = set()
        for tag, (n, times, t_ref) in series.items():
            ps = list(times)
            ticks.update(ps)
            ax.plot(ps, [t_ref / times[p] for p in ps], "o-", label=f"{tag}, {n}^3")
        ticks = sorted(ticks)
        ax.plot(ticks, ticks, "--", color="gray", label="ideal")
        if ser:
            bound = STREAM_GBS * 1e3 / BYTES_PER_UPDATE / statistics.median(ser)
            ax.axhline(bound, color="red", linestyle=":", label=f"memory-bandwidth limit ({STREAM_GBS} GB/s STREAM)")
            print(f"\nmemory-bandwidth speedup limit at {n0}^3: {bound:.2f}x")
        ax.set_xscale("log", base=2)
        ax.set_yscale("log", base=2)
        ax.set_xticks(ticks)
        ax.set_xticklabels([str(p) for p in ticks])
        ax.set_yticks(ticks)
        ax.set_yticklabels([str(p) for p in ticks])
        ax.set_xlabel("OpenMP threads / MPI ranks")
        ax.set_ylabel("speedup vs serial")
        ax.set_title(f"Speedup vs memory-bandwidth limit, {n0}^3 grid")
        ax.grid(alpha=0.3)
        ax.legend(fontsize=8)
        fig.tight_layout()
        path = f"{OUT}/speedup_vs_stream_{n0}.png"
        fig.savefig(path, dpi=150)
        plt.close(fig)
        print(f"comparison plot -> {path}")


if __name__ == "__main__":
    main()
