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

#ifndef _VMCL_CALLBACK_RPC_H_RPCGEN
#define _VMCL_CALLBACK_RPC_H_RPCGEN

#include <rpc/rpc.h>

#include "vmcl_rpc.h"

struct VMCLCallbackOp {
   VMCLContextId cid;
   VMAccelId queueId;
   VMAccelId eventId;
   VMAccelId fenceId;
   struct {
      u_int payload_len;
      char *payload_val;
   } payload;
};
typedef struct VMCLCallbackOp VMCLCallbackOp;

#define VMCL_CALLBACK 0x20000082
#define VMCL_CALLBACK_VERSION 1

#if defined(__STDC__) || defined(__cplusplus)
#define VMCL_CALLBACKOP 1
extern VMAccelReturnStatus *vmcl_callbackop_1(VMCLCallbackOp *, CLIENT *);
extern VMAccelReturnStatus *vmcl_callbackop_1_svc(VMCLCallbackOp *,
                                                  struct svc_req *);
extern int vmcl_callback_1_freeresult(SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define VMCL_CALLBACKOP 1
extern VMAccelReturnStatus *vmcl_callbackop_1();
extern VMAccelReturnStatus *vmcl_callbackop_1_svc();
extern int vmcl_callback_1_freeresult();
#endif /* K&R C */

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern bool_t xdr_VMCLCallbackOp(XDR *, VMCLCallbackOp *);

#else /* K&R C */
extern bool_t xdr_VMCLCallbackOp();

#endif /* K&R C */

#endif /* !_VMCL_CALLBACK_RPC_H_RPCGEN */
