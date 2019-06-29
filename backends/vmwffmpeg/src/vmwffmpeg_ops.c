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

#include "vmwffmpeg.h"
#include "vmaccel_utils.h"

#include "log_level.h"

VMAccelStatus *vmwffmpeg_poweroff();

VMAccelAllocateStatus *vmwffmpeg_poweron(VMCODECOps *ops,
                                         unsigned int accelArch,
                                         unsigned int accelIndex) {
   static VMAccelAllocateStatus result;

   memset(&result, 0, sizeof(result));

   VMACCEL_LOG("Powering on vmwffmpeg backend...\n");

   result.status = VMACCEL_FAIL;

   return (&result);
}

VMAccelStatus *vmwffmpeg_poweroff() {
   static VMAccelStatus result;

   memset(&result, 0, sizeof(result));

   return (&result);
}

VMCODECContextAllocateStatus *
vmwffmpeg_contextalloc_1(VMCODECContextAllocateDesc *argp) {
   static VMCODECContextAllocateStatus result;

   memset(&result, 0, sizeof(result));

   result.status = VMACCEL_RESOURCE_UNAVAILABLE;

   return (&result);
}

VMAccelStatus *vmwffmpeg_contextdestroy_1(VMCODECContextId *argp) {
   static VMAccelStatus result;

   memset(&result, 0, sizeof(result));

   result.status = VMACCEL_FAIL;

   return (&result);
}

VMAccelSurfaceAllocateStatus *
vmwffmpeg_surfacealloc_1(VMCODECSurfaceAllocateDesc *argp) {
   static VMAccelSurfaceAllocateStatus result;

   memset(&result, 0, sizeof(result));

   result.status = VMACCEL_RESOURCE_UNAVAILABLE;

   return (&result);
}

VMAccelStatus *vmwffmpeg_surfacedestroy_1(VMCODECSurfaceId *argp) {
   static VMAccelStatus result;

   memset(&result, 0, sizeof(result));

   result.status = VMACCEL_FAIL;

   return (&result);
}

VMAccelStatus *vmwffmpeg_imageupload_1(VMCODECImageUploadOp *argp) {
   static VMAccelStatus result;

   memset(&result, 0, sizeof(result));

   result.status = VMACCEL_FAIL;

   return (&result);
}

VMAccelDownloadStatus *vmwffmpeg_imagedownload_1(VMCODECImageDownloadOp *argp) {
   static VMAccelDownloadStatus result;

   memset(&result, 0, sizeof(result));

   result.status = VMACCEL_FAIL;

   return (&result);
}

VMAccelStatus *vmwffmpeg_decode_1(VMCODECDecodeOp *argp) {
   static VMAccelStatus result;

   memset(&result, 0, sizeof(result));

   result.status = VMACCEL_FAIL;

   return (&result);
}

VMAccelStatus *vmwffmpeg_encode_1(VMCODECEncodeOp *argp) {
   static VMAccelStatus result;

   memset(&result, 0, sizeof(result));

   result.status = VMACCEL_FAIL;

   return (&result);
}

/*
 * Setup the backend op dispatch
 */
VMCODECOps vmwffmpegOps = {
   vmwffmpeg_poweron, vmwffmpeg_poweroff, NULL, NULL, vmwffmpeg_contextalloc_1,
   vmwffmpeg_contextdestroy_1, vmwffmpeg_surfacealloc_1,
   vmwffmpeg_surfacedestroy_1, vmwffmpeg_imageupload_1,
   vmwffmpeg_imagedownload_1, vmwffmpeg_decode_1, vmwffmpeg_encode_1,
};
