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

#ifndef _VMCL_H_
#define _VMCL_H_ 1

#include "vmaccel_defs.h"

#define VMCL_MAX_CONTEXTS 32
#define VMCL_MAX_SURFACES 32
#define VMCL_MAX_QUEUES 32
#define VMCL_MAX_FENCES 32
#define VMCL_MAX_EVENTS 32
#define VMCL_MAX_SAMPLERS 32
#define VMCL_MAX_KERNELS 32

enum VMCLCapsShift {
   VMCL_SPIRV_32_BIT_SHIFT = 0,
   VMCL_SPIRV_64_BIT_SHIFT = 1,
   VMCL_SPIRV_1_0_CAP_SHIFT = 2,
   VMCL_SPIRV_1_1_CAP_SHIFT = 3,
   VMCL_SPIRV_1_2_CAP_SHIFT = 4,
   VMCL_UPLOAD_FROM_VADDR_CAP_SHIFT = 5,
   VMCL_DOWNLOAD_TO_VADDR_CAP_SHIFT = 6,
   VMCL_CAP_MAX = 7,
};
typedef enum VMCLCapsShift VMCLCapsShift;

#define VMCL_SPIRV_32_BIT (1 << VMCL_SPIRV_32_BIT_SHIFT)
#define VMCL_SPIRV_64_BIT (1 << VMCL_SPIRV_64_BIT_SHIFT)
#define VMCL_SPIRV_1_0_CAP (1 << VMCL_SPIRV_1_0_CAP_SHIFT)
#define VMCL_SPIRV_1_1_CAP (1 << VMCL_SPIRV_1_1_CAP_SHIFT)
#define VMCL_SPIRV_1_0_CAP (1 << VMCL_SPIRV_1_0_CAP_SHIFT)
#define VMCL_SPIRV_1_1_CAP (1 << VMCL_SPIRV_1_1_CAP_SHIFT)
#define VMCL_SPIRV_1_2_CAP (1 << VMCL_SPIRV_1_2_CAP_SHIFT)
#define VMCL_UPLOAD_FROM_VADDR_CAP (1 << VMCL_UPLOAD_FROM_VADDR_CAP_SHIFT)
#define VMCL_DOWNLOAD_TO_VADDR_CAP (1 << VMCL_DOWNLOAD_TO_VADDR_CAP_SHIFT)
#define VMCL_CAP_MASK ((1 << VMCL_CAP_MAX) - 1)
typedef unsigned int VMCLCaps;

enum VMCLKernelLanguageType {
   VMCL_UNKNOWN = 0,
   VMCL_GLSL = 1,
   VMCL_HLSL = 2,
   VMCL_DXIL = 3,
   VMCL_OPENCL_C_1_0 = 4,
   VMCL_LLVM = 5,
   VMCL_SPIR_1_2 = 6,
   VMCL_OPENCL_C_1_1 = 7,
   VMCL_OPENCL_C_1_2 = 8,
   VMCL_OPENCL_C_2_0 = 9,
   VMCL_SPIRV_1_0 = 10,
   VMCL_OPENCL_CPP_1_0 = 11,
   VMCL_SPIRV_1_1 = 12,
   VMCL_SPIRV_1_2 = 13,
   VMCL_LANGUAGE_MAX = 14,
};

enum VMCLKernelArchitecture {
   VMCL_IR_NATIVE = 0,
   VMCL_32BIT = 1,
   VMCL_64BIT = 2,
};

enum VMCLModifierType {
   VMCL_MODIFIER_MAX = 0,
};

#define VMCL_MODIFIER_MASK ((1 << VMCL_MODIFIER_MAX) - 1)
typedef unsigned int VMCLModifierFlags;

#endif /* _VMCL_H_ */
