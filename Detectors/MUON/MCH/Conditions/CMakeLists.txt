# Copyright CERN and copyright holders of ALICE O2. This software is distributed
# under the terms of the GNU General Public License v3 (GPL Version 3), copied
# verbatim in the file "COPYING".
#
# See http://alice-o2.web.cern.ch/license for full licensing information.
#
# In applying this license CERN does not waive the privileges and immunities
# granted to it by virtue of its status as an Intergovernmental Organization or
# submit itself to any jurisdiction.

o2_add_library(
  MCHConditions
  SOURCES src/DCSNamer.cxx
  PUBLIC_LINK_LIBRARIES O2::MCHMappingImpl4)

if(BUILD_TESTING)
  o2_add_test(
    dcs-namer
    SOURCES src/testDCSNamer.cxx src/HVAliases.cxx src/LVAliases.cxx
    COMPONENT_NAME mch
    LABELS "muon;mch;dcs"
    PUBLIC_LINK_LIBRARIES O2::MCHConditions)
endif()

if(benchmark_FOUND)
  # o2_add_executable(bitset COMPONENT_NAME mchraw SOURCES benchBitSet.cxx
  # PUBLIC_LINK_LIBRARIES O2::MCHRawEncoderPayload benchmark::benchmark
  # IS_BENCHMARK)
endif()

o2_add_executable(
  generate-hv-alias
  SOURCES src/generateHVAliases.cxx
  COMPONENT_NAME mch
  PUBLIC_LINK_LIBRARIES O2::MCHConditions O2::DetectorsDCS)
