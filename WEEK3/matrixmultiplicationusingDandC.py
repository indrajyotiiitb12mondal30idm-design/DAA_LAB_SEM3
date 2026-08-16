import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

df = pd.read_csv("q4_data.csv")

fig, axes = plt.subplots(1, 2, figsize=(11.5, 4.4))

# ---- Panel (a): all three, small-to-medium n, log-log ----
ax = axes[0]
sub = df.dropna(subset=["strassen_pure_sec"])
ax.plot(df["n"], df["naive_sec"], marker='^', ms=5, lw=2, color="#d62728",
        label=r"Naive ($\Theta(n^3)$)")
ax.plot(sub["n"], sub["strassen_pure_sec"], marker='x', ms=6, lw=2, color="#9467bd",
        label="Strassen - pure recursion")
ax.plot(df["n"], df["strassen_hybrid_sec"], marker='s', ms=5, lw=2, color="#2ca02c",
        label=r"Strassen - hybrid ($n^{2.807}$, best)")
ax.set_xscale("log", base=2)
ax.set_yscale("log")
ax.set_xlabel("Matrix size $n$ (log scale)")
ax.set_ylabel("Time (seconds, log scale)")
ax.set_title("(a) Pure Strassen's overhead dominates\nat every practical size")
ax.grid(True, alpha=0.3, which="both")
ax.legend(fontsize=8, loc="upper left")

# ---- Panel (b): naive vs hybrid only, out to n=2048, shows crossover ----
ax = axes[1]
ax.plot(df["n"], df["naive_sec"], marker='^', ms=6, lw=2.2, color="#d62728",
        label=r"Naive ($\Theta(n^3)$)")
ax.plot(df["n"], df["strassen_hybrid_sec"], marker='s', ms=6, lw=2.2, color="#2ca02c",
        label="Strassen - hybrid (best)")
ax.set_xscale("log", base=2)
ax.set_yscale("log")
ax.set_xlabel("Matrix size $n$ (log scale)")
ax.set_ylabel("Time (seconds, log scale)")
ax.set_title("(b) Hybrid Strassen overtakes naive\nas $n$ grows (10x faster at n=2048)")
ax.grid(True, alpha=0.3, which="both")
ax.legend(fontsize=9, loc="upper left")
ax.annotate("10.3x faster", xy=(2048, 7.7), xytext=(600, 25),
            fontsize=9, arrowprops=dict(arrowstyle="->", color="grey"))

plt.tight_layout()
plt.savefig("q4_strassen.pdf", bbox_inches="tight")
plt.savefig("q4_strassen.png", dpi=150, bbox_inches="tight")
print("Saved q4_strassen.pdf/png")
print("Speedup at n=2048:", df[df.n==2048]["naive_sec"].values[0] / df[df.n==2048]["strassen_hybrid_sec"].values[0])