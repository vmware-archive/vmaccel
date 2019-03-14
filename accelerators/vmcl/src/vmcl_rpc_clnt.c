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

#include <memory.h>
#include "vmcl_rpc.h"


/* Default timeout can be changed using clnt_control() */
static struct timeval TIMEOUT = {25, 0};

VMCLContextAllocateReturnStatus *
vmcl_contextalloc_1(VMCLContextAllocateDesc *argp, CLIENT *clnt) {
   static VMCLContextAllocateReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_CONTEXTALLOC,
                 (xdrproc_t)xdr_VMCLContextAllocateDesc, (caddr_t)argp,
                 (xdrproc_t)xdr_VMCLContextAllocateReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelReturnStatus *vmcl_contextdestroy_1(VMCLContextId *argp, CLIENT *clnt) {
   static VMAccelReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_CONTEXTDESTROY, (xdrproc_t)xdr_VMCLContextId,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelSurfaceAllocateReturnStatus *
vmcl_surfacealloc_1(VMCLSurfaceAllocateDesc *argp, CLIENT *clnt) {
   static VMAccelSurfaceAllocateReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_SURFACEALLOC,
                 (xdrproc_t)xdr_VMCLSurfaceAllocateDesc, (caddr_t)argp,
                 (xdrproc_t)xdr_VMAccelSurfaceAllocateReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelReturnStatus *vmcl_surfacedestroy_1(VMCLSurfaceId *argp, CLIENT *clnt) {
   static VMAccelReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_SURFACEDESTROY, (xdrproc_t)xdr_VMCLSurfaceId,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelSharedHandleReturnStatus *
vmcl_surfacegetsharedhandle_1(VMCLSurfaceId *argp, CLIENT *clnt) {
   static VMAccelSharedHandleReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_SURFACEGETSHAREDHANDLE,
                 (xdrproc_t)xdr_VMCLSurfaceId, (caddr_t)argp,
                 (xdrproc_t)xdr_VMAccelSharedHandleReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelReturnStatus *vmcl_surfacereleasesharedhandle_1(VMCLSharedHandle *argp,
                                                       CLIENT *clnt) {
   static VMAccelReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_SURFACERELEASESHAREDHANDLE,
                 (xdrproc_t)xdr_VMCLSharedHandle, (caddr_t)argp,
                 (xdrproc_t)xdr_VMAccelReturnStatus, (caddr_t)&clnt_res,
                 TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelQueueReturnStatus *vmcl_queuealloc_1(VMCLQueueAllocateDesc *argp,
                                            CLIENT *clnt) {
   static VMAccelQueueReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_QUEUEALLOC, (xdrproc_t)xdr_VMCLQueueAllocateDesc,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelQueueReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelReturnStatus *vmcl_queuedestroy_1(VMCLQueueId *argp, CLIENT *clnt) {
   static VMAccelReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_QUEUEDESTROY, (xdrproc_t)xdr_VMCLQueueId,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelEventReturnStatus *vmcl_eventalloc_1(VMCLEventAllocateDesc *argp,
                                            CLIENT *clnt) {
   static VMAccelEventReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_EVENTALLOC, (xdrproc_t)xdr_VMCLEventAllocateDesc,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelEventReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelEventReturnStatus *vmcl_eventgetstatus_1(VMCLEventId *argp,
                                                CLIENT *clnt) {
   static VMAccelEventReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_EVENTGETSTATUS, (xdrproc_t)xdr_VMCLEventId,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelEventReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelEventReturnStatus *vmcl_eventdestroy_1(VMCLEventId *argp, CLIENT *clnt) {
   static VMAccelEventReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_EVENTDESTROY, (xdrproc_t)xdr_VMCLEventId,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelEventReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelFenceReturnStatus *vmcl_fencealloc_1(VMCLFenceAllocateDesc *argp,
                                            CLIENT *clnt) {
   static VMAccelFenceReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_FENCEALLOC, (xdrproc_t)xdr_VMCLFenceAllocateDesc,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelFenceReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelFenceReturnStatus *vmcl_fencegetstatus_1(VMCLFenceId *argp,
                                                CLIENT *clnt) {
   static VMAccelFenceReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_FENCEGETSTATUS, (xdrproc_t)xdr_VMCLFenceId,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelFenceReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelFenceReturnStatus *vmcl_fencedestroy_1(VMCLFenceId *argp, CLIENT *clnt) {
   static VMAccelFenceReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_FENCEDESTROY, (xdrproc_t)xdr_VMCLFenceId,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelFenceReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelReturnStatus *vmcl_queueflush_1(VMCLQueueId *argp, CLIENT *clnt) {
   static VMAccelReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_QUEUEFLUSH, (xdrproc_t)xdr_VMCLQueueId,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelReturnStatus *vmcl_eventinsert_1(VMCLEventInsertOp *argp, CLIENT *clnt) {
   static VMAccelReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_EVENTINSERT, (xdrproc_t)xdr_VMCLEventInsertOp,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelReturnStatus *vmcl_fenceinsert_1(VMCLFenceInsertOp *argp, CLIENT *clnt) {
   static VMAccelReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_FENCEINSERT, (xdrproc_t)xdr_VMCLFenceInsertOp,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelReturnStatus *vmcl_imageupload_1(VMCLImageUploadOp *argp, CLIENT *clnt) {
   static VMAccelReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_IMAGEUPLOAD, (xdrproc_t)xdr_VMCLImageUploadOp,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelDownloadReturnStatus *vmcl_imagedownload_1(VMCLImageDownloadOp *argp,
                                                  CLIENT *clnt) {
   static VMAccelDownloadReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_IMAGEDOWNLOAD, (xdrproc_t)xdr_VMCLImageDownloadOp,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelDownloadReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelSurfaceMapReturnStatus *vmcl_surfacemap_1(VMCLSurfaceMapOp *argp,
                                                 CLIENT *clnt) {
   static VMAccelSurfaceMapReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_SURFACEMAP, (xdrproc_t)xdr_VMCLSurfaceMapOp,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelSurfaceMapReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelReturnStatus *vmcl_surfaceunmap_1(VMCLSurfaceUnmapOp *argp,
                                         CLIENT *clnt) {
   static VMAccelReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_SURFACEUNMAP, (xdrproc_t)xdr_VMCLSurfaceUnmapOp,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelReturnStatus *vmcl_surfacecopy_1(VMCLSurfaceCopyOp *argp, CLIENT *clnt) {
   static VMAccelReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_SURFACECOPY, (xdrproc_t)xdr_VMCLSurfaceCopyOp,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelReturnStatus *vmcl_imagefill_1(VMCLImageFillOp *argp, CLIENT *clnt) {
   static VMAccelReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_IMAGEFILL, (xdrproc_t)xdr_VMCLImageFillOp,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMCLSamplerAllocateReturnStatus *
vmcl_sampleralloc_1(VMCLSamplerAllocateDesc *argp, CLIENT *clnt) {
   static VMCLSamplerAllocateReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_SAMPLERALLOC,
                 (xdrproc_t)xdr_VMCLSamplerAllocateDesc, (caddr_t)argp,
                 (xdrproc_t)xdr_VMCLSamplerAllocateReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelReturnStatus *vmcl_samplerdestroy_1(VMCLSamplerId *argp, CLIENT *clnt) {
   static VMAccelReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_SAMPLERDESTROY, (xdrproc_t)xdr_VMCLSamplerId,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMCLKernelAllocateReturnStatus *vmcl_kernelalloc_1(VMCLKernelAllocateDesc *argp,
                                                   CLIENT *clnt) {
   static VMCLKernelAllocateReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_KERNELALLOC, (xdrproc_t)xdr_VMCLKernelAllocateDesc,
                 (caddr_t)argp, (xdrproc_t)xdr_VMCLKernelAllocateReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelReturnStatus *vmcl_kerneldestroy_1(VMCLKernelId *argp, CLIENT *clnt) {
   static VMAccelReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_KERNELDESTROY, (xdrproc_t)xdr_VMCLKernelId,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}

VMAccelReturnStatus *vmcl_dispatch_1(VMCLDispatchOp *argp, CLIENT *clnt) {
   static VMAccelReturnStatus clnt_res;

   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_DISPATCH, (xdrproc_t)xdr_VMCLDispatchOp,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      return (NULL);
   }
   return (&clnt_res);
}
