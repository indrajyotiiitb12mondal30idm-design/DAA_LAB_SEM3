import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

df = pd.read_csv("q2_timings.csv")

outdir = "figures"
os.makedirs(outdir, exist_ok=True)

two = df[df["algorithm"] == "MergeSort2Way"].sort_values("n")
three = df[df["algorithm"] == "MergeSort3Way"].sort_values("n")

COL2 = "blue"
COL3 = "red"

# ----------------------------- Figure 1 -----------------------------

fig, (axL, axR) = plt.subplots(1, 2, figsize=(11.5, 4.5))

axL.plot(
    two["n"],
    two["time_ns"] / 1e6,
    "o-",
    color=COL2,
    label="Merge Sort (2-way, halves)"
)

axL.plot(
    three["n"],
    three["time_ns"] / 1e6,
    "s-",
    color=COL3,
    label="Modified Merge Sort (3-way, thirds)"
)

axL.set_xlabel("n (array size)")
axL.set_ylabel("time (milliseconds)")
axL.set_title("Linear axes")
axL.legend(fontsize=9)

axR.plot(
    two["n"],
    two["time_ns"] / 1e6,
    "o-",
    color=COL2,
    label="Merge Sort (2-way)"
)

axR.plot(
    three["n"],
    three["time_ns"] / 1e6,
    "s-",
    color=COL3,
    label="Modified Merge Sort (3-way)"
)

axR.set_xscale("log")
axR.set_yscale("log")
axR.set_xlabel("n (log scale)")
axR.set_ylabel("time (ms, log scale)")
axR.set_title("Log-log axes")
axR.legend(fontsize=9)

fig.suptitle(
    "Question 2 — Merge Sort vs. Modified 3-way Merge Sort: Raw Timings",
    fontweight="bold",
    fontsize=13
)

fig.tight_layout(rect=[0, 0, 1, 0.94])

fig.savefig(f"{outdir}/q2_raw_timings.png")

plt.close(fig)


# ----------------------------- Figure 2 -----------------------------

fig, ax = plt.subplots(figsize=(7.5, 5))

x2 = two["n"] * np.log2(two["n"])
x3 = three["n"] * np.log2(three["n"])

y2 = two["time_ns"] / 1e6
y3 = three["time_ns"] / 1e6

ax.plot(
    x2,
    y2,
    "o",
    color=COL2,
    label="Merge Sort (2-way) — measured"
)

ax.plot(
    x3,
    y3,
    "s",
    color=COL3,
    label="Modified Merge Sort (3-way) — measured"
)

# Best-fit straight line through the origin:
# y = c*x

c2 = float(np.sum(x2 * y2) / np.sum(x2 * x2))
c3 = float(np.sum(x3 * y3) / np.sum(x3 * x3))

xs = np.linspace(
    0,
    max(x2.max(), x3.max()),
    50
)

ax.plot(
    xs,
    c2 * xs,
    "--",
    color=COL2,
    alpha=0.6,
    label=f"2-way fit: c={c2:.3e}"
)

ax.plot(
    xs,
    c3 * xs,
    "--",
    color=COL3,
    alpha=0.6,
    label=f"3-way fit: c={c3:.3e}"
)

ax.set_xlabel("x = n · log₂(n)")
ax.set_ylabel("time (milliseconds)")

ax.set_title(
    "The straight-line test for Θ(n log n)",
    fontweight="bold"
)

ax.legend(fontsize=8.5)

fig.tight_layout()

fig.savefig(f"{outdir}/q2_nlogn_fit.png")

plt.close(fig)


print(f"Fitted constants: 2-way c = {c2:.4e} ms per (n log2 n) unit")
print(f"Fitted constants: 3-way c = {c3:.4e} ms per (n log2 n) unit")
print("Q2 plots written.")