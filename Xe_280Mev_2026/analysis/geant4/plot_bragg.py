import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("build/geant4_bragg.csv")

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))

for mat, group in df.groupby("material"):
    ax1.plot(group["depth_mm"], group["edep_per_mm_MeV"],
             "o-", label=mat, markersize=4)
ax1.set_xlabel("Depth from entrance (mm)")
ax1.set_ylabel("dE/dx (MeV/mm)")
ax1.set_title("Bragg curve (Geant4)")
ax1.legend()
ax1.grid(True)

for mat, group in df.groupby("material"):
    ax2.plot(group["depth_mm"], group["edep_MeV"],
             "s-", label=mat, markersize=4)
ax2.set_xlabel("Depth from entrance (mm)")
ax2.set_ylabel("Energy deposited (MeV)")
ax2.set_title("Energy per layer")
ax2.legend()
ax2.grid(True)

plt.tight_layout()
plt.savefig("build/geant4_bragg.png", dpi=150)
print("Saved build/geant4_bragg.png")
