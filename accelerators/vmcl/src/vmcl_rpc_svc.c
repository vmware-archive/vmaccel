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

#include "vmaccel_mgr.h"
#include "vmcl_ops.h"
#include "vmcl_rpc.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <syslog.h>

#include "vmaccel_types_address.h"
#include "log_level.h"

#ifndef SIG_PF
#define SIG_PF void (*)(int)
#endif

typedef struct VMAccelMgrClient {
   CLIENT *clnt;
   char *host;
   VMAccelId accelId;
} VMAccelMgrClient;

static VMAccelMgrClient vmaccelmgr_register(char *host, char *iface,
                                            VMAccelDesc *accelDesc) {
   VMAccelMgrClient mgrClient = {NULL, NULL, -1};
   struct ifaddrs *netifs = NULL, *netif;
   char localHost[NI_MAXHOST];
   bool netifFound = false;

   Log("Connecting to management host %s\n", host);

   if (getifaddrs(&netifs) == -1) {
      Warning("Unable to get network address\n");
      return mgrClient;
   }

   netif = netifs;
   while (netif != NULL) {
      int family, ret;

      if (netif->ifa_addr == NULL)
         continue;

      ret = getnameinfo(netif->ifa_addr, sizeof(struct sockaddr_in), localHost,
                        NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

      if (ret != 0) {
         Warning("getnameinfo() failed: %s\n", gai_strerror(ret));
         continue;
      }

      if (netif->ifa_addr->sa_family == AF_INET) {
         Log("  Interface: <%s>\n", netif->ifa_name);
         Log("    Address: <%s>\n", localHost);

         /*
          * Match network interface
          */
         if (iface != NULL) {
            if (strcmp(netif->ifa_name, iface) == 0) {
               Log("Selecting interface <%s>\n", netif->ifa_name);
               netifFound = true;
               break;
            }
         }
      }

      netif = netif->ifa_next;
   }

   /*
    * Register with the management server.
    */
   if (netifFound) {
      VMAccelAllocateReturnStatus *result_2;
      VMAccelRegisterDesc vmaccelmgr_register_1_arg;
      char localAddr[VMACCEL_MAX_LOCATION_SIZE];

      memset(&vmaccelmgr_register_1_arg, 0, sizeof(vmaccelmgr_register_1_arg));

      mgrClient.clnt = clnt_create(host, VMACCELMGR, VMACCELMGR_VERSION, "udp");
      if (mgrClient.clnt == NULL) {
         clnt_pcreateerror(host);
         exit(1);
      }

      vmaccelmgr_register_1_arg.desc = *accelDesc;
      VMAccelAddressStringToOpaqueAddr(localHost, localAddr, sizeof(localAddr));
      vmaccelmgr_register_1_arg.desc.parentAddr.addr.addr_val = &localAddr[0];
      vmaccelmgr_register_1_arg.desc.parentAddr.addr.addr_len =
         sizeof(localAddr);

      result_2 =
         vmaccelmgr_register_1(&vmaccelmgr_register_1_arg, mgrClient.clnt);
      if (result_2 == (VMAccelAllocateReturnStatus *)NULL) {
         clnt_perror(mgrClient.clnt, "call failed");
         clnt_destroy(mgrClient.clnt);
         mgrClient.clnt = NULL;
      }

      assert(result_2->VMAccelAllocateReturnStatus_u.ret != NULL);

      mgrClient.accelId = result_2->VMAccelAllocateReturnStatus_u.ret->id;
   }

   freeifaddrs(netifs);

   return mgrClient;
}

static void vmaccelmgr_unregister(VMAccelMgrClient *mgrClient) {
   VMAccelReturnStatus *result_3;
   VMAccelId vmaccelmgr_unregister_1_arg;

   /* TODO: Populate the Accelerator Global ID */
   vmaccelmgr_unregister_1_arg = mgrClient->accelId;

   result_3 =
      vmaccelmgr_unregister_1(&vmaccelmgr_unregister_1_arg, mgrClient->clnt);

   if (result_3 == (VMAccelReturnStatus *)NULL) {
      clnt_perror(mgrClient->clnt, "call failed");
   }

   clnt_destroy(mgrClient->clnt);
   mgrClient->clnt = NULL;

   free(mgrClient->host);
   mgrClient->host = NULL;
}

static void vmcl_1(struct svc_req *rqstp, register SVCXPRT *transp) {
   union {
      VMCLContextAllocateDesc vmcl_contextalloc_1_arg;
      VMCLContextId vmcl_contextdestroy_1_arg;
      VMCLSurfaceAllocateDesc vmcl_surfacealloc_1_arg;
      VMCLSurfaceId vmcl_surfacedestroy_1_arg;
      VMCLSurfaceId vmcl_surfacegetsharedhandle_1_arg;
      VMCLSharedHandle vmcl_surfacereleasesharedhandle_1_arg;
      VMCLQueueAllocateDesc vmcl_queuealloc_1_arg;
      VMCLQueueId vmcl_queuedestroy_1_arg;
      VMCLEventAllocateDesc vmcl_eventalloc_1_arg;
      VMCLEventId vmcl_eventgetstatus_1_arg;
      VMCLEventId vmcl_eventdestroy_1_arg;
      VMCLFenceAllocateDesc vmcl_fencealloc_1_arg;
      VMCLFenceId vmcl_fencegetstatus_1_arg;
      VMCLFenceId vmcl_fencedestroy_1_arg;
      VMCLQueueId vmcl_queueflush_1_arg;
      VMCLEventInsertOp vmcl_eventinsert_1_arg;
      VMCLFenceInsertOp vmcl_fenceinsert_1_arg;
      VMCLImageUploadOp vmcl_imageupload_1_arg;
      VMCLImageDownloadOp vmcl_imagedownload_1_arg;
      VMCLSurfaceMapOp vmcl_surfacemap_1_arg;
      VMCLSurfaceUnmapOp vmcl_surfaceunmap_1_arg;
      VMCLSurfaceCopyOp vmcl_surfacecopy_1_arg;
      VMCLImageFillOp vmcl_imagefill_1_arg;
      VMCLSamplerAllocateDesc vmcl_sampleralloc_1_arg;
      VMCLSamplerId vmcl_samplerdestroy_1_arg;
      VMCLKernelAllocateDesc vmcl_kernelalloc_1_arg;
      VMCLKernelId vmcl_kerneldestroy_1_arg;
      VMCLDispatchOp vmcl_dispatch_1_arg;
   } argument;
   char *result;
   xdrproc_t _xdr_argument, _xdr_result;
   char *(*local)(char *, struct svc_req *);

   switch (rqstp->rq_proc) {
      case NULLPROC:
         (void)svc_sendreply(transp, (xdrproc_t)xdr_void, (char *)NULL);
         return;

      case VMCL_CONTEXTALLOC:
         _xdr_argument = (xdrproc_t)xdr_VMCLContextAllocateDesc;
         _xdr_result = (xdrproc_t)xdr_VMCLContextAllocateReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_contextalloc_1_svc;
         break;

      case VMCL_CONTEXTDESTROY:
         _xdr_argument = (xdrproc_t)xdr_VMCLContextId;
         _xdr_result = (xdrproc_t)xdr_VMAccelReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_contextdestroy_1_svc;
         break;

      case VMCL_SURFACEALLOC:
         _xdr_argument = (xdrproc_t)xdr_VMCLSurfaceAllocateDesc;
         _xdr_result = (xdrproc_t)xdr_VMAccelSurfaceAllocateReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_surfacealloc_1_svc;
         break;

      case VMCL_SURFACEDESTROY:
         _xdr_argument = (xdrproc_t)xdr_VMCLSurfaceId;
         _xdr_result = (xdrproc_t)xdr_VMAccelReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_surfacedestroy_1_svc;
         break;

      case VMCL_SURFACEGETSHAREDHANDLE:
         _xdr_argument = (xdrproc_t)xdr_VMCLSurfaceId;
         _xdr_result = (xdrproc_t)xdr_VMAccelSharedHandleReturnStatus;
         local = (char *(*)(char *,
                            struct svc_req *))vmcl_surfacegetsharedhandle_1_svc;
         break;

      case VMCL_SURFACERELEASESHAREDHANDLE:
         _xdr_argument = (xdrproc_t)xdr_VMCLSharedHandle;
         _xdr_result = (xdrproc_t)xdr_VMAccelReturnStatus;
         local = (char *(
            *)(char *, struct svc_req *))vmcl_surfacereleasesharedhandle_1_svc;
         break;

      case VMCL_QUEUEALLOC:
         _xdr_argument = (xdrproc_t)xdr_VMCLQueueAllocateDesc;
         _xdr_result = (xdrproc_t)xdr_VMAccelQueueReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_queuealloc_1_svc;
         break;

      case VMCL_QUEUEDESTROY:
         _xdr_argument = (xdrproc_t)xdr_VMCLQueueId;
         _xdr_result = (xdrproc_t)xdr_VMAccelReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_queuedestroy_1_svc;
         break;

      case VMCL_EVENTALLOC:
         _xdr_argument = (xdrproc_t)xdr_VMCLEventAllocateDesc;
         _xdr_result = (xdrproc_t)xdr_VMAccelEventReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_eventalloc_1_svc;
         break;

      case VMCL_EVENTGETSTATUS:
         _xdr_argument = (xdrproc_t)xdr_VMCLEventId;
         _xdr_result = (xdrproc_t)xdr_VMAccelEventReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_eventgetstatus_1_svc;
         break;

      case VMCL_EVENTDESTROY:
         _xdr_argument = (xdrproc_t)xdr_VMCLEventId;
         _xdr_result = (xdrproc_t)xdr_VMAccelEventReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_eventdestroy_1_svc;
         break;

      case VMCL_FENCEALLOC:
         _xdr_argument = (xdrproc_t)xdr_VMCLFenceAllocateDesc;
         _xdr_result = (xdrproc_t)xdr_VMAccelFenceReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_fencealloc_1_svc;
         break;

      case VMCL_FENCEGETSTATUS:
         _xdr_argument = (xdrproc_t)xdr_VMCLFenceId;
         _xdr_result = (xdrproc_t)xdr_VMAccelFenceReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_fencegetstatus_1_svc;
         break;

      case VMCL_FENCEDESTROY:
         _xdr_argument = (xdrproc_t)xdr_VMCLFenceId;
         _xdr_result = (xdrproc_t)xdr_VMAccelFenceReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_fencedestroy_1_svc;
         break;

      case VMCL_QUEUEFLUSH:
         _xdr_argument = (xdrproc_t)xdr_VMCLQueueId;
         _xdr_result = (xdrproc_t)xdr_VMAccelReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_queueflush_1_svc;
         break;

      case VMCL_EVENTINSERT:
         _xdr_argument = (xdrproc_t)xdr_VMCLEventInsertOp;
         _xdr_result = (xdrproc_t)xdr_VMAccelReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_eventinsert_1_svc;
         break;

      case VMCL_FENCEINSERT:
         _xdr_argument = (xdrproc_t)xdr_VMCLFenceInsertOp;
         _xdr_result = (xdrproc_t)xdr_VMAccelReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_fenceinsert_1_svc;
         break;

      case VMCL_IMAGEUPLOAD:
         _xdr_argument = (xdrproc_t)xdr_VMCLImageUploadOp;
         _xdr_result = (xdrproc_t)xdr_VMAccelReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_imageupload_1_svc;
         break;

      case VMCL_IMAGEDOWNLOAD:
         _xdr_argument = (xdrproc_t)xdr_VMCLImageDownloadOp;
         _xdr_result = (xdrproc_t)xdr_VMAccelDownloadReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_imagedownload_1_svc;
         break;

      case VMCL_SURFACEMAP:
         _xdr_argument = (xdrproc_t)xdr_VMCLSurfaceMapOp;
         _xdr_result = (xdrproc_t)xdr_VMAccelSurfaceMapReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_surfacemap_1_svc;
         break;

      case VMCL_SURFACEUNMAP:
         _xdr_argument = (xdrproc_t)xdr_VMCLSurfaceUnmapOp;
         _xdr_result = (xdrproc_t)xdr_VMAccelReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_surfaceunmap_1_svc;
         break;

      case VMCL_SURFACECOPY:
         _xdr_argument = (xdrproc_t)xdr_VMCLSurfaceCopyOp;
         _xdr_result = (xdrproc_t)xdr_VMAccelReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_surfacecopy_1_svc;
         break;

      case VMCL_IMAGEFILL:
         _xdr_argument = (xdrproc_t)xdr_VMCLImageFillOp;
         _xdr_result = (xdrproc_t)xdr_VMAccelReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_imagefill_1_svc;
         break;

      case VMCL_SAMPLERALLOC:
         _xdr_argument = (xdrproc_t)xdr_VMCLSamplerAllocateDesc;
         _xdr_result = (xdrproc_t)xdr_VMCLSamplerAllocateReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_sampleralloc_1_svc;
         break;

      case VMCL_SAMPLERDESTROY:
         _xdr_argument = (xdrproc_t)xdr_VMCLSamplerId;
         _xdr_result = (xdrproc_t)xdr_VMAccelReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_samplerdestroy_1_svc;
         break;

      case VMCL_KERNELALLOC:
         _xdr_argument = (xdrproc_t)xdr_VMCLKernelAllocateDesc;
         _xdr_result = (xdrproc_t)xdr_VMCLKernelAllocateReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_kernelalloc_1_svc;
         break;

      case VMCL_KERNELDESTROY:
         _xdr_argument = (xdrproc_t)xdr_VMCLKernelId;
         _xdr_result = (xdrproc_t)xdr_VMAccelReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_kerneldestroy_1_svc;
         break;

      case VMCL_DISPATCH:
         _xdr_argument = (xdrproc_t)xdr_VMCLDispatchOp;
         _xdr_result = (xdrproc_t)xdr_VMAccelReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmcl_dispatch_1_svc;
         break;

      default:
         svcerr_noproc(transp);
         return;
   }
   memset((char *)&argument, 0, sizeof(argument));
   if (!svc_getargs(transp, (xdrproc_t)_xdr_argument, (caddr_t)&argument)) {
      svcerr_decode(transp);
      return;
   }
   result = (*local)((char *)&argument, rqstp);
   if (result != NULL &&
       !svc_sendreply(transp, (xdrproc_t)_xdr_result, result)) {
      svcerr_systemerr(transp);
   }
   if (!svc_freeargs(transp, (xdrproc_t)_xdr_argument, (caddr_t)&argument)) {
      syslog(LOG_ERR, "%s", "unable to free arguments");
      exit(1);
   }
   return;
}

int main(int argc, char **argv) {
   register SVCXPRT *transp;
   VMAccelAllocateStatus *allocStatus;
   VMAccelMgrClient mgrClient = {NULL, NULL, -1};

   pmap_unset(VMCL, VMCL_VERSION);
   openlog("vmcl_rpc", LOG_PID, LOG_DAEMON);

   transp = svcudp_create(RPC_ANYSOCK);
   if (transp == NULL) {
      syslog(LOG_ERR, "%s", "cannot create udp service.");
      exit(1);
   }
   if (!svc_register(transp, VMCL, VMCL_VERSION, vmcl_1, IPPROTO_UDP)) {
      syslog(LOG_ERR, "%s", "unable to register (VMCL, VMCL_VERSION, udp).");
      exit(1);
   }

   transp = svctcp_create(RPC_ANYSOCK, 0, 0);
   if (transp == NULL) {
      syslog(LOG_ERR, "%s", "cannot create tcp service.");
      exit(1);
   }
   if (!svc_register(transp, VMCL, VMCL_VERSION, vmcl_1, IPPROTO_TCP)) {
      syslog(LOG_ERR, "%s", "unable to register (VMCL, VMCL_VERSION, tcp).");
      exit(1);
   }

   allocStatus = vmcl_poweron_svc(NULL);

   if ((allocStatus == NULL) || (allocStatus->status != VMACCEL_SUCCESS)) {
      Warning("Failed to power on VMCL...\n");
      exit(1);
   }

   /*
    * Managment host specified.
    */
   if (argc >= 3) {
      char *iface = argv[1];
      char *host = argv[2];

      mgrClient = vmaccelmgr_register(host, iface, &allocStatus->desc);

      if (mgrClient.clnt == NULL) {
         Warning("Unable to register with management server...\n");
         vmcl_poweroff_svc();
         exit(1);
      }
   }

   svc_run();
   syslog(LOG_ERR, "%s", "svc_run returned");

   if (mgrClient.clnt != NULL) {
      vmaccelmgr_unregister(&mgrClient);
   }

   vmcl_poweroff_svc();

   exit(1);
   /* NOTREACHED */
}
