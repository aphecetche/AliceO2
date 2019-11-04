o2_add_library(MCHRawUserLogicDecoder
               SOURCES UserLogicDecoder.cxx
               PUBLIC_LINK_LIBRARIES fmt::fmt ms_gsl::ms_gsl Boost::boost
                                     O2::MCHRawDecoder
               PRIVATE_LINK_LIBRARIES O2::MCHRawImplHelpers)
