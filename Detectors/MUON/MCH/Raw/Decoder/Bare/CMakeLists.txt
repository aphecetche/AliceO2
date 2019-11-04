o2_add_library(MCHRawBareDecoder
               SOURCES BareDecoder.cxx BareCRUDecoder.cxx BareElinkDecoder.cxx
                       BareGBTDecoder.cxx
               PUBLIC_LINK_LIBRARIES O2::MCHRawDecoder
               PRIVATE_LINK_LIBRARIES O2::MCHRawImplHelpers O2::MCHRawDecoder)

if(BUILD_TESTING)
  o2_add_test(bare-elink-decoder
              SOURCES testBareElinkDecoder.cxx
              COMPONENT_NAME mch
              LABELS "mch;raw"
              PUBLIC_LINK_LIBRARIES O2::MCHRawBareDecoder)

  o2_add_test(bare-gbt-decoder
              SOURCES testBareGBTDecoder.cxx
              COMPONENT_NAME mch
              LABELS "mch;raw"
              PUBLIC_LINK_LIBRARIES O2::MCHRawBareDecoder O2::MCHTestRawRefBuffers)

endif()
