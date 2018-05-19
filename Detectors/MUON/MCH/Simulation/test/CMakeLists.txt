O2_SETUP(NAME MCHSimulationTest)
set(BUCKET_NAME mch_simulation_test_bucket)

# trying to get only one test exe for this module
O2_GENERATE_EXECUTABLE(
        EXE_NAME test_MCHSimulation
        SOURCES testGeometryTransformer.cxx
        BUCKET_NAME mch_simulation_test_bucket
)

target_link_libraries(test_MCHSimulation Boost::unit_test_framework MCHSimulation)
add_test(NAME test_MCHSimulation COMMAND test_MCHSimulation)

# if (benchmark_FOUND)
#     O2_GENERATE_EXECUTABLE(
#             EXE_NAME mch-mapping-bench-segmentation3
#             SOURCES src/BenchSegmentation.cxx 
#             BUCKET_NAME mch_mapping_test_bucket)
# endif ()
