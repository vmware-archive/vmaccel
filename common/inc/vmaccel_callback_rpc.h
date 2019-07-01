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

#ifndef _VMACCEL_CALLBACK_RPC_H_RPCGEN
#ifdef __cplusplus
extern "C" {
#endif
#define _VMACCEL_CALLBACK_RPC_H_RPCGEN

#include <rpc/rpc.h>

#include "vmaccel_rpc.h"

struct VMAccelCallbackOp {
   VMAccelId queueId;
   VMAccelId eventId;
   VMAccelAddress fence;
   struct {
      u_int payload_len;
      char *payload_val;
   } payload;
};
typedef struct VMAccelCallbackOp VMAccelCallbackOp;

#define VMACCEL_CALLBACK 0x20000080
#define VMACCEL_CALLBACK_VERSION 1

#if defined(__STDC__) || defined(__cplusplus)
#define VMACCEL_CALLBACKOP 1
extern VMAccelReturnStatus *vmaccel_callbackop_1(VMAccelCallbackOp *, CLIENT *);
extern VMAccelReturnStatus *vmaccel_callbackop_1_svc(VMAccelCallbackOp *,
                                                     struct svc_req *);
extern int vmaccel_callback_1_freeresult(SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define VMACCEL_CALLBACKOP 1
extern VMAccelReturnStatus *vmaccel_callbackop_1();
extern VMAccelReturnStatus *vmaccel_callbackop_1_svc();
extern int vmaccel_callback_1_freeresult();
#endif /* K&R C */

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern bool_t xdr_VMAccelCallbackOp(XDR *, VMAccelCallbackOp *);

#else /* K&R C */
extern bool_t xdr_VMAccelCallbackOp();

#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_VMACCEL_CALLBACK_RPC_H_RPCGEN */
