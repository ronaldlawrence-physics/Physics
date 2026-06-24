#include <G4RunManagerFactory.hh>
#include <G4UImanager.hh>
#include <G4VisManager.hh>
#include <G4VisExecutive.hh>
#include <FTFP_BERT.hh>
#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"

int main(int argc, char** argv)
{
  auto* runManager = G4RunManagerFactory::CreateRunManager();
  runManager->SetUserInitialization(new DetectorConstruction());
  runManager->SetUserInitialization(new FTFP_BERT());
  runManager->SetUserAction(new PrimaryGeneratorAction());
  runManager->SetUserAction(new RunAction());
  runManager->SetUserAction(new EventAction());
  runManager->SetUserAction(new SteppingAction());

  // Visualization manager
  auto* visManager = new G4VisExecutive();
  visManager->Initialize();

  G4UImanager* ui = G4UImanager::GetUIpointer();
  if (argc == 2) {
    ui->ApplyCommand("/control/execute " + G4String(argv[1]));
  } else {
    ui->ApplyCommand("/run/initialize");
    ui->ApplyCommand("/run/beamOn 1000");
  }

  delete visManager;
  delete runManager;
  return 0;
}
