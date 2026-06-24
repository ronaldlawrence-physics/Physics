#include "RunAction.hh"
#include "SteppingAction.hh"
#include "DetectorConstruction.hh"

#include <G4Run.hh>
#include <G4SystemOfUnits.hh>
#include <G4AccumulableManager.hh>
#include <fstream>

RunAction::RunAction() = default;

void RunAction::BeginOfRunAction(const G4Run*)
{
  SteppingAction::fTotalEdep.clear();
  SteppingAction::fTotalTrackLen.clear();
  SteppingAction::fTotalSteps.clear();
  SteppingAction::fTotalEvents = 0;
}

void RunAction::EndOfRunAction(const G4Run* run)
{
  G4int nEvents = run->GetNumberOfEvent();
  if (nEvents == 0) return;

  const auto& layers = DetectorConstruction::GetLayerInfos();
  G4int nLayers = layers.size();

  std::ofstream csv("geant4_bragg.csv");
  csv << "layer,material,depth_mm,edep_MeV,edep_per_mm_MeV,"
      << "trackLen_mm,norm_edep\n";

  G4double maxEdepPerMm = 0;
  for (G4int i = 0; i < nLayers; ++i) {
    G4double edepPerMm =
      (i < (G4int)SteppingAction::fTotalEdep.size())
        ? SteppingAction::fTotalEdep[i] / (nEvents * layers[i].thickness / mm)
        : 0;
    if (edepPerMm > maxEdepPerMm) maxEdepPerMm = edepPerMm;
  }

  for (G4int i = 0; i < nLayers; ++i) {
    G4double depth = layers[i].depthCenter / mm;
    G4double edep = (i < (G4int)SteppingAction::fTotalEdep.size())
      ? SteppingAction::fTotalEdep[i] / nEvents / MeV : 0;
    G4double trackLen = (i < (G4int)SteppingAction::fTotalTrackLen.size())
      ? SteppingAction::fTotalTrackLen[i] / nEvents / mm : 0;
    G4double edepPerMm =
      (i < (G4int)SteppingAction::fTotalEdep.size())
        ? SteppingAction::fTotalEdep[i] / (nEvents * layers[i].thickness / mm)
        : 0;

    csv << i << ","
        << layers[i].name << ","
        << depth << ","
        << edep << ","
        << edepPerMm << ","
        << trackLen << ","
        << (maxEdepPerMm > 0 ? edepPerMm / maxEdepPerMm : 0)
        << "\n";
  }

  csv.close();

  std::ofstream rangeCsv("geant4_range.csv");
  rangeCsv << "ion,energy,z_final_mm\n";

  G4int totalSteps = 0;
  for (auto s : SteppingAction::fTotalSteps) totalSteps += s;
  G4double totalEdepAll = 0;
  for (auto e : SteppingAction::fTotalEdep) totalEdepAll += e;

  G4cout << "\n=== Geant4 Bragg curve written to geant4_bragg.csv ===\n"
         << "Events processed: " << nEvents << "\n"
         << "Layers with data: "
         << (G4int)SteppingAction::fTotalEdep.size() << "\n"
         << "Total steps: " << totalSteps << "\n"
         << "Total energy deposited: " << totalEdepAll / MeV << " MeV"
         << G4endl;
}
