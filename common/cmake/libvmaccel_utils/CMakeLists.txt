set(SOURCES
   ../../src/vmaccel_utils.c
   ../../src/vmaccel_utils.cpp)

add_library(vmaccel_utils ${SOURCES})
set_target_properties(
   PROPERTIES
   COMPILE_OPTIONS "")
