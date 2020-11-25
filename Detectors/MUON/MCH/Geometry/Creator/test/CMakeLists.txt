# Copyright CERN and copyright holders of ALICE O2. This software is distributed
# under the terms of the GNU General Public License v3 (GPL Version 3), copied
# verbatim in the file "COPYING".
#
# See http://alice-o2.web.cern.ch/license for full licensing information.
#
# In applying this license CERN does not waive the privileges and immunities
# granted to it by virtue of its status as an Intergovernmental Organization or
# submit itself to any jurisdiction.

o2_add_test(
  geometry-creator
  COMPONENT_NAME mch
  SOURCES testGeometryCreator.cxx
  PUBLIC_LINK_LIBRARIES O2::MCHGeometryCreator
  LABELS muon mch)

o2_add_test(geometry-transformer-legacy
  NAME mch/geometry-transformer-legacy
  COMPONENT_NAME mch
  SOURCES testGeometryTransformer.cxx
  PUBLIC_LINK_LIBRARIES O2::MCHGeometryCreator
  COMMAND_LINE_ARGS ${CMAKE_CURRENT_LIST_DIR}/data/ideal-geometry-aliroot.json
  LABELS muon mch)

o2_add_test(geometry-transformer
                NAME mch/geometry-transformer
  COMPONENT_NAME mch
  SOURCES testGeometryTransformer.cxx
  PUBLIC_LINK_LIBRARIES O2::MCHGeometryCreator
  COMMAND_LINE_ARGS ${CMAKE_CURRENT_LIST_DIR}/data/ideal-geometry-o2.json
  LABELS muon mch)
