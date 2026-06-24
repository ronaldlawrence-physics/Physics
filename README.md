# Xe-124 280 MeV/nucleon analysis in nuclear emulsion

Automated analysis of nuclear emulsion exposed to 124Xe+54 ions at about 300 MeV per nucleon at the JINR Nuclotron. Three stacks exposed transversely to the beam, Bragg peak identification, particle classification via Gaussian Mixture Model, beam profile analysis, and Geant4 simulation.

## Repository structure

```
├── Xe_280Mev_2026/
│   ├── Stack_01/              # Stack 01 data (1 cycle)
│   ├── Stack_02/              # Stack 02 data (5 cycles)
│   ├── Stack_03/              # Stack 03 data (10 cycles)
│   ├── 20x.jpg / 40x.jpg / 4x.jpg  # Emulsion microscopy images
│   └── analysis/              # Python scripts, ROOT macros, Geant4, SRIM, report
│       ├── analysis.py            # Core processing pipeline
│       ├── beam_profile_params.py # Per-plate beam profile stats
│       ├── plot_bragg.C           # ROOT Bragg peak visualizations
│       ├── plot_class2.C          # Class 2 (primary beam) analysis
│       ├── gen_3d_stack.py        # 3D stack model renderer
│       ├── SRIM/                  # SRIM-2013 simulation output files
│       ├── geant4/                # Geant4 simulation (source + build)
│       │   ├── XeEmulsionSim.cc   # Main simulation
│       │   ├── DetectorConstruction.cc/hh
│       │   ├── EventAction.cc/hh
│       │   ├── PrimaryGeneratorAction.cc/hh
│       │   ├── RunAction.cc/hh
│       │   ├── SteppingAction.cc/hh
│       │   └── build/             # Compiled binary + output plots
│       ├── Finalreport.tex        # LaTeX source (comprehensive report)
│       └── Finalreport.pdf        # Compiled final report (48 pp)
└── README.md
```

## Key results

- **Bragg peak**: 8–10 mm (Stack 01), 10–12 mm (Stacks 02/03)
- **GMM classification**: 3 particle classes (light fragments, mid-mass fragments, primary beam)
- **Beam profile**: Uniform, FWHM ~10–15 mm, circularity ~0.6–0.8
- **SRIM**: 280 MeV total (~2.1 MeV/nucleon), range 16.4 µm (not representative of experiment)
- **Geant4**: Full detector simulation with energy deposition and Bragg curve reproduced

## Requirements

- Python 3.10+ with pandas, numpy, scikit-learn, matplotlib
- ROOT 6 (for plot macros)
- Geant4 11+ (for simulation rebuild)
- CMake 3.20+ (for simulation build)
- LaTeX (for report compilation)
