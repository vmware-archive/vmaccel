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

/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "vmaccel_rpc.h"

bool_t xdr_VMAccelStatusCode(XDR *xdrs, VMAccelStatusCode *objp) {
   if (!xdr_u_int(xdrs, objp))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelResourceType(XDR *xdrs, VMAccelResourceType *objp) {
   if (!xdr_u_int(xdrs, objp))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelArchitectureType(XDR *xdrs, VMAccelArchitectureType *objp) {
   if (!xdr_u_int(xdrs, objp))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelSelectionMask(XDR *xdrs, VMAccelSelectionMask *objp) {
   if (!xdr_u_int(xdrs, objp))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelId(XDR *xdrs, VMAccelId *objp) {
   if (!xdr_u_int(xdrs, objp))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelCaps(XDR *xdrs, VMAccelCaps *objp) {
   if (!xdr_u_int(xdrs, objp))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelSurfacePool(XDR *xdrs, VMAccelSurfacePool *objp) {
   if (!xdr_u_int(xdrs, objp))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelSurfaceType(XDR *xdrs, VMAccelSurfaceType *objp) {
   if (!xdr_u_int(xdrs, objp))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelSurfaceUsage(XDR *xdrs, VMAccelSurfaceUsage *objp) {
   if (!xdr_u_int(xdrs, objp))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelSurfaceMapFlags(XDR *xdrs, VMAccelSurfaceMapFlags *objp) {
   if (!xdr_u_int(xdrs, objp))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelSurfaceReadConsistency(XDR *xdrs,
                                         VMAccelSurfaceReadConsistency *objp) {
   if (!xdr_u_int(xdrs, objp))
      return FALSE;
   return TRUE;
}

bool_t
xdr_VMAccelSurfaceWriteConsistency(XDR *xdrs,
                                   VMAccelSurfaceWriteConsistency *objp) {
   if (!xdr_u_int(xdrs, objp))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelFormatCaps(XDR *xdrs, VMAccelFormatCaps *objp) {
   if (!xdr_u_int(xdrs, objp))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelSurfaceBindFlags(XDR *xdrs, VMAccelSurfaceBindFlags *objp) {
   if (!xdr_u_int(xdrs, objp))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelSurfaceFormat(XDR *xdrs, VMAccelSurfaceFormat *objp) {
   if (!xdr_u_int(xdrs, objp))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelSurfaceFormatCaps(XDR *xdrs, VMAccelSurfaceFormatCaps *objp) {
   if (!xdr_u_int(xdrs, objp))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelStatus(XDR *xdrs, VMAccelStatus *objp) {
   if (!xdr_bytes(xdrs, (char **)&objp->nonce.nonce_val,
                  (u_int *)&objp->nonce.nonce_len, VMACCEL_MAX_NONCE_SIZE))
      return FALSE;
   if (!xdr_VMAccelStatusCode(xdrs, &objp->status))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelAddress(XDR *xdrs, VMAccelAddress *objp) {
   if (!xdr_bytes(xdrs, (char **)&objp->addr.addr_val,
                  (u_int *)&objp->addr.addr_len, VMACCEL_MAX_LOCATION_SIZE))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->port))
      return FALSE;
   if (!xdr_VMAccelResourceType(xdrs, &objp->resourceType))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->subDevice))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelCallback(XDR *xdrs, VMAccelCallback *objp) {
   if (!xdr_VMAccelAddress(xdrs, &objp->addr))
      return FALSE;
   if (!xdr_bytes(xdrs, (char **)&objp->payload.payload_val,
                  (u_int *)&objp->payload.payload_len, ~0))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelFormatDesc(XDR *xdrs, VMAccelFormatDesc *objp) {
   if (!xdr_u_int(xdrs, &objp->format))
      return FALSE;
   if (!xdr_VMAccelFormatCaps(xdrs, &objp->caps))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelWorkloadDesc(XDR *xdrs, VMAccelWorkloadDesc *objp) {
   register int32_t *buf;


   if (xdrs->x_op == XDR_ENCODE) {
      buf = XDR_INLINE(xdrs, 9 * BYTES_PER_XDR_UNIT);
      if (buf == NULL) {
         if (!xdr_u_int(xdrs, &objp->megaFlops))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->megaOps))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->llcSizeKB))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->llcBandwidthMBSec))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->localMemSizeKB))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->localMemBandwidthMBSec))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->nonLocalMemSizeKB))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->nonLocalMemBandwidthMBSec))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->interconnectBandwidthMBSec))
            return FALSE;
      } else {
         IXDR_PUT_U_LONG(buf, objp->megaFlops);
         IXDR_PUT_U_LONG(buf, objp->megaOps);
         IXDR_PUT_U_LONG(buf, objp->llcSizeKB);
         IXDR_PUT_U_LONG(buf, objp->llcBandwidthMBSec);
         IXDR_PUT_U_LONG(buf, objp->localMemSizeKB);
         IXDR_PUT_U_LONG(buf, objp->localMemBandwidthMBSec);
         IXDR_PUT_U_LONG(buf, objp->nonLocalMemSizeKB);
         IXDR_PUT_U_LONG(buf, objp->nonLocalMemBandwidthMBSec);
         IXDR_PUT_U_LONG(buf, objp->interconnectBandwidthMBSec);
      }
      return TRUE;
   } else if (xdrs->x_op == XDR_DECODE) {
      buf = XDR_INLINE(xdrs, 9 * BYTES_PER_XDR_UNIT);
      if (buf == NULL) {
         if (!xdr_u_int(xdrs, &objp->megaFlops))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->megaOps))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->llcSizeKB))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->llcBandwidthMBSec))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->localMemSizeKB))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->localMemBandwidthMBSec))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->nonLocalMemSizeKB))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->nonLocalMemBandwidthMBSec))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->interconnectBandwidthMBSec))
            return FALSE;
      } else {
         objp->megaFlops = IXDR_GET_U_LONG(buf);
         objp->megaOps = IXDR_GET_U_LONG(buf);
         objp->llcSizeKB = IXDR_GET_U_LONG(buf);
         objp->llcBandwidthMBSec = IXDR_GET_U_LONG(buf);
         objp->localMemSizeKB = IXDR_GET_U_LONG(buf);
         objp->localMemBandwidthMBSec = IXDR_GET_U_LONG(buf);
         objp->nonLocalMemSizeKB = IXDR_GET_U_LONG(buf);
         objp->nonLocalMemBandwidthMBSec = IXDR_GET_U_LONG(buf);
         objp->interconnectBandwidthMBSec = IXDR_GET_U_LONG(buf);
      }
      return TRUE;
   }

   if (!xdr_u_int(xdrs, &objp->megaFlops))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->megaOps))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->llcSizeKB))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->llcBandwidthMBSec))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->localMemSizeKB))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->localMemBandwidthMBSec))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->nonLocalMemSizeKB))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->nonLocalMemBandwidthMBSec))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->interconnectBandwidthMBSec))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelDesc(XDR *xdrs, VMAccelDesc *objp) {
   register int32_t *buf;


   if (xdrs->x_op == XDR_ENCODE) {
      if (!xdr_VMAccelId(xdrs, &objp->parentId))
         return FALSE;
      if (!xdr_VMAccelAddress(xdrs, &objp->parentAddr))
         return FALSE;
      if (!xdr_VMAccelResourceType(xdrs, &objp->typeMask))
         return FALSE;
      if (!xdr_VMAccelArchitectureType(xdrs, &objp->architecture))
         return FALSE;
      if (!xdr_VMAccelCaps(xdrs, &objp->caps))
         return FALSE;
      if (!xdr_array(xdrs, (char **)&objp->formatCaps.formatCaps_val,
                     (u_int *)&objp->formatCaps.formatCaps_len, ~0,
                     sizeof(VMAccelFormatDesc),
                     (xdrproc_t)xdr_VMAccelFormatDesc))
         return FALSE;
      if (!xdr_VMAccelWorkloadDesc(xdrs, &objp->capacity))
         return FALSE;
      buf = XDR_INLINE(xdrs, 6 * BYTES_PER_XDR_UNIT);
      if (buf == NULL) {
         if (!xdr_u_int(xdrs, &objp->maxContexts))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->maxQueues))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->maxEvents))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->maxFences))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->maxSurfaces))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->maxMappings))
            return FALSE;
      } else {
         IXDR_PUT_U_LONG(buf, objp->maxContexts);
         IXDR_PUT_U_LONG(buf, objp->maxQueues);
         IXDR_PUT_U_LONG(buf, objp->maxEvents);
         IXDR_PUT_U_LONG(buf, objp->maxFences);
         IXDR_PUT_U_LONG(buf, objp->maxSurfaces);
         IXDR_PUT_U_LONG(buf, objp->maxMappings);
      }
      return TRUE;
   } else if (xdrs->x_op == XDR_DECODE) {
      if (!xdr_VMAccelId(xdrs, &objp->parentId))
         return FALSE;
      if (!xdr_VMAccelAddress(xdrs, &objp->parentAddr))
         return FALSE;
      if (!xdr_VMAccelResourceType(xdrs, &objp->typeMask))
         return FALSE;
      if (!xdr_VMAccelArchitectureType(xdrs, &objp->architecture))
         return FALSE;
      if (!xdr_VMAccelCaps(xdrs, &objp->caps))
         return FALSE;
      if (!xdr_array(xdrs, (char **)&objp->formatCaps.formatCaps_val,
                     (u_int *)&objp->formatCaps.formatCaps_len, ~0,
                     sizeof(VMAccelFormatDesc),
                     (xdrproc_t)xdr_VMAccelFormatDesc))
         return FALSE;
      if (!xdr_VMAccelWorkloadDesc(xdrs, &objp->capacity))
         return FALSE;
      buf = XDR_INLINE(xdrs, 6 * BYTES_PER_XDR_UNIT);
      if (buf == NULL) {
         if (!xdr_u_int(xdrs, &objp->maxContexts))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->maxQueues))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->maxEvents))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->maxFences))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->maxSurfaces))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->maxMappings))
            return FALSE;
      } else {
         objp->maxContexts = IXDR_GET_U_LONG(buf);
         objp->maxQueues = IXDR_GET_U_LONG(buf);
         objp->maxEvents = IXDR_GET_U_LONG(buf);
         objp->maxFences = IXDR_GET_U_LONG(buf);
         objp->maxSurfaces = IXDR_GET_U_LONG(buf);
         objp->maxMappings = IXDR_GET_U_LONG(buf);
      }
      return TRUE;
   }

   if (!xdr_VMAccelId(xdrs, &objp->parentId))
      return FALSE;
   if (!xdr_VMAccelAddress(xdrs, &objp->parentAddr))
      return FALSE;
   if (!xdr_VMAccelResourceType(xdrs, &objp->typeMask))
      return FALSE;
   if (!xdr_VMAccelArchitectureType(xdrs, &objp->architecture))
      return FALSE;
   if (!xdr_VMAccelCaps(xdrs, &objp->caps))
      return FALSE;
   if (!xdr_array(xdrs, (char **)&objp->formatCaps.formatCaps_val,
                  (u_int *)&objp->formatCaps.formatCaps_len, ~0,
                  sizeof(VMAccelFormatDesc), (xdrproc_t)xdr_VMAccelFormatDesc))
      return FALSE;
   if (!xdr_VMAccelWorkloadDesc(xdrs, &objp->capacity))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->maxContexts))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->maxQueues))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->maxEvents))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->maxFences))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->maxSurfaces))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->maxMappings))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelResourceDesc(XDR *xdrs, VMAccelResourceDesc *objp) {
   if (!xdr_VMAccelId(xdrs, &objp->parentId))
      return FALSE;
   if (!xdr_VMAccelAddress(xdrs, &objp->parentAddr))
      return FALSE;
   if (!xdr_VMAccelResourceType(xdrs, &objp->typeMask))
      return FALSE;
   if (!xdr_VMAccelSelectionMask(xdrs, &objp->selectionMask))
      return FALSE;
   if (!xdr_VMAccelCaps(xdrs, &objp->caps))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->num))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->size))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelAllocateStatus(XDR *xdrs, VMAccelAllocateStatus *objp) {
   if (!xdr_VMAccelStatusCode(xdrs, &objp->status))
      return FALSE;
   if (!xdr_VMAccelId(xdrs, &objp->id))
      return FALSE;
   if (!xdr_VMAccelDesc(xdrs, &objp->desc))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelRegisterDesc(XDR *xdrs, VMAccelRegisterDesc *objp) {
   if (!xdr_VMAccelDesc(xdrs, &objp->desc))
      return FALSE;
   if (!xdr_VMAccelResourceType(xdrs, &objp->typeMask))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelResourceAllocateStatus(XDR *xdrs,
                                         VMAccelResourceAllocateStatus *objp) {
   if (!xdr_VMAccelStatusCode(xdrs, &objp->status))
      return FALSE;
   if (!xdr_VMAccelId(xdrs, &objp->id))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelRegisterAllocationDesc(XDR *xdrs,
                                         VMAccelRegisterAllocationDesc *objp) {
   if (!xdr_VMAccelAddress(xdrs, &objp->parentAddr))
      return FALSE;
   if (!xdr_VMAccelResourceType(xdrs, &objp->typeMask))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->num))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->size))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelRegisterStatus(XDR *xdrs, VMAccelRegisterStatus *objp) {
   if (!xdr_VMAccelStatusCode(xdrs, &objp->status))
      return FALSE;
   if (!xdr_VMAccelId(xdrs, &objp->id))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelElementDouble4D(XDR *xdrs, VMAccelElementDouble4D *objp) {
   if (!xdr_double(xdrs, &objp->x))
      return FALSE;
   if (!xdr_double(xdrs, &objp->y))
      return FALSE;
   if (!xdr_double(xdrs, &objp->z))
      return FALSE;
   if (!xdr_double(xdrs, &objp->w))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelElementFloat4D(XDR *xdrs, VMAccelElementFloat4D *objp) {
   if (!xdr_float(xdrs, &objp->x))
      return FALSE;
   if (!xdr_float(xdrs, &objp->y))
      return FALSE;
   if (!xdr_float(xdrs, &objp->z))
      return FALSE;
   if (!xdr_float(xdrs, &objp->w))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelElementUINT4D(XDR *xdrs, VMAccelElementUINT4D *objp) {
   register int32_t *buf;


   if (xdrs->x_op == XDR_ENCODE) {
      buf = XDR_INLINE(xdrs, 4 * BYTES_PER_XDR_UNIT);
      if (buf == NULL) {
         if (!xdr_u_int(xdrs, &objp->r))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->g))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->b))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->a))
            return FALSE;
      } else {
         IXDR_PUT_U_LONG(buf, objp->r);
         IXDR_PUT_U_LONG(buf, objp->g);
         IXDR_PUT_U_LONG(buf, objp->b);
         IXDR_PUT_U_LONG(buf, objp->a);
      }
      return TRUE;
   } else if (xdrs->x_op == XDR_DECODE) {
      buf = XDR_INLINE(xdrs, 4 * BYTES_PER_XDR_UNIT);
      if (buf == NULL) {
         if (!xdr_u_int(xdrs, &objp->r))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->g))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->b))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->a))
            return FALSE;
      } else {
         objp->r = IXDR_GET_U_LONG(buf);
         objp->g = IXDR_GET_U_LONG(buf);
         objp->b = IXDR_GET_U_LONG(buf);
         objp->a = IXDR_GET_U_LONG(buf);
      }
      return TRUE;
   }

   if (!xdr_u_int(xdrs, &objp->r))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->g))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->b))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->a))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelCoordinate2DUINT(XDR *xdrs, VMAccelCoordinate2DUINT *objp) {
   if (!xdr_u_int(xdrs, &objp->x))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->y))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelCoordinate3DUINT(XDR *xdrs, VMAccelCoordinate3DUINT *objp) {
   if (!xdr_u_int(xdrs, &objp->x))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->y))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->z))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelCoordinate4DUINT(XDR *xdrs, VMAccelCoordinate4DUINT *objp) {
   register int32_t *buf;


   if (xdrs->x_op == XDR_ENCODE) {
      buf = XDR_INLINE(xdrs, 4 * BYTES_PER_XDR_UNIT);
      if (buf == NULL) {
         if (!xdr_u_int(xdrs, &objp->x))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->y))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->z))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->w))
            return FALSE;
      } else {
         IXDR_PUT_U_LONG(buf, objp->x);
         IXDR_PUT_U_LONG(buf, objp->y);
         IXDR_PUT_U_LONG(buf, objp->z);
         IXDR_PUT_U_LONG(buf, objp->w);
      }
      return TRUE;
   } else if (xdrs->x_op == XDR_DECODE) {
      buf = XDR_INLINE(xdrs, 4 * BYTES_PER_XDR_UNIT);
      if (buf == NULL) {
         if (!xdr_u_int(xdrs, &objp->x))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->y))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->z))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->w))
            return FALSE;
      } else {
         objp->x = IXDR_GET_U_LONG(buf);
         objp->y = IXDR_GET_U_LONG(buf);
         objp->z = IXDR_GET_U_LONG(buf);
         objp->w = IXDR_GET_U_LONG(buf);
      }
      return TRUE;
   }

   if (!xdr_u_int(xdrs, &objp->x))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->y))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->z))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->w))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelSurfaceDesc(XDR *xdrs, VMAccelSurfaceDesc *objp) {
   register int32_t *buf;


   if (xdrs->x_op == XDR_ENCODE) {
      if (!xdr_VMAccelId(xdrs, &objp->parentId))
         return FALSE;
      if (!xdr_VMAccelSurfaceType(xdrs, &objp->type))
         return FALSE;
      buf = XDR_INLINE(xdrs, 6 * BYTES_PER_XDR_UNIT);
      if (buf == NULL) {
         if (!xdr_u_int(xdrs, &objp->width))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->height))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->depth))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->mipLevels))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->arraySize))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->samples))
            return FALSE;

      } else {
         IXDR_PUT_U_LONG(buf, objp->width);
         IXDR_PUT_U_LONG(buf, objp->height);
         IXDR_PUT_U_LONG(buf, objp->depth);
         IXDR_PUT_U_LONG(buf, objp->mipLevels);
         IXDR_PUT_U_LONG(buf, objp->arraySize);
         IXDR_PUT_U_LONG(buf, objp->samples);
      }
      if (!xdr_VMAccelSurfaceFormat(xdrs, &objp->format))
         return FALSE;
      if (!xdr_VMAccelSurfaceFormatCaps(xdrs, &objp->formatCaps))
         return FALSE;
      if (!xdr_VMAccelSurfacePool(xdrs, &objp->pool))
         return FALSE;
      if (!xdr_VMAccelSurfaceUsage(xdrs, &objp->usage))
         return FALSE;
      if (!xdr_VMAccelSurfaceBindFlags(xdrs, &objp->bindFlags))
         return FALSE;
      if (!xdr_u_int(xdrs, &objp->slicePitch))
         return FALSE;
      if (!xdr_u_int(xdrs, &objp->rowPitch))
         return FALSE;
      return TRUE;
   } else if (xdrs->x_op == XDR_DECODE) {
      if (!xdr_VMAccelId(xdrs, &objp->parentId))
         return FALSE;
      if (!xdr_VMAccelSurfaceType(xdrs, &objp->type))
         return FALSE;
      buf = XDR_INLINE(xdrs, 6 * BYTES_PER_XDR_UNIT);
      if (buf == NULL) {
         if (!xdr_u_int(xdrs, &objp->width))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->height))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->depth))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->mipLevels))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->arraySize))
            return FALSE;
         if (!xdr_u_int(xdrs, &objp->samples))
            return FALSE;

      } else {
         objp->width = IXDR_GET_U_LONG(buf);
         objp->height = IXDR_GET_U_LONG(buf);
         objp->depth = IXDR_GET_U_LONG(buf);
         objp->mipLevels = IXDR_GET_U_LONG(buf);
         objp->arraySize = IXDR_GET_U_LONG(buf);
         objp->samples = IXDR_GET_U_LONG(buf);
      }
      if (!xdr_VMAccelSurfaceFormat(xdrs, &objp->format))
         return FALSE;
      if (!xdr_VMAccelSurfaceFormatCaps(xdrs, &objp->formatCaps))
         return FALSE;
      if (!xdr_VMAccelSurfacePool(xdrs, &objp->pool))
         return FALSE;
      if (!xdr_VMAccelSurfaceUsage(xdrs, &objp->usage))
         return FALSE;
      if (!xdr_VMAccelSurfaceBindFlags(xdrs, &objp->bindFlags))
         return FALSE;
      if (!xdr_u_int(xdrs, &objp->slicePitch))
         return FALSE;
      if (!xdr_u_int(xdrs, &objp->rowPitch))
         return FALSE;
      return TRUE;
   }

   if (!xdr_VMAccelId(xdrs, &objp->parentId))
      return FALSE;
   if (!xdr_VMAccelSurfaceType(xdrs, &objp->type))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->width))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->height))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->depth))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->mipLevels))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->arraySize))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->samples))
      return FALSE;
   if (!xdr_VMAccelSurfaceFormat(xdrs, &objp->format))
      return FALSE;
   if (!xdr_VMAccelSurfaceFormatCaps(xdrs, &objp->formatCaps))
      return FALSE;
   if (!xdr_VMAccelSurfacePool(xdrs, &objp->pool))
      return FALSE;
   if (!xdr_VMAccelSurfaceUsage(xdrs, &objp->usage))
      return FALSE;
   if (!xdr_VMAccelSurfaceBindFlags(xdrs, &objp->bindFlags))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->slicePitch))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->rowPitch))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelHandleType(XDR *xdrs, VMAccelHandleType *objp) {
   if (!xdr_u_int(xdrs, objp))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelSurfaceId(XDR *xdrs, VMAccelSurfaceId *objp) {
   if (!xdr_VMAccelSurfaceType(xdrs, &objp->type))
      return FALSE;
   if (!xdr_VMAccelHandleType(xdrs, &objp->handleType))
      return FALSE;
   if (!xdr_VMAccelId(xdrs, &objp->id))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->instance))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->offset))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->generation))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelSurfaceAllocateStatus(XDR *xdrs,
                                        VMAccelSurfaceAllocateStatus *objp) {
   if (!xdr_VMAccelStatusCode(xdrs, &objp->status))
      return FALSE;
   if (!xdr_VMAccelSurfaceDesc(xdrs, &objp->surfaceDesc))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelQueueFlags(XDR *xdrs, VMAccelQueueFlags *objp) {
   if (!xdr_u_int(xdrs, objp))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelQueueDesc(XDR *xdrs, VMAccelQueueDesc *objp) {
   if (!xdr_VMAccelQueueFlags(xdrs, &objp->flags))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->size))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelQueueStatus(XDR *xdrs, VMAccelQueueStatus *objp) {
   if (!xdr_VMAccelStatusCode(xdrs, &objp->status))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelSurfaceRegion(XDR *xdrs, VMAccelSurfaceRegion *objp) {
   if (!xdr_u_int(xdrs, &objp->mipLevel))
      return FALSE;
   if (!xdr_VMAccelCoordinate3DUINT(xdrs, &objp->coord))
      return FALSE;
   if (!xdr_VMAccelCoordinate3DUINT(xdrs, &objp->size))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelSurfaceCopyOp(XDR *xdrs, VMAccelSurfaceCopyOp *objp) {
   if (!xdr_VMAccelSurfaceRegion(xdrs, &objp->dstRegion))
      return FALSE;
   if (!xdr_VMAccelSurfaceRegion(xdrs, &objp->srcRegion))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelImageFillOp(XDR *xdrs, VMAccelImageFillOp *objp) {
   if (!xdr_VMAccelSurfaceRegion(xdrs, &objp->dstRegion))
      return FALSE;
   if (!xdr_VMAccelElementDouble4D(xdrs, &objp->d))
      return FALSE;
   if (!xdr_VMAccelElementFloat4D(xdrs, &objp->f))
      return FALSE;
   if (!xdr_VMAccelElementUINT4D(xdrs, &objp->u))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelImageTransferOp(XDR *xdrs, VMAccelImageTransferOp *objp) {
   if (!xdr_VMAccelSurfaceRegion(xdrs, &objp->imgRegion))
      return FALSE;
   if (!xdr_bytes(xdrs, (char **)&objp->ptr.ptr_val,
                  (u_int *)&objp->ptr.ptr_len, ~0))
      return FALSE;
   if (!xdr_array(xdrs, (char **)&objp->callbacks.callbacks_val,
                  (u_int *)&objp->callbacks.callbacks_len, ~0,
                  sizeof(VMAccelCallback), (xdrproc_t)xdr_VMAccelCallback))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelDownloadStatus(XDR *xdrs, VMAccelDownloadStatus *objp) {
   if (!xdr_VMAccelStatusCode(xdrs, &objp->status))
      return FALSE;
   if (!xdr_VMAccelId(xdrs, &objp->fence))
      return FALSE;
   if (!xdr_bytes(xdrs, (char **)&objp->ptr.ptr_val,
                  (u_int *)&objp->ptr.ptr_len, ~0))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelSurfaceMapOp(XDR *xdrs, VMAccelSurfaceMapOp *objp) {
   if (!xdr_VMAccelSurfaceId(xdrs, &objp->surf))
      return FALSE;
   if (!xdr_VMAccelCoordinate3DUINT(xdrs, &objp->coord))
      return FALSE;
   if (!xdr_VMAccelCoordinate2DUINT(xdrs, &objp->size))
      return FALSE;
   if (!xdr_VMAccelSurfaceMapFlags(xdrs, &objp->mapFlags))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelSurfaceUnmapOp(XDR *xdrs, VMAccelSurfaceUnmapOp *objp) {
   if (!xdr_VMAccelSurfaceId(xdrs, &objp->surf))
      return FALSE;
   if (!xdr_VMAccelSurfaceMapFlags(xdrs, &objp->mapFlags))
      return FALSE;
   if (!xdr_bytes(xdrs, (char **)&objp->ptr.ptr_val,
                  (u_int *)&objp->ptr.ptr_len, ~0))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelSurfaceMapStatus(XDR *xdrs, VMAccelSurfaceMapStatus *objp) {
   if (!xdr_VMAccelStatusCode(xdrs, &objp->status))
      return FALSE;
   if (!xdr_bytes(xdrs, (char **)&objp->ptr.ptr_val,
                  (u_int *)&objp->ptr.ptr_len, ~0))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelComputeArgDesc(XDR *xdrs, VMAccelComputeArgDesc *objp) {
   if (!xdr_u_int(xdrs, &objp->index))
      return FALSE;
   if (!xdr_VMAccelSurfaceUsage(xdrs, &objp->usage))
      return FALSE;
   if (!xdr_VMAccelSurfaceId(xdrs, &objp->surf))
      return FALSE;
   if (!xdr_bytes(xdrs, (char **)&objp->data.data_val,
                  (u_int *)&objp->data.data_len, ~0))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelComputeOp(XDR *xdrs, VMAccelComputeOp *objp) {
   if (!xdr_u_int(xdrs, &objp->kernelType))
      return FALSE;
   if (!xdr_bytes(xdrs, (char **)&objp->kernelSource.kernelSource_val,
                  (u_int *)&objp->kernelSource.kernelSource_len, ~0))
      return FALSE;
   if (!xdr_bytes(xdrs, (char **)&objp->kernelBinary32.kernelBinary32_val,
                  (u_int *)&objp->kernelBinary32.kernelBinary32_len, ~0))
      return FALSE;
   if (!xdr_bytes(xdrs, (char **)&objp->kernelBinary64.kernelBinary64_val,
                  (u_int *)&objp->kernelBinary64.kernelBinary64_len, ~0))
      return FALSE;
   if (!xdr_bytes(xdrs, (char **)&objp->kernelName.kernelName_val,
                  (u_int *)&objp->kernelName.kernelName_len, ~0))
      return FALSE;
   if (!xdr_u_int(xdrs, &objp->dimension))
      return FALSE;
   if (!xdr_array(xdrs, (char **)&objp->globalWorkOffset.globalWorkOffset_val,
                  (u_int *)&objp->globalWorkOffset.globalWorkOffset_len, ~0,
                  sizeof(u_int), (xdrproc_t)xdr_u_int))
      return FALSE;
   if (!xdr_array(xdrs, (char **)&objp->globalWorkSize.globalWorkSize_val,
                  (u_int *)&objp->globalWorkSize.globalWorkSize_len, ~0,
                  sizeof(u_int), (xdrproc_t)xdr_u_int))
      return FALSE;
   if (!xdr_array(xdrs, (char **)&objp->localWorkSize.localWorkSize_val,
                  (u_int *)&objp->localWorkSize.localWorkSize_len, ~0,
                  sizeof(u_int), (xdrproc_t)xdr_u_int))
      return FALSE;
   if (!xdr_array(xdrs, (char **)&objp->args.args_val,
                  (u_int *)&objp->args.args_len, ~0,
                  sizeof(VMAccelComputeArgDesc),
                  (xdrproc_t)xdr_VMAccelComputeArgDesc))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelComputeStatus(XDR *xdrs, VMAccelComputeStatus *objp) {
   if (!xdr_VMAccelStatusCode(xdrs, &objp->status))
      return FALSE;
   if (!xdr_array(xdrs, (char **)&objp->outputs.outputs_val,
                  (u_int *)&objp->outputs.outputs_len, ~0,
                  sizeof(VMAccelComputeArgDesc),
                  (xdrproc_t)xdr_VMAccelComputeArgDesc))
      return FALSE;
   return TRUE;
}

bool_t xdr_VMAccelReturnStatus(XDR *xdrs, VMAccelReturnStatus *objp) {
   if (!xdr_int(xdrs, &objp->errno))
      return FALSE;
   switch (objp->errno) {
      case 0:
         if (!xdr_pointer(xdrs, (char **)&objp->VMAccelReturnStatus_u.ret,
                          sizeof(VMAccelStatus), (xdrproc_t)xdr_VMAccelStatus))
            return FALSE;
         break;
      default:
         break;
   }
   return TRUE;
}

bool_t xdr_VMAccelAllocateReturnStatus(XDR *xdrs,
                                       VMAccelAllocateReturnStatus *objp) {
   if (!xdr_int(xdrs, &objp->errno))
      return FALSE;
   switch (objp->errno) {
      case 0:
         if (!xdr_pointer(xdrs,
                          (char **)&objp->VMAccelAllocateReturnStatus_u.ret,
                          sizeof(VMAccelAllocateStatus),
                          (xdrproc_t)xdr_VMAccelAllocateStatus))
            return FALSE;
         break;
      default:
         break;
   }
   return TRUE;
}

bool_t xdr_VMAccelResourceAllocateReturnStatus(
   XDR *xdrs, VMAccelResourceAllocateReturnStatus *objp) {
   if (!xdr_int(xdrs, &objp->errno))
      return FALSE;
   switch (objp->errno) {
      case 0:
         if (!xdr_pointer(
                xdrs, (char **)&objp->VMAccelResourceAllocateReturnStatus_u.ret,
                sizeof(VMAccelResourceAllocateStatus),
                (xdrproc_t)xdr_VMAccelResourceAllocateStatus))
            return FALSE;
         break;
      default:
         break;
   }
   return TRUE;
}

bool_t xdr_VMAccelRegisterReturnStatus(XDR *xdrs,
                                       VMAccelRegisterReturnStatus *objp) {
   if (!xdr_int(xdrs, &objp->errno))
      return FALSE;
   switch (objp->errno) {
      case 0:
         if (!xdr_pointer(xdrs,
                          (char **)&objp->VMAccelRegisterReturnStatus_u.ret,
                          sizeof(VMAccelRegisterStatus),
                          (xdrproc_t)xdr_VMAccelRegisterStatus))
            return FALSE;
         break;
      default:
         break;
   }
   return TRUE;
}

bool_t xdr_VMAccelQueueReturnStatus(XDR *xdrs, VMAccelQueueReturnStatus *objp) {
   if (!xdr_int(xdrs, &objp->errno))
      return FALSE;
   switch (objp->errno) {
      case 0:
         if (!xdr_pointer(xdrs, (char **)&objp->VMAccelQueueReturnStatus_u.ret,
                          sizeof(VMAccelQueueStatus),
                          (xdrproc_t)xdr_VMAccelQueueStatus))
            return FALSE;
         break;
      default:
         break;
   }
   return TRUE;
}

bool_t xdr_VMAccelSurfaceAllocateReturnStatus(
   XDR *xdrs, VMAccelSurfaceAllocateReturnStatus *objp) {
   if (!xdr_int(xdrs, &objp->errno))
      return FALSE;
   switch (objp->errno) {
      case 0:
         if (!xdr_pointer(
                xdrs, (char **)&objp->VMAccelSurfaceAllocateReturnStatus_u.ret,
                sizeof(VMAccelSurfaceAllocateStatus),
                (xdrproc_t)xdr_VMAccelSurfaceAllocateStatus))
            return FALSE;
         break;
      default:
         break;
   }
   return TRUE;
}

bool_t xdr_VMAccelDownloadReturnStatus(XDR *xdrs,
                                       VMAccelDownloadReturnStatus *objp) {
   if (!xdr_int(xdrs, &objp->errno))
      return FALSE;
   switch (objp->errno) {
      case 0:
         if (!xdr_pointer(xdrs,
                          (char **)&objp->VMAccelDownloadReturnStatus_u.ret,
                          sizeof(VMAccelDownloadStatus),
                          (xdrproc_t)xdr_VMAccelDownloadStatus))
            return FALSE;
         break;
      default:
         break;
   }
   return TRUE;
}

bool_t xdr_VMAccelSurfaceMapReturnStatus(XDR *xdrs,
                                         VMAccelSurfaceMapReturnStatus *objp) {
   if (!xdr_int(xdrs, &objp->errno))
      return FALSE;
   switch (objp->errno) {
      case 0:
         if (!xdr_pointer(xdrs,
                          (char **)&objp->VMAccelSurfaceMapReturnStatus_u.ret,
                          sizeof(VMAccelSurfaceMapStatus),
                          (xdrproc_t)xdr_VMAccelSurfaceMapStatus))
            return FALSE;
         break;
      default:
         break;
   }
   return TRUE;
}

bool_t xdr_VMAccelComputeReturnStatus(XDR *xdrs,
                                      VMAccelComputeReturnStatus *objp) {
   if (!xdr_int(xdrs, &objp->errno))
      return FALSE;
   switch (objp->errno) {
      case 0:
         if (!xdr_pointer(xdrs,
                          (char **)&objp->VMAccelComputeReturnStatus_u.ret,
                          sizeof(VMAccelComputeStatus),
                          (xdrproc_t)xdr_VMAccelComputeStatus))
            return FALSE;
         break;
      default:
         break;
   }
   return TRUE;
}
