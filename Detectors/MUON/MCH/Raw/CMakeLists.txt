o2_add_library(MCHRaw
        SOURCES src/BitSet.cxx src/ElinkEncoder.cxx src/SampaHeader.cxx src/ElinkDecoder.cxx src/GBTEncoder.cxx src/GBTDecoder.cxx
        PUBLIC_LINK_LIBRARIES fmt::fmt ms_gsl::ms_gsl Boost::boost)

o2_add_executable(rawdump
        SOURCES src/rawdump.cxx
        COMPONENT_NAME mch
        PUBLIC_LINK_LIBRARIES O2::MCHMappingImpl3
        Boost::program_options)

if(BUILD_TESTING)
        add_subdirectory(test)
endif()
