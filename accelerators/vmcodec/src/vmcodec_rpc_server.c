/******************************************************************************

Copyright (c) 2019 VMware, Inc.
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

/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "vmcodec_ops.h"
#include "vmcodec_rpc.h"
#include "vmwffmpeg.h"
#include <assert.h>
#include <string.h>

#include "log_level.h"

#if VMW_FFMPEG
static VMCODECOps *ffmpeg = &vmwffmpegOps;
#else
static VMCODECOps *ffmpeg = NULL;
#endif

VMAccelAllocateStatus *vmcodec_poweron_svc(VMCODECOps *ops) {
   VMAccelAllocateStatus *ret = NULL;
   /*
    * Loop through all the Accelerator architectures until one powers on.
    */
   assert(VMACCEL_SELECT_MAX > 0);
   for (int i = 0; i < VMACCEL_SELECT_MAX; i++) {
      ret = ffmpeg->poweron(NULL, i, 0);
      if (ret->status == VMACCEL_SUCCESS) {
         return ret;
      }
   }
   VMACCEL_WARNING("%s: Unable to power on any VMCODEC capable backends.\n",
                   __FUNCTION__);
   return ret;
}

VMAccelStatus *vmcodec_poweroff_svc() {
   return ffmpeg->poweroff();
}

VMCODECContextAllocateReturnStatus *
vmcodec_contextalloc_1_svc(VMCODECContextAllocateDesc *argp,
                           struct svc_req *rqstp) {
   static VMCODECContextAllocateReturnStatus result;

   /*
    * insert server code here
    */
   result.VMCODECContextAllocateReturnStatus_u.ret =
      ffmpeg->contextalloc_1(argp);

   return &result;
}

VMAccelReturnStatus *vmcodec_contextdestroy_1_svc(VMCODECContextId *argp,
                                                  struct svc_req *rqstp) {
   static VMAccelReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelReturnStatus_u.ret = ffmpeg->contextdestroy_1(argp);

   return &result;
}

VMAccelSurfaceAllocateReturnStatus *
vmcodec_surfacealloc_1_svc(VMCODECSurfaceAllocateDesc *argp,
                           struct svc_req *rqstp) {
   static VMAccelSurfaceAllocateReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelSurfaceAllocateReturnStatus_u.ret =
      ffmpeg->surfacealloc_1(argp);

   return &result;
}

VMAccelReturnStatus *vmcodec_surfacedestroy_1_svc(VMCODECSurfaceId *argp,
                                                  struct svc_req *rqstp) {
   static VMAccelReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelReturnStatus_u.ret = ffmpeg->surfacedestroy_1(argp);

   return &result;
}

VMAccelReturnStatus *vmcodec_imageupload_1_svc(VMCODECImageUploadOp *argp,
                                               struct svc_req *rqstp) {
   static VMAccelReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelReturnStatus_u.ret = ffmpeg->imageupload_1(argp);

   return &result;
}

VMAccelDownloadReturnStatus *
vmcodec_imagedownload_1_svc(VMCODECImageDownloadOp *argp,
                            struct svc_req *rqstp) {
   static VMAccelDownloadReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelDownloadReturnStatus_u.ret = ffmpeg->imagedownload_1(argp);

   return &result;
}

VMAccelReturnStatus *vmcodec_decode_1_svc(VMCODECDecodeOp *argp,
                                          struct svc_req *rqstp) {
   static VMAccelReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelReturnStatus_u.ret = ffmpeg->decode_1(argp);

   return &result;
}

VMAccelReturnStatus *vmcodec_encode_1_svc(VMCODECEncodeOp *argp,
                                          struct svc_req *rqstp) {
   static VMAccelReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelReturnStatus_u.ret = ffmpeg->encode_1(argp);

   return &result;
}
