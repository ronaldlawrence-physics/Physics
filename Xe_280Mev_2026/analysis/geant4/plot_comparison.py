import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

exp = pd.read_csv("../bragg_data.csv")
sim = pd.read_csv("build/geant4_bragg.csv")

# Convert Geant4 depths to depth from entrance
entrance_z = sim["depth_mm"].max()
sim["depth_from_entrance"] = entrance_z - sim["depth_mm"]

colors = {"Stack_01": "#1f77b4", "Stack_02": "#ff7f0e", "Stack_03": "#2ca02c"}
sim_color = "#d62728"
markers = {"Stack_01": "o", "Stack_02": "s", "Stack_03": "^"}

# Normalize experimental energy_dep_frac per stack
exp_norm = exp.copy()
for stack in exp["stack"].unique():
    mask = exp["stack"] == stack
    peak = exp.loc[mask, "energy_dep_frac"].max()
    exp_norm.loc[mask, "norm_edep"] = exp.loc[mask, "energy_dep_frac"] / peak

sim["norm_edep_max"] = sim["edep_per_mm_MeV"] / sim["edep_per_mm_MeV"].max()

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 6))

for stack in ["Stack_01", "Stack_02", "Stack_03"]:
    sub = exp_norm[exp_norm["stack"] == stack]
    ax1.plot(sub["depth_mm"], sub["norm_edep"],
             marker=markers[stack], linestyle="-", color=colors[stack],
             label=f"{stack.replace('_', ' ')} (exp)", markersize=6, linewidth=1.5)

for mat, group in sim.groupby("material"):
    ax1.plot(group["depth_from_entrance"], group["norm_edep_max"],
             marker="D", linestyle="--", color=sim_color,
             label=f"Geant4 ({mat})", markersize=5, linewidth=1.5, alpha=0.8)

ax1.set_xlabel("Depth from entrance (mm)", fontsize=12)
ax1.set_ylabel("Normalised dE/dx (peak = 1)", fontsize=12)
ax1.set_title("Bragg curve: Geant4 vs experiment (normalised)", fontsize=14)
ax1.legend(fontsize=10, loc="upper right")
ax1.grid(True, alpha=0.3)

# Second panel: absolute comparison for Stack 03 (best statistics)
ax2_twin = ax2.twinx()
ax2_twin.set_ylabel("Geant4 dE/dx (MeV/mm)", color=sim_color, fontsize=12)
ax2_twin.tick_params(axis="y", labelcolor=sim_color)

s03 = exp[exp["stack"] == "Stack_03"]
ax2.plot(s03["depth_mm"], s03["energy_dep_frac"],
         marker="^", linestyle="-", color=colors["Stack_03"],
         label="Exp Stack 03 (energy dep frac)", markersize=6, linewidth=1.5)

for mat, group in sim.groupby("material"):
    ax2_twin.plot(group["depth_from_entrance"], group["edep_per_mm_MeV"],
                  marker="D", linestyle="--", color=sim_color,
                  label=f"Geant4 ({mat})", markersize=5, linewidth=1.5, alpha=0.8)

ax2.set_xlabel("Depth from entrance (mm)", fontsize=12)
ax2.set_ylabel("Exp. energy deposition fraction", fontsize=12)
ax2.set_title("Absolute comparison: Stack 03 vs Geant4", fontsize=14)
ax2.legend(fontsize=10, loc="upper left")
ax2.grid(True, alpha=0.3)

# Add peak markers
for stack in ["Stack_01", "Stack_02", "Stack_03"]:
    sub = exp[exp["stack"] == stack]
    peak_idx = sub["energy_dep_frac"].idxmax()
    pdepth = sub.loc[peak_idx, "depth_mm"]
    ax1.axvline(pdepth, color=colors[stack], linestyle=":", alpha=0.5, linewidth=1)
    ax1.annotate(f"{stack.split('_')[1]} peak: {pdepth:.0f} mm",
                 xy=(pdepth, 0.95), rotation=90,
                 fontsize=7, color=colors[stack],
                 verticalalignment="top", alpha=0.7)

sim_peak_depth = sim.loc[sim["edep_per_mm_MeV"].idxmax(), "depth_from_entrance"]
ax1.axvline(sim_peak_depth, color=sim_color, linestyle=":", alpha=0.5, linewidth=1)
ax1.annotate(f"Geant4 peak: {sim_peak_depth:.0f} mm",
             xy=(sim_peak_depth, 0.9), rotation=90,
             fontsize=7, color=sim_color,
             verticalalignment="top", alpha=0.7)

plt.tight_layout()
plt.savefig("build/geant4_vs_experiment.png", dpi=150, bbox_inches="tight")
print(f"Saved. Geant4 peak: {sim_peak_depth:.2f} mm")
