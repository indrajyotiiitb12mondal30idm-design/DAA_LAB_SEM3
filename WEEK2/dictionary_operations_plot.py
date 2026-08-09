import os
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("q1_timings.csv")

outdir = "figures"
os.makedirs(outdir, exist_ok=True)

STRUCT_ORDER = [
    "Unsorted Array",
    "Sorted Array",
    "Unsorted SLL",
    "Sorted SLL",
    "Unsorted DLL",
    "Sorted DLL"
]

OPS = [
    "Search",
    "Insert",
    "Delete",
    "Max",
    "Min",
    "Predecessor",
    "Successor"
]

STRUCT_COLORS = {
    "Unsorted Array": "blue",
    "Sorted Array": "red",
    "Unsorted SLL": "green",
    "Sorted SLL": "orange",
    "Unsorted DLL": "purple",
    "Sorted DLL": "brown"
}

STRUCT_MARKERS = {
    "Unsorted Array": "o",
    "Sorted Array": "s",
    "Unsorted SLL": "^",
    "Sorted SLL": "D",
    "Unsorted DLL": "v",
    "Sorted DLL": "P"
}

for op in OPS:
    sub = df[df["operation"] == op]

    fig, (axL, axR) = plt.subplots(1, 2, figsize=(11.5, 4.3))

    for struct in STRUCT_ORDER:
        s = sub[sub["structure"] == struct].sort_values("n")

        if s.empty:
            continue

        for ax in (axL, axR):
            ax.plot(
                s["n"],
                s["avg_time_ns"],
                marker=STRUCT_MARKERS[struct],
                color=STRUCT_COLORS[struct],
                label=struct,
                linewidth=1.8,
                markersize=5
            )

    axR.set_yscale("log")

    axL.set_xlabel("n (number of elements)")
    axR.set_xlabel("n (number of elements)")

    axL.set_ylabel("average time per operation (ns)")
    axR.set_ylabel("average time per operation (ns, log scale)")

    axL.set_title("Linear scale – true shape of the curve")
    axR.set_title("Log scale – separates the small values")

    axR.legend(loc="upper left", framealpha=0.9, fontsize=8.3)

    fig.suptitle(
        f"{op}(D, x) – measured running time vs. n",
        fontweight="bold",
        fontsize=13
    )

    fig.tight_layout(rect=[0, 0, 1, 0.94])

    fname = f"{outdir}/q1_{op.lower()}.png"
    fig.savefig(fname)

    plt.close(fig)

    print("wrote", fname)

print("All Q1 plots generated.")