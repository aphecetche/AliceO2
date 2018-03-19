SET(MODULE_NAME MCHSimulation)

O2_SETUP(NAME ${MODULE_NAME})

set(SRCS
        src/Detector.cxx
        src/SlatGeometry.cxx
        src/SlatMaterials.cxx
        )

set(HEADERS
        include/MCHSimulation/Detector.h
        )

SET(LINKDEF src/MCHSimulationLinkDef.h)
SET(LIBRARY_NAME ${MODULE_NAME})
SET(BUCKET_NAME mch_simulation_bucket)

O2_GENERATE_LIBRARY()
