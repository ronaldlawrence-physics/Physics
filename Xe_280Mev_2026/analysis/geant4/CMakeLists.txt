cmake_minimum_required(VERSION 3.16)
project(XeEmulsionSim CXX)

set(CMAKE_CXX_STANDARD 17)
find_package(Geant4 REQUIRED)

include(${Geant4_USE_FILE})

add_executable(XeEmulsionSim
  XeEmulsionSim.cc
  DetectorConstruction.cc
  PrimaryGeneratorAction.cc
  SteppingAction.cc
  EventAction.cc
  RunAction.cc
)

target_link_libraries(XeEmulsionSim ${Geant4_LIBRARIES})
