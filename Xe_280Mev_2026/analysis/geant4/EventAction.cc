#include "EventAction.hh"
#include "SteppingAction.hh"

EventAction::EventAction() = default;

void EventAction::BeginOfEventAction(const G4Event*)
{
  SteppingAction::ResetEvent();
}

void EventAction::EndOfEventAction(const G4Event*)
{
  SteppingAction::EndOfEvent();
}
