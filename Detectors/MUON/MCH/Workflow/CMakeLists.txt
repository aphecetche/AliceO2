add_executable(mch-hit-generator hitGeneratorWorkflow.cxx generateHitPositions.cxx)
target_link_libraries(mch-hit-generator Framework MCHMappingSegContour3)
install(TARGETS mch-hit-generator RUNTIME DESTINATION bin)

add_executable(mch-test test.cxx)
target_link_libraries(mch-test Headers Framework)
