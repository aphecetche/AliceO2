
add_executable(mch-clustering-workflow ClusterReader.cxx ClusterWriter.cxx mch-clustering-workflow.cxx)
target_link_libraries(mch-clustering-workflow Framework flatbuffers::flatbuffers)
install(TARGETS mch-clustering-workflow RUNTIME DESTINATION bin)


