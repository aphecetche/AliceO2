set(MODULE_NAME "MCHSimulation")

O2_SETUP(NAME ${MODULE_NAME})

set(SRCS
   src/Detector.cxx
   src/Materials.cxx
   src/SlatGeometry.cxx
   src/Stepper.cxx
)

set(HEADERS
   include/${MODULE_NAME}/Detector.h
   include/${MODULE_NAME}/Hit.h
   include/${MODULE_NAME}/Materials.h
   include/${MODULE_NAME}/SlatGeometry.h
   include/${MODULE_NAME}/Stepper.h
)

SET(LINKDEF src/MCHSimulationLinkDef.h)
Set(LIBRARY_NAME ${MODULE_NAME})
set(BUCKET_NAME mch_simulation_bucket)

O2_GENERATE_LIBRARY()
