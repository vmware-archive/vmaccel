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
project(vmxc)

#
# Sets up the SPIRV-Tools compiler environment
#
function(VMXCSPIRVDefaultCompileOptions TARGET)
  target_compile_options(${TARGET} PRIVATE ${SPIRV_WARNINGS})

  if (${COMPILER_IS_LIKE_GNU})
    target_compile_options(${TARGET} PRIVATE
      -std=c++11 -fno-exceptions -fno-rtti)
    target_compile_options(${TARGET} PRIVATE
      -Wall -Wextra -Wno-long-long -Wshadow -Wundef -Wconversion
      -Wno-sign-conversion)
    # For good call stacks in profiles, keep the frame pointers.
    if(NOT "${SPIRV_PERF}" STREQUAL "")
      target_compile_options(${TARGET} PRIVATE -fno-omit-frame-pointer)
    endif()
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      set(SPIRV_USE_SANITIZER "" CACHE STRING
        "Use the clang sanitizer [address|memory|thread|...]")
      if(NOT "${SPIRV_USE_SANITIZER}" STREQUAL "")
        target_compile_options(${TARGET} PRIVATE
          -fsanitize=${SPIRV_USE_SANITIZER})
      endif()
    else()
      target_compile_options(${TARGET} PRIVATE
         -Wno-missing-field-initializers)
    endif()
  endif()

  if (MSVC)
    # Specify /EHs for exception handling. This makes using SPIRV-Tools as
    # dependencies in other projects easier.
    target_compile_options(${TARGET} PRIVATE /EHs)
  endif()

  # For MinGW cross compile, statically link to the C++ runtime.
  # But it still depends on MSVCRT.dll.
  if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
      set_target_properties(${TARGET} PROPERTIES
        LINK_FLAGS -static -static-libgcc -static-libstdc++)
    endif()
  endif()
endfunction()

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
      VMXCGenOutputDir ALL
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_GEN_OUTPUT_DIRECTORY})
   add_custom_target(
      VMXCGen ALL
      DEPENDS ${GenFileList}
      COMMENT "Processing auto-generated files")
   add_dependencies(VMXCGen VMXCGenOutputDir)
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
      VMXCIncludeOutputDir ALL
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INCLUDE_OUTPUT_DIRECTORY})
   add_custom_target(
      VMXCIncludeExports ALL
      DEPENDS ${IncludeFileList}
      COMMENT "Processing include exports")
   add_dependencies(VMXCIncludeExports VMXCIncludeOutputDir)
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
      VMXCSpecsOutputDir ALL
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SPECS_OUTPUT_DIRECTORY})
   add_custom_target(
      VMXCSpecsExports ALL
      DEPENDS ${SpecsFileList}
      COMMENT "Processing spec file exports")
   add_dependencies(VMXCSpecsExports VMXCSpecsOutputDir)
endfunction()

#
# Import various external projects for use in the VMCL libraries
#
message(${CMAKE_CURRENT_SOURCE_DIR})
message(${CMAKE_CURRENT_BINARY_DIR})

IF(WIN32)
   set(DXC_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/external/dxc)
ENDIF()

set(SPIRV_HEADERS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/external/spirv-headers)
set(SPIRV_CROSS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/external/spirv-cross)
set(SPIRV_TOOLS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/external/spirv-tools)
set(SPIRV_TOOLS_BINARY_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/external/spirv-tools)

include_directories(
   ${GLOBAL_INC}
   ${SPIRV_HEADERS_DIRECTORY}/include
   ${SPIRV_TOOLS_DIRECTORY}
   ${SPIRV_TOOLS_DIRECTORY}/include
   ${SPIRV_TOOLS_DIRECTORY}/source
   ${SPIRV_TOOLS_BINARY_DIRECTORY})
 
ProcessGen(
   specs/vmxc.x)

#
# Setup SPIRV-Tools environment
#
if(("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang"))
  set(COMPILER_IS_LIKE_GNU TRUE)
endif()
if(${COMPILER_IS_LIKE_GNU})
  set(SPIRV_WARNINGS -Wall -Wextra -Wnon-virtual-dtor -Wno-missing-field-initializers)

  option(SPIRV_WARN_EVERYTHING "Enable -Weverything" ${SPIRV_WARN_EVERYTHING})
  if(${SPIRV_WARN_EVERYTHING})
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      set(SPIRV_WARNINGS ${SPIRV_WARNINGS}
        -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded)
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
      set(SPIRV_WARNINGS ${SPIRV_WARNINGS} -Wpedantic -pedantic-errors)
    else()
      message(STATUS "Unknown compiler ${CMAKE_CXX_COMPILER_ID}, "
                     "so SPIRV_WARN_EVERYTHING has no effect")
    endif()
  endif()

  if(${SPIRV_WERROR})
    set(SPIRV_WARNINGS ${SPIRV_WARNINGS} -Werror)
  endif()
elseif(MSVC)
  set(SPIRV_WARNINGS -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS /wd4800)

  if(${SPIRV_WERROR})
    set(SPIRV_WARNINGS ${SPIRV_WARNINGS} /WX)
  endif()
endif()

#
# Trigger a build of the external modules for inclusion of generated files
#
add_subdirectory(external/spirv-headers)
add_subdirectory(external/spirv-cross)
add_subdirectory(external/spirv-tools)

#
# Internal modules
#
add_subdirectory(cmake/libvmxc)
add_subdirectory(cmake/libvmxc_service)
add_subdirectory(cmake/libvmxc_client)
add_subdirectory(cmake/vmxc_service)
add_subdirectory(cmake/vmxc_client)

ProcessIncludeExports(
   ${GLOBAL_INC}/vmxc.h)

ProcessSpecExports("")
