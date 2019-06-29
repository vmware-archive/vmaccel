#
# Copyright (c) 2019 VMware, Inc.
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

set(SOURCES
    ../../../common/src/vmaccel_utils.c
    ../../../frontends/vmaccel/src/vmaccel.c
    ../../../accelerators/vmaccel/src/vmaccel_rpc_clnt.c
    ../../../accelerators/vmaccel/src/vmaccel_rpc_xdr.c
    ../../../accelerators/vmaccel/src/vmaccel_mgr_clnt.c
    ../../../accelerators/vmcl/src/vmcl_rpc_clnt.c
    ../../../accelerators/vmcl/src/vmcl_rpc_xdr.c
    basic_compute.cpp)

set(GLOBAL_INC ../../../common/inc)

include_directories(${GLOBAL_INC})

set(LIBRARIES)

add_executable(basic_compute_nolib ${SOURCES})
target_link_libraries(basic_compute_nolib ${LIBRARIES})
set_target_properties(basic_compute_nolib
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_EXAMPLES_OUTPUT_DIRECTORY}")
