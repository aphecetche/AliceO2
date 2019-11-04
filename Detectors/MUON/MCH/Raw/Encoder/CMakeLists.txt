add_subdirectory(Bare)
add_subdirectory(UserLogic)

o2_add_library(MCHRawEncoder
               SOURCES src/Encoder.cxx
               PUBLIC_LINK_LIBRARIES O2::MCHRawCommon
               PRIVATE_LINK_LIBRARIES O2::MCHRawBareEncoder
                                      O2::MCHRawUserLogicEncoder
                                      O2::MCHRawImplHelpers)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
