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

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../../external/vtk/CMake")

message("CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}")

find_package(FFMPEG COMPONENTS AVCODEC AVDEVICE AVFOMAT AVUTIL REQUIRED)

set(SERVER_SOURCES
   ../../src/vmcodec_rpc_server.c
   ../../src/vmcodec_rpc_xdr.c)

add_library(vmcodec_server ${SERVER_SOURCES})

if (FFMPEG_FOUND)
   target_link_libraries(vmcodec_server vmwffmpeg vmaccelmgr_client)
else()
   target_link_libraries(vmcodec_server vmaccelmgr_client)
endif()
