#ifndef STEPPINGACTION_HH
#define STEPPINGACTION_HH

#include <G4UserSteppingAction.hh>
#include <G4String.hh>
#include <vector>
#include <map>

class SteppingAction : public G4UserSteppingAction
{
public:
  SteppingAction();
  virtual ~SteppingAction() = default;
  virtual void UserSteppingAction(const G4Step*) override;

  static void ResetEvent();
  static void EndOfEvent();

  struct LayerData {
    G4double edep = 0;
    G4double trackLen = 0;
    G4int steps = 0;
  };

  // Per-layer data for current event: map layer_index -> data
  static std::map<G4int, LayerData> fEventData;

  // Accumulated across all events
  static std::vector<G4double> fTotalEdep;
  static std::vector<G4double> fTotalTrackLen;
  static std::vector<G4int> fTotalSteps;
  static G4int fTotalEvents;
};

#endif
