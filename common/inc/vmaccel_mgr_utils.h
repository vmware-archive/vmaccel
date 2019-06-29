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

#ifndef _VMACCEL_MGR_UTILS_H_
#define _VMACCEL_MGR_UTILS_H_

#include "vmaccel_utils.h"

typedef struct VMAccelMgrClient {
   CLIENT *clnt;
   char *host;
   VMAccelId accelId;
} VMAccelMgrClient;

#ifdef __cplusplus
extern "C" {
#endif

static VMAccelMgrClient vmaccelmgr_register(char *host, char *iface,
                                            VMAccelDesc *accelDesc) {
   VMAccelMgrClient mgrClient = {NULL, NULL, -1};
   struct ifaddrs *netifs = NULL, *netif;
   char localHost[NI_MAXHOST];
   bool netifFound = false;

   VMACCEL_LOG("Connecting to management host %s\n", host);

   if (getifaddrs(&netifs) == -1) {
      VMACCEL_WARNING("Unable to get network address\n");
      return mgrClient;
   }

   netif = netifs;
   while (netif != NULL) {
      int ret;

      if (netif->ifa_addr == NULL) {
         netif = netif->ifa_next;
         continue;
      }

      ret = getnameinfo(netif->ifa_addr, sizeof(struct sockaddr_in), localHost,
                        NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

      if (ret != 0) {
         VMACCEL_WARNING("getnameinfo() failed: %s\n", gai_strerror(ret));
         netif = netif->ifa_next;
         continue;
      }

      if (netif->ifa_addr->sa_family == AF_INET) {
         VMACCEL_LOG("  Interface: <%s>\n", netif->ifa_name);
         VMACCEL_LOG("    Address: <%s>\n", localHost);

         /*
          * Match network interface
          */
         if (iface != NULL) {
            if (strcmp(netif->ifa_name, iface) == 0) {
               VMACCEL_LOG("Selecting interface <%s>\n", netif->ifa_name);
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
      VMAccel_AddressStringToOpaqueAddr(localHost, localAddr,
                                        sizeof(localAddr));
      vmaccelmgr_register_1_arg.desc.parentAddr.addr.addr_val = &localAddr[0];
      vmaccelmgr_register_1_arg.desc.parentAddr.addr.addr_len =
         sizeof(localAddr);

      result_2 =
         vmaccelmgr_register_1(&vmaccelmgr_register_1_arg, mgrClient.clnt);
      if ((result_2 == (VMAccelAllocateReturnStatus *)NULL) ||
          (result_2->VMAccelAllocateReturnStatus_u.ret == NULL)) {
         clnt_perror(mgrClient.clnt, "call failed");
         clnt_destroy(mgrClient.clnt);
         mgrClient.clnt = NULL;
      } else {
         mgrClient.accelId = result_2->VMAccelAllocateReturnStatus_u.ret->id;
      }
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

#ifdef __cplusplus
}
#endif

#endif /* !_VMACCEL_MGR_UTILS_H_ */
