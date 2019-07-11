# Copyright CERN and copyright holders of ALICE O2. This software is distributed
# under the terms of the GNU General Public License v3 (GPL Version 3), copied
# verbatim in the file "COPYING".
#
# See http://alice-o2.web.cern.ch/license for full licensing information.
#
# In applying this license CERN does not waive the privileges and immunities
# granted to it by virtue of its status as an Intergovernmental Organization or
# submit itself to any jurisdiction.

include_guard()

function(o2_cuda_clone_target src)
  string(REPLACE "::" "_" cloneName ${src})
  set(cloneName CUDA_${cloneName})
  add_library(${cloneName})
  get_property(sourceFiles TARGET ${src} PROPERTY SOURCES)
  get_property(loc TARGET ${src} PROPERTY SOURCE_DIR)
  foreach(s ${sourceFiles})
    if(NOT IS_ABSOLUTE ${s})
      list(APPEND sources ${loc}/${s})
    else()
      list(APPEND sources ${s})
    endif()
  endforeach()
  get_property(incdir TARGET ${src} PROPERTY INCLUDE_DIRECTORIES)
  message(STATUS "${src} -> ${incdir}")

  set_target_properties(
    ${cloneName}
    PROPERTIES SOURCES
               ${sources}
               INCLUDE_DIRECTORIES
               "$<TARGET_PROPERTY:${src},INTERFACE_INCLUDE_DIRECTORIES>"
               LINK_LIBRARIES
               "$<TARGET_PROPERTY:${src},INTERFACE_LINK_LIBRARIES>;-ltbb"
               COMPILE_DEFINITIONS
               "$<TARGET_PROPERTY:${src},COMPILE_DEFINITIONS>")


  get_property(incdir
               TARGET ${cloneName}
               PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
  message(STATUS "${cloneName} -> ${incdir}")

  install(TARGETS ${cloneName}
          EXPORT O2Targets
          LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

  add_library(CUDA::${src} ALIAS ${cloneName})
endfunction()
