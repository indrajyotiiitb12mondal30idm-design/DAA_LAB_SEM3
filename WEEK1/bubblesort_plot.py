import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv("bubble_sort_data.csv")

plt.figure(figsize=(8,5))

plt.plot(data["n"], data["adaptive_comparisons"],
         marker='o',
         label="Adaptive")

plt.plot(data["n"], data["nonadaptive_comparisons"],
         marker='s',
         label="Non-Adaptive")

plt.xlabel("Array Size (n)")
plt.ylabel("Number of Comparisons")
plt.title("Bubble Sort Comparisons")
plt.legend()

plt.grid(True)

plt.savefig("bubble_sort_comparisons.png")

plt.show()


"""
For random data, the two lines almost overlap 
because both versions make nearly the same number of comparisons.

"""