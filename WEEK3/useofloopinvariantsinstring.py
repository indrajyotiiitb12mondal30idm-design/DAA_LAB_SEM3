import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

df_order = pd.read_csv("q6_order_data.csv")
df_growth = pd.read_csv("q6_growth_data.csv")

fig, axes = plt.subplots(1, 2, figsize=(12, 4.6))

# ---- Panel (a): comparisons vs swaps across orderings, for a few n ----
ax = axes[0]
ns_to_show = [10, 50, 100, 500, 1000]
sub = df_order[df_order["n"].isin(ns_to_show)]
x = np.arange(len(ns_to_show))
width = 0.25
colors = {"sorted": "#2ca02c", "reverse": "#d62728", "random": "#1f77b4"}
for i, order in enumerate(["sorted", "reverse", "random"]):
    vals = [sub[(sub.n == n) & (sub.order == order)]["swaps"].values[0] for n in ns_to_show]
    ax.bar(x + (i - 1) * width, vals, width, label=f"{order} (swaps)", color=colors[order])

ax2 = ax.twinx()
cmp_vals = [sub[(sub.n == n) & (sub.order == "sorted")]["comparisons"].values[0] for n in ns_to_show]
ax2.plot(x, cmp_vals, marker='D', color="black", lw=1.8, ms=6,
          label="comparisons (identical for all 3 orders)")
ax2.set_ylabel("Comparisons  (same for all orderings)")
ax2.set_yscale("log")

ax.set_xticks(x)
ax.set_xticklabels([str(n) for n in ns_to_show])
ax.set_xlabel("Array size $n$")
ax.set_ylabel("Number of swaps")
ax.set_title("(a) Selection sort: swaps vary,\ncomparisons never do")
lines1, labels1 = ax.get_legend_handles_labels()
lines2, labels2 = ax2.get_legend_handles_labels()
ax.legend(lines1 + lines2, labels1 + labels2, fontsize=7.5, loc="upper left")

# ---- Panel (b): selection sort vs heap sort growth ----
ax = axes[1]
ax.plot(df_growth["n"], df_growth["selection_cmp"], marker='o', ms=5, lw=2,
        color="#d62728", label=r"Selection sort  $\Theta(n^2)$")
ax.plot(df_growth["n"], df_growth["heap_cmp"], marker='s', ms=5, lw=2,
        color="#2ca02c", label=r"Heap sort  $\Theta(n\log n)$  (best)")
ax.set_xscale("log")
ax.set_yscale("log")
ax.set_xlabel("Array size $n$ (log scale)")
ax.set_ylabel("Comparisons (log scale)")
ax.set_title("(b) Selection sort vs. Heap sort\n(1650x fewer comparisons at n=100,000)")
ax.grid(True, alpha=0.3, which="both")
ax.legend(fontsize=9, loc="upper left")

plt.tight_layout()
plt.savefig("q6_sort.pdf", bbox_inches="tight")
plt.savefig("q6_sort.png", dpi=150, bbox_inches="tight")
print("Saved q6_sort.pdf/png")