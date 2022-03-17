set(SOURCES
   ../../src/vmaccel.c
   ../../src/vmaccel.cpp)

set(LIBRARIES
   vmaccel_utils_local
   vmcl_local_client
   vmaccelmgr_local_client
   vmaccel_local_client)

add_library(vmaccel_local ${SOURCES})
target_link_libraries(vmaccel_local ${LIBRARIES})
