o2_add_test(elink-decoder
            SOURCES testElinkDecoder.cxx common.cxx
            COMPONENT_NAME mch
            LABELS "mch;raw"
            PUBLIC_LINK_LIBRARIES O2::MCHMappingImpl3 O2::MCHRaw Boost::boost)

o2_add_test(elink-encoder
            SOURCES testElinkEncoder.cxx common.cxx
            COMPONENT_NAME mch
            LABELS "mch;raw"
            PUBLIC_LINK_LIBRARIES O2::MCHMappingImpl3 O2::MCHRaw Boost::boost)

o2_add_test(gbt-encoder
            SOURCES testGBTEncoder.cxx
            COMPONENT_NAME mch
            LABELS "mch;raw"
            PUBLIC_LINK_LIBRARIES O2::MCHMappingImpl3 O2::MCHRaw Boost::boost)

o2_add_test(gbt-decoder
            SOURCES testGBTDecoder.cxx
            COMPONENT_NAME mch
            LABELS "mch;raw"
            PUBLIC_LINK_LIBRARIES O2::MCHMappingImpl3 O2::MCHRaw Boost::boost)

o2_add_test(sampa-header
            SOURCES testSampaHeader.cxx
            COMPONENT_NAME mch
            LABELS "mch;raw"
            PUBLIC_LINK_LIBRARIES O2::MCHMappingImpl3 O2::MCHRaw Boost::boost)

o2_add_test(bitset
            SOURCES testBitSet.cxx
            COMPONENT_NAME mch
            LABELS "mch;raw"
            PUBLIC_LINK_LIBRARIES O2::MCHRaw Boost::boost fmt::fmt)

o2_add_test(encoder
            SOURCES testEncoder.cxx
            COMPONENT_NAME mch
            LABELS "mch;raw"
            PUBLIC_LINK_LIBRARIES O2::MCHMappingImpl3 O2::MCHRaw Boost::boost)

if(benchmark_FOUND)
  o2_add_executable(bitset
                    SOURCES benchmarkBitSet.cxx
                    IS_BENCHMARK
                    PUBLIC_LINK_LIBRARIES O2::MCHRaw benchmark::benchmark
                    COMPONENT_NAME mch)
endif()
