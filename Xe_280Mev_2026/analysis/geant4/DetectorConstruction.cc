#include "DetectorConstruction.hh"

#include <G4Box.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4NistManager.hh>
#include <G4SystemOfUnits.hh>
#include <G4PhysicalConstants.hh>

std::vector<DetectorConstruction::LayerInfo> DetectorConstruction::fLayerInfos;
G4int DetectorConstruction::fTotalLayers = 0;

DetectorConstruction::DetectorConstruction()
{
  DefineMaterials();
}

void DetectorConstruction::DefineMaterials()
{
  auto* nist = G4NistManager::Instance();

  // Ilford G5 emulsion: Ag (49%) + Br (36%) + C (7%) + O (7%) + H (1%)
  // Approximated as AgBr (85%) + gelatin (15%) by volume
  auto* Ag = nist->FindOrBuildElement("Ag");
  auto* Br = nist->FindOrBuildElement("Br");
  auto* C  = nist->FindOrBuildElement("C");
  auto* O  = nist->FindOrBuildElement("O");
  auto* H  = nist->FindOrBuildElement("H");
  auto* Si = nist->FindOrBuildElement("Si");
  auto* Na = nist->FindOrBuildElement("Na");
  auto* Ca = nist->FindOrBuildElement("Ca");
  auto* Mg = nist->FindOrBuildElement("Mg");
  auto* Al = nist->FindOrBuildElement("Al");

  // Emulsion (density 3.907 g/cm3)
  auto* emulsionMat = new G4Material("Emulsion_G5", 3.907 * g/cm3, 5);
  emulsionMat->AddElement(Ag, 0.49);
  emulsionMat->AddElement(Br, 0.36);
  emulsionMat->AddElement(C, 0.07);
  emulsionMat->AddElement(O, 0.07);
  emulsionMat->AddElement(H, 0.01);

  // Soda-lime glass (density 2.330 g/cm3)
  auto* glassMat = new G4Material("SodaLime", 2.330 * g/cm3, 6);
  glassMat->AddElement(O,  0.60);
  glassMat->AddElement(Si, 0.25);
  glassMat->AddElement(Na, 0.10);
  glassMat->AddElement(Ca, 0.03);
  glassMat->AddElement(Mg, 0.01);
  glassMat->AddElement(Al, 0.01);

  // Vacuum
  new G4Material("Vacuum", 1, 1.01 * g/mole, universe_mean_density);
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  auto* nist = G4NistManager::Instance();
  auto* vacuum = nist->FindOrBuildMaterial("Vacuum");
  auto* emulsionMat = nist->FindOrBuildMaterial("Emulsion_G5");
  auto* glassMat = nist->FindOrBuildMaterial("SodaLime");

  const G4double worldSize = 50 * mm;
  auto* worldSolid = new G4Box("World", worldSize, worldSize, worldSize);
  auto* worldLog = new G4LogicalVolume(worldSolid, vacuum, "World");
  auto* worldPhys = new G4PVPlacement(nullptr, G4ThreeVector(), worldLog,
                                      "World", nullptr, false, 0);

  const G4double plateXY = 40 * mm;
  const G4double emThick = 60 * micrometer;
  const G4double glassThick = 2.0 * mm;
  const G4int nPlates = 10;

  fLayerInfos.clear();
  // Build from top (positive Z) downward to bottom (negative Z)
  G4double zPos = nPlates * (emThick + glassThick) / 2.0;
  G4int copyIdx = 0;

  for (G4int i = 0; i < nPlates; ++i) {
    // Alternating layer order: even plates = emulsion first, odd = glass first
    // "First" = at the top (higher Z), since beam enters from top going -Z
    if (i % 2 == 0) {
      // Emulsion at top of plate, glass below
      auto* emSolid = new G4Box("Emulsion_" + std::to_string(i),
                                plateXY/2, plateXY/2, emThick/2);
      auto* emLog = new G4LogicalVolume(emSolid, emulsionMat,
                                        "EmulsionLog_" + std::to_string(i));
      new G4PVPlacement(nullptr,
                        G4ThreeVector(0, 0, zPos - emThick/2),
                        emLog, "Emulsion_" + std::to_string(i),
                        worldLog, false, copyIdx);
      fLayerInfos.push_back({"emulsion", emThick, zPos - emThick/2});
      zPos -= emThick;
      ++copyIdx;

      auto* glassSolid = new G4Box("Glass_" + std::to_string(i),
                                   plateXY/2, plateXY/2, glassThick/2);
      auto* glassLog = new G4LogicalVolume(glassSolid, glassMat,
                                           "GlassLog_" + std::to_string(i));
      new G4PVPlacement(nullptr,
                        G4ThreeVector(0, 0, zPos - glassThick/2),
                        glassLog, "Glass_" + std::to_string(i),
                        worldLog, false, copyIdx);
      fLayerInfos.push_back({"glass", glassThick, zPos - glassThick/2});
      zPos -= glassThick;
      ++copyIdx;
    } else {
      // Glass at top of plate, emulsion below
      auto* glassSolid = new G4Box("Glass_" + std::to_string(i),
                                   plateXY/2, plateXY/2, glassThick/2);
      auto* glassLog = new G4LogicalVolume(glassSolid, glassMat,
                                           "GlassLog_" + std::to_string(i));
      new G4PVPlacement(nullptr,
                        G4ThreeVector(0, 0, zPos - glassThick/2),
                        glassLog, "Glass_" + std::to_string(i),
                        worldLog, false, copyIdx);
      fLayerInfos.push_back({"glass", glassThick, zPos - glassThick/2});
      zPos -= glassThick;
      ++copyIdx;

      auto* emSolid = new G4Box("Emulsion_" + std::to_string(i),
                                plateXY/2, plateXY/2, emThick/2);
      auto* emLog = new G4LogicalVolume(emSolid, emulsionMat,
                                        "EmulsionLog_" + std::to_string(i));
      new G4PVPlacement(nullptr,
                        G4ThreeVector(0, 0, zPos - emThick/2),
                        emLog, "Emulsion_" + std::to_string(i),
                        worldLog, false, copyIdx);
      fLayerInfos.push_back({"emulsion", emThick, zPos - emThick/2});
      zPos -= emThick;
      ++copyIdx;
    }
  }

  fTotalLayers = fLayerInfos.size();
  return worldPhys;
}
