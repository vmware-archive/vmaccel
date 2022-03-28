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

/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "vmaccel_manager.h"
#include "vmaccel_mgr.h"
#include <memory.h>
#include <netinet/in.h>
#include <rpc/pmap_clnt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <syslog.h>

#ifndef SIG_PF
#define SIG_PF void (*)(int)
#endif

static void vmaccelmgr_1(struct svc_req *rqstp, register SVCXPRT *transp) {
   union {
      VMAccelRegisterDesc vmaccelmgr_register_1_arg;
      VMAccelId vmaccelmgr_unregister_1_arg;
      VMAccelDesc vmaccelmgr_alloc_1_arg;
      VMAccelId vmaccelmgr_free_1_arg;
   } argument;
   char *result;
   xdrproc_t _xdr_argument, _xdr_result;
   char *(*local)(char *, struct svc_req *);

   switch (rqstp->rq_proc) {
      case NULLPROC:
         (void)svc_sendreply(transp, (xdrproc_t)xdr_void, (char *)NULL);
         return;

      case VMACCELMGR_REGISTER:
         _xdr_argument = (xdrproc_t)xdr_VMAccelRegisterDesc;
         _xdr_result = (xdrproc_t)xdr_VMAccelAllocateReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmaccelmgr_register_1_svc;
         break;

      case VMACCELMGR_UNREGISTER:
         _xdr_argument = (xdrproc_t)xdr_VMAccelId;
         _xdr_result = (xdrproc_t)xdr_VMAccelReturnStatus;
         local =
            (char *(*)(char *, struct svc_req *))vmaccelmgr_unregister_1_svc;
         break;

      case VMACCELMGR_ALLOC:
         _xdr_argument = (xdrproc_t)xdr_VMAccelDesc;
         _xdr_result = (xdrproc_t)xdr_VMAccelAllocateReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmaccelmgr_alloc_1_svc;
         break;

      case VMACCELMGR_FREE:
         _xdr_argument = (xdrproc_t)xdr_VMAccelId;
         _xdr_result = (xdrproc_t)xdr_VMAccelReturnStatus;
         local = (char *(*)(char *, struct svc_req *))vmaccelmgr_free_1_svc;
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

   pmap_unset(VMACCELMGR, VMACCELMGR_VERSION);
   openlog("vmaccel_mgr", LOG_PID, LOG_DAEMON);

   transp = svcudp_create(RPC_ANYSOCK);
   if (transp == NULL) {
      syslog(LOG_ERR, "%s", "cannot create udp service.");
      exit(1);
   }
   if (!svc_register(transp, VMACCELMGR, VMACCELMGR_VERSION, vmaccelmgr_1,
                     IPPROTO_UDP)) {
      syslog(LOG_ERR, "%s",
             "unable to register (VMACCELMGR, VMACCELMGR_VERSION, udp).");
      exit(1);
   }

   transp = svctcp_create(RPC_ANYSOCK, 0, 0);
   if (transp == NULL) {
      syslog(LOG_ERR, "%s", "cannot create tcp service.");
      exit(1);
   }
   if (!svc_register(transp, VMACCELMGR, VMACCELMGR_VERSION, vmaccelmgr_1,
                     IPPROTO_TCP)) {
      syslog(LOG_ERR, "%s",
             "unable to register (VMACCELMGR, VMACCELMGR_VERSION, tcp).");
      exit(1);
   }

   vmaccel_manager_poweron();

   svc_run();
   syslog(LOG_ERR, "%s", "svc_run returned");

   vmaccel_manager_poweroff();

   exit(1);
   /* NOTREACHED */
}
