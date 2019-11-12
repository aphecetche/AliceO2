# Copyright CERN and copyright holders of ALICE O2. This software is distributed
# under the terms of the GNU General Public License v3 (GPL Version 3), copied
# verbatim in the file "COPYING".
#
# See http://alice-o2.web.cern.ch/license for full licensing information.
#
# In applying this license CERN does not waive the privileges and immunities
# granted to it by virtue of its status as an Intergovernmental Organization or
# submit itself to any jurisdiction.

o2_add_library(MCHRawEncoder
               SOURCES src/Encoder.cxx
                       src/Paginator.cxx
                       src/UserLogicElinkEncoderMerger.cxx
                       src/BitSet.cxx
                       src/BareElinkEncoderMerger.cxx
               PUBLIC_LINK_LIBRARIES O2::MCHRawCommon
               PRIVATE_LINK_LIBRARIES O2::MCHRawImplHelpers)
if(BUILD_TESTING)

  o2_add_test(elink-encoder
              SOURCES src/testElinkEncoder.cxx
              COMPONENT_NAME mch
              LABELS "mch;raw"
              PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder)

  o2_add_test(gbt-encoder
              SOURCES src/testGBTEncoder.cxx
              COMPONENT_NAME mch
              LABELS "mch;raw"
              PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder O2::MCHTestRawRefBuffers
                                    O2::MCHRawImplHelpers)
  o2_add_test(cru-encoder
              SOURCES src/testCRUEncoder.cxx
              COMPONENT_NAME mch
              LABELS "mch;raw"
              PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder)

  o2_add_test(encoder
              SOURCES src/testEncoder.cxx src/TestBuffers.cxx
              COMPONENT_NAME mch
              LABELS "mch;raw"
              PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder)

  o2_add_test(paginator
              SOURCES src/testPaginator.cxx src/TestBuffers.cxx
              COMPONENT_NAME mch
              LABELS "mch;raw"
              PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder)

  o2_add_test(bitset
              SOURCES src/testBitSet.cxx
              COMPONENT_NAME mch
              LABELS "mch;raw"
              PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder Boost::boost fmt::fmt)

  o2_add_test(bare-elink-encoder
              SOURCES src/testBareElinkEncoder.cxx
              COMPONENT_NAME mch
              LABELS "mch;raw"
              PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder Boost::boost)

endif()

if(benchmark_FOUND)
  o2_add_executable(bitset
                    COMPONENT_NAME mch
                    SOURCES src/benchBitSet.cxx
                    PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder benchmark::benchmark
                    IS_BENCHMARK)
endif()
