o2_add_test(cru-encoder
            SOURCES testCRUEncoder.cxx
            COMPONENT_NAME mch
            LABELS "mch;raw"
            PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder)

# o2_add_test(encoder
#             SOURCES testEncoder.cxx
#             COMPONENT_NAME mch
#             LABELS "mch;raw"
#             PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder Boost::boost)
