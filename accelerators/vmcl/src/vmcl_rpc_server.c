/******************************************************************************

Copyright (c) 2016-2022 VMware, Inc.
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

#include "vmcl_ops.h"
#include "vmcl_rpc.h"
#include "vmwopencl.h"
#include <assert.h>
#include <string.h>

#include "log_level.h"

static VMCLOps *cl = &vmwopenclOps;
static volatile unsigned long clRefCount = 0;

VMAccelAllocateStatus *vmcl_poweron_svc(VMCLOps *ops, unsigned int useDataStreaming) {
   VMAccelAllocateStatus *ret = NULL;
   /*
    * Loop through all the Accelerator architectures until one powers on.
    */
   assert(VMACCEL_SELECT_MAX > 0);
   for (int i = 0; i < VMACCEL_SELECT_MAX; i++) {
      ret = cl->poweron(NULL, i, 0, useDataStreaming);
      if (ret->status == VMACCEL_SUCCESS) {
         clRefCount++;
         return ret;
      }
   }
   VMACCEL_WARNING("%s: Unable to power on any VMCL capable backends.\n",
                   __FUNCTION__);
   return ret;
}

VMAccelStatus *vmcl_poweroff_svc() {
   if (clRefCount == 1) {
      clRefCount = 0;
      return cl->poweroff();
   } else if (clRefCount > 1) {
      clRefCount--;
   }
   return NULL;
}

VMCLContextAllocateReturnStatus *
vmcl_contextalloc_1_svc(VMCLContextAllocateDesc *argp, struct svc_req *rqstp) {
   static VMCLContextAllocateReturnStatus result;

   /*
    * insert server code here
    */
   result.VMCLContextAllocateReturnStatus_u.ret = cl->contextalloc_1(argp);

   return (&result);
}

VMAccelReturnStatus *vmcl_contextdestroy_1_svc(VMCLContextId *argp,
                                               struct svc_req *rqstp) {

   static VMAccelReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelReturnStatus_u.ret = cl->contextdestroy_1(argp);

   return (&result);
}

VMAccelSurfaceAllocateReturnStatus *
vmcl_surfacealloc_1_svc(VMCLSurfaceAllocateDesc *argp, struct svc_req *rqstp) {

   static VMAccelSurfaceAllocateReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelSurfaceAllocateReturnStatus_u.ret = cl->surfacealloc_1(argp);

   return (&result);
}

VMAccelReturnStatus *vmcl_surfacedestroy_1_svc(VMCLSurfaceId *argp,
                                               struct svc_req *rqstp) {

   static VMAccelReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelReturnStatus_u.ret = cl->surfacedestroy_1(argp);

   return (&result);
}

VMAccelSharedHandleReturnStatus *
vmcl_surfacegetsharedhandle_1_svc(VMCLSurfaceId *argp, struct svc_req *rqstp) {

   static VMAccelSharedHandleReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelSharedHandleReturnStatus_u.ret =
      cl->surfacegetsharedhandle_1(argp);

   return (&result);
}

VMAccelReturnStatus *
vmcl_surfacereleasesharedhandle_1_svc(VMCLSharedHandle *argp,
                                      struct svc_req *rqstp) {

   static VMAccelReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelReturnStatus_u.ret = cl->surfacereleasesharedhandle_1(argp);

   return (&result);
}

VMAccelQueueReturnStatus *vmcl_queuealloc_1_svc(VMCLQueueAllocateDesc *argp,
                                                struct svc_req *rqstp) {

   static VMAccelQueueReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelQueueReturnStatus_u.ret = cl->queuealloc_1(argp);

   return (&result);
}

VMAccelReturnStatus *vmcl_queuedestroy_1_svc(VMCLQueueId *argp,
                                             struct svc_req *rqstp) {

   static VMAccelReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelReturnStatus_u.ret = cl->queuedestroy_1(argp);

   return (&result);
}

VMAccelEventReturnStatus *vmcl_eventalloc_1_svc(VMCLEventAllocateDesc *argp,
                                                struct svc_req *rqstp) {

   static VMAccelEventReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelEventReturnStatus_u.ret = cl->eventalloc_1(argp);

   return (&result);
}

VMAccelEventReturnStatus *vmcl_eventgetstatus_1_svc(VMCLEventId *argp,
                                                    struct svc_req *rqstp) {

   static VMAccelEventReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelEventReturnStatus_u.ret = cl->eventgetstatus_1(argp);

   return (&result);
}

VMAccelEventReturnStatus *vmcl_eventdestroy_1_svc(VMCLEventId *argp,
                                                  struct svc_req *rqstp) {

   static VMAccelEventReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelEventReturnStatus_u.ret = cl->eventdestroy_1(argp);

   return (&result);
}

VMAccelFenceReturnStatus *vmcl_fencealloc_1_svc(VMCLFenceAllocateDesc *argp,
                                                struct svc_req *rqstp) {

   static VMAccelFenceReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelFenceReturnStatus_u.ret = cl->fencealloc_1(argp);

   return (&result);
}

VMAccelFenceReturnStatus *vmcl_fencegetstatus_1_svc(VMCLFenceId *argp,
                                                    struct svc_req *rqstp) {

   static VMAccelFenceReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelFenceReturnStatus_u.ret = cl->fencegetstatus_1(argp);

   return (&result);
}

VMAccelFenceReturnStatus *vmcl_fencedestroy_1_svc(VMCLFenceId *argp,
                                                  struct svc_req *rqstp) {

   static VMAccelFenceReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelFenceReturnStatus_u.ret = cl->fencedestroy_1(argp);

   return (&result);
}

VMAccelReturnStatus *vmcl_queueflush_1_svc(VMCLQueueId *argp,
                                           struct svc_req *rqstp) {

   static VMAccelReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelReturnStatus_u.ret = cl->queueflush_1(argp);

   return (&result);
}

VMAccelReturnStatus *vmcl_eventinsert_1_svc(VMCLEventInsertOp *argp,
                                            struct svc_req *rqstp) {

   static VMAccelReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelReturnStatus_u.ret = cl->eventinsert_1(argp);

   return (&result);
}

VMAccelReturnStatus *vmcl_fenceinsert_1_svc(VMCLFenceInsertOp *argp,
                                            struct svc_req *rqstp) {

   static VMAccelReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelReturnStatus_u.ret = cl->fenceinsert_1(argp);

   return (&result);
}

VMAccelReturnStatus *vmcl_imageupload_1_svc(VMCLImageUploadOp *argp,
                                            struct svc_req *rqstp) {

   static VMAccelReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelReturnStatus_u.ret = cl->imageupload_1(argp);

   return (&result);
}

VMAccelDownloadReturnStatus *vmcl_imagedownload_1_svc(VMCLImageDownloadOp *argp,
                                                      struct svc_req *rqstp) {

   static VMAccelDownloadReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelDownloadReturnStatus_u.ret = cl->imagedownload_1(argp);

   return (&result);
}

VMAccelSurfaceMapReturnStatus *vmcl_surfacemap_1_svc(VMCLSurfaceMapOp *argp,
                                                     struct svc_req *rqstp) {

   static VMAccelSurfaceMapReturnStatus result;

   /*
    * Don't free the incoming pointers in local mode.
    */
   if (rqstp == NULL) {
      argp->op.mapFlags |= VMACCEL_MAP_NO_FREE_PTR_FLAG;
   } else if (argp->op.mapFlags & VMACCEL_MAP_NO_FREE_PTR_FLAG) {
      static VMAccelSurfaceMapStatus mapResult;
      mapResult.status = VMACCEL_FAIL;
      result.VMAccelSurfaceMapReturnStatus_u.ret = (&mapResult);
      return (&result);
   }
   result.VMAccelSurfaceMapReturnStatus_u.ret = cl->surfacemap_1(argp);

   return (&result);
}

VMAccelReturnStatus *vmcl_surfaceunmap_1_svc(VMCLSurfaceUnmapOp *argp,
                                             struct svc_req *rqstp) {

   static VMAccelReturnStatus result;

   /*
    * Don't free the incoming pointers in local mode.
    */
   if (rqstp == NULL) {
      argp->op.mapFlags |= VMACCEL_MAP_NO_FREE_PTR_FLAG;
   } else if (argp->op.mapFlags & VMACCEL_MAP_NO_FREE_PTR_FLAG) {
      static VMAccelStatus unmapResult;
      unmapResult.status = VMACCEL_FAIL;
      result.VMAccelReturnStatus_u.ret = (&unmapResult);
      return (&result);
   }
   result.VMAccelReturnStatus_u.ret = cl->surfaceunmap_1(argp);

   return (&result);
}

VMAccelReturnStatus *vmcl_surfacecopy_1_svc(VMCLSurfaceCopyOp *argp,
                                            struct svc_req *rqstp) {

   static VMAccelReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelReturnStatus_u.ret = cl->surfacecopy_1(argp);

   return (&result);
}

VMAccelReturnStatus *vmcl_imagefill_1_svc(VMCLImageFillOp *argp,
                                          struct svc_req *rqstp) {

   static VMAccelReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelReturnStatus_u.ret = cl->imagefill_1(argp);

   return (&result);
}

VMCLSamplerAllocateReturnStatus *
vmcl_sampleralloc_1_svc(VMCLSamplerAllocateDesc *argp, struct svc_req *rqstp) {

   static VMCLSamplerAllocateReturnStatus result;

   /*
    * insert server code here
    */
   result.VMCLSamplerAllocateReturnStatus_u.ret = cl->sampleralloc_1(argp);

   return (&result);
}

VMAccelReturnStatus *vmcl_samplerdestroy_1_svc(VMCLSamplerId *argp,
                                               struct svc_req *rqstp) {

   static VMAccelReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelReturnStatus_u.ret = cl->samplerdestroy_1(argp);

   return (&result);
}

VMCLKernelAllocateReturnStatus *
vmcl_kernelalloc_1_svc(VMCLKernelAllocateDesc *argp, struct svc_req *rqstp) {

   static VMCLKernelAllocateReturnStatus result;

   /*
    * insert server code here
    */
   result.VMCLKernelAllocateReturnStatus_u.ret = cl->kernelalloc_1(argp);

   return (&result);
}

VMAccelReturnStatus *vmcl_kerneldestroy_1_svc(VMCLKernelId *argp,
                                              struct svc_req *rqstp) {

   static VMAccelReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelReturnStatus_u.ret = cl->kerneldestroy_1(argp);

   return (&result);
}

VMAccelReturnStatus *vmcl_dispatch_1_svc(VMCLDispatchOp *argp,
                                         struct svc_req *rqstp) {

   static VMAccelReturnStatus result;

   /*
    * insert server code here
    */
   result.VMAccelReturnStatus_u.ret = cl->dispatch_1(argp);

   return (&result);
}
