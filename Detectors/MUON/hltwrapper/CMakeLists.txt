# @author Laurent Aphecetche

set(MODULE_NAME "MUONhltwrapper")

O2_SETUP(NAME ${MODULE_NAME})

# Define the source and header files
set(SRCS
    src/MCHDigitInspector.cxx
    src/MCHDigitOccupancy.cxx
    )

# HEADERS is not needed if we don't generate a dictionary.
#set(HEADERS
#    include/${MODULE_NAME}/Foo.h
#    src/Bar.h
#    )

set(LIBRARY_NAME ${MODULE_NAME})
set(BUCKET_NAME muon_hltwrapper_bucket)

O2_GENERATE_LIBRARY()

# todo we repeat ourselves because the above macro dares deleting the variables we pass to it.
set(LIBRARY_NAME ${MODULE_NAME})
set(BUCKET_NAME muon_hltwrapper_bucket)

# Define application
O2_GENERATE_EXECUTABLE(
    EXE_NAME mch-digit-inspector
    SOURCES src/runDigitInspector.cxx
    MODULE_LIBRARY_NAME ${LIBRARY_NAME}
    BUCKET_NAME ${BUCKET_NAME}
)

O2_GENERATE_EXECUTABLE(
    EXE_NAME mch-digit-occupancy
    SOURCES src/runDigitOccupancy.cxx
    MODULE_LIBRARY_NAME ${LIBRARY_NAME}
    BUCKET_NAME ${BUCKET_NAME}
)
