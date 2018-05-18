set(MODULE_NAME "MCHSimulation")

O2_SETUP(NAME ${MODULE_NAME})

set(SRCS
   src/Detector.cxx
   src/Materials.cxx
   src/Materials.h
   src/SlatGeometry.cxx
   src/SlatGeometry.h
   src/Stepper.cxx
   src/Stepper.h
)

set(HEADERS
   include/${MODULE_NAME}/Detector.h
   include/${MODULE_NAME}/Hit.h
)

SET(LINKDEF src/MCHSimulationLinkDef.h)
Set(LIBRARY_NAME ${MODULE_NAME})
set(BUCKET_NAME mch_simulation_bucket)

O2_GENERATE_LIBRARY()
