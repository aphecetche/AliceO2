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
        SOURCES
        Page/Counter.cxx
        Page/DigitEncoder.cxx
        Page/FeeIdRange.cxx
        Page/FeeLinkSetter.cxx
        Page/Inserter.cxx
        Page/Normalizer.cxx
        Page/Paginator.cxx
        Payload/BitSet.cxx
        Payload/DataBlock.cxx
        Payload/ElinkEncoder.cxx
        Payload/Encoder.cxx
        PUBLIC_LINK_LIBRARIES
        O2::CommonDataFormat
        O2::DetectorsRaw
        O2::MCHBase
        O2::MCHMappingImpl4
        O2::MCHRawCommon
        O2::MCHRawElecMap
        PRIVATE_LINK_LIBRARIES O2::MCHRawImplHelpers)

if(BUILD_TESTING)

        o2_add_test(elink-encoder
                SOURCES Payload/testElinkEncoder.cxx
                COMPONENT_NAME mchraw
                LABELS "muon;mch;raw"
                PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder O2::MCHRawImplHelpers)

        o2_add_test(gbt-encoder
                SOURCES Payload/testGBTEncoder.cxx
                COMPONENT_NAME mchraw
                LABELS "muon;mch;raw"
                PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder O2::MCHTestRawRefBuffers
                O2::MCHRawImplHelpers)

        o2_add_test(encoder
                SOURCES Payload/testEncoder.cxx Payload/CruBufferCreator.cxx
                COMPONENT_NAME mchraw
                LABELS "muon;mch;raw"
                PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder O2::MCHRawImplHelpers)

        o2_add_test(normalizer
                SOURCES Page/testNormalizer.cxx Payload/CruBufferCreator.cxx
                COMPONENT_NAME mchraw
                LABELS "muon;mch;raw"
                PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder O2::MCHRawImplHelpers)

        o2_add_test(bitset
                SOURCES Payload/testBitSet.cxx
                COMPONENT_NAME mchraw
                LABELS "muon;mch;raw"
                PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder)

        o2_add_test(bare-elink-encoder
                SOURCES Payload/testBareElinkEncoder.cxx
                COMPONENT_NAME mchraw
                LABELS "muon;mch;raw"
                PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder O2::MCHRawImplHelpers)

        o2_add_test(paginator
                SOURCES Page/testPaginator.cxx
                COMPONENT_NAME mchraw
                LABELS "muon;mch;raw"
                PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder O2::MCHRawImplHelpers)

        o2_add_test(counter
                SOURCES Page/testCounter.cxx
                COMPONENT_NAME mchraw
                LABELS "muon;mch;raw"
                PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder
                O2::MCHRawImplHelpers)

        o2_add_test(inserter
                SOURCES Page/testInserter.cxx
                COMPONENT_NAME mchraw
                LABELS "muon;mch;raw"
                PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder
                O2::MCHRawImplHelpers)

        o2_add_test(create-test-buffers
                SOURCES Page/testCreateTestBuffers.cxx Payload/CruBufferCreator.cxx
                COMPONENT_NAME mchraw
                LABELS "muon;mch;raw"
                PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder
                O2::MCHRawImplHelpers)
endif()

if(benchmark_FOUND)
        o2_add_executable(bitset
                COMPONENT_NAME mchraw
                SOURCES Payload/benchBitSet.cxx
                PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder benchmark::benchmark
                IS_BENCHMARK)
endif()

