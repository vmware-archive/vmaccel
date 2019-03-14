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

#ifndef _VMXC_H_RPCGEN
#define _VMXC_H_RPCGEN

#include <rpc/rpc.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "vmaccel_rpc.h"
#include "vmcl_rpc.h"

enum VMXCStatusCodeEnum {
   VMXC_SUCCESS = 0,
   VMXC_FAIL = 1,
   VMXC_SEMANTIC_ERROR = 2,
   VMXC_LINK_ERROR = 3,
   VMXC_STATUS_CODE_MAX = 4,
};
typedef enum VMXCStatusCodeEnum VMXCStatusCodeEnum;

struct VMXCKernelArgDesc {
   u_int index;
   VMCLKernelSemanticType semantic;
   struct {
      u_int semanticDecl_len;
      char *semanticDecl_val;
   } semanticDecl;
   VMCLKernelArgType type;
   VMAccelSurfaceDesc surfDesc;
   VMCLSamplerDesc samplerDesc;
};
typedef struct VMXCKernelArgDesc VMXCKernelArgDesc;

struct VMXCKernelDesc {
   VMCLKernelLanguage language;
   struct {
      u_int source_len;
      char *source_val;
   } source;
   struct {
      u_int kernelName_len;
      char *kernelName_val;
   } kernelName;
   struct {
      u_int inputLayout_len;
      VMXCKernelArgDesc *inputLayout_val;
   } inputLayout;
   struct {
      u_int outputLayout_len;
      VMXCKernelArgDesc *outputLayout_val;
   } outputLayout;
};
typedef struct VMXCKernelDesc VMXCKernelDesc;

struct VMXCodeOp {
   VMCLKernelLanguage targetLanguage;
   VMXCKernelDesc kernel;
};
typedef struct VMXCodeOp VMXCodeOp;

struct VMXCStatus {
   VMXCStatusCodeEnum status;
   VMXCKernelDesc kernel;
};
typedef struct VMXCStatus VMXCStatus;

struct VMXCReturnStatus {
   int errno;
   union {
      VMXCStatus *ret;
   } VMXCReturnStatus_u;
};
typedef struct VMXCReturnStatus VMXCReturnStatus;

#define VMXC 0x20000082
#define VMXC_VERSION 1

#if defined(__STDC__) || defined(__cplusplus)
#define VMXC_XCODE 1
extern VMXCReturnStatus *vmxc_xcode_1(VMXCodeOp *, CLIENT *);
extern VMXCReturnStatus *vmxc_xcode_1_svc(VMXCodeOp *, struct svc_req *);
#define VMXC_VALIDATE 2
extern VMXCReturnStatus *vmxc_validate_1(VMXCKernelDesc *, CLIENT *);
extern VMXCReturnStatus *vmxc_validate_1_svc(VMXCKernelDesc *,
                                             struct svc_req *);
extern int vmxc_1_freeresult(SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define VMXC_XCODE 1
extern VMXCReturnStatus *vmxc_xcode_1();
extern VMXCReturnStatus *vmxc_xcode_1_svc();
#define VMXC_VALIDATE 2
extern VMXCReturnStatus *vmxc_validate_1();
extern VMXCReturnStatus *vmxc_validate_1_svc();
extern int vmxc_1_freeresult();
#endif /* K&R C */

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern bool_t xdr_VMXCStatusCodeEnum(XDR *, VMXCStatusCodeEnum *);
extern bool_t xdr_VMXCKernelArgDesc(XDR *, VMXCKernelArgDesc *);
extern bool_t xdr_VMXCKernelDesc(XDR *, VMXCKernelDesc *);
extern bool_t xdr_VMXCodeOp(XDR *, VMXCodeOp *);
extern bool_t xdr_VMXCStatus(XDR *, VMXCStatus *);
extern bool_t xdr_VMXCReturnStatus(XDR *, VMXCReturnStatus *);

#else /* K&R C */
extern bool_t xdr_VMXCStatusCodeEnum();
extern bool_t xdr_VMXCKernelArgDesc();
extern bool_t xdr_VMXCKernelDesc();
extern bool_t xdr_VMXCodeOp();
extern bool_t xdr_VMXCStatus();
extern bool_t xdr_VMXCReturnStatus();

#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_VMXC_H_RPCGEN */
