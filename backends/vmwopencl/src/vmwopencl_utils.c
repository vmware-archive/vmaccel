/******************************************************************************

Copyright (c) 2016-2019 VMware, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

******************************************************************************/

#include <assert.h>
#include <string.h>

#include "vmwopencl.h"
#include "vmwopencl_utils.h"


void Log_cl_bool(char *prefix, cl_bool val) {
   Log("%s: %s\n", prefix, val ? "TRUE" : "FALSE");
}

void Log_cl_uint(char *prefix, cl_uint val) {
   Log("%s: 0x%x (%u)\n", prefix, val, val);
}

void Log_cl_ulong(char *prefix, cl_ulong val) {
   Log("%s: 0x%lx (%lu)\n", prefix, val, val);
}

void Log_size_t(char *prefix, size_t val) {
   Log("%s: 0x%zx (%zu)\n", prefix, val, val);
}

void Log_cl_device_type(char *prefix, cl_device_type val) {
   const char *typeStr[] = {"CL_DEVICE_TYPE_DEFAULT", "CL_DEVICE_TYPE_CPU",
                            "CL_DEVICE_TYPE_GPU", "CL_DEVICE_TYPE_ACCELERATOR",
                            "CL_DEVICE_TYPE_ALL"};
   if (val < sizeof(typeStr) / sizeof(const char *)) {
      Log("%s: %s\n", prefix, typeStr[val]);
   } else {
      Log("%s: Unknown cl_device_type\n", prefix);
   }
}

void Log_cl_device_local_mem_type(char *prefix, cl_device_local_mem_type val) {
   const char *typeStr[] = {"CL_GLOBAL", "CL_LOCAL"};
   if (val < sizeof(typeStr) / sizeof(const char *)) {
      Log("%s: %s\n", prefix, typeStr[val]);
   } else {
      Log("%s: Unknown cl_device_local_mem_type\n", prefix);
   }
}

void Log_cl_device_exec_capabilities(char *prefix,
                                     cl_device_exec_capabilities val) {
   const char *typeStr[] = {"CL_EXEC_KERNEL", "CL_EXEC_NATIVE_KERNEL"};
   if (val < sizeof(typeStr) / sizeof(const char *)) {
      Log("%s: %s\n", prefix, typeStr[val]);
   } else {
      Log("%s: Unknown cl_device_exec_capabilities\n", prefix);
   }
}

void Log_cl_device_fp_config(char *prefix, cl_device_fp_config val) {
   const char *typeStr[] = {"CL_FP_DENORM", "CL_FP_FMA", "CL_FP_INF_NAN",
                            "CL_FP_ROUND_TO_INF", "CL_FP_ROUND_TO_NEAREST",
                            "CL_FP_ROUND_TO_ZERO"};
   if (val < sizeof(typeStr) / sizeof(const char *)) {
      Log("%s: %s\n", prefix, typeStr[val]);
   } else {
      Log("%s: Unknown cl_device_fp_config\n", prefix);
   }
}

void Log_cl_device_id(char *prefix, cl_device_id val) {
   Log("%s: %p\n", prefix, val);
}

#if CL_VERSION_2_0
void Log_cl_device_svm_capabilities(char *prefix,
                                    cl_device_svm_capabilities val) {
   const char *typeStr[] = {"CL_EXEC_KERNEL", "CL_EXEC_NATIVE_KERNEL"};
   if (val < sizeof(typeStr) / sizeof(const char *)) {
      Log("%s: %s\n", prefix, typeStr[val]);
   } else {
      Log("%s: Unknown cl_device_svm_capabilities\n", prefix);
   }
}
#endif

void Log_cl_command_queue_properties(char *prefix,
                                     cl_command_queue_properties val) {
   const char *typeStr[] = {"CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE",
                            "CL_QUEUE_PROFILING_ENABLE"};
   if (val < sizeof(typeStr) / sizeof(const char *)) {
      Log("%s: %s\n", prefix, typeStr[val]);
   } else {
      Log("%s: Unknown cl_command_queue_properties\n", prefix);
   }
}
