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
#include "vmaccel_ops.h"
#include "vmaccel_rpc.h"
#include <memory.h> /* for memset */
#include <pthread.h>

/* Default timeout can be changed using clnt_control() */
static struct timeval TIMEOUT = {25, 0};

extern pthread_mutex_t svc_compute_mutex;
extern pthread_mutex_t svc_data_mutex;
extern pthread_mutex_t svc_state_mutex;

VMAccelResourceAllocateReturnStatus *
vmaccel_resourcealloc_1(VMAccelResourceDesc *argp, CLIENT *clnt) {
#if ENABLE_VMACCEL_LOCAL
   if (clnt == NULL) {
      VMAccelResourceAllocateReturnStatus *ret;
      if (pthread_mutex_lock(&svc_state_mutex) != 0) {
         VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
         return (NULL);
      }
      ret = vmaccel_resourcealloc_1_svc(argp, NULL);
      pthread_mutex_unlock(&svc_state_mutex);
      return ret;
   }
#endif
#if ENABLE_VMACCEL_RPC
   static VMAccelResourceAllocateReturnStatus clnt_res;
   if (pthread_mutex_lock(&svc_state_mutex) != 0) {
      VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
      return (NULL);
   }
   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMACCEL_RESOURCEALLOC,
                 (xdrproc_t)xdr_VMAccelResourceDesc, (caddr_t)argp,
                 (xdrproc_t)xdr_VMAccelResourceAllocateReturnStatus,
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

VMAccelReturnStatus *vmaccel_resourcerelease_1(VMAccelId *argp, CLIENT *clnt) {
#if ENABLE_VMACCEL_LOCAL
   if (clnt == NULL) {
      VMAccelReturnStatus *ret;
      if (pthread_mutex_lock(&svc_state_mutex) != 0) {
         VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
         return (NULL);
      }
      ret = vmaccel_resourcerelease_1_svc(argp, NULL);
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
   if (clnt_call(clnt, VMACCEL_RESOURCERELEASE, (xdrproc_t)xdr_VMAccelId,
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

VMAccelComputeReturnStatus *vmaccel_compute_1(VMAccelComputeOp *argp,
                                              CLIENT *clnt) {
#if ENABLE_VMACCEL_LOCAL
   if (clnt == NULL) {
      VMAccelComputeReturnStatus *ret;
      if (pthread_mutex_lock(&svc_compute_mutex) != 0) {
         VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
         return (NULL);
      }
      ret = vmaccel_compute_1_svc(argp, NULL);
      pthread_mutex_unlock(&svc_compute_mutex);
      return ret;
   }
#endif
#if ENABLE_VMACCEL_RPC
   static VMAccelComputeReturnStatus clnt_res;
   if (pthread_mutex_lock(&svc_compute_mutex) != 0) {
      VMACCEL_WARNING("%s: Unable to acquire svc lock\n", __FUNCTION__);
      return (NULL);
   }
   memset((char *)&clnt_res, 0, sizeof(clnt_res));
   if (clnt_call(clnt, VMACCEL_COMPUTE, (xdrproc_t)xdr_VMAccelComputeOp,
                 (caddr_t)argp, (xdrproc_t)xdr_VMAccelComputeReturnStatus,
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
