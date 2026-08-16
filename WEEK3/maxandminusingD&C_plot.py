import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

df = pd.read_csv("q3_data.csv")

fig, axes = plt.subplots(1, 2, figsize=(11.5, 4.4))

# ---- Panel (a): all three methods vs n ----
ax = axes[0]
ax.plot(df["n"], df["naive"], marker='^', ms=4, lw=1.8, color="#d62728",
        label="Naive linear scan  (~2n)")
ax.plot(df["n"], df["dc_recursive"], marker='o', ms=4, lw=1.8, color="#ff7f0e",
        label="D&C recursive (\"simple\")")
ax.plot(df["n"], df["iterative"], marker='s', ms=3.5, lw=1.8, color="#2ca02c",
        label="Pairing / iterative (\"best\")")
ax.plot(df["n"], df["bound_3n_2"], '--', lw=1.3, color="black", alpha=0.6,
        label=r"Tight bound $\lceil 3n/2\rceil-2$")
ax.set_xscale("log", base=2)
ax.set_yscale("log", base=2)
ax.set_xlabel("Array size $n$ (log scale)")
ax.set_ylabel("Number of comparisons (log scale)")
ax.set_title("(a) Naive vs. D&C vs. pairing")
ax.grid(True, alpha=0.3)
ax.legend(fontsize=8, loc="upper left")

# ---- Panel (b): zoom on D&C-recursive vs bound only, showing the gap ----
ax = axes[1]
ax.plot(df["n"], df["dc_recursive"] / df["n"], marker='o', ms=4, lw=1.8,
        color="#ff7f0e", label="D&C recursive: T(n)/n")
ax.plot(df["n"], df["iterative"] / df["n"], marker='s', ms=3.5, lw=1.8,
        color="#2ca02c", label="Pairing: T(n)/n")
ax.axhline(1.5, color="black", ls="--", lw=1.3, alpha=0.6, label=r"$1.5$  (tight ratio)")
ax.axhline(5/3, color="grey", ls=":", lw=1.3, alpha=0.8, label=r"$5/3\approx1.667$ (proven worst case)")
ax.set_xscale("log", base=2)
ax.set_xlabel("Array size $n$ (log scale)")
ax.set_ylabel("Comparisons per element,  $T(n)/n$")
ax.set_title("(b) Constant-factor: recursive can drift up to 5/3")
ax.set_ylim(1.35, 1.85)
ax.grid(True, alpha=0.3)
ax.legend(fontsize=8, loc="upper right")

plt.tight_layout()
plt.savefig("q3_maxmin.pdf", bbox_inches="tight")
plt.savefig("q3_maxmin.png", dpi=150, bbox_inches="tight")
print("Saved q3_maxmin.pdf/png")