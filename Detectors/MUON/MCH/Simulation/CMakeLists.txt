set(MODULE_NAME "MCHSimulation")

O2_SETUP(NAME ${MODULE_NAME})

link_directories( ${LINK_DIRECTORIES})

set(SRCS
   src/SlatGeometry.cxx
)

set(HEADERS
   include/${MODULE_NAME}/SlatGeometry.h
)

Set(LIBRARY_NAME ${MODULE_NAME})
set(BUCKET_NAME mch_simulation_bucket)

O2_GENERATE_LIBRARY()
