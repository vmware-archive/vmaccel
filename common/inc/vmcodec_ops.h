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

#ifndef _VMCODEC_OPS_H_
#ifdef __cplusplus
extern "C" {
#endif
#define _VMCODEC_OPS_H_

/*
 * Dervied from vmcodec_rpc_server.h
 */
#include "vmaccel_ops.h"
#include "vmcodec_rpc.h"

typedef struct VMCODECOps {
   /*
    * Management Plane
    */
   VMAccelAllocateStatus *(*poweron)(struct VMCODECOps *,
                                     unsigned int accelArch,
                                     unsigned int accelIndex);
   VMAccelStatus *(*poweroff)(void);
   VMAccelStatus *(*checkpoint)(void);
   VMAccelStatus *(*restore)(void);

   /*
    * Tracked State
    */
   VMCODECContextAllocateStatus *(*contextalloc_1)(
      VMCODECContextAllocateDesc *);
   VMAccelStatus *(*contextdestroy_1)(VMCODECContextId *);
   VMAccelSurfaceAllocateStatus *(*surfacealloc_1)(
      VMCODECSurfaceAllocateDesc *);
   VMAccelStatus *(*surfacedestroy_1)(VMCODECSurfaceId *);

   /*
    * Operations
    */
   VMAccelStatus *(*imageupload_1)(VMCODECImageUploadOp *);
   VMAccelDownloadStatus *(*imagedownload_1)(VMCODECImageDownloadOp *);
   VMAccelStatus *(*decode_1)(VMCODECDecodeOp *);
   VMAccelStatus *(*encode_1)(VMCODECEncodeOp *);
} VMCODECOps;

VMAccelAllocateStatus *vmcodec_poweron_svc(VMCODECOps *ops);
VMAccelStatus *vmcodec_poweroff_svc();

#ifdef __cplusplus
}
#endif

#endif /* !defined _VMCODEC_OPS_H_ */
