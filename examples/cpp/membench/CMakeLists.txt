#
# Copyright (c) 2020-2021 VMware, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#     1. Redistributions of source code must retain the above copyright notice,
#        this list of conditions and the following disclaimer.
#
#     2. Redistributions in binary form must reproduce the above copyright
#        notice, this list of conditions and the following disclaimer in the
#        documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

set(CMAKE_CXX_STANDARD 11)

set(SOURCES
    membench.cpp)

set(LOCAL_SOURCES
    ../../../frontends/vmaccel/src/vmaccel.cpp)

set(GLOBAL_INC ../../../common/inc)

include_directories(${GLOBAL_INC})

add_executable(vmcl_rpc_membench ${SOURCES})
target_link_libraries(vmcl_rpc_membench vmaccel)
target_compile_definitions(vmcl_rpc_membench PUBLIC ENABLE_VMACCEL_LOCAL=0 ENABLE_VMACCEL_RPC=1)
set_target_properties(vmcl_rpc_membench
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_EXAMPLES_OUTPUT_DIRECTORY}")

add_executable(vmcl_membench ${SOURCES} ${LOCAL_SOURCES})
target_link_libraries(vmcl_membench vmaccel_local)
target_compile_definitions(vmcl_membench PUBLIC ENABLE_VMACCEL_LOCAL=1 ENABLE_VMACCEL_RPC=0)
set_target_properties(vmcl_membench
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_EXAMPLES_OUTPUT_DIRECTORY}")


