o2_add_library(MCHRaw
               SOURCES src/BareDecoder.cxx
                       src/BitSet.cxx
                       src/CRUBareDecoder.cxx
                       src/CRUEncoder.cxx
                       src/CompactBitSetString.cxx
                       src/Decoder.cxx
                       src/ElinkDecoder.cxx
                       src/ElinkEncoder.cxx
                       src/Encoder.cxx
                       src/GBTDecoder.cxx
                       src/GBTEncoder.cxx
                       src/NofBits.cxx
                       src/RAWDataHeader.cxx
                       src/SampaCluster.cxx
                       src/SampaHeader.cxx
                       src/UserLogicDecoder.cxx
               PUBLIC_LINK_LIBRARIES fmt::fmt ms_gsl::ms_gsl Boost::boost)

o2_add_executable(rawdump
                  SOURCES src/rawdump.cxx
                  COMPONENT_NAME mch
                  PUBLIC_LINK_LIBRARIES O2::MCHMappingImpl3 O2::MCHRaw
                                        Boost::program_options)
o2_add_executable(readout-debug
                  SOURCES src/readoutDebug.cxx
                  COMPONENT_NAME mch
                  PUBLIC_LINK_LIBRARIES O2::MCHMappingImpl3 O2::MCHRaw
                                        O2::Framework Boost::program_options)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
