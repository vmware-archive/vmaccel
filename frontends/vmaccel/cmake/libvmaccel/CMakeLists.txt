set(SOURCES
   ../../src/vmaccel.c
   ../../src/vmaccel.cpp)

set(LIBRARIES
   vmaccel_utils
   vmcl_rpc_client
   vmaccelmgr_rpc_client
   vmaccel_rpc_client)

add_library(vmaccel ${SOURCES})
target_link_libraries(vmaccel ${LIBRARIES})
