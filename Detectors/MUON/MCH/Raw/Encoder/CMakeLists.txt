# Copyright CERN and copyright holders of ALICE O2. This software is distributed
# under the terms of the GNU General Public License v3 (GPL Version 3), copied
# verbatim in the file "COPYING".
#
# See http://alice-o2.web.cern.ch/license for full licensing information.
#
# In applying this license CERN does not waive the privileges and immunities
# granted to it by virtue of its status as an Intergovernmental Organization or
# submit itself to any jurisdiction.

add_subdirectory(Bare)
add_subdirectory(UserLogic)

o2_add_library(MCHRawEncoder
               SOURCES src/Encoder.cxx src/Paginator.cxx
               PUBLIC_LINK_LIBRARIES O2::MCHRawCommon
               PRIVATE_LINK_LIBRARIES O2::MCHRawBareEncoder
                                      O2::MCHRawUserLogicEncoder
                                      O2::MCHRawImplHelpers)

if(BUILD_TESTING)
  o2_add_test(cru-encoder
              SOURCES src/testCRUEncoder.cxx
              COMPONENT_NAME mch
              LABELS "mch;raw"
              PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder)

  o2_add_test(encoder
              SOURCES src/testEncoder.cxx
              COMPONENT_NAME mch
              LABELS "mch;raw"
              PUBLIC_LINK_LIBRARIES O2::MCHRawEncoder)
endif()
