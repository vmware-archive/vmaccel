set(SOURCES
   ../../src/vmaccel_stream.c
   ../../src/vmaccel_utils.c
   ../../src/vmaccel_utils.cpp)

add_library(vmaccel_utils ${SOURCES})
target_link_libraries(vmaccel_utils pthread)
set_target_properties(
   PROPERTIES
   COMPILE_OPTIONS "")
