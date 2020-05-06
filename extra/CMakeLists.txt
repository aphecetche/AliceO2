
o2_add_executable(extra-config
        SOURCES extra-config.cxx
        PUBLIC_LINK_LIBRARIES O2::CommonUtils)
o2_add_executable(extra-config-phos
        SOURCES extra-config.cxx
        PUBLIC_LINK_LIBRARIES O2::CommonUtils O2::PHOSSimulation)
o2_add_executable(extra-config-hbfutils
        SOURCES extra-config.cxx
        PUBLIC_LINK_LIBRARIES O2::CommonUtils O2::DetectorsRaw)

if(TARGET O2::BazParam)
  o2_add_executable(extra-config-baz
          SOURCES extra-config.cxx
          PUBLIC_LINK_LIBRARIES O2::CommonUtils O2::BazParam)
endif()
