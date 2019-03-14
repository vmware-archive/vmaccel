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

#ifndef _VMWOPENCL_UTILS_H_
#define _VMWOPENCL_UTILS_H_ 1

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include "log_level.h"


void Log_cl_bool(char *prefix, cl_bool val);
void Log_cl_uint(char *prefix, cl_uint val);
void Log_cl_ulong(char *prefix, cl_ulong val);
void Log_size_t(char *prefix, size_t val);
void Log_cl_device_type(char *prefix, cl_device_type val);
void Log_cl_device_local_mem_type(char *prefix, cl_device_local_mem_type val);
void Log_cl_device_exec_capabilities(char *prefix,
                                     cl_device_exec_capabilities val);
void Log_cl_device_fp_config(char *prefix, cl_device_fp_config val);
void Log_cl_device_id(char *prefix, cl_device_id val);
#if CL_OPENCL_2_0
void Log_cl_device_svm_capabilities(char *prefix,
                                    cl_device_svm_capabilities val);
#endif
void Log_cl_command_queue_properties(char *prefix,
                                     cl_command_queue_properties val);

#endif /* _VMWOPENCL_UTILS_H_ */
