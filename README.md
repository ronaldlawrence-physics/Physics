# Xe-124 280 MeV/nucleon analysis in nuclear emulsion

Automated analysis of nuclear emulsion exposed to 124Xe+54 ions at about 300 MeV per nucleon at the JINR Nuclotron. Three stacks exposed transversely to the beam, Bragg peak identification, particle classification via Gaussian Mixture Model, beam profile analysis, and SRIM simulation.

## Repository structure

```
├── Xe_280Mev_2026/
│   ├── Stack_01/              # Stack 01 data (1 cycle)
│   ├── Stack_02/              # Stack 02 data (5 cycles)
│   ├── Stack_03/              # Stack 03 data (10 cycles)
│   ├── 20x.jpg / 40x.jpg / 4x.jpg  # Emulsion microscopy images
│   └── analysis/              # Python scripts, ROOT macros, SRIM, report
│       ├── analysis.py            # Core processing pipeline
│       ├── beam_profile_params.py # Per-plate beam profile stats
│       ├── plot_bragg.C           # ROOT Bragg peak visualizations
│       ├── plot_class2.C          # Class 2 (primary beam) analysis
│       ├── gen_3d_stack.py        # 3D stack model renderer
│       ├── SRIM/                  # SRIM-2013 simulation output files
│       ├── Finalreport.tex        # LaTeX source (comprehensive report)
│       └── Finalreport.pdf        # Compiled final report (33 pp)
└── README.md
```

## Key results

- **Bragg peak**: 8–10 mm (Stack 01), 10–12 mm (Stacks 02/03)
- **GMM classification**: 3 particle classes (light fragments, mid-mass fragments, primary beam)
- **Beam profile**: Uniform, FWHM ~10–15 mm, circularity ~0.6–0.8
- **SRIM**: 280 MeV/nucleon, range 11.55 mm, Bragg peak at 8.30 mm (5505 MeV/mm)

## Requirements

- Python 3.10+ with pandas, numpy, scikit-learn, matplotlib
- ROOT 6 (for plot macros)
- LaTeX (for report compilation)
