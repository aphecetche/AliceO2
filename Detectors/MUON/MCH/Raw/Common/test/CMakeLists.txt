o2_add_test(rdh-manip
            SOURCES testRDHManip.cxx
            COMPONENT_NAME mch
            LABELS "mch;raw"
            PUBLIC_LINK_LIBRARIES O2::MCHRawCommon Boost::boost)

o2_add_test(sampa-header
            SOURCES testSampaHeader.cxx
            COMPONENT_NAME mch
            LABELS "mch;raw"
            PUBLIC_LINK_LIBRARIES O2::MCHRawCommon Boost::boost)

o2_add_test(sampa-cluster
            SOURCES testSampaCluster.cxx
            COMPONENT_NAME mch
            LABELS "mch;raw"
            PUBLIC_LINK_LIBRARIES O2::MCHRawCommon Boost::boost)
