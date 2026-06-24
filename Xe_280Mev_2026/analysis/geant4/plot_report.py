import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

df = pd.read_csv("build/geant4_bragg.csv")

# Convert depth to depth_from_entrance (positive, increasing)
entrance_z = df["depth_mm"].max()
df["depth_from_entrance"] = entrance_z - df["depth_mm"]

colors = {"emulsion": "#3498db", "glass": "#e74c3c"}
labels = {"emulsion": "Emulsion (G5)", "glass": "Soda-lime glass"}

fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(14, 10))

for mat, group in df.groupby("material"):
    ax1.plot(group["depth_from_entrance"], group["edep_per_mm_MeV"],
             "o-", color=colors[mat], label=labels[mat], markersize=6, linewidth=1.5)
ax1.set_xlabel("Depth from entrance (mm)", fontsize=12)
ax1.set_ylabel("dE/dx (MeV/mm)", fontsize=12)
ax1.set_title("Bragg curve — Xe-124 280 MeV/nucleon", fontsize=14)
ax1.legend(fontsize=11)
ax1.grid(True, alpha=0.3)

peak_idx = df["edep_per_mm_MeV"].idxmax()
peak_row = df.loc[peak_idx]
peak_depth = peak_row["depth_from_entrance"]
ax1.annotate(f"Peak: {peak_row['edep_per_mm_MeV']:.0f} MeV/mm\n"
             f"at {peak_depth:.1f} mm ({peak_row['material']})",
             xy=(peak_depth, peak_row["edep_per_mm_MeV"]),
             xytext=(peak_depth + 2, peak_row["edep_per_mm_MeV"] * 0.7),
             arrowprops=dict(arrowstyle="->", color="black", lw=1.5),
             fontsize=10, bbox=dict(boxstyle="round,pad=0.3", facecolor="white", alpha=0.8))

for mat, group in df.groupby("material"):
    ax2.plot(group["depth_from_entrance"], group["edep_MeV"],
             "s-", color=colors[mat], label=labels[mat], markersize=6, linewidth=1.5)
ax2.set_xlabel("Depth from entrance (mm)", fontsize=12)
ax2.set_ylabel("Energy deposited (MeV)", fontsize=12)
ax2.set_title("Energy deposition per layer", fontsize=14)
ax2.legend(fontsize=11)
ax2.grid(True, alpha=0.3)

total_edep = df["edep_MeV"].sum()
for mat, group in df.groupby("material"):
    ax3.plot(group["depth_from_entrance"], group["norm_edep"],
             "D-", color=colors[mat], label=labels[mat], markersize=6, linewidth=1.5)
ax3.set_xlabel("Depth from entrance (mm)", fontsize=12)
ax3.set_ylabel("Normalized dE/dx", fontsize=12)
ax3.set_title(f"Normalized dE/dx (Deposited: {total_edep:.0f}/{280*124:.0f} MeV)", fontsize=14)
ax3.legend(fontsize=11)
ax3.grid(True, alpha=0.3)

sorted_df = df.sort_values("depth_from_entrance")
cumulative = sorted_df["edep_MeV"].cumsum()
ax4.plot(sorted_df["depth_from_entrance"], cumulative, "-", color="#2c3e50", linewidth=2)
ax4.fill_between(sorted_df["depth_from_entrance"], cumulative, alpha=0.3, color="#2c3e50")
ax4.axhline(total_edep, color="gray", linestyle="--", alpha=0.5)
ax4.set_xlabel("Depth from entrance (mm)", fontsize=12)
ax4.set_ylabel("Cumulative energy (MeV)", fontsize=12)
ax4.set_title(f"Cumulative energy (total: {total_edep:.0f} MeV)", fontsize=14)
ax4.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig("build/geant4_report_bragg.png", dpi=150, bbox_inches="tight")
print(f"Saved. Deposited: {total_edep:.0f}/{280*124:.0f} MeV = {total_edep/(280*124)*100:.1f}%")
