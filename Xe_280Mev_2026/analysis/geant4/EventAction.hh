#ifndef EVENTACTION_HH
#define EVENTACTION_HH

#include <G4UserEventAction.hh>

class EventAction : public G4UserEventAction
{
public:
  EventAction();
  virtual ~EventAction() = default;
  virtual void BeginOfEventAction(const G4Event*) override;
  virtual void EndOfEventAction(const G4Event*) override;
};

#endif
