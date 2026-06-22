"""Beam profile summary: circ from CSV, area/count from bragg_data.csv for consistency."""
import os, glob, numpy as np, pandas as pd

base = "/home/ronald/JINR/LHEP/Data/Xe_280Mev_2026"

cfg = {
    "Stack_01": {"circ_col": "Circ."},
    "Stack_02": {"circ_col": "Circ."},
    "Stack_03": {"circ_col": "Circ."},
}

MIN_COUNT = 5000

# Load bragg data for count and area
bd = pd.read_csv(os.path.join(base, "analysis", "bragg_data.csv"))
bd.rename(columns={"stack": "Stack", "plate": "Plate"}, inplace=True)

rows = []
for sn, c in cfg.items():
    d = os.path.join(base, sn, "Results_Files")
    files = sorted(glob.glob(os.path.join(d, "*_Results.csv")) +
                   glob.glob(os.path.join(d, "*_Results.csv.gz")))
    for fpath in files:
        fname = os.path.basename(fpath)
        pname = fname.replace("_Results.csv.gz", "").replace("_Results.csv", "")
        comp = "gzip" if fpath.endswith(".gz") else None
        engine = "python" if comp else "c"
        print(f"  {fname}...", end=" ", flush=True)

        # Read just Circ column
        df = pd.read_csv(fpath, compression=comp, usecols=[c["circ_col"]],
                         engine=engine)
        n = len(df)
        if n == 0:
            print("empty"); continue
        avg_circ = df[c["circ_col"]].mean()

        # Get count and area from bragg_data.csv
        m = bd[(bd["Stack"] == sn) & (bd["Plate"] == pname)]
        if len(m) == 0:
            print(f"WARN: no bragg_data for {sn}/{pname}")
            cnt = n
            avg_area = np.nan
        else:
            cnt = int(m["count"].values[0])
            avg_area = m["avg_area_um2"].values[0]

        if n < MIN_COUNT:
            rows.append({"Stack": sn, "Plate": pname, "Count": cnt,
                          "Avg_Area_um2": round(avg_area, 1) if not np.isnan(avg_area) else "---",
                          "Avg_Circ": round(avg_circ, 3)})
            print(f"N={cnt} (low stats)")
            continue

        rows.append({"Stack": sn, "Plate": pname, "Count": cnt,
                      "Avg_Area_um2": round(avg_area, 1) if not np.isnan(avg_area) else "---",
                      "Avg_Circ": round(avg_circ, 3)})
        print(f"N={cnt}, area={avg_area:.1f}, circ={avg_circ:.3f}")

out = pd.DataFrame(rows)
out.to_csv(os.path.join(base, "analysis", "beam_profile_params.csv"), index=False)
print("\nDone.")
print(out.to_string(index=False))
