set(SOURCES
   ../../../../common/src/vmaccel_utils.c
   ../../src/vmaccel.c
   ../../src/vmaccel.cpp)

set(LIBRARIES
   vmcl_client
   vmaccelmgr_client
   vmaccel_client)

add_library(vmaccel ${SOURCES})
target_link_libraries(vmaccel ${LIBRARIES})
