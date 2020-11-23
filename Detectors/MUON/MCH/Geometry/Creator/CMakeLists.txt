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
  MCHGeometryCreator
  SOURCES src/Geometry.cxx
          src/GeometryTest.cxx
          src/Materials.cxx
          src/Materials.h
          src/Station1Geometry.cxx
          src/Station1Geometry.h
          src/Station2Geometry.cxx
          src/Station2Geometry.h
          src/Station345Geometry.cxx
          src/Station345Geometry.h
  PUBLIC_LINK_LIBRARIES ROOT::Geom RapidJSON::RapidJSON O2::MCHGeometryTransformer
  O2::DetectorsBase)

o2_target_root_dictionary(
  MCHGeometryCreator HEADERS include/MCHGeometryCreator/Geometry.h
                             include/MCHGeometryCreator/GeometryTest.h)

if(BUILD_TESTING)
  add_subdirectory(test)
  o2_add_test_root_macro(
    macros/drawMCHGeometry.C
    PUBLIC_LINK_LIBRARIES O2::MCHGeometryCreator O2::MathUtils
    LABELS "muon;mch")
endif()
