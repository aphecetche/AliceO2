o2_add_library(MCHRawUserLogicEncoder
               SOURCES UserLogicGBTEncoder.cxx
               PUBLIC_LINK_LIBRARIES O2::MCHRawCommon
               PRIVATE_LINK_LIBRARIES O2::MCHRawImplHelpers
               PUBLIC_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR})
