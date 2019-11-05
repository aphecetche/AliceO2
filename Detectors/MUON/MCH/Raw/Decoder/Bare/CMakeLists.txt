# Copyright CERN and copyright holders of ALICE O2. This software is distributed
# under the terms of the GNU General Public License v3 (GPL Version 3), copied
# verbatim in the file "COPYING".
#
# See http://alice-o2.web.cern.ch/license for full licensing information.
#
# In applying this license CERN does not waive the privileges and immunities
# granted to it by virtue of its status as an Intergovernmental Organization or
# submit itself to any jurisdiction.

o2_add_library(MCHRawBareDecoder
               SOURCES BareDecoder.cxx BareCRUDecoder.cxx BareElinkDecoder.cxx
                       BareGBTDecoder.cxx
               PUBLIC_LINK_LIBRARIES O2::MCHRawDecoder
               PRIVATE_LINK_LIBRARIES O2::MCHRawImplHelpers O2::MCHRawDecoder)

if(BUILD_TESTING)
  o2_add_test(bare-elink-decoder
              SOURCES testBareElinkDecoder.cxx
              COMPONENT_NAME mch
              LABELS "mch;raw"
              PUBLIC_LINK_LIBRARIES O2::MCHRawBareDecoder)

  o2_add_test(bare-gbt-decoder
              SOURCES testBareGBTDecoder.cxx
              COMPONENT_NAME mch
              LABELS "mch;raw"
              PUBLIC_LINK_LIBRARIES O2::MCHRawBareDecoder O2::MCHTestRawRefBuffers)

endif()
