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

set(CMAKE_CXX_STANDARD 11)

function(add_unittest)
   set(one_value_args TARGET)
   set(multi_value_args SRCS LIBS ENVIRONMENT)
   cmake_parse_arguments(
      ARG "" "${one_value_args}" "${multi_value_args}" ${ARGN})
   set(target test_${ARG_TARGET})
   add_executable(${target} ${ARG_SRCS})
   target_link_libraries(${target} PRIVATE ${ARG_LIBS})
   set_target_properties(${target}
      PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_TEST_OUTPUT_DIRECTORY}")
endfunction()

set(GLOBAL_INC ../common/inc)

include_directories(${GLOBAL_INC})

set(TEST_SOURCES
  vmaccel_allocator_int_test.cpp
  vmaccel_allocator_desc_test.cpp
)

add_unittest(
  TARGET vmaccel_allocator_int_test
  SRCS vmaccel_allocator_int_test.cpp
  LIBS vmaccelmgr_server)

add_unittest(
  TARGET vmaccel_allocator_desc_test
  SRCS vmaccel_allocator_desc_test.cpp
  LIBS vmaccelmgr_server)
