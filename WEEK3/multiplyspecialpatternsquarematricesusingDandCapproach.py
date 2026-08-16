import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

df = pd.read_csv("q5_data.csv")

fig, axes = plt.subplots(1, 2, figsize=(11.5, 4.4))

# ---- Panel (a): scalar operation counts, log-log ----
ax = axes[0]
ax.plot(df["n"], df["naive_ops"], marker='^', ms=5, lw=2, color="#d62728",
        label=r"Naive full multiply: $n^3$")
ax.plot(df["n"], df["simple_ops"], marker='o', ms=5, lw=2, color="#ff7f0e",
        label=r"Simple (symmetry only): $n^3/2$")
ax.plot(df["n"], df["best_ops"], marker='s', ms=5, lw=2, color="#2ca02c",
        label=r"Best (P/Q/R/S, compact form): $n$")
ax.set_xscale("log", base=2)
ax.set_yscale("log")
ax.set_xlabel("Matrix size $n$ (log scale)")
ax.set_ylabel("Scalar multiplications (log scale)")
ax.set_title("(a) Operation counts: cubic vs. linear")
ax.grid(True, alpha=0.3, which="both")
ax.legend(fontsize=8, loc="upper left")

# ---- Panel (b): wall-clock time ----
ax = axes[1]
ax.plot(df["n"], df["naive_sec"], marker='^', ms=5, lw=2, color="#d62728",
        label="Naive full multiply")
ax.plot(df["n"], df["simple_sec"], marker='o', ms=5, lw=2, color="#ff7f0e",
        label="Simple (symmetry only)")
ax.plot(df["n"], df["best_sec"], marker='s', ms=5, lw=2, color="#2ca02c",
        label="Best (P/Q/R/S, compact form)")
ax.set_xscale("log", base=2)
ax.set_yscale("log")
ax.set_xlabel("Matrix size $n$ (log scale)")
ax.set_ylabel("Time (seconds, log scale)")
ax.set_title("(b) Measured wall-clock time")
ax.grid(True, alpha=0.3, which="both")
ax.legend(fontsize=8, loc="upper left")

plt.tight_layout()
plt.savefig("q5_special.pdf", bbox_inches="tight")
plt.savefig("q5_special.png", dpi=150, bbox_inches="tight")
print("Saved q5_special.pdf/png")