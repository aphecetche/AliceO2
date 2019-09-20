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
        SOURCES src/Encoder.cxx src/BitSet.cxx src/ElinkEncoder.cxx
        src/Paginator.cxx src/Counter.cxx
        src/LinkRange.cxx src/Inserter.cxx
        src/DataBlock.cxx
        PUBLIC_LINK_LIBRARIES O2::MCHRawCommon O2::MCHMappingFactory
        O2::MCHRawElecMap
        O2::CommonDataFormat
        PRIVATE_LINK_LIBRARIES O2::MCHRawImplHelpers)

if(BUILD_TESTING)

        o2_add_test(elink-encoder
                SOURCES src/testElinkEncoder.cxx
                COMPONENT_NAME mchraw
                LABELS "muon;mch;raw"
                PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder O2::MCHRawImplHelpers)

        o2_add_test(gbt-encoder
                SOURCES src/testGBTEncoder.cxx
                COMPONENT_NAME mchraw
                LABELS "muon;mch;raw"
                PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder O2::MCHTestRawRefBuffers
                O2::MCHRawImplHelpers)

        o2_add_test(encoder
                SOURCES src/testEncoder.cxx
                COMPONENT_NAME mchraw
                LABELS "muon;mch;raw"
                PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder O2::MCHRawImplHelpers)

        o2_add_test(bitset
                SOURCES src/testBitSet.cxx
                COMPONENT_NAME mchraw
                LABELS "muon;mch;raw"
                PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder)

        o2_add_test(bare-elink-encoder
                SOURCES src/testBareElinkEncoder.cxx
                COMPONENT_NAME mchraw
                LABELS "muon;mch;raw"
                PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder O2::MCHRawImplHelpers)

        o2_add_test(paginator
                SOURCES src/testPaginator.cxx
                COMPONENT_NAME mchraw
                LABELS "muon;mch;raw"
                PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder O2::MCHRawImplHelpers)

        o2_add_test(counter
                SOURCES src/testCounter.cxx
                COMPONENT_NAME mchraw
                LABELS "muon;mch;raw"
                PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder
                O2::MCHRawImplHelpers)

        o2_add_test(inserter
                SOURCES src/testInserter.cxx
                COMPONENT_NAME mchraw
                LABELS "muon;mch;raw"
                PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder
                O2::MCHRawImplHelpers)
endif()

if(benchmark_FOUND)
        o2_add_executable(bitset
                COMPONENT_NAME mchraw
                SOURCES src/benchBitSet.cxx
                PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder benchmark::benchmark
                IS_BENCHMARK)
endif()

