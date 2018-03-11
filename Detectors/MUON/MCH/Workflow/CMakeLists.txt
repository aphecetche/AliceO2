add_executable(mch-hit-generator hitGeneratorWorkflow.cxx)
target_link_libraries(mch-hit-generator Framework MCHMappingImpl3)
install(TARGETS mch-hit-generator RUNTIME DESTINATION bin)