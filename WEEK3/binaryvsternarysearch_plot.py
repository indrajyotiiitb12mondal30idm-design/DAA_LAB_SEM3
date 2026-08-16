import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Read from the current directory
df = pd.read_csv("q1_data.csv")

fig, axes = plt.subplots(1, 2, figsize=(11, 4.3))

# ---- Panel 1: worst-case comparisons vs n -------------------------------
ax = axes[0]
ax.plot(df["n"], df["binary_worst"], marker='o', ms=4, lw=2,
        color="#1f77b4", label="Binary Search (measured)")
ax.plot(df["n"], df["ternary_worst"], marker='s', ms=4, lw=2,
        color="#d62728", label="Ternary Search (measured)")

# theoretical curves: 2*log2(n) and 4*log3(n)
n_theory = df["n"].values
ax.plot(n_theory, 2*np.log2(n_theory), '--', lw=1.3, color="#1f77b4",
        alpha=0.55, label=r"Theory: $2\log_2 n$")
ax.plot(n_theory, 4*np.log(n_theory)/np.log(3), '--', lw=1.3, color="#d62728",
        alpha=0.55, label=r"Theory: $4\log_3 n$")

ax.set_xscale("log", base=2)
ax.set_xlabel("Array size $n$ (log scale)")
ax.set_ylabel("Worst-case number of comparisons")
ax.set_title("(a) Comparisons: Binary vs Ternary Search")
ax.grid(True, alpha=0.3)
ax.legend(fontsize=8, loc="upper left")

# ---- Panel 2: recursion levels vs n --------------------------------------
ax = axes[1]
ax.plot(df["n"], df["binary_levels"], marker='o', ms=4, lw=2,
        color="#1f77b4", label="Binary levels")
ax.plot(df["n"], df["ternary_levels"], marker='s', ms=4, lw=2,
        color="#d62728", label="Ternary levels")
ax.set_xscale("log", base=2)
ax.set_xlabel("Array size $n$ (log scale)")
ax.set_ylabel("Recursion depth (levels)")
ax.set_title("(b) Depth only: Ternary looks better here...")
ax.grid(True, alpha=0.3)
ax.legend(fontsize=8, loc="upper left")

plt.tight_layout()

# Save to the current directory
plt.savefig("q1_search.pdf", bbox_inches="tight")
plt.savefig("q1_search.png", dpi=150, bbox_inches="tight")
print("Saved q1_search.pdf/png")