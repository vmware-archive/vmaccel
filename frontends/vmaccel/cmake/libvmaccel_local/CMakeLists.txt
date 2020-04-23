set(SOURCES
   ../../src/vmaccel.c
   ../../src/vmaccel.cpp)

set(LIBRARIES
   vmaccel_utils_local
   vmcl_server
   vmaccelmgr_server
   vmaccel_server)

add_library(vmaccel_local ${SOURCES})
target_link_libraries(vmaccel_local ${LIBRARIES})
