# Copyright CERN and copyright holders of ALICE O2. This software is distributed
# under the terms of the GNU General Public License v3 (GPL Version 3), copied
# verbatim in the file "COPYING".
#
# See http://alice-o2.web.cern.ch/license for full licensing information.
#
# In applying this license CERN does not waive the privileges and immunities
# granted to it by virtue of its status as an Intergovernmental Organization or
# submit itself to any jurisdiction.

o2_add_library(MCHRawBareEncoder
               SOURCES BitSet.cxx BareElinkEncoder.cxx BareGBTEncoder.cxx
               PUBLIC_LINK_LIBRARIES O2::MCHRawCommon
               PRIVATE_LINK_LIBRARIES O2::MCHRawImplHelpers
               PUBLIC_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR})

if(BUILD_TESTING)
o2_add_test(bitset
            SOURCES testBitSet.cxx
            COMPONENT_NAME mch
            LABELS "mch;raw"
            PUBLIC_LINK_LIBRARIES O2::MCHRawBareEncoder Boost::boost fmt::fmt)

o2_add_test(bare-elink-encoder
            SOURCES testBareElinkEncoder.cxx
            COMPONENT_NAME mch
            LABELS "mch;raw"
            PUBLIC_LINK_LIBRARIES O2::MCHRawBareEncoder Boost::boost)

o2_add_test(bare-gbt-encoder
            SOURCES testBareGBTEncoder.cxx
            COMPONENT_NAME mch
            LABELS "mch;raw"
            PUBLIC_LINK_LIBRARIES O2::MCHRawBareEncoder O2::MCHTestRawRefBuffers)

endif()

if(benchmark_FOUND)
  o2_add_executable(bitset
                    COMPONENT_NAME mch
                    SOURCES benchBitSet.cxx
                    PUBLIC_LINK_LIBRARIES O2::MCHRawBareEncoder benchmark::benchmark
                    IS_BENCHMARK)
endif()
