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

#include <stdlib.h>
#include <string.h>

#include "vmxc.h"
#include "vmxc_spirv.h"
#include "log_level.h"

VMXCReturnStatus *vmxc_xcode_1_svc(VMXCodeOp *argp, struct svc_req *rqstp) {
   static VMXCReturnStatus result;
   static VMXCStatus ret;

   memset(&ret, 0, sizeof(VMXCStatus));

#ifdef _DEBUG
   if (argp->targetLanguage == -1) {
      if (vmxc_spirv_xcode((uint32_t *)argp->kernel.source.source_val,
                           argp->kernel.source.source_len / sizeof(uint32_t),
                           (uint32_t)-1, NULL, NULL) != SPV_SUCCESS) {
         Warning("Transcode failed...\n");
         ret.status = VMXC_FAIL;
      } else {
         ret.status = VMXC_SUCCESS;
      }
      result.VMXCReturnStatus_u.ret = &ret;
   } else
#endif

      /*
       * Handle identity cloning scenario.
       */
      if (argp->targetLanguage == argp->kernel.language) {
      ret.kernel.source.source_val = malloc(argp->kernel.source.source_len);
      ret.kernel.source.source_len = argp->kernel.source.source_len;

      if (vmxc_spirv_clone((uint32_t *)argp->kernel.source.source_val,
                           argp->kernel.source.source_len / sizeof(uint32_t),
                           (uint32_t *)ret.kernel.source.source_val) !=
          SPV_SUCCESS) {
         Warning("Cloning failed...\n");
         ret.status = VMXC_FAIL;
      } else {
         ret.status = VMXC_SUCCESS;
      }
      result.VMXCReturnStatus_u.ret = &ret;
   }

   return &result;
}

VMXCReturnStatus *vmxc_validate_1_svc(VMXCKernelDesc *argp,
                                      struct svc_req *rqstp) {
   static VMXCReturnStatus result;
   static VMXCStatus ret;

   memset(&ret, 0, sizeof(VMXCStatus));

   ret.kernel = *argp;

   if (vmxc_spirv_validate((uint32_t *)argp->source.source_val,
                           argp->source.source_len / sizeof(uint32_t)) !=
       SPV_SUCCESS) {
      Warning("Validation failed...\n");
      ret.status = VMXC_FAIL;
   } else {
      ret.status = VMXC_SUCCESS;
   }

   result.VMXCReturnStatus_u.ret = &ret;

   return &result;
}
