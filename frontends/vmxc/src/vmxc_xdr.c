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

#include "vmxc.h"

bool_t xdr_VMXCStatusCodeEnum(XDR *xdrs, VMXCStatusCodeEnum *objp) {
   register int32_t *buf;

   if (!xdr_enum(xdrs, (enum_t *)objp))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMXCKernelArgDesc(XDR *xdrs, VMXCKernelArgDesc *objp) {
   register int32_t *buf;

   if (!xdr_u_int(xdrs, &objp->index))
      return FALSE;
   if (!xdr_VMCLKernelSemanticType(xdrs, &objp->semantic))
      return FALSE;
   if (!xdr_bytes(xdrs, (char **)&objp->semanticDecl.semanticDecl_val,
                  (u_int *)&objp->semanticDecl.semanticDecl_len, ~0))
      return FALSE;
   if (!xdr_VMCLKernelArgType(xdrs, &objp->type))
      return FALSE;
   if (!xdr_VMAccelSurfaceDesc(xdrs, &objp->surfDesc))
      return FALSE;
   if (!xdr_VMCLSamplerDesc(xdrs, &objp->samplerDesc))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMXCKernelDesc(XDR *xdrs, VMXCKernelDesc *objp) {
   register int32_t *buf;

   if (!xdr_VMCLKernelLanguage(xdrs, &objp->language))
      return FALSE;
   if (!xdr_bytes(xdrs, (char **)&objp->source.source_val,
                  (u_int *)&objp->source.source_len, ~0))
      return FALSE;
   if (!xdr_bytes(xdrs, (char **)&objp->kernelName.kernelName_val,
                  (u_int *)&objp->kernelName.kernelName_len, ~0))
      return FALSE;
   if (!xdr_array(xdrs, (char **)&objp->inputLayout.inputLayout_val,
                  (u_int *)&objp->inputLayout.inputLayout_len, ~0,
                  sizeof(VMXCKernelArgDesc), (xdrproc_t)xdr_VMXCKernelArgDesc))
      return FALSE;
   if (!xdr_array(xdrs, (char **)&objp->outputLayout.outputLayout_val,
                  (u_int *)&objp->outputLayout.outputLayout_len, ~0,
                  sizeof(VMXCKernelArgDesc), (xdrproc_t)xdr_VMXCKernelArgDesc))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMXCodeOp(XDR *xdrs, VMXCodeOp *objp) {
   register int32_t *buf;

   if (!xdr_VMCLKernelLanguage(xdrs, &objp->targetLanguage))
      return FALSE;
   if (!xdr_VMXCKernelDesc(xdrs, &objp->kernel))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMXCStatus(XDR *xdrs, VMXCStatus *objp) {
   register int32_t *buf;

   if (!xdr_VMXCStatusCodeEnum(xdrs, &objp->status))
      return FALSE;
   if (!xdr_VMXCKernelDesc(xdrs, &objp->kernel))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMXCReturnStatus(XDR *xdrs, VMXCReturnStatus *objp) {
   register int32_t *buf;

   if (!xdr_int(xdrs, &objp->errno))
      return FALSE;
   switch (objp->errno) {
      case 0:
         if (!xdr_pointer(xdrs, (char **)&objp->VMXCReturnStatus_u.ret,
                          sizeof(VMXCStatus), (xdrproc_t)xdr_VMXCStatus))
            return FALSE;
         break;
      default:
         break;
   }
   return TRUE;
}
