o2_add_library(MCHRawCommon
               SOURCES src/RDHManip.cxx src/SampaHeader.cxx src/SampaCluster.cxx
               PUBLIC_LINK_LIBRARIES fmt::fmt ms_gsl::ms_gsl Boost::boost
                                     O2::Headers
               PRIVATE_LINK_LIBRARIES O2::MCHRawImplHelpers)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
