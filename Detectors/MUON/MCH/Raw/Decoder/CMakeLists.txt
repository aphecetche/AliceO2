o2_add_library(MCHRawDecoder
               SOURCES src/Decoder.cxx
               PUBLIC_LINK_LIBRARIES O2::MCHRawCommon)

add_subdirectory(Bare)
# add_subdirectory(UserLogic)

if(BUILD_TESTING)
  o2_add_test(decoder
              SOURCES src/testDecoder.cxx
              COMPONENT_NAME mch
              LABELS "mch;raw"
              PUBLIC_LINK_LIBRARIES O2::MCHRawBareDecoder
                                    O2::MCHTestRawRefBuffers)

endif()
