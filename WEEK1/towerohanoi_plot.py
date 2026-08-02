import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path


script_dir = Path(__file__).resolve().parent
data_path = script_dir / "hanoi_data.csv"
data = pd.read_csv(data_path)


fig, axes = plt.subplots(1, 2, figsize=(12, 5))


axes[0].plot(
    data["n"],
    data["moves"],
    marker="o",
    color="darkred"
)

axes[0].set_xlabel("Number of discs (n)")
axes[0].set_ylabel("Number of moves")
axes[0].set_title("Linear scale: T(n) = 2^n - 1")
axes[0].grid(alpha=0.3)


axes[1].plot(
    data["n"],
    data["moves"],
    marker="o",
    color="darkblue"
)

axes[1].set_yscale("log")
axes[1].set_xlabel("Number of discs (n)")
axes[1].set_ylabel("Number of moves (log scale)")
axes[1].set_title("Log scale: confirms exponential growth")
axes[1].grid(alpha=0.3, which="both")


plt.tight_layout()


plt.savefig("hanoi_moves.png", dpi=150, bbox_inches="tight")


plt.show()