#include "SteppingAction.hh"
#include "DetectorConstruction.hh"

#include <G4Step.hh>
#include <G4VTouchable.hh>
#include <G4VPhysicalVolume.hh>
#include <G4SystemOfUnits.hh>

std::map<G4int, SteppingAction::LayerData> SteppingAction::fEventData;
std::vector<G4double> SteppingAction::fTotalEdep;
std::vector<G4double> SteppingAction::fTotalTrackLen;
std::vector<G4int> SteppingAction::fTotalSteps;
G4int SteppingAction::fTotalEvents = 0;

SteppingAction::SteppingAction() = default;

void SteppingAction::UserSteppingAction(const G4Step* step)
{
  G4int layerIdx = step->GetPreStepPoint()->GetTouchable()->GetReplicaNumber(0);
  G4double edep = step->GetTotalEnergyDeposit();
  G4double stepLen = step->GetStepLength();

  auto& data = fEventData[layerIdx];
  data.edep += edep;
  data.trackLen += stepLen;
  data.steps += 1;
}

void SteppingAction::ResetEvent()
{
  fEventData.clear();
}

void SteppingAction::EndOfEvent()
{
  G4int nLayers = DetectorConstruction::GetTotalLayers();
  if (fTotalEdep.empty()) {
    fTotalEdep.resize(nLayers, 0);
    fTotalTrackLen.resize(nLayers, 0);
    fTotalSteps.resize(nLayers, 0);
  }

  for (const auto& [idx, data] : fEventData) {
    if (idx >= 0 && idx < nLayers) {
      fTotalEdep[idx] += data.edep;
      fTotalTrackLen[idx] += data.trackLen;
      fTotalSteps[idx] += data.steps;
    }
  }
  fTotalEvents++;
}
