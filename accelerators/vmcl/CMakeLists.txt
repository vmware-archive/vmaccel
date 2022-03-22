#
# Copyright (c) 2016-2020 VMware, Inc.
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
project(vmcl)

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

   set(CMAKE_BUILD_TYPE Debug)
   set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
ENDIF()

set(GLOBAL_INC ../../common/inc)

function(ProcessGen)
   set(GenFileList "")
   foreach(inFile IN LISTS ARGN)
      get_filename_component(realPath ${inFile} REALPATH BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
      get_filename_component(outFile ${realPath} NAME)
      add_custom_command(
         OUTPUT ${CMAKE_GEN_OUTPUT_DIRECTORY}/${outFile}
         COMMAND ${CMAKE_COMMAND} -E copy ${realPath} ${CMAKE_GEN_OUTPUT_DIRECTORY}
         COMMAND rpcgen -K 2 -L -a -C ${outFile}
         WORKING_DIRECTORY ${CMAKE_GEN_OUTPUT_DIRECTORY}
         DEPENDS ${realPath})
      list(APPEND GenFileList ${CMAKE_GEN_OUTPUT_DIRECTORY}/${outFile})
   endforeach()
   add_custom_target(
      VMCLGenOutputDir ALL
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_GEN_OUTPUT_DIRECTORY})
   add_custom_target(
      VMCLGen ALL
      DEPENDS ${GenFileList}
      COMMENT "Processing auto-generated files")
   add_dependencies(VMCLGen VMCLGenOutputDir)
endfunction()

function(ProcessIncludeExports)
   set(IncludeFileList "")
   foreach(inFile IN LISTS ARGN)
      get_filename_component(realPath ${inFile} REALPATH BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
      get_filename_component(outFile ${realPath} NAME)
      add_custom_command(
         OUTPUT ${CMAKE_INCLUDE_OUTPUT_DIRECTORY}/${outFile}
         COMMAND ${CMAKE_COMMAND} -E copy ${realPath} ${CMAKE_INCLUDE_OUTPUT_DIRECTORY}
         DEPENDS ${realPath})
      list(APPEND IncludeFileList ${CMAKE_INCLUDE_OUTPUT_DIRECTORY}/${outFile})
   endforeach()
   add_custom_target(
      VMCLIncludeOutputDir ALL
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INCLUDE_OUTPUT_DIRECTORY})
   add_custom_target(
      VMCLIncludeExports ALL
      DEPENDS ${IncludeFileList}
      COMMENT "Processing include exports")
   add_dependencies(VMCLIncludeExports VMCLIncludeOutputDir)
endfunction()

function(ProcessSpecExports)
   set(SpecsFileList "")
   foreach(inFile IN LISTS ARGN)
      get_filename_component(realPath ${inFile} REALPATH BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
      get_filename_component(outFile ${realPath} NAME)
      add_custom_command(
         OUTPUT ${CMAKE_SPECS_OUTPUT_DIRECTORY}/${outFile}
         COMMAND ${CMAKE_COMMAND} -E copy ${realPath} ${CMAKE_SPECS_OUTPUT_DIRECTORY}
         DEPENDS ${realPath})
      list(APPEND SpecsFileList ${CMAKE_SPECS_OUTPUT_DIRECTORY}/${outFile})
   endforeach()
   add_custom_target(
      VMCLSpecsOutputDir ALL
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SPECS_OUTPUT_DIRECTORY})
   add_custom_target(
      VMCLSpecsExports ALL
      DEPENDS ${SpecsFileList}
      COMMENT "Processing spec file exports")
   add_dependencies(VMCLSpecsExports VMCLSpecsOutputDir)
endfunction()

include_directories(${GLOBAL_INC})

#
# Import various external projects for use in the VMCL libraries
#
message(${CMAKE_CURRENT_SOURCE_DIR})

set(SPIRV_HEADERS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/external/spirv-headers)

ProcessGen(
   specs/vmcl_rpc.x
   specs/vmcl_callback_rpc.x)

add_subdirectory(cmake/libvmcl_rpc)
add_subdirectory(cmake/libvmcl_server)
add_subdirectory(cmake/libvmcl_hybrid_client)
add_subdirectory(cmake/libvmcl_local_client)
add_subdirectory(cmake/libvmcl_rpc_client)
add_subdirectory(cmake/vmcl_server)

ProcessIncludeExports(
   ${GLOBAL_INC}/vmcl_defs.h
   ${GLOBAL_INC}/vmcl_ops.h
   ${GLOBAL_INC}/vmcl_rpc.h
   ${GLOBAL_INC}/vmcl_callback_rpc.h)

ProcessSpecExports("")
