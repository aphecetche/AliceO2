
find_path(FLATBUFFERS_INCLUDE_DIR
  NAMES flatbuffers/flatbuffers.h
  HINTS ${FLATBUFFERS_ROOT} $ENV{FLATBUFFERS_ROOT}
  PATH_SUFFIXES include
)

find_path(FLATBUFFERS_LIBRARY_DIR
  NAMES libflatbuffers.a
  HINTS ${FLATBUFFERS_ROOT} $ENV{FLATBUFFERS_ROOT}
  PATH_SUFFIXES lib lib64
)

find_library(FLATBUFFERS_STATIC_LIBRARY
  NAMES libflatbuffers.a
  HINTS ${FLATBUFFERS_ROOT} $ENV{FLATBUFFERS_ROOT}
  PATH_SUFFIXES lib lib64
)

find_path(FLATBUFFERS_BINARY_DIR
  NAMES flatc
  HINTS ${FLATBUFFERS_ROOT} $ENV{FLATBUFFERS_ROOT}
  PATH_SUFFIXES bin
)

find_program(FLATBUFFERS_BINARY_FLATC
  NAMES flatc
  HINTS ${FLATBUFFERS_ROOT} $ENV{FLATBUFFERS_ROOT}
  PATH_SUFFIXES bin
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FlatBuffers
  REQUIRED_VARS
  FLATBUFFERS_INCLUDE_DIR
  FLATBUFFERS_LIBRARY_DIR
  FLATBUFFERS_BINARY_DIR
)

# idempotently import targets
if(NOT TARGET flatbuffers::flatbuffers)
  if(FLATBUFFERS_FOUND)
    # import target
    add_library(flatbuffers::flatbuffers STATIC IMPORTED)
    set_target_properties(flatbuffers::flatbuffers PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${FLATBUFFERS_INCLUDE_DIR}
      IMPORTED_LOCATION ${FLATBUFFERS_STATIC_LIBRARY}
    )
  endif()
endif()

if(NOT TARGET flatbuffers::flatc)
  if(FLATBUFFERS_FOUND)
  # import target
  add_executable(flatbuffers::flatc IMPORTED)
  set_target_properties(flatbuffers::flatc PROPERTIES
    IMPORTED_LOCATION ${FLATBUFFERS_BINARY_FLATC}
  )
  endif()
endif()
