import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

df = pd.read_csv("q2_data.csv")

fig, ax = plt.subplots(figsize=(7.2, 4.6))

ax.plot(df["n"], df["binary_worst"], marker='o', ms=4, lw=2,
        color="#1f77b4", label="Binary-split (measured)")
ax.plot(df["n"], df["ternary_worst"], marker='s', ms=4, lw=2,
        color="#2ca02c", label="Ternary-split (measured, 'best')")

n_theory = df["n"].values
ax.plot(n_theory, np.floor(np.log2(n_theory)) + 1, '--', lw=1.3,
        color="#1f77b4", alpha=0.55, label=r"Theory: $\lfloor\log_2 n\rfloor + c$")
ax.plot(n_theory, np.ceil(np.log(n_theory)/np.log(3)) + 1, '--', lw=1.3,
        color="#2ca02c", alpha=0.55, label=r"Theory: $\lceil\log_3 n\rceil + c$")

ax.set_xscale("log", base=2)
ax.set_xlabel("Number of coins $n$ (log scale)")
ax.set_ylabel("Worst-case number of weighings")
ax.set_title("Defective-coin search: weighings required vs. $n$")
ax.grid(True, alpha=0.3)
ax.legend(fontsize=9, loc="upper left")

plt.tight_layout()
plt.savefig("q2_coin.pdf", bbox_inches="tight")
plt.savefig("q2_coin.png", dpi=150, bbox_inches="tight")
print("Saved q2_coin.pdf/png")  