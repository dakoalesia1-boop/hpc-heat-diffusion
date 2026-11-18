"""Draw the temperature field from a raw dump: python3 scripts/plot_field.py <file> <n>"""
import os, sys
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

path, n = sys.argv[1], int(sys.argv[2])
size = os.path.getsize(path)
assert size == n**3 * 8, f"{size} bytes, expected {n**3 * 8}: the file has another format"
u = np.fromfile(path, dtype=np.float64).reshape(n, n, n)
fig, (a1, a2) = plt.subplots(1, 2, figsize=(11, 4.5))
im = a1.imshow(u[n // 2], cmap="inferno", origin="lower")
a1.set_title(f"Temperature, middle slice of the {n}^3 block")
fig.colorbar(im, ax=a1, label="temperature")
a2.plot(u[n // 2, n // 2, :], color="#E8692C")
a2.set_title("Temperature along a line through the centre")
a2.set_xlabel("cell index"); a2.grid(alpha=0.3)
fig.tight_layout()
out = f"results/plots/heat_field_{n}.png"
fig.savefig(out, dpi=150)
print(out)
