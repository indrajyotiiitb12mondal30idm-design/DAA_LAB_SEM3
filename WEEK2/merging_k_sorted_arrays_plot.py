import pandas as pd
import numpy as np
import matplotlib.pyplot as plt


METHOD_LABEL = {
    "Method1_Sequential": "Method 1: Sequential merge - O(n k²)",
    "Method2_Pairwise": "Method 2: Pairwise (D&C) merge - O(n k log k)",
    "Method3_Heap": "Method 3: Min-heap k-way merge - O(n k log k)"
}

COLORS = {
    "Method1_Sequential": "#d62728",
    "Method2_Pairwise": "#1f77b4",
    "Method3_Heap": "#2ca02c"
}

MARKERS = {
    "Method1_Sequential": "o",
    "Method2_Pairwise": "s",
    "Method3_Heap": "^"
}


# Read CSV files
dfN = pd.read_csv("q3_vary_n.csv")
dfK = pd.read_csv("q3_vary_k.csv")


# ---------------------------------------------------------
# Figure 1: Vary n
# ---------------------------------------------------------

fig, ax = plt.subplots(figsize=(7.2, 5))

for method in METHOD_LABEL:
    data = dfN[dfN["method"] == method].sort_values("n")

    ax.plot(
        data["n"],
        data["time_ns"] / 1e6,
        marker=MARKERS[method],
        color=COLORS[method],
        label=METHOD_LABEL[method],
        linewidth=1.8
    )

ax.set_xlabel("n (size of each of the k sorted arrays), k = 16 fixed")
ax.set_ylabel("Time (milliseconds)")
ax.set_title(
    "Fixing k, growing n - all three methods scale linearly in n",
    fontweight="bold",
    fontsize=12
)

ax.legend(fontsize=8)
ax.grid(True, alpha=0.3)

fig.tight_layout()
fig.savefig("q3_vary_n.png", dpi=300)
plt.show()
plt.close(fig)


# ---------------------------------------------------------
# Figure 2: Vary k
# ---------------------------------------------------------

fig, (axL, axR) = plt.subplots(1, 2, figsize=(12, 5))

for method in METHOD_LABEL:
    data = dfK[dfK["method"] == method].sort_values("k")

    # Left graph
    axL.plot(
        data["k"],
        data["time_ns"] / 1e6,
        marker=MARKERS[method],
        color=COLORS[method],
        label=METHOD_LABEL[method],
        linewidth=1.8
    )

    # Right graph - log-log scale
    axR.plot(
        data["k"],
        data["time_ns"] / 1e6,
        marker=MARKERS[method],
        color=COLORS[method],
        label=METHOD_LABEL[method],
        linewidth=1.8
    )

axL.set_xlabel("k (number of arrays), n = 200 fixed")
axL.set_ylabel("Time (milliseconds)")
axL.set_title("Linear axes")

axL.legend(fontsize=8)
axL.grid(True, alpha=0.3)


axR.set_xscale("log")
axR.set_yscale("log")
axR.set_xlabel("k (number of arrays), n = 200 fixed")
axR.set_ylabel("Time (ms, log scale)")
axR.set_title("Log-log axes - slope reveals the exponent")

axR.legend(fontsize=8)
axR.grid(True, alpha=0.3)


# ---------------------------------------------------------
# Calculate slopes for Methods 1 and 2
# ---------------------------------------------------------

data1 = dfK[dfK["method"] == "Method1_Sequential"].sort_values("k")
data2 = dfK[dfK["method"] == "Method2_Pairwise"].sort_values("k")


def calculate_slope(data):
    k1 = data["k"].iloc[-2]
    k2 = data["k"].iloc[-1]

    t1 = data["time_ns"].iloc[-2]
    t2 = data["time_ns"].iloc[-1]

    return (np.log(t2) - np.log(t1)) / (np.log(k2) - np.log(k1))


slope1 = calculate_slope(data1)
slope2 = calculate_slope(data2)


# Display slopes on the graph
axR.text(
    0.03,
    0.92,
    f"Measured slope (Method 1) ≈ {slope1:.2f}\n"
    "Theory: 2",
    transform=axR.transAxes,
    fontsize=8.3,
    color=COLORS["Method1_Sequential"]
)

axR.text(
    0.03,
    0.85,
    f"Measured slope (Method 2) ≈ {slope2:.2f}\n"
    "Theory: ~1",
    transform=axR.transAxes,
    fontsize=8.3,
    color=COLORS["Method2_Pairwise"]
)


fig.suptitle(
    "Fixing n, growing k - Method 1 (k²) diverges from Methods 2 & 3 (k log k)",
    fontweight="bold",
    fontsize=12.5
)

fig.tight_layout(rect=[0, 0, 1, 0.93])

fig.savefig("q3_vary_k.png", dpi=300)
plt.show()
plt.close(fig)


# Print slope results
print(f"Slope 1 (Method 1, should be ~2): {slope1:.3f}")
print(f"Slope 2 (Method 2, should be ~1): {slope2:.3f}")
print("Q3 plots written successfully.")