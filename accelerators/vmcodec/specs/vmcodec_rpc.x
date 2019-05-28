/******************************************************************************

Copyright (c) 2019 VMware, Inc.
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

#ifdef RPC_HDR
%#include "vmaccel_rpc.h"
%#include "vmcodec_defs.h"
#endif

typedef unsigned int VMCODECCaps;

/*
 * Accelerator identifier.
 */
typedef VMAccelId VMCODECContextId;

/*
 * Accelerator context allocation descriptor structure.
 */
struct VMCODECContextAllocateDesc {
   VMAccelId                 accelId;
   VMCODECContextId          clientId;

   /*
    * Selection mask for context placement hints.
    */
   VMAccelSelectionMask      selectionMask;

   /*
    * Required capabilities for context allocation.
    */
   VMCODECCaps                  requiredCaps;
};

/*
 * Accelerator context allocation.
 */
struct VMCODECContextAllocateStatus {
   VMAccelStatusCode         status;
   VMCODECCaps               caps;
};

/*
 * Accelerator surface allocation descriptor structure.
 */
struct VMCODECSurfaceId {
   VMCODECContextId          cid;
   VMAccelSurfaceId          accel;
};

struct VMCODECSurfaceAllocateDesc {
   /*
    * Used to create a mapping for the client's requested surfaces to
    * avoid feedback of the ID in a return status.
    */
   VMAccelSurfaceId          client;
   VMAccelSurfaceDesc        desc;
};

/*
 * Accelerator image transfer operations
 */
struct VMCODECImageUploadOp {
   VMCODECSurfaceId                 img;
   VMAccelImageTransferOp           op;
   VMAccelSurfaceWriteConsistency   mode;
};

struct VMCODECImageDownloadOp {
   VMCODECSurfaceId              img;
   VMAccelImageTransferOp        op;
   VMAccelSurfaceReadConsistency mode;
};

/*
 * Decode operation.
 */
struct VMCODECDecodeOp {
   VMCODECContextId          cid;
   VMCODECSurfaceId          output<>;
};

/*
 * Encode operation.
 */
struct VMCODECEncodeOp {
   VMCODECContextId          cid;
   VMCODECSurfaceId          inptut<>;
};

/*
 * The result of a Context allocation operation.
 */
union VMCODECContextAllocateReturnStatus switch (int errno) {
   case 0:
      VMCODECContextAllocateStatus *ret;
   default:
      void;
};

/*
 * VM Accelerator program definition.
 */
program VMCODEC {
   version VMCODEC_VERSION {
      /*
       * Context allocation/destruction.
       */
      VMCODECContextAllocateReturnStatus
         VMCODEC_CONTEXTALLOC(VMCODECContextAllocateDesc) = 1;
      VMAccelReturnStatus
         VMCODEC_CONTEXTDESTROY(VMCODECContextId) = 2;

      /*
       * Surface allocation/destruction.
       */
      VMAccelSurfaceAllocateReturnStatus
         VMCODEC_SURFACEALLOC(VMCODECSurfaceAllocateDesc) = 3;
      VMAccelReturnStatus
         VMCODEC_SURFACEDESTROY(VMCODECSurfaceId) = 4;

      /*
       * Basic surface content operations.
       */
      VMAccelReturnStatus
         VMCODEC_IMAGEUPLOAD(VMCODECImageUploadOp) = 5;
      VMAccelDownloadReturnStatus
         VMCODEC_IMAGEDOWNLOAD(VMCODECImageDownloadOp) = 6;

      /*
       * Codec decode operation.
       */
      VMAccelReturnStatus
         VMCODEC_DECODE(VMCODECDecodeOp) = 7;

      /*
       * Codec encode operation.
       */
      VMAccelReturnStatus
         VMCODEC_ENCODE(VMCODECEncodeOp) = 8;
   } = 1;
} = 0x20000082;
