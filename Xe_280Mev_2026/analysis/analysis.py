#!/usr/bin/env python3
import numpy as np
import pandas as pd
import os, gzip, glob, warnings
from sklearn.mixture import GaussianMixture
import json
warnings.filterwarnings('ignore')

BASE = '/home/ronald/JINR/LHEP/Data/Xe_280Mev_2026'
OUT = os.path.join(BASE, 'analysis')
EMULSION_UM = 60.0
GLASS_UM = 2000.0

def cumulative_depth(plate_idx, plates_total):
    center_of_plate = plate_idx * (EMULSION_UM + GLASS_UM) + EMULSION_UM / 2.0
    return center_of_plate / 1000.0

def read_csv_or_gz(path):
    if os.path.exists(path):
        if path.endswith('.gz'):
            return pd.read_csv(path, compression='gzip')
        return pd.read_csv(path)
    if not path.endswith('.gz'):
        gzpath = path + '.gz'
        if os.path.exists(gzpath):
            return pd.read_csv(gzpath, compression='gzip')
    raise FileNotFoundError(f"Could not find: {path}")

def stacked_summary(df):
    return {
        'count': len(df),
        'total_area': df['Area'].sum(),
        'avg_area': df['Area'].mean(),
        'min_x': df['x'].min(),
        'max_x': df['x'].max(),
        'min_y': df['y'].min(),
        'max_y': df['y'].max(),
    }

# ====== STACK CONFIGURATIONS ======
stacks = {}

# --- Stack 01 ---
plates_s01 = ['1_Plate_09', '2_Plate_08', '3_Plate_07', '4_Plate_06',
              '5_Plate_05', '6_Plate_04', '7_Plate_03', '8_Plate_02']
results_s01 = []
pdir = os.path.join(BASE, 'Stack_01', 'Results_Files')
for name in plates_s01:
    f = os.path.join(pdir, f'{name}_Results.csv')
    df = read_csv_or_gz(f)
    scale = 1.717
    df_std = pd.DataFrame({
        'Area': df['Area'] / (scale * scale),
        'x': df['FeretX'] / scale,
        'y': df['FeretY'] / scale,
    })
    results_s01.append(df_std)

# --- Stack 02 ---
plates_s02 = [f'{i}_Plate-0001' for i in range(1, 10)] + ['10_Plate_001']
results_s02 = []
pdir = os.path.join(BASE, 'Stack_02', 'Results_Files')
for name in plates_s02:
    f = os.path.join(pdir, f'{name}_Results.csv')
    df = read_csv_or_gz(f)
    df_std = pd.DataFrame({
        'Area': df['Area'],
        'x': df['XM'],
        'y': df['YM'],
    })
    results_s02.append(df_std)

# --- Stack 03 ---
plates_s03 = [f'Plate_{i:02d}_p1' for i in range(1, 11)]
results_s03 = []
pdir = os.path.join(BASE, 'Stack_03', 'Results_Files')
for name in plates_s03:
    f = os.path.join(pdir, f'{name}_Results.csv')
    df = read_csv_or_gz(f)
    df_std = pd.DataFrame({
        'Area': df['Area'],
        'x': df['XM'],
        'y': df['YM'],
    })
    results_s03.append(df_std)

stack_configs = [
    ('Stack_01', plates_s01, results_s01, [1.717], 1),
    ('Stack_02', plates_s02, results_s02, [1.0],   5),
    ('Stack_03', plates_s03, results_s03, [1.0],  10),
]

# ====== MAIN ANALYSIS ======
all_plate_data = []
gmm_results = []

for sname, plates, results, scales, ncycles in stack_configs:
    for i, (pname, df) in enumerate(zip(plates, results)):
        depth    = cumulative_depth(i, len(plates))
        sx       = df['x']
        sy       = df['y']
        area_um2 = df['Area']
        count    = len(df)
        scan_area_um2 = (sx.max() - sx.min()) * (sy.max() - sy.min())
        if scan_area_um2 <= 0:
            scan_area_um2 = 1.0
        track_density  = count / (scan_area_um2 / 1e6)
        energy_dep     = area_um2.sum() / scan_area_um2
        row = {
            'stack': sname, 'plate': pname, 'plate_num': i+1, 'n_plates': len(plates),
            'depth_mm': depth, 'count': count, 'cycles': ncycles,
            'scan_area_mm2': scan_area_um2 / 1e6,
            'track_density_per_mm2': track_density,
            'track_density_per_cycle': track_density / ncycles,
            'total_area_um2': area_um2.sum(),
            'avg_area_um2': area_um2.mean(),
            'energy_dep_frac': energy_dep,
        }
        all_plate_data.append(row)

        # ----- Particle classification via GMM (sorted by mean area) -----
        logA = np.log10(area_um2.values).reshape(-1, 1)
        n_comp = 3
        if len(logA) < 500:
            n_comp = 2
        gmm = GaussianMixture(n_components=n_comp, random_state=42, max_iter=500)
        labels = gmm.fit_predict(logA)
        # Sort components by mean area ascending: class 0=smallest, 2=largest
        means_raw = gmm.means_.flatten()
        order = np.argsort(means_raw)
        label_map = {old: new for new, old in enumerate(order)}
        labels_sorted = np.array([label_map[l] for l in labels])
        sorted_means = means_raw[order]
        sorted_stds = np.sqrt(gmm.covariances_.flatten())[order]
        counts = np.bincount(labels_sorted, minlength=n_comp)
        for k in range(n_comp):
            gmm_results.append({
                'stack': sname, 'plate': pname, 'plate_num': i+1, 'depth_mm': depth,
                'class': k, 'count': int(counts[k]),
                'frac': float(counts[k] / len(labels)),
                'log10_area_mean': float(sorted_means[k]),
                'log10_area_std': float(sorted_stds[k]),
                'area_mean_um2': float(10**sorted_means[k]),
            })

# ====== OUTPUT DATA TABLES ======
df_bragg = pd.DataFrame(all_plate_data)
df_gmm   = pd.DataFrame(gmm_results)

df_bragg.to_csv(os.path.join(OUT, 'bragg_data.csv'), index=False)
df_gmm.to_csv(os.path.join(OUT, 'particle_classes.csv'), index=False)

print("=== BRAGG DATA ===")
print(df_bragg[['stack', 'plate', 'depth_mm', 'cycles', 'track_density_per_mm2', 'track_density_per_cycle', 'energy_dep_frac', 'avg_area_um2', 'scan_area_mm2']].to_string(index=False))

print("\n=== PARTICLE CLASSES ===")
print(df_gmm.to_string(index=False))

# ====== EXPORT FOR ROOT ======
# Simple text tables
with open(os.path.join(OUT, 'bragg_root.dat'), 'w') as f:
    f.write('#stack plate depth_mm cycles track_density_per_mm2 track_density_per_cycle energy_dep_frac avg_area_um2\n')
    for r in all_plate_data:
        f.write(f"{r['stack']} {r['plate']} {r['depth_mm']:.4f} {r['cycles']} {r['track_density_per_mm2']:.2f} {r['track_density_per_cycle']:.2f} {r['energy_dep_frac']:.6f} {r['avg_area_um2']:.2f}\n")

with open(os.path.join(OUT, 'classes_root.dat'), 'w') as f:
    f.write('#stack plate depth_mm class frac area_mean_um2\n')
    for r in gmm_results:
        f.write(f"{r['stack']} {r['plate']} {r['depth_mm']:.4f} {r['class']} {r['frac']:.4f} {r['area_mean_um2']:.2f}\n")

print("\nOutput files written to:", OUT)
