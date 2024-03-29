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

#include "log_level.h"
#include "vmcl_rpc.h"
#include <memory.h>
#include <pthread.h>

#if ENABLE_VMACCEL_RPC
/* Default timeout can be changed using clnt_control() */
static struct timeval TIMEOUT = {25, 0};
#endif

extern pthread_mutex_t svc_compute_mutex;
extern pthread_mutex_t svc_data_mutex;
extern pthread_mutex_t svc_state_mutex;

VMCLContextAllocateReturnStatus *
vmcl_contextalloc_2(VMCLContextAllocateDesc *argp, CLIENT *clnt) {
#if ENABLE_VMACCEL_LOCAL
   if (clnt == NULL) {
      VMCLContextAllocateReturnStatus *ret;
      if (pthread_mutex_lock(&svc_state_mutex) != 0) {
         VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
         return (NULL);
      }
      ret = vmcl_contextalloc_2_svc(argp, NULL);
      pthread_mutex_unlock(&svc_state_mutex);
      return ret;
   }
#endif
#if ENABLE_VMACCEL_RPC
   static VMCLContextAllocateReturnStatus clnt_res;
   if (pthread_mutex_lock(&svc_state_mutex) != 0) {
      VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
      return (NULL);
   }
   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_CONTEXTALLOC,
                 (xdrproc_t)xdr_VMCLContextAllocateDesc, (caddr_t)argp,
                 (xdrproc_t)xdr_VMCLContextAllocateReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      pthread_mutex_unlock(&svc_state_mutex);
      return (NULL);
   }
   pthread_mutex_unlock(&svc_state_mutex);
   return (&clnt_res);
#else
   return (NULL);
#endif
}

VMAccelReturnStatus *vmcl_contextdestroy_2(VMCLContextId *argp, CLIENT *clnt) {
#if ENABLE_VMACCEL_LOCAL
   if (clnt == NULL) {
      VMAccelReturnStatus *ret;
      if (pthread_mutex_lock(&svc_state_mutex) != 0) {
         VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
         return (NULL);
      }
      ret = vmcl_contextdestroy_2_svc(argp, NULL);
      pthread_mutex_unlock(&svc_state_mutex);
      return ret;
   }
#endif
#if ENABLE_VMACCEL_RPC
   static VMAccelReturnStatus clnt_res;
   if (pthread_mutex_lock(&svc_state_mutex) != 0) {
      VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
      return (NULL);
   }
   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_CONTEXTDESTROY, (xdrproc_t)xdr_VMCLContextId,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      pthread_mutex_unlock(&svc_state_mutex);
      return (NULL);
   }
   pthread_mutex_unlock(&svc_state_mutex);
   return (&clnt_res);
#else
   return (NULL);
#endif
}

VMAccelSurfaceAllocateReturnStatus *
vmcl_surfacealloc_2(VMCLSurfaceAllocateDesc *argp, CLIENT *clnt) {
#if ENABLE_VMACCEL_LOCAL
   if (clnt == NULL) {
      VMAccelSurfaceAllocateReturnStatus *ret;
      if (pthread_mutex_lock(&svc_state_mutex) != 0) {
         VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
         return (NULL);
      }
      ret = vmcl_surfacealloc_2_svc(argp, NULL);
      pthread_mutex_unlock(&svc_state_mutex);
      return ret;
   }
#endif
#if ENABLE_VMACCEL_RPC
   static VMAccelSurfaceAllocateReturnStatus clnt_res;
   if (pthread_mutex_lock(&svc_state_mutex) != 0) {
      VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
      return (NULL);
   }
   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_SURFACEALLOC,
                 (xdrproc_t)xdr_VMCLSurfaceAllocateDesc, (caddr_t)argp,
                 (xdrproc_t)xdr_VMAccelSurfaceAllocateReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      pthread_mutex_unlock(&svc_state_mutex);
      return (NULL);
   }
   pthread_mutex_unlock(&svc_state_mutex);
   return (&clnt_res);
#else
   return (NULL);
#endif
}

VMAccelReturnStatus *vmcl_surfacedestroy_2(VMCLSurfaceId *argp, CLIENT *clnt) {
#if ENABLE_VMACCEL_LOCAL
   if (clnt == NULL) {
      VMAccelReturnStatus *ret;
      if (pthread_mutex_lock(&svc_state_mutex) != 0) {
         VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
         return (NULL);
      }
      ret = vmcl_surfacedestroy_2_svc(argp, NULL);
      pthread_mutex_unlock(&svc_state_mutex);
      return ret;
   }
#endif
#if ENABLE_VMACCEL_RPC
   static VMAccelReturnStatus clnt_res;
   if (pthread_mutex_lock(&svc_state_mutex) != 0) {
      VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
      return (NULL);
   }
   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_SURFACEDESTROY, (xdrproc_t)xdr_VMCLSurfaceId,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      pthread_mutex_unlock(&svc_state_mutex);
      return (NULL);
   }
   pthread_mutex_unlock(&svc_state_mutex);
   return (&clnt_res);
#else
   return (NULL);
#endif
}

VMAccelQueueReturnStatus *vmcl_queuealloc_2(VMCLQueueAllocateDesc *argp,
                                            CLIENT *clnt) {
#if ENABLE_VMACCEL_LOCAL
   if (clnt == NULL) {
      VMAccelQueueReturnStatus *ret;
      if (pthread_mutex_lock(&svc_state_mutex) != 0) {
         VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
         return (NULL);
      }
      ret = vmcl_queuealloc_2_svc(argp, NULL);
      pthread_mutex_unlock(&svc_state_mutex);
      return ret;
   }
#endif
#if ENABLE_VMACCEL_RPC
   static VMAccelQueueReturnStatus clnt_res;
   if (pthread_mutex_lock(&svc_state_mutex) != 0) {
      VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
      return (NULL);
   }
   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_QUEUEALLOC, (xdrproc_t)xdr_VMCLQueueAllocateDesc,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelQueueReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      pthread_mutex_unlock(&svc_state_mutex);
      return (NULL);
   }
   pthread_mutex_unlock(&svc_state_mutex);
   return (&clnt_res);
#else
   return (NULL);
#endif
}

VMAccelReturnStatus *vmcl_queuedestroy_2(VMCLQueueId *argp, CLIENT *clnt) {
#if ENABLE_VMACCEL_LOCAL
   if (clnt == NULL) {
      VMAccelReturnStatus *ret;
      if (pthread_mutex_lock(&svc_state_mutex) != 0) {
         VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
         return (NULL);
      }
      ret = vmcl_queuedestroy_2_svc(argp, NULL);
      pthread_mutex_unlock(&svc_state_mutex);
      return ret;
   }
#endif
#if ENABLE_VMACCEL_RPC
   static VMAccelReturnStatus clnt_res;
   if (pthread_mutex_lock(&svc_state_mutex) != 0) {
      VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
      return (NULL);
   }
   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_QUEUEDESTROY, (xdrproc_t)xdr_VMCLQueueId,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      pthread_mutex_unlock(&svc_state_mutex);
      return (NULL);
   }
   pthread_mutex_unlock(&svc_state_mutex);
   return (&clnt_res);
#else
   return (NULL);
#endif
}

VMAccelReturnStatus *vmcl_queueflush_2(VMCLQueueId *argp, CLIENT *clnt) {
#if ENABLE_VMACCEL_LOCAL
   if (clnt == NULL) {
      VMAccelReturnStatus *ret;
      if (pthread_mutex_lock(&svc_state_mutex) != 0) {
         VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
         return (NULL);
      }
      ret = vmcl_queueflush_2_svc(argp, NULL);
      pthread_mutex_unlock(&svc_state_mutex);
      return ret;
   }
#endif
#if ENABLE_VMACCEL_RPC
   static VMAccelReturnStatus clnt_res;
   if (pthread_mutex_lock(&svc_state_mutex) != 0) {
      VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
      return (NULL);
   }
   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_QUEUEFLUSH, (xdrproc_t)xdr_VMCLQueueId,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      pthread_mutex_unlock(&svc_state_mutex);
      return (NULL);
   }
   pthread_mutex_unlock(&svc_state_mutex);
   return (&clnt_res);
#else
   return (NULL);
#endif
}

VMAccelReturnStatus *vmcl_imageupload_2(VMCLImageUploadOp *argp, CLIENT *clnt) {
#if ENABLE_VMACCEL_LOCAL
   if (clnt == NULL) {
      VMAccelReturnStatus *ret;
      if (pthread_mutex_lock(&svc_data_mutex) != 0) {
         VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
         return (NULL);
      }
      ret = vmcl_imageupload_2_svc(argp, NULL);
      pthread_mutex_unlock(&svc_data_mutex);
      return ret;
   }
#endif
#if ENABLE_VMACCEL_RPC
   static VMAccelReturnStatus clnt_res;
   if (pthread_mutex_lock(&svc_data_mutex) != 0) {
      VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
      return (NULL);
   }
   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_IMAGEUPLOAD, (xdrproc_t)xdr_VMCLImageUploadOp,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      pthread_mutex_unlock(&svc_data_mutex);
      return (NULL);
   }
   pthread_mutex_unlock(&svc_data_mutex);
   return (&clnt_res);
#else
   return (NULL);
#endif
}

VMAccelDownloadReturnStatus *vmcl_imagedownload_2(VMCLImageDownloadOp *argp,
                                                  CLIENT *clnt) {
#if ENABLE_VMACCEL_LOCAL
   if (clnt == NULL) {
      VMAccelDownloadReturnStatus *ret;
      if (pthread_mutex_lock(&svc_data_mutex) != 0) {
         VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
         return (NULL);
      }
      ret = vmcl_imagedownload_2_svc(argp, NULL);
      pthread_mutex_unlock(&svc_data_mutex);
      return ret;
   }
#endif
#if ENABLE_VMACCEL_RPC
   static VMAccelDownloadReturnStatus clnt_res;
   if (pthread_mutex_lock(&svc_data_mutex) != 0) {
      VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
      return (NULL);
   }
   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_IMAGEDOWNLOAD, (xdrproc_t)xdr_VMCLImageDownloadOp,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelDownloadReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      pthread_mutex_unlock(&svc_data_mutex);
      return (NULL);
   }
   pthread_mutex_unlock(&svc_data_mutex);
   return (&clnt_res);
#else
   return (NULL);
#endif
}

VMAccelSurfaceMapReturnStatus *vmcl_surfacemap_2(VMCLSurfaceMapOp *argp,
                                                 CLIENT *clnt) {
#if ENABLE_VMACCEL_LOCAL
   if (clnt == NULL) {
      VMAccelSurfaceMapReturnStatus *ret;
      if (pthread_mutex_lock(&svc_data_mutex) != 0) {
         VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
         return (NULL);
      }
      ret = vmcl_surfacemap_2_svc(argp, NULL);
      pthread_mutex_unlock(&svc_data_mutex);
      return ret;
   }
#endif
#if ENABLE_VMACCEL_RPC
   static VMAccelSurfaceMapReturnStatus clnt_res;
   if (pthread_mutex_lock(&svc_data_mutex) != 0) {
      VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
      return (NULL);
   }
   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_SURFACEMAP, (xdrproc_t)xdr_VMCLSurfaceMapOp,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelSurfaceMapReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      pthread_mutex_unlock(&svc_data_mutex);
      return (NULL);
   }
   pthread_mutex_unlock(&svc_data_mutex);
   return (&clnt_res);
#else
   return (NULL);
#endif
}

VMAccelReturnStatus *vmcl_surfaceunmap_2(VMCLSurfaceUnmapOp *argp,
                                         CLIENT *clnt) {
#if ENABLE_VMACCEL_LOCAL
   if (clnt == NULL) {
      VMAccelReturnStatus *ret;
      if (pthread_mutex_lock(&svc_data_mutex) != 0) {
         VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
         return (NULL);
      }
      ret = vmcl_surfaceunmap_2_svc(argp, NULL);
      pthread_mutex_unlock(&svc_data_mutex);
      return ret;
   }
#endif
#if ENABLE_VMACCEL_RPC
   static VMAccelReturnStatus clnt_res;
   if (pthread_mutex_lock(&svc_data_mutex) != 0) {
      VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
      return (NULL);
   }
   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_SURFACEUNMAP, (xdrproc_t)xdr_VMCLSurfaceUnmapOp,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      pthread_mutex_unlock(&svc_data_mutex);
      return (NULL);
   }
   pthread_mutex_unlock(&svc_data_mutex);
   return (&clnt_res);
#else
   return (NULL);
#endif
}

VMAccelReturnStatus *vmcl_surfacecopy_2(VMCLSurfaceCopyOp *argp, CLIENT *clnt) {
#if ENABLE_VMACCEL_LOCAL
   if (clnt == NULL) {
      VMAccelReturnStatus *ret;
      if (pthread_mutex_lock(&svc_data_mutex) != 0) {
         VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
         return (NULL);
      }
      ret = vmcl_surfacecopy_2_svc(argp, NULL);
      pthread_mutex_unlock(&svc_data_mutex);
      return ret;
   }
#endif
#if ENABLE_VMACCEL_RPC
   static VMAccelReturnStatus clnt_res;
   if (pthread_mutex_lock(&svc_data_mutex) != 0) {
      VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
      return (NULL);
   }
   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_SURFACECOPY, (xdrproc_t)xdr_VMCLSurfaceCopyOp,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      pthread_mutex_unlock(&svc_data_mutex);
      return (NULL);
   }
   pthread_mutex_unlock(&svc_data_mutex);
   return (&clnt_res);
#else
   return (NULL);
#endif
}

VMAccelReturnStatus *vmcl_imagefill_2(VMCLImageFillOp *argp, CLIENT *clnt) {
#if ENABLE_VMACCEL_LOCAL
   if (clnt == NULL) {
      VMAccelReturnStatus *ret;
      if (pthread_mutex_lock(&svc_data_mutex) != 0) {
         VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
         return (NULL);
      }
      ret = vmcl_imagefill_2_svc(argp, NULL);
      pthread_mutex_unlock(&svc_data_mutex);
      return ret;
   }
#endif
#if ENABLE_VMACCEL_RPC
   static VMAccelReturnStatus clnt_res;
   if (pthread_mutex_lock(&svc_data_mutex) != 0) {
      VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
      return (NULL);
   }
   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_IMAGEFILL, (xdrproc_t)xdr_VMCLImageFillOp,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      pthread_mutex_unlock(&svc_data_mutex);
      return (NULL);
   }
   pthread_mutex_unlock(&svc_data_mutex);
   return (&clnt_res);
#else
   return (NULL);
#endif
}

VMCLSamplerAllocateReturnStatus *
vmcl_sampleralloc_2(VMCLSamplerAllocateDesc *argp, CLIENT *clnt) {
#if ENABLE_VMACCEL_LOCAL
   if (clnt == NULL) {
      VMCLSamplerAllocateReturnStatus *ret;
      if (pthread_mutex_lock(&svc_state_mutex) != 0) {
         VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
         return (NULL);
      }
      ret = vmcl_sampleralloc_2_svc(argp, NULL);
      pthread_mutex_unlock(&svc_state_mutex);
      return ret;
   }
#endif
#if ENABLE_VMACCEL_RPC
   static VMCLSamplerAllocateReturnStatus clnt_res;
   if (pthread_mutex_lock(&svc_state_mutex) != 0) {
      VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
      return (NULL);
   }
   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_SAMPLERALLOC,
                 (xdrproc_t)xdr_VMCLSamplerAllocateDesc, (caddr_t)argp,
                 (xdrproc_t)xdr_VMCLSamplerAllocateReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      pthread_mutex_unlock(&svc_state_mutex);
      return (NULL);
   }
   pthread_mutex_unlock(&svc_state_mutex);
   return (&clnt_res);
#else
   return (NULL);
#endif
}

VMAccelReturnStatus *vmcl_samplerdestroy_2(VMCLSamplerId *argp, CLIENT *clnt) {
#if ENABLE_VMACCEL_LOCAL
   if (clnt == NULL) {
      VMAccelReturnStatus *ret;
      if (pthread_mutex_lock(&svc_state_mutex) != 0) {
         VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
         return (NULL);
      }
      ret = vmcl_samplerdestroy_2_svc(argp, NULL);
      pthread_mutex_unlock(&svc_state_mutex);
      return ret;
   }
#endif
#if ENABLE_VMACCEL_RPC
   static VMAccelReturnStatus clnt_res;
   if (pthread_mutex_lock(&svc_state_mutex) != 0) {
      VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
      return (NULL);
   }
   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_SAMPLERDESTROY, (xdrproc_t)xdr_VMCLSamplerId,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      pthread_mutex_unlock(&svc_state_mutex);
      return (NULL);
   }
   pthread_mutex_unlock(&svc_state_mutex);
   return (&clnt_res);
#else
   return (NULL);
#endif
}

VMCLKernelAllocateReturnStatus *vmcl_kernelalloc_2(VMCLKernelAllocateDesc *argp,
                                                   CLIENT *clnt) {
#if ENABLE_VMACCEL_LOCAL
   if (clnt == NULL) {
      VMCLKernelAllocateReturnStatus *ret;
      if (pthread_mutex_lock(&svc_state_mutex) != 0) {
         VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
         return (NULL);
      }
      ret = vmcl_kernelalloc_2_svc(argp, NULL);
      pthread_mutex_unlock(&svc_state_mutex);
      return ret;
   }
#endif
#if ENABLE_VMACCEL_RPC
   static VMCLKernelAllocateReturnStatus clnt_res;
   if (pthread_mutex_lock(&svc_state_mutex) != 0) {
      VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
      return (NULL);
   }
   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_KERNELALLOC, (xdrproc_t)xdr_VMCLKernelAllocateDesc,
                 (caddr_t)argp, (xdrproc_t)xdr_VMCLKernelAllocateReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      pthread_mutex_unlock(&svc_state_mutex);
      return (NULL);
   }
   pthread_mutex_unlock(&svc_state_mutex);
   return (&clnt_res);
#else
   return (NULL);
#endif
}

VMAccelReturnStatus *vmcl_kerneldestroy_2(VMCLKernelId *argp, CLIENT *clnt) {
#if ENABLE_VMACCEL_LOCAL
   if (clnt == NULL) {
      VMAccelReturnStatus *ret;
      if (pthread_mutex_lock(&svc_state_mutex) != 0) {
         VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
         return (NULL);
      }
      ret = vmcl_kerneldestroy_2_svc(argp, NULL);
      pthread_mutex_unlock(&svc_state_mutex);
      return ret;
   }
#endif
#if ENABLE_VMACCEL_RPC
   static VMAccelReturnStatus clnt_res;
   if (pthread_mutex_lock(&svc_state_mutex) != 0) {
      VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
      return (NULL);
   }
   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_KERNELDESTROY, (xdrproc_t)xdr_VMCLKernelId,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      pthread_mutex_unlock(&svc_state_mutex);
      return (NULL);
   }
   pthread_mutex_unlock(&svc_state_mutex);
   return (&clnt_res);
#else
   return (NULL);
#endif
}

VMAccelReturnStatus *vmcl_dispatch_2(VMCLDispatchOp *argp, CLIENT *clnt) {
#if ENABLE_VMACCEL_LOCAL
   if (clnt == NULL) {
      VMAccelReturnStatus *ret;
      if (pthread_mutex_lock(&svc_compute_mutex) != 0) {
         VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
         return (NULL);
      }
      ret = vmcl_dispatch_2_svc(argp, NULL);
      pthread_mutex_unlock(&svc_compute_mutex);
      return ret;
   }
#endif
#if ENABLE_VMACCEL_RPC
   static VMAccelReturnStatus clnt_res;
   if (pthread_mutex_lock(&svc_compute_mutex) != 0) {
      VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
      return (NULL);
   }
   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMCL_DISPATCH, (xdrproc_t)xdr_VMCLDispatchOp,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelReturnStatus,
                 (caddr_t)&clnt_res, TIMEOUT) != RPC_SUCCESS) {
      pthread_mutex_unlock(&svc_compute_mutex);
      return (NULL);
   }
   pthread_mutex_unlock(&svc_compute_mutex);
   return (&clnt_res);
#else
   return (NULL);
#endif
}
