set(SOURCES
   ../../src/vmaccel_stream.c
   ../../src/vmaccel_utils.c
   ../../src/vmaccel_utils.cpp)

add_definitions(-DVMACCEL_LOCAL=1)
add_library(vmaccel_utils_local ${SOURCES})
target_link_libraries(vmaccel_utils_local pthread)
