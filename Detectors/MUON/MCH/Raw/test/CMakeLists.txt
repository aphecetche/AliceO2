o2_add_test(elink-encoder
        SOURCES
        testElinkEncoder.cxx
        COMPONENT_NAME mch
        LABELS "mch;raw"
        PUBLIC_LINK_LIBRARIES O2::MCHMappingImpl3 O2::MCHRaw
        Boost::boost)

o2_add_test(encoder
        SOURCES
        testEncoder.cxx
        COMPONENT_NAME mch
        LABELS "mch;raw"
        PUBLIC_LINK_LIBRARIES O2::MCHMappingImpl3 O2::MCHRaw
        Boost::boost)

o2_add_test(sampa-header
        SOURCES
        testSampaHeader.cxx
        COMPONENT_NAME mch
        LABELS "mch;raw"
        PUBLIC_LINK_LIBRARIES O2::MCHMappingImpl3 O2::MCHRaw
        Boost::boost)

o2_add_test(bitset
        SOURCES
        testBitSet.cxx
        COMPONENT_NAME mch
        LABELS "mch;raw"
        PUBLIC_LINK_LIBRARIES O2::MCHRaw Boost::boost fmt::fmt)
