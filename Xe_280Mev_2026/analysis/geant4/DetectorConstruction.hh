#ifndef DETECTORCONSTRUCTION_HH
#define DETECTORCONSTRUCTION_HH

#include <G4VUserDetectorConstruction.hh>
#include <G4Material.hh>
#include <G4LogicalVolume.hh>
#include <vector>

class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
  DetectorConstruction();
  virtual ~DetectorConstruction() = default;
  virtual G4VPhysicalVolume* Construct() override;

  struct LayerInfo {
    G4String name;
    G4double thickness;
    G4double depthCenter;  // center Z of this layer from beam entrance
  };

  static const std::vector<LayerInfo>& GetLayerInfos() { return fLayerInfos; }
  static G4int GetTotalLayers() { return fTotalLayers; }

private:
  void DefineMaterials();
  static std::vector<LayerInfo> fLayerInfos;
  static G4int fTotalLayers;
};

#endif
