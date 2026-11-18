#!/usr/bin/env python3
"""Collect every RESULT line from the Slurm logs into one CSV.

Usage (from the repository root):
    python3 scripts/collect.py                 # reads results/logs/*.log -> results/csv/results.csv

Only the Python standard library is used, so this runs on the cluster login node.
"""
import csv
import glob
import os
import re
import sys

FIELDS = ["version", "nx", "ny", "nz", "steps", "nodes", "ranks", "threads", "gpus",
          "total_s", "compute_s", "comm_s", "checksum", "center", "mlups"]


def main(log_glob="results/logs/*.log", out_path="results/csv/results.csv"):
    rows = []
    for path in sorted(glob.glob(log_glob)):
        m = re.search(r"_(\d+)\.log$", path)
        jobid = m.group(1) if m else ""
        commit = ""
        bind = ""
        node = ""
        with open(path, errors="replace") as f:
            for line in f:
                if line.startswith("=== git commit:"):
                    commit = line.split(":", 1)[1].strip()
                if "OMP_PROC_BIND=" in line:
                    bind = line.split("OMP_PROC_BIND=", 1)[1].split()[0]
                if line.startswith("=== partition:") and "nodes:" in line:
                    node = line.split("nodes:", 1)[1].split()[0]
                if not line.startswith("RESULT,"):
                    continue
                values = line.strip().split(",")[1:]
                if len(values) != len(FIELDS):
                    print(f"skipping malformed line in {path}: {line.strip()}", file=sys.stderr)
                    continue
                row = dict(zip(FIELDS, values))
                row.update(jobid=jobid, commit=commit, log=os.path.basename(path), bind=bind, node=node)
                rows.append(row)

    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=FIELDS + ["jobid", "commit", "log", "bind", "node"])
        w.writeheader()
        w.writerows(rows)
    print(f"{len(rows)} results from {len(set(r['log'] for r in rows))} logs -> {out_path}")


if __name__ == "__main__":
    main(*sys.argv[1:])
