o2_add_test(elink-encoder
            SOURCES testElinkEncoder.cxx
            COMPONENT_NAME mch
            LABELS "mch;raw"
            PUBLIC_LINK_LIBRARIES O2::MCHRawBareEncoder
                                  O2::MCHRawUserLogicEncoder)

o2_add_test(gbt-encoder
            SOURCES testGBTEncoder.cxx
            COMPONENT_NAME mch
            LABELS "mch;raw"
            PUBLIC_LINK_LIBRARIES O2::MCHRawBareEncoder
                                  O2::MCHRawUserLogicEncoder
                                  O2::MCHTestRawRefBuffers
                                  O2::MCHRawImplHelpers)
