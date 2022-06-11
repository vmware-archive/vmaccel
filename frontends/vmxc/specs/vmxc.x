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

#ifdef RPC_HDR
%#include "vmaccel_rpc.h"
%#include "vmcl_rpc.h"
#endif

enum VMXCStatusCodeEnum {
   VMXC_SUCCESS,
   VMXC_FAIL,
   VMXC_SEMANTIC_ERROR,
   VMXC_LINK_ERROR,
   VMXC_STATUS_CODE_MAX
};

/*
 * Kernel argument descriptor for input and output mappings/validation.
 */
struct VMXCKernelArgDesc {
   unsigned int            index;
   VMCLKernelSemanticType  semantic;
   opaque                  semanticDecl<>;

   VMCLKernelArgType       type;
   VMAccelSurfaceDesc      surfDesc;
   VMCLSamplerDesc         samplerDesc;
};

/*
 * Accelerator kernel allocation descriptor structure.
 */
struct VMXCKernelDesc {
   VMCLKernelLanguage      language;
   opaque                  source<>;

   /*
    * Name of the kernel in the source/binary.
    */
   opaque                  kernelName<>;

   /*
    * Input/Output layout for validation and bindings.
    */
   VMXCKernelArgDesc       inputLayout<>;
   VMXCKernelArgDesc       outputLayout<>;
};

/*
 * Transcode operation.
 */
struct VMXCodeOp {
   VMCLKernelLanguage      targetLanguage;
   VMXCKernelDesc          kernel;
};

/*
 * Transcode return status, results include original argument.
 */
struct VMXCStatus {
   VMXCStatusCodeEnum      status;
   VMXCKernelDesc          kernel;
};

union VMXCReturnStatus switch (int errno) {
   case 0:
      VMXCStatus *ret;
   default:
      void;
};

/*
 * VMXC program definition.
 */
program VMXC {
   version VMXC_VERSION {
      /*
       * Transcode operation.
       */
      VMXCReturnStatus
         VMXC_XCODE(VMXCodeOp) = 1;

      /*
       * Validation operation.
       */
      VMXCReturnStatus
         VMXC_VALIDATE(VMXCKernelDesc) = 2;
  } = 1;
} = 0x20000082;
