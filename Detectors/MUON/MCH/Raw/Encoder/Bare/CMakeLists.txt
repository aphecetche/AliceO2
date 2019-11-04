o2_add_library(MCHRawBareEncoder
               SOURCES BitSet.cxx BareElinkEncoder.cxx BareGBTEncoder.cxx
               PUBLIC_LINK_LIBRARIES O2::MCHRawCommon
               PRIVATE_LINK_LIBRARIES O2::MCHRawImplHelpers
               PUBLIC_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR})

if(BUILD_TESTING)
o2_add_test(bitset
            SOURCES testBitSet.cxx
            COMPONENT_NAME mch
            LABELS "mch;raw"
            PUBLIC_LINK_LIBRARIES O2::MCHRawBareEncoder Boost::boost fmt::fmt)

o2_add_test(bare-elink-encoder
            SOURCES testBareElinkEncoder.cxx
            COMPONENT_NAME mch
            LABELS "mch;raw"
            PUBLIC_LINK_LIBRARIES O2::MCHRawBareEncoder Boost::boost)

o2_add_test(bare-gbt-encoder
            SOURCES testBareGBTEncoder.cxx
            COMPONENT_NAME mch
            LABELS "mch;raw"
            PUBLIC_LINK_LIBRARIES O2::MCHRawBareEncoder O2::MCHTestRawRefBuffers)

endif()

if(benchmark_FOUND)
  o2_add_executable(bitset
                    COMPONENT_NAME mch
                    SOURCES benchBitSet.cxx
                    PUBLIC_LINK_LIBRARIES O2::MCHRawBareEncoder benchmark::benchmark
                    IS_BENCHMARK)
endif()
