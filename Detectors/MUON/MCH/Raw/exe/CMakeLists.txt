o2_add_executable(rawdump
                  SOURCES rawdump.cxx
                  COMPONENT_NAME mch
                  PUBLIC_LINK_LIBRARIES O2::MCHMappingImpl3 O2::MCHRawDecoder
                                        Boost::program_options)
o2_add_executable(readout-debug
                  SOURCES readoutDebug.cxx
                  COMPONENT_NAME mch
                  PUBLIC_LINK_LIBRARIES O2::MCHMappingImpl3 O2::MCHRawDecoder
                                        O2::Framework Boost::program_options)

