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

#ifndef _VMCL_OPS_H_
#define _VMCL_OPS_H_

/*
 * Dervied from vmcl_rpc_server.h
 */
#include "vmaccel_ops.h"
#include "vmcl_rpc.h"

typedef struct VMCLOps {
   /*
    * Management Plane
    */
   VMAccelAllocateStatus *(*poweron)(struct VMCLOps *, unsigned int accelArch,
                                     unsigned int accelIndex);
   VMAccelStatus *(*poweroff)(void);
   VMAccelStatus *(*checkpoint)(void);
   VMAccelStatus *(*restore)(void);

   /*
    * Tracked State
    */
   VMCLContextAllocateStatus *(*contextalloc_1)(VMCLContextAllocateDesc *);
   VMAccelStatus *(*contextdestroy_1)(VMCLContextId *);
   VMAccelSurfaceAllocateStatus *(*surfacealloc_1)(VMCLSurfaceAllocateDesc *);
   VMAccelStatus *(*surfacedestroy_1)(VMCLSurfaceId *);
   VMAccelQueueStatus *(*queuealloc_1)(VMCLQueueAllocateDesc *);
   VMAccelStatus *(*queuedestroy_1)(VMCLQueueId *);
   VMCLSamplerAllocateStatus *(*sampleralloc_1)(VMCLSamplerAllocateDesc *);
   VMAccelStatus *(*samplerdestroy_1)(VMCLSamplerId *);
   VMCLKernelAllocateStatus *(*kernelalloc_1)(VMCLKernelAllocateDesc *);
   VMAccelStatus *(*kerneldestroy_1)(VMCLKernelId *);

   /*
    * Control Flow
    */
   VMAccelStatus *(*queueflush_1)(VMCLQueueId *);

   /*
    * Operations
    */
   VMAccelStatus *(*imageupload_1)(VMCLImageUploadOp *);
   VMAccelDownloadStatus *(*imagedownload_1)(VMCLImageDownloadOp *);
   VMAccelSurfaceMapStatus *(*surfacemap_1)(VMCLSurfaceMapOp *);
   VMAccelStatus *(*surfaceunmap_1)(VMCLSurfaceUnmapOp *);
   VMAccelStatus *(*surfacecopy_1)(VMCLSurfaceCopyOp *);
   VMAccelStatus *(*imagefill_1)(VMCLImageFillOp *);
   VMAccelStatus *(*dispatch_1)(VMCLDispatchOp *);
} VMCLOps;

VMAccelAllocateStatus *vmcl_poweron_svc(VMCLOps *ops);
VMAccelStatus *vmcl_poweroff_svc();

#endif /* !defined _VMCL_OPS_H_ */
