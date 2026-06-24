#include "PrimaryGeneratorAction.hh"

#include <G4ParticleTable.hh>
#include <G4IonTable.hh>
#include <G4SystemOfUnits.hh>
#include <Randomize.hh>

PrimaryGeneratorAction::PrimaryGeneratorAction()
  : G4VUserPrimaryGeneratorAction(), fParticleGun(nullptr)
{
  fParticleGun = new G4ParticleGun(1);
  fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0, 0, -1));
  fParticleGun->SetParticlePosition(G4ThreeVector(0, 0, 25 * mm));
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fParticleGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
  G4int Z = 54, A = 124;
  // 280 MeV/nucleon -> total kinetic energy ~34.7 GeV
  G4double totalE = 280.0 * MeV * A;

  auto* particleTable = G4ParticleTable::GetParticleTable();
  auto* ion = particleTable->GetIonTable()->GetIon(Z, A, 0);
  fParticleGun->SetParticleDefinition(ion);
  fParticleGun->SetParticleEnergy(totalE);
  fParticleGun->GeneratePrimaryVertex(event);
}
