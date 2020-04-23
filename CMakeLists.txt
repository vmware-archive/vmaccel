#
# Copyright (c) 2016-2019 VMware, Inc.
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

cmake_minimum_required(VERSION 3.4.3)
project(vmwaccel)

#
# Apply settings for the project
#
IF (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
   set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
   set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
   set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
   set(CMAKE_INCLUDE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/inc)
   set(CMAKE_SPECS_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/specs)
   set(CMAKE_GEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/gen)
   set(CMAKE_TEST_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)
   set(CMAKE_EXAMPLES_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples)

   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=unused-function -Werror=unused-variable")
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=misleading-indentation")

   set(CMAKE_BUILD_TYPE Debug)
   set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
ENDIF()

#
# Build external dependencies first
#
add_subdirectory(external/spirv-llvm)

set(CLANG_BINARY_DIR ${CMAKE_BINARY_DIR}/external/spirv-llvm/bin)
add_subdirectory(cmake/developer)

add_subdirectory(common)
add_subdirectory(backends)
add_subdirectory(accelerators)
add_subdirectory(frontends)
add_subdirectory(tests)
add_subdirectory(examples)
